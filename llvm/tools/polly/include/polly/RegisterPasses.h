//===------ polly/RegisterPasses.h - Register the Polly passes *- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Functions to register the Polly passes in a LLVM pass manager.
//
//===----------------------------------------------------------------------===//

#ifndef POLLY_REGISTER_PASSES_H
#define POLLY_REGISTER_PASSES_H

#include "llvm/PassManager.h"

namespace llvm {

namespace legacy {
class PassManagerBase;
}
}

namespace polly {
void initializePollyPasses(llvm::PassRegistry &Registry);
}
#endif
