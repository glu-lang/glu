#ifndef GLU_BASIC_LLVMCOMPAT_HPP
#define GLU_BASIC_LLVMCOMPAT_HPP

#include <llvm/Config/llvm-config.h>

#if LLVM_VERSION_MAJOR >= 21
    #define GLU_GET_SINGLE_TRAILING_OBJECTS(Child) this->getTrailingObjects()
#else
    #define GLU_GET_SINGLE_TRAILING_OBJECTS(Child) \
        this->template getTrailingObjects<Child>()
#endif

#endif // GLU_BASIC_LLVMCOMPAT_HPP
