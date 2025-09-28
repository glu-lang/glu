add_library(Sema SHARED)

target_include_directories(Sema
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/Sema
)

target_link_libraries(Sema
        PUBLIC
        AST
        ASTPrinter
        LLVM
        gluBasic
        Parser
)

target_sources(Sema
        PRIVATE
        GlobalScopeVisitor.cpp
        Constraint.cpp
        ConstraintSystem.cpp
        ConversionVisitor.cpp
        CSWalker.cpp
        ImportManager.cpp
        ScopeTable.cpp
        Solver.cpp
        TypeVariableTyMapper.cpp
        SubstitutionMapper.cpp
        OccursCheckVisitor.cpp
        UnificationVisitor.cpp
        UnresolvedNameTyMapper.hpp
)
