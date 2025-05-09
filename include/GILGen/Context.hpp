#ifndef GLU_GILGEN_CONTEXT_HPP
#define GLU_GILGEN_CONTEXT_HPP

#include "Decls.hpp"
#include "Stmts.hpp"

#include "BasicBlock.hpp"
#include "Instructions.hpp"

namespace glu::gilgen {

/// @brief The context/builder for the GIL code generation.
class Context {
    gil::Function *_function;
    gil::BasicBlock *_currentBB;
    gil::InstBase *_insertBefore = nullptr; // Insert at end of block by default
    ast::FunctionDecl *_functionDecl;
    llvm::BumpPtrAllocator &_arena;

public:
    Context(ast::FunctionDecl *decl, llvm::BumpPtrAllocator &arena);

    /// Returns the AST function being compiled.
    ast::FunctionDecl *getASTFunction() const { return _functionDecl; }

    /// Returns the GIL function being generated.
    gil::Function *getCurrentFunction() const { return _function; }

    /// Returns the current GIL basic block.
    gil::BasicBlock *getCurrentBasicBlock() const { return _currentBB; }

    /// Returns the current insertion point.
    gil::InstBase *getInsertionPoint() const { return _insertBefore; }

    /// Sets the current insertion point.
    void setInsertionPoint(gil::BasicBlock *bb, gil::InstBase *inst)
    {
        _currentBB = bb;
        _insertBefore = inst;
    }

    /// Positions the insertion point at the end of the given basic block.
    void positionAtEnd(gil::BasicBlock *bb)
    {
        _currentBB = bb;
        _insertBefore = nullptr;
    }

private:
    template <typename T> T *insertInstruction(T *inst)
    {
        static_assert(
            std::is_base_of_v<gil::InstBase, T>, "Invalid instruction type"
        );
        static_assert(
            !std::is_base_of_v<gil::TerminatorInst, T>,
            "Use insertTerminator for terminators"
        );
        assert(_currentBB && "Invalid context: no current basic block");
        _currentBB->addInstructionBefore(inst, _insertBefore);
        return inst;
    }

    template <typename T> T *insertTerminator(T *term)
    {
        static_assert(
            std::is_base_of_v<gil::TerminatorInst, T>, "Invalid terminator type"
        );
        assert(_currentBB && "Invalid context: no current basic block");
        assert(
            _currentBB->getTerminator() == nullptr
            && "Basic block already has a terminator"
        );
        assert(
            _insertBefore == nullptr
            && "Terminator must be inserted at the end of the block"
        );
        _currentBB->addInstructionAtEnd(term);
        _currentBB = nullptr;
        return term;
    }

public:
    /// Generate an unreachable basic block — no basic block branches to it.
    gil::BasicBlock *buildUnreachableBB()
    {
        auto *bb = gil::BasicBlock::create(_arena, "unreachable", {});
        _function->addBasicBlockAtEnd(bb);
        return bb;
    }

    gil::BasicBlock *buildBB(std::string const &name)
    {
        auto *bb = new (_arena) gil::BasicBlock(name);
        _function->addBasicBlockAtEnd(bb);
        return bb;
    }

    gil::BrInst *buildBr(gil::BasicBlock *dest)
    {
        return insertTerminator(gil::BrInst::create(_arena, dest));
    }

    gil::BrInst *buildBr(gil::BasicBlock *dest, llvm::ArrayRef<gil::Value> args)
    {
        return insertTerminator(gil::BrInst::create(_arena, dest, args));
    }

    gil::UnreachableInst *buildUnreachable()
    {
        return insertTerminator(new (_arena) gil::UnreachableInst());
    }

    gil::ReturnInst *buildRetVoid()
    {
        // TODO: design proper way to return void
        return insertTerminator(new (_arena)
                                    gil::ReturnInst(gil::Value::getEmptyKey()));
    }

    gil::ReturnInst *buildRet(gil::Value retValue)
    {
        return insertTerminator(new (_arena) gil::ReturnInst(retValue));
    }

    gil::StoreInst *buildStore(gil::Value value, gil::Value ptr)
    {
        return insertInstruction(new (_arena) gil::StoreInst(value, ptr));
    }

    gil::CastIntToPtrInst *
    buildCastIntToPtr(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::CastIntToPtrInst(destType, value));
    }

    gil::CastPtrToIntInst *
    buildCastPtrToInt(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::CastPtrToIntInst(destType, value));
    }

    gil::BitcastInst *buildBitcast(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena) gil::BitcastInst(destType, value)
        );
    }

    gil::IntTruncInst *buildIntTrunc(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena) gil::IntTruncInst(destType, value)
        );
    }

    gil::IntZextInst *buildIntZext(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena) gil::IntZextInst(destType, value)
        );
    }

    gil::IntSextInst *buildIntSext(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena) gil::IntSextInst(destType, value)
        );
    }

    gil::FloatTruncInst *buildFloatTrunc(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::FloatTruncInst(destType, value));
    }

    gil::FloatExtInst *buildFloatExt(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena) gil::FloatExtInst(destType, value)
        );
    }

    /// Converts an AST type to a GIL type
    gil::Type translateType(types::TypeBase *type);

    gil::CondBrInst *buildCondBr(
        gil::Value cond, gil::BasicBlock *thenBB, gil::BasicBlock *elseBB
    )
    {
        return insertTerminator(
            gil::CondBrInst::create(_arena, cond, thenBB, elseBB)
        );
    }

    gil::CondBrInst *buildCondBr(
        gil::Value cond, gil::BasicBlock *thenBB, gil::BasicBlock *elseBB,
        llvm::ArrayRef<gil::Value> thenArgs, llvm::ArrayRef<gil::Value> elseArgs
    )
    {
        return insertTerminator(
            gil::CondBrInst::create(
                _arena, cond, thenBB, elseBB, thenArgs, elseArgs
            )
        );
    }

    gil::CallInst *
    buildCall(std::string const &opName, llvm::ArrayRef<gil::Value> args)
    {
        // Create a Function* with the operator name
        auto *func = new (_arena) gil::Function(opName, nullptr);
        return insertInstruction(new (_arena) gil::CallInst(func, args));
    }

    gil::CallInst *
    buildCall(gil::Value functionPtr, llvm::ArrayRef<gil::Value> args)
    {
        return insertInstruction(new (_arena) gil::CallInst(functionPtr, args));
    }

    gil::CallInst *
    buildCall(ast::FunctionDecl *func, llvm::ArrayRef<gil::Value> args)
    {
        // When sema is implemented, it will provide the resolved function
        // declaration For now, we create a function with the name from the
        // declaration
        auto *gilFunc = new (_arena)
            gil::Function(func->getName().str(), func->getType());
        return insertInstruction(new (_arena) gil::CallInst(gilFunc, args));
    }

    /// Creates an integer literal instruction
    gil::IntegerLiteralInst *
    buildIntegerLiteral(gil::Type type, llvm::APInt value)
    {
        return insertInstruction(new (_arena)
                                     gil::IntegerLiteralInst(type, value));
    }

    /// Creates a floating-point literal instruction
    gil::FloatLiteralInst *
    buildFloatLiteral(gil::Type type, llvm::APFloat value)
    {
        return insertInstruction(new (_arena) gil::FloatLiteralInst(type, value)
        );
    }

    /// Creates a boolean literal instruction (represented as an integer literal
    /// with value 0 or 1)
    gil::IntegerLiteralInst *buildBoolLiteral(gil::Type type, bool value)
    {
        llvm::APInt boolValue(1, value ? 1 : 0);
        return buildIntegerLiteral(type, boolValue);
    }

    /// Creates a string literal instruction
    gil::StringLiteralInst *
    buildStringLiteral(gil::Type type, llvm::StringRef value)
    {
        return insertInstruction(new (_arena)
                                     gil::StringLiteralInst(type, value.str()));
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_CONTEXT_HPP
