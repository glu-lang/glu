#include "ImportManager.hpp"
#include "ImportHandler.hpp"
#include "Sema.hpp"

#include "ClangImporter/ClangImporter.hpp"
#include "IRDec/ModuleLifter.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>

namespace glu::sema {

bool ImportManager::handleImport(
    ast::ImportDecl *importDecl, ScopeTable *intoScope
)
{
    bool success = true;
    assert(
        _context.getSourceManager()
        && "SourceManager must be available to handle imports"
    );
    auto path = importDecl->getImportPath();
    SourceLocation importLoc = importDecl->getLocation();
    ast::Visibility visibility = importDecl->getVisibility();
    bool isImplementImport
        = importDecl->hasAttribute(ast::AttributeKind::ImplementKind);

    for (auto selector : path.selectors) {
        ImportHandler handler(*this, importDecl, selector.name);
        if (auto result = handler.resolveImport()) {
            if (intoScope) {
                // For @implement imports, we need to validate and track
                if (isImplementImport) {
                    recordImplementImport(
                        importDecl, result->scope, selector.name, intoScope,
                        selector.getEffectiveName()
                    );
                } else {
                    importModuleIntoScope(
                        importLoc, result->scope, result->selector, intoScope,
                        selector.getEffectiveName(), visibility
                    );
                }
            }
        } else {
            success = false;
        }
    }

    return success;
}

bool ImportManager::handleDefaultImport(ScopeTable *intoScope)
{
    assert(
        _context.getSourceManager()
        && "SourceManager must be available to handle imports"
    );
    assert(intoScope && "intoScope must be provided for default imports");
    assert(
        !_importStack.empty()
        && "Import stack must not be empty to handle default imports"
    );
    FileID importingFileID = _importStack.back();
    llvm::StringRef defaultImportPath[] = { "defaultImports", "@all" };
    ImportHandler handler(*this, importingFileID, defaultImportPath);
    auto result = handler.resolveImport();
    if (!result) {
        return false;
    }
    importModuleIntoScope(
        SourceLocation::invalid, result->scope, result->selector, intoScope, "",
        ast::Visibility::Private
    );
    return true;
}

// MARK: - Import File Loading

std::optional<glu::sema::ScopeTable *>
ImportManager::tryLoadingFile(SourceLocation importLoc, FileID fid)
{
    if (_failedImports.contains(fid)) {
        // Previous import failed, do not try again. Do not generate new errors.
        return std::nullopt;
    }
    if (std::find(_importStack.begin(), _importStack.end(), fid)
        != _importStack.end()) {
        // Cyclic import detected.
        _diagManager.error(
            importLoc,
            "Cyclic import detected, module may be re-exporting itself"
        );
        return std::nullopt;
    }
    if (!_importedFiles[fid]) {
        // File has not been imported yet.
        if (!loadModule(importLoc, fid, detectModuleType(fid))) {
            _failedImports.insert(fid);
            return std::nullopt; // Import failed.
        }
    }
    // File has already been imported.
    return _importedFiles[fid];
}

ModuleType ImportManager::detectModuleType(FileID fid)
{
    auto *sm = _context.getSourceManager();
    llvm::StringRef filename = sm->getBufferName(fid);
    auto ext = llvm::sys::path::extension(filename);
    return llvm::StringSwitch<ModuleType>(ext)
        .Case(".glu", ModuleType::GluModule)
        .Case(".h", ModuleType::CHeader)
        .Case(".ll", ModuleType::IRModule)
        .Case(".bc", ModuleType::IRModule)
        .Default(ModuleType::Unknown);
}

bool ImportManager::loadModule(
    SourceLocation importLoc, FileID fid, ModuleType type
)
{
    switch (type) {
    case ModuleType::GluModule: return loadGluModule(fid);
    case ModuleType::CHeader: return loadCHeader(importLoc, fid);
    case ModuleType::IRModule: return loadIRModule(importLoc, fid);
    case ModuleType::Unknown:
    default:
        // This should only happen if the file extension is set
        // manually via the @file_extension attribute.
        _diagManager.error(
            importLoc, "Could not recognize module type from file extension"
        );
        return false;
    }
}

bool ImportManager::loadGluModule(FileID fid)
{
    auto *sm = _context.getSourceManager();
    auto contentLoaded = sm->ensureContentLoaded(fid);
    if (!contentLoaded) {
        return false;
    }
    glu::Scanner scanner(sm->getBuffer(fid), _context.getScannerAllocator());
    glu::Parser parser(scanner, _context, *sm, _diagManager);
    if (!parser.parse()) {
        return false;
    }
    auto *ast = llvm::cast<ast::ModuleDecl>(parser.getAST());
    if (!ast) {
        return false;
    }
    _importStack.push_back(fid);
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

bool ImportManager::loadCHeader(SourceLocation importLoc, FileID fid)
{
    auto *sm = _context.getSourceManager();
    llvm::StringRef headerPath = sm->getBufferName(fid);

    auto *ast = glu::clangimporter::importHeader(_context, headerPath, {});

    if (!ast) {
        _diagManager.error(
            importLoc, "Failed to import C header file: " + headerPath
        );
        return false;
    }

    _importStack.push_back(fid);
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

bool ImportManager::loadIRModule(SourceLocation importLoc, FileID fid)
{
    llvm::SMDiagnostic err;
    llvm::LLVMContext localContext;
    auto *sm = _context.getSourceManager();
    auto llvmModule
        = llvm::parseIRFile(sm->getBufferName(fid), err, localContext);

    if (!llvmModule) {
        _diagManager.error(
            importLoc, "Failed to parse LLVM module: " + err.getMessage()
        );
        return false;
    }

    auto *ast = glu::irdec::liftModule(_context, llvmModule.get());

    _importStack.push_back(fid);
    _importedFiles[fid] = sema::fastConstrainAST(ast, _diagManager, this);
    assert(_importStack.back() == fid);
    _importStack.pop_back();
    return _importedFiles[fid] != nullptr;
}

// MARK: - Module Copying

static llvm::StringRef isOperatorOverload(llvm::StringRef name)
{
    return llvm::StringSwitch<bool>(name)
#define OPERATOR(Name, Symbol, Type) .Case(Symbol, true)
#include "Basic/TokenKind.def"
               .Case("[", true)
               .Case("begin", true)
               .Case("end", true)
               .Case("next", true)
               .Default(false)
        ? name
        : "";
}

void ImportManager::importModuleIntoScope(
    SourceLocation importLoc, ScopeTable *module, llvm::StringRef selector,
    ScopeTable *intoScope, llvm::StringRef namespaceName,
    ast::Visibility visibility
)
{
    if (selector.empty()) {
        // Import module as a namespace.
        intoScope->insertNamespace(namespaceName, module, visibility);
        // Copy operator overloads into the scope directly.
        module->copyInto(
            intoScope, isOperatorOverload, _diagManager, importLoc, visibility
        );
        return;
    }
    // Import selected items from the module.
    std::function<llvm::StringRef(llvm::StringRef)> selectorFunc;
    if (selector == "@all") {
        selectorFunc = [](llvm::StringRef name) { return name; };
    } else {
        selectorFunc = [selector, namespaceName](llvm::StringRef name) {
            return name == selector ? namespaceName : "";
        };
    }
    if (!module->copyInto(
            intoScope, selectorFunc, _diagManager, importLoc, visibility
        )) {
        // No elements were imported.
        if (selector == "@all") {
            _diagManager.error(
                importLoc,
                "Could not find any public declarations in imported module"
            );
        } else {
            _diagManager.error(
                importLoc,
                "Could not find '" + selector + "' in imported module"
            );
        }
    }
}

void ImportManager::recordImplementImport(
    ast::ImportDecl *importDecl, ScopeTable *importedModule,
    llvm::StringRef selector, ScopeTable *intoScope,
    llvm::StringRef effectiveName
)
{
    SourceLocation importLoc = importDecl->getLocation();
    // @implement cannot use namespace imports (empty selector)
    if (selector.empty()) {
        _diagManager.error(
            importLoc,
            "@implement import cannot import a namespace; "
            "specify function names explicitly"
        );
        return;
    }
    // @implement cannot use wildcards
    if (selector == "@all") {
        _diagManager.error(
            importLoc,
            "@implement import cannot use wildcard imports; "
            "specify function names explicitly"
        );
        return;
    }
    // Find the function being imported
    auto *item = importedModule->lookupItem(selector);
    if (!item || item->decls.empty()) {
        _diagManager.error(
            importLoc,
            "Could not find function '" + selector + "' in imported module"
        );
        return;
    }
    // Find a function declaration
    ast::FunctionDecl *importedFunc = nullptr;
    for (auto &decl : item->decls) {
        if (auto *func = llvm::dyn_cast<ast::FunctionDecl>(decl.item)) {
            importedFunc = func;
            break;
        }
    }
    if (!importedFunc) {
        _diagManager.error(
            importLoc,
            "'" + selector
                + "' is not a function; @implement can only be "
                  "used with functions"
        );
        return;
    }
    // Track this @implement import
    _implementImports.push_back(
        ImplementImportInfo { importDecl, importedFunc, intoScope, selector,
                              effectiveName }
    );
}

bool ImportManager::processSkippedImports()
{
    while (!_skippedImports.empty()) {
        auto *decl = _skippedImports.back();
        _skippedImports.pop_back();
        if (!handleImport(decl, nullptr)) {
            return false;
        }
    }
    return true;
}

} // namespace glu::sema
