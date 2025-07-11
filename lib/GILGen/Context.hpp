// disable clang-format for this file because the CI uses an outdated version
// clang-format off
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
    /// Converts an AST type to a GIL type
    gil::Type translateType(types::TypeBase *type);

    gil::BasicBlock *buildBB(std::string const &name)
    {
        auto *bb = gil::BasicBlock::create(_arena, name, {});
        _function->addBasicBlockAtEnd(bb);
        return bb;
    }

    // - MARK: Terminator Instructions

    /// Generate an unreachable basic block — no basic block branches to it.
    gil::BasicBlock *buildUnreachableBB()
    {
        auto *bb = gil::BasicBlock::create(_arena, "unreachable", {});
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
        return insertTerminator(new (_arena)
                                    gil::ReturnInst(gil::Value::getEmptyKey()));
    }

    gil::ReturnInst *buildRet(gil::Value retValue)
    {
        return insertTerminator(new (_arena) gil::ReturnInst(retValue));
    }
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

    // - MARK: Memory Instructions

    gil::AllocaInst *buildAlloca(gil::Type type)
    {
        auto *ptrType = _functionDecl->getModule()
                            ->getContext()
                            ->getTypesMemoryArena()
                            .create<types::PointerTy>(&*type);
        return insertInstruction(
            new (_arena) gil::AllocaInst(type, translateType(ptrType))
        );
    }

    gil::StoreInst *buildStore(gil::Value value, gil::Value ptr)
    {
        return insertInstruction(new (_arena) gil::StoreInst(value, ptr));
    }

    gil::LoadInst *buildLoad(gil::Type type, gil::Value ptr)
    {
        return insertInstruction(new (_arena) gil::LoadInst(ptr, type));
    }

    // - MARK: Cast Instructions

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
        return insertInstruction(new (_arena)
                                     gil::BitcastInst(destType, value));
    }

    gil::IntTruncInst *buildIntTrunc(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::IntTruncInst(destType, value));
    }

    gil::IntZextInst *buildIntZext(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::IntZextInst(destType, value));
    }

    gil::IntSextInst *buildIntSext(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::IntSextInst(destType, value));
    }

    gil::FloatTruncInst *buildFloatTrunc(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::FloatTruncInst(destType, value));
    }

    gil::FloatExtInst *buildFloatExt(gil::Type destType, gil::Value value)
    {
        return insertInstruction(new (_arena)
                                     gil::FloatExtInst(destType, value));
    }

    // - MARK: Call Instructions

    gil::CallInst *
    buildCall(gil::Value functionPtr, llvm::ArrayRef<gil::Value> args)
    {
        types::PointerTy pointerType
            = llvm::cast<types::PointerTy>(functionPtr.getType().getType());
        types::FunctionTy *funcType
            = llvm::cast<types::FunctionTy>(pointerType.getPointee());
        return insertInstruction(
            gil::CallInst::create(
                _arena, translateType(funcType->getReturnType()), functionPtr,
                args
            )
        );
    }

    gil::CallInst *
    buildCall(ast::FunctionDecl *func, llvm::ArrayRef<gil::Value> args)
    {
        // FIXME: When sema is implemented, it will provide the resolved
        // function declaration For now, we create a function with the name from
        // the declaration
        auto *gilFunc = new (_arena)
            gil::Function(func->getName().str(), func->getType());
        return insertInstruction(
            gil::CallInst::create(
                _arena, translateType(func->getType()->getReturnType()),
                gilFunc, args
            )
        );
    }

    // - MARK: Constant Instructions

    /// Creates an integer literal instruction
    gil::IntegerLiteralInst *
    buildIntegerLiteral(gil::Type type, llvm::APInt value)
    {
        return insertInstruction(
            gil::IntegerLiteralInst::create(_arena, type, value)
        );
    }

    /// Creates a floating-point literal instruction
    gil::FloatLiteralInst *
    buildFloatLiteral(gil::Type type, llvm::APFloat value)
    {
        return insertInstruction(
            gil::FloatLiteralInst::create(_arena, type, value)
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

    gil::FunctionPtrInst *buildFunctionPtr(gil::Type type, gil::Function *func)
    {
        return insertInstruction(new (_arena) gil::FunctionPtrInst(func, type));
    }
};

} // namespace glu::gilgen

#endif // GLU_GILGEN_CONTEXT_HPP
