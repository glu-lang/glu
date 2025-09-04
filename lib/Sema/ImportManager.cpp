#include "ImportManager.hpp"

namespace glu::sema {

bool ImportManager::handleImport(
    llvm::ArrayRef<llvm::StringRef> components, llvm::StringRef selector,
    FileID ref, ScopeTable *intoScope
)
{
    // First: determine the file to import from the components, and maybe the
    // selector. The selector can be part of the components, or it can be a
    // selector within the module. The selector can also be "*", which means
    // import all.
    return false;
}

} // namespace glu::sema
