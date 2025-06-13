#ifndef IS_OPTIMIZABLE_CPP
#define IS_OPTIMIZABLE_CPP
#include <llvm/IR/Function.h>
#include <llvm/IR/Attributes.h>
#include "constants.h"

bool isOptimizable(Function* F) {
    if (!F->hasSection())
        return false;

    StringRef section = F->getSection();
    
    if (section == OPTIMIZABLE_SECTION_FLAG) {
        return true;
    }
    
    return false;
}
#endif