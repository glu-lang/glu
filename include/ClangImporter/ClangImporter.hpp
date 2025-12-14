#ifndef GLU_CLANGIMPORTER_CLANGIMPORTER_HPP
#define GLU_CLANGIMPORTER_CLANGIMPORTER_HPP

namespace glu::clangimporter {

/// \brief ClangImporter imports C/C++ header files and converts function
/// prototypes and declarations to Glu AST nodes.
///
/// This class uses the Clang API to parse header files and extract
/// declarations that can be used in Glu code.
class ClangImporter {
public:
    ClangImporter();
    ~ClangImporter();

    // TODO: Add methods for importing header files
};

} // namespace glu::clangimporter

#endif // GLU_CLANGIMPORTER_CLANGIMPORTER_HPP
