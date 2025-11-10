#ifndef GLU_GIL_INSTRUCTIONS_AGGREGATE_INST_HPP
#define GLU_GIL_INSTRUCTIONS_AGGREGATE_INST_HPP

#include "../InstBase.hpp"

namespace glu::gil {

class AggregateInst : public InstBase {

public:
    AggregateInst(InstKind kind) : InstBase(kind) { }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() >= InstKind::AggregateInstFirstKind
            && inst->getKind() <= InstKind::AggregateInstLastKind;
    }
};
}

#endif // GLU_GIL_INSTRUCTIONS_AGGREGATEINST_HPP
