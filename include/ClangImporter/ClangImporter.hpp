//===--- ClangImporter.hpp - Import C/C++ declarations ---------*- C++ -*-===//
//
// Part of the Glu Project, under the Apache License v2.0.
// See LICENSE for license information.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief This file defines the ClangImporter class, which imports C/C++
/// header files and converts them to Glu AST nodes.
///
//===----------------------------------------------------------------------===//

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

} // namespace glu

#endif // GLU_CLANGIMPORTER_CLANGIMPORTER_HPP
