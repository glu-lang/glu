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
        ConstraintSystem/ConcreteTypeVisitor.cpp
        ConstraintSystem/Constraint.cpp
        ConstraintSystem/ConstraintPrinter.cpp
        ConstraintSystem/ConstraintSystem.cpp
        ConstraintSystem/ConversionVisitor.cpp
        ConstraintSystem/LocalCSWalker.cpp
        ConstraintSystem/OccursCheckVisitor.cpp
        ConstraintSystem/Solver.cpp
        ConstraintSystem/SubstitutionMapper.cpp
        ConstraintSystem/TypeVariableCollector.cpp
        ConstraintSystem/TypeVariableTyMapper.cpp
        ConstraintSystem/UnificationVisitor.cpp
        ModuleWalker.cpp
        GlobalScopeVisitor.cpp
        ImportManager.cpp
        ScopeTable.cpp
        UnresolvedNameTyMapper.hpp
)
