# Instructions for Glu

Glu is a modern systems programming language compiler built with LLVM.

## File Structure
- `include/` — Public headers, organized by module (AST/, Basic/, GIL/, Sema/, etc.)
- `lib/` — Implementation files and private headers, mirrors the include structure
- `test/` — Unit tests, organized by module but all compiled together in one CMakeLists.txt
- `tools/` — Main function
- `build/` — Build artifacts and binaries

## Naming Conventions
- **Classes**: PascalCase with suffixes: `ConstraintSystem`, `IntTy`, `BinaryOpExpr`, `IfStmt`, `FunctionDecl`
- **Methods**: camelCase: `getType()`, `applyConstraint()`, `createBind()`
- **Variables**: camelCase: `typeVar`, `constraint`
- **Private Members**: underscore prefix: `_constraints`, `_typeBindings`

### Code Patterns

- Use GLU_DIRNAME_FILENAME_HPP for header guards.
- Use project includes first, then system and dependency includes.
- Use includes that include all decls and types in the AST, not individual types.
- Use Memory Arenas in the context for AST and types.
- Use LLVM BumpPtrAllocator for GIL and anything else that can't be on the stack.
- Use Doxygen comments: `/// @brief Description`
- Use assertions: `assert(condition && "Error message")`
- Prefer LLVM containers, casts and functions over STL
- Don't define classes that are only used in one implementation file in header files, keep them in the `.cpp` file.

### Testing

Using Google Test framework with fixtures. Using CMake for build configuration.

**Build & Run:**
```bash
# Configure, build, and run tests
./coverage.sh

# For quick incremental tests, you can:
cmake --build build -j8 && ./build/test/unit_tests
```

Run clang-format before committing:
```bash
clang-format -i filename
```

Create unit tests for new features or bug fixes, following the existing test structure.
Test all edge cases and ensure good coverage.
You can also create functional tests that run the compiler on example programs and check the output, in `test/functional/`.
