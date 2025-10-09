// disable clang-format for this file because the CI uses an outdated version
// clang-format off
#ifndef GLU_GILGEN_CONTEXT_HPP
#define GLU_GILGEN_CONTEXT_HPP

#include "Decls.hpp"
#include "Stmts.hpp"
#include "GILGen.hpp"

#include "BasicBlock.hpp"
#include "Instructions.hpp"
#include "Module.hpp"

namespace glu::gilgen {

/// @brief The context/builder for the GIL code generation within a function.
class Context {
    gil::Function *_function;
    gil::BasicBlock *_currentBB;
    gil::InstBase *_insertBefore = nullptr; // Insert at end of block by default
    gil::Module *_module;
    ast::FunctionDecl *_functionDecl;
    llvm::BumpPtrAllocator &_arena;
    SourceLocation _sourceLoc = SourceLocation::invalid;
    GlobalContext *_globalCtx = nullptr;

public:
    Context(gil::Module *module, ast::FunctionDecl *decl, GlobalContext &globalCtx);
    Context(gil::Module *module, ast::VarLetDecl *decl, GlobalContext &globalCtx);
    Context(gil::Module *module, gil::Function *function, llvm::BumpPtrAllocator &arena);

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

    SourceLocation getSourceLoc() const { return _sourceLoc; }
    void setSourceLoc(SourceLocation loc) { _sourceLoc = loc; }
    void setSourceLocNode(ast::ASTNode *node) { _sourceLoc = node ? node->getLocation() : SourceLocation::invalid; }

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
        inst->setLocation(
            _sourceLoc
        );
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
        term->setLocation(
            _sourceLoc
        );
        _currentBB->addInstructionAtEnd(term);
        _currentBB = nullptr;
        return term;
    }

    glu::gil::Function *getOrCreateGILFunction(glu::ast::FunctionDecl *fn)
    {
        // Try to find an existing function by FunctionDecl
        auto *existingFn = _module->getFunctionByDecl(fn);
        if (existingFn) {
            return existingFn;
        }

        // Otherwise, create a new GIL function
        auto *newFn = createNewGILFunction(fn->getName(), fn->getType(), fn);

        if (_globalCtx && fn->hasAttribute(ast::AttributeKind::InlineKind)) {
            _globalCtx->_inlinableFunctions.insert(fn);
        }

        return newFn;
    }

    glu::gil::Function *createNewGILFunction(llvm::StringRef name, glu::types::FunctionTy *type, ast::FunctionDecl *fn)
    {
        auto *gilFunc = new (_arena) gil::Function(name, type, fn);
        _module->addFunction(gilFunc);
        return gilFunc;
    }

public:
    /// Converts an AST type to a GIL type
    gil::Type translateType(types::TypeBase *type);

    glu::gil::Global *getOrCreateGlobal(glu::ast::VarLetDecl *decl)
    {
        return GILGen().getOrCreateGlobal(_module, decl, _arena);
    }

    gil::BasicBlock *buildBB(llvm::StringRef name, llvm::ArrayRef<gil::Type> argTypes = {})
    {
        auto *bb = gil::BasicBlock::create(_arena, name, argTypes);
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

    gil::StructExtractInst *buildStructExtract(
        gil::Value structValue, gil::Member member
    )
    {
        return insertInstruction(
            new (_arena) gil::StructExtractInst(structValue, member)
        );
    }

    gil::StructCreateInst *buildStructCreate(gil::Type structType, llvm::ArrayRef<gil::Value> members)
    {
        return insertInstruction(gil::StructCreateInst::create(_arena, structType, members));
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
        auto *gilFunc = this->getOrCreateGILFunction(func);
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
                                     gil::StringLiteralInst(type, value));
    }

    gil::EnumVariantInst *
    buildEnumVariant(gil::Type enumType, llvm::StringRef name)
    {
        return insertInstruction(
            new (_arena) gil::EnumVariantInst(gil::Member(name, enumType, enumType))
        );
    }

    gil::FunctionPtrInst *buildFunctionPtr(gil::Type type, gil::Function *func)
    {
        return insertInstruction(new (_arena) gil::FunctionPtrInst(func, type));
    }

    gil::GlobalPtrInst *buildGlobalPtr(gil::Type type, gil::Global *global)
    {
        return insertInstruction(new (_arena) gil::GlobalPtrInst(global, type));
    }

    // - MARK: Pointer Instructions

    gil::StructFieldPtrInst *buildStructFieldPtr(
        gil::Value structPtr, gil::Member member
    )
    {
        // Create a pointer type to the field type using the TypeTranslator
        auto *fieldPtrType = _functionDecl->getModule()->getContext()
                                 ->getTypesMemoryArena()
                                 .create<glu::types::PointerTy>(
                                     member.getType().getType()
                                 );
        gil::Type pointerType = translateType(fieldPtrType);
        return insertInstruction(new (_arena) gil::StructFieldPtrInst(
            structPtr, member, pointerType
        ));
    }

    gil::PtrOffsetInst *buildPtrOffset(
        gil::Value basePtr, gil::Value offset
    )
    {
        return insertInstruction(new (_arena) gil::PtrOffsetInst(
            basePtr, offset
        ));
    }

    // - MARK: Debug Instruction

    gil::DebugInst *buildDebug(
        llvm::StringRef varName, gil::Value ptr, gil::DebugBindingType bindingType
    )
    {
        return insertInstruction(new (_arena)
                                     gil::DebugInst(varName, ptr, bindingType));
    }

    // - MARK: OSSA Instructions

    gil::DropInst *buildDrop(gil::Value value)
    {
        if (value.getType()->isTrivial()) {
            // No need to drop trivial types
            return nullptr;
        }
        if (auto *structure = llvm::dyn_cast<types::StructTy>(value.getType().getType())) {
            if (structure->getDecl()->hasOverloadedDropFunction()) {
                // Make sure the drop function is created
                getOrCreateGILFunction(structure->getDecl()->getDropFunction());
            }
        }
        return insertInstruction(new (_arena) gil::DropInst(value));
    }
};


} // namespace glu::gilgen

#endif // GLU_GILGEN_CONTEXT_HPP
