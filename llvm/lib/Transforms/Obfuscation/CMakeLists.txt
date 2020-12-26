add_llvm_component_library(LLVMObfuscation
  src/Passes/FunctionCallObfuscate.cpp
  src/CryptoUtils.cpp
  src/Passes/BogusControlFlow.cpp
  src/Passes/Substitution.cpp
  src/Passes/Flattening.cpp
  src/Utils.cpp
  src/Passes/SplitBasicBlocks.cpp
  src/Passes/AntiClassDump.cpp
  src/Passes/StringEncryption.cpp
  src/Passes/IndirectBranch.cpp
  src/Passes/FunctionWrapper.cpp
  src/PassScheduler.cpp
  src/LegacyLowerSwitch.cpp

  ADDITIONAL_HEADER_DIRS
  ${LLVM_MAIN_INCLUDE_DIR}/llvm/Transforms

  DEPENDS
  intrinsics_gen
  )
