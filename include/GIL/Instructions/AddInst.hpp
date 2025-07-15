#ifndef GLU_GIL_INSTRUCTIONS_ADD_INST_HPP
#define GLU_GIL_INSTRUCTIONS_ADD_INST_HPP

#include "ArithmeticInst.hpp"

namespace glu::gil {

/// @brief Integer addition instruction
class AddInst : public ArithmeticInst {
public:
    AddInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::AddInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::AddInstKind;
    }
};

/// @brief Integer subtraction instruction
class SubInst : public ArithmeticInst {
public:
    SubInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::SubInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::SubInstKind;
    }
};

/// @brief Integer multiplication instruction
class MulInst : public ArithmeticInst {
public:
    MulInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::MulInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::MulInstKind;
    }
};

/// @brief Integer division instruction
class DivInst : public ArithmeticInst {
public:
    DivInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::DivInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::DivInstKind;
    }
};

/// @brief Floating-point addition instruction
class FAddInst : public ArithmeticInst {
public:
    FAddInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::FAddInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FAddInstKind;
    }
};

/// @brief Floating-point subtraction instruction
class FSubInst : public ArithmeticInst {
public:
    FSubInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::FSubInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FSubInstKind;
    }
};

/// @brief Floating-point multiplication instruction
class FMulInst : public ArithmeticInst {
public:
    FMulInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::FMulInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FMulInstKind;
    }
};

/// @brief Floating-point division instruction
class FDivInst : public ArithmeticInst {
public:
    FDivInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::FDivInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FDivInstKind;
    }
};

/// @brief Floating-point remainder instruction
class FRemInst : public ArithmeticInst {
public:
    FRemInst(Value lhs, Value rhs, Type resultType)
        : ArithmeticInst(InstKind::FRemInstKind, lhs, rhs, resultType)
    {
    }

    static bool classof(InstBase const *inst)
    {
        return inst->getKind() == InstKind::FRemInstKind;
    }
};

} // namespace glu::gil

#endif // GLU_GIL_INSTRUCTIONS_ADD_INST_HPP
