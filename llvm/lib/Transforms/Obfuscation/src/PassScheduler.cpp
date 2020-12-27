// For licensing terms, please read LICENSE.md in this repository.
//===----------------------------------------------------------------------===//
/*
  Hikari 's own "Pass Scheduler".
  Because currently there is no way to add dependency to transform passes
  Ref : http://lists.llvm.org/pipermail/llvm-dev/2011-February/038109.html
*/
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Transforms/Obfuscation/Obfuscation.h"
#include <iostream>
using namespace llvm;
using namespace std;
// Begin Obfuscator Options
static cl::opt<uint64_t> AesSeed("aesSeed", cl::init(0x1337),
                                 cl::desc("seed for the PRNG"));
static cl::opt<bool> EnableAntiClassDump("enable-acdobf", cl::init(false),
                                         cl::NotHidden,
                                         cl::desc("Enable AntiClassDump."));
static cl::opt<bool>
    EnableBogusControlFlow("enable-bcfobf", cl::init(false), cl::NotHidden,
                           cl::desc("Enable BogusControlFlow."));
static cl::opt<bool> EnableFlattening("enable-cffobf", cl::init(false),
                                      cl::NotHidden,
                                      cl::desc("Enable Flattening."));
static cl::opt<bool>
    EnableBasicBlockSplit("enable-splitobf", cl::init(false), cl::NotHidden,
                          cl::desc("Enable BasicBlockSpliting."));
static cl::opt<bool>
    EnableSubstitution("enable-subobf", cl::init(false), cl::NotHidden,
                       cl::desc("Enable Instruction Substitution."));
static cl::opt<bool> EnableAllObfuscation("enable-allobf", cl::init(false),
                                          cl::NotHidden,
                                          cl::desc("Enable All Obfuscation."));
static cl::opt<bool> EnableFunctionCallObfuscate(
    "enable-fco", cl::init(false), cl::NotHidden,
    cl::desc("Enable Function CallSite Obfuscation."));
static cl::opt<bool>
    EnableStringEncryption("enable-strcry", cl::init(false), cl::NotHidden,
                           cl::desc("Enable Function CallSite Obfuscation."));
static cl::opt<bool>
    EnableIndirectBranching("enable-indibran", cl::init(false), cl::NotHidden,
                            cl::desc("Enable Indirect Branching."));
static cl::opt<bool>
    EnableFunctionWrapper("enable-funcwra", cl::init(false), cl::NotHidden,
                          cl::desc("Enable Function Wrapper."));
// Being able to disable options

// Why is this one there? For the LLVM C backend.
static cl::opt<bool>
    DisableIndirectBranching("disable-indibran", cl::init(false), cl::NotHidden,
                             cl::desc("Disable Indirect Branching."));
static cl::opt<bool> DisableAntiClassDump("disable-acdobf", cl::init(false),
                                          cl::NotHidden,
                                          cl::desc("Disable AntiClassDump."));

// End Obfuscator Options

static void LoadEnv() {
  if (getenv("SPLITOBF")) {
    EnableBasicBlockSplit = true;
  }
  if (getenv("SUBOBF")) {
    EnableSubstitution = true;
  }
  if (getenv("ALLOBF")) {
    EnableAllObfuscation = true;
  }
  if (getenv("FCO")) {
    EnableFunctionCallObfuscate = true;
  }
  if (getenv("STRCRY")) {
    EnableStringEncryption = true;
  }
  if (getenv("INDIBRAN")) {
    EnableIndirectBranching = true;
  }
  if (getenv("NOINDIBRAN")) {
    DisableIndirectBranching = true;
  }
  if (getenv("FUNCWRA")) {
    EnableFunctionWrapper = true; // Broken
  }
  if (getenv("BCFOBF")) {
    EnableBogusControlFlow = true;
  }
  if (getenv("ACDOBF")) {
    EnableAntiClassDump = true;
  }
  if (getenv("NOACDOBF")) {
    DisableAntiClassDump = true;
  }
  if (getenv("CFFOBF")) {
    EnableFlattening = true;
  }
}
namespace llvm {
struct Obfuscation : public ModulePass {
  static char ID;
  Obfuscation() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return StringRef("HikariObfuscationScheduler");
  }
  bool runOnModule(Module &M) override {
    // Initial ACD Pass
    if ((EnableAllObfuscation || EnableAntiClassDump) &&
        !DisableAntiClassDump) {
      ModulePass *P = createAntiClassDumpPass();
      P->doInitialization(M);
      P->runOnModule(M);
      delete P;
    }
    // Now do FCO
    FunctionPass *FP = createFunctionCallObfuscatePass(
        EnableFunctionCallObfuscate);
    FP->doInitialization(M);
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
      Function &F = *iter;
      if (!F.isDeclaration()) {
        FP->runOnFunction(F);
      }
    }
    delete FP;
    // Now Encrypt Strings
    ModulePass *MP = createStringEncryptionPass(EnableAllObfuscation ||
                                                EnableStringEncryption);
    MP->runOnModule(M);
    delete MP;
    /*
    // Placing FW here does provide the most obfuscation however the compile
    time
    // and product size would be literally unbearable for any large project
    // Move it to post run
    if (EnableAllObfuscation || EnableFunctionWrapper) {
      ModulePass *P = createFunctionWrapperPass();
      P->runOnModule(M);
      delete P;
    }*/
    // Now perform Function-Level Obfuscation
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
      Function &F = *iter;
      if (!F.isDeclaration()) {
        FunctionPass *P = NULL;
        P = createSplitBasicBlockPass(EnableAllObfuscation ||
                                      EnableBasicBlockSplit);
        P->runOnFunction(F);
        delete P;
        P = createBogusControlFlowPass(EnableAllObfuscation ||
                                       EnableBogusControlFlow);
        P->runOnFunction(F);
        delete P;
        P = createFlatteningPass(EnableAllObfuscation || EnableFlattening);
        P->runOnFunction(F);
        delete P;
        P = createSubstitutionPass(EnableAllObfuscation || EnableSubstitution);
        P->runOnFunction(F);
        delete P;
      }
    }
    //errs() << "Doing Post-Run Cleanup\n";
    FunctionPass *P = createIndirectBranchPass(
        (EnableAllObfuscation || EnableIndirectBranching) &&
        !DisableIndirectBranching);
    vector<Function *> funcs;
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
      funcs.push_back(&*iter);
    }
    for (Function *F : funcs) {
      P->runOnFunction(*F);
    }
    delete P;
    MP = createFunctionWrapperPass(EnableAllObfuscation ||
                                   EnableFunctionWrapper);
    MP->runOnModule(M);
    delete MP;
    // Cleanup Flags
    vector<Function *> toDelete;
    for (Module::iterator iter = M.begin(); iter != M.end(); iter++) {
      Function &F = *iter;
      if (F.isDeclaration() && F.hasName() && F.getName().contains("hikari_")) {
        for (User *U : F.users()) {
          if (Instruction *Inst = dyn_cast<Instruction>(U)) {
            Inst->eraseFromParent();
          }
        }
        toDelete.push_back(&F);
      }
    }
    for (Function *F : toDelete) {
      F->eraseFromParent();
    }
    //errs() << "Hikari Out\n";
    return true;
  } // End runOnModule
};

ModulePass *createObfuscationPass() {
  LoadEnv();
  if (AesSeed != 0x1337) {
    cryptoutils->prng_seed(AesSeed);
  } else {
    cryptoutils->prng_seed();
  }
  //cout << "Initializing Hikari Core with Revision ID:" <<""// GIT_COMMIT_HASH
  //     << endl;
  return new Obfuscation();
}
} // namespace llvm

char Obfuscation::ID = 0;
INITIALIZE_PASS_BEGIN(Obfuscation, "obfus", "Enable Obfuscation", true, true)
INITIALIZE_PASS_DEPENDENCY(AntiClassDump);
INITIALIZE_PASS_DEPENDENCY(BogusControlFlow);
INITIALIZE_PASS_DEPENDENCY(Flattening);
INITIALIZE_PASS_DEPENDENCY(FunctionCallObfuscate);
INITIALIZE_PASS_DEPENDENCY(IndirectBranch);
INITIALIZE_PASS_DEPENDENCY(SplitBasicBlock);
INITIALIZE_PASS_DEPENDENCY(StringEncryption);
INITIALIZE_PASS_DEPENDENCY(Substitution);
INITIALIZE_PASS_END(Obfuscation, "obfus", "Enable Obfuscation", true, true)
