#ifndef GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP
#define GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP

#include "Basic/Diagnostic.hpp"
#include "GIL/BasicBlock.hpp"
#include "GIL/Function.hpp"
#include "GIL/InstVisitor.hpp"
#include "GIL/Module.hpp"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/SmallVector.h>

namespace glu::gilgen {

class GILDetectUninitializedPass
    : public gil::InstVisitor<GILDetectUninitializedPass> {
private:
    DiagnosticManager &diagManager;

public:
    GILDetectUninitializedPass(DiagnosticManager &diagManager)
        : diagManager(diagManager)
    {
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_GILPASSES_DETECTUNINITIALIZED_HPP
