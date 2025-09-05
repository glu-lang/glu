add_library (AST SHARED)

target_include_directories(AST
    PUBLIC
    ${CMAKE_SOURCE_DIR}/include/AST
)

target_link_libraries(AST
    PUBLIC
        gluBasic
        LLVM
)

target_sources(AST
    PRIVATE
    AST.cpp
    HashType.cpp
    ASTNode.cpp
    ASTChildReplacerVisitor.cpp
    Types/TypeBase.cpp
    Types/StructTy.cpp
    Types/EnumTy.cpp
)
