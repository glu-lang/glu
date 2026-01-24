#include "ImportManager.hpp"
#include "ImportHandler.hpp"
#include "Sema.hpp"

#include "ClangImporter/ClangImporter.hpp"
#include "IRDec/ModuleLifter.hpp"
#include "Lexer/Scanner.hpp"
#include "Parser/Parser.hpp"

#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <llvm/TargetParser/Host.h>

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
    for (auto selector : path.selectors) {
        ImportHandler handler(*this, importDecl, selector.name);
        if (auto result = handler.resolveImport()) {
            if (intoScope) {
                importModuleIntoScope(
                    importLoc, result->scope, result->selector, intoScope,
                    selector.getEffectiveName(), visibility
                );
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
        .Case(".c", ModuleType::CSource)
        .Case(".cpp", ModuleType::CxxSource)
        .Case(".cc", ModuleType::CxxSource)
        .Case(".cxx", ModuleType::CxxSource)
        .Case(".C", ModuleType::CxxSource)
        .Case(".rs", ModuleType::RustSource)
        .Case(".zig", ModuleType::ZigSource)
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
    case ModuleType::CSource: return loadCSource(importLoc, fid);
    case ModuleType::CxxSource: return loadCxxSource(importLoc, fid);
    case ModuleType::RustSource: return loadRustSource(importLoc, fid);
    case ModuleType::ZigSource: return loadZigSource(importLoc, fid);
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
    auto *sm = _context.getSourceManager();
    return loadIRModuleFromPath(importLoc, fid, sm->getBufferName(fid));
}

bool ImportManager::compileToIR(
    SourceLocation importLoc, FileID fid, llvm::StringRef compilerName,
    llvm::StringRef fileExtension, llvm::ArrayRef<llvm::StringRef> compilerArgs,
    llvm::StringRef outputFlag
)
{
    auto cachedPath = _generatedBitcodePaths.find(fid);
    if (cachedPath != _generatedBitcodePaths.end()) {
        return loadIRModuleFromPath(importLoc, fid, cachedPath->second);
    }

    auto *sm = _context.getSourceManager();
    llvm::StringRef sourcePath = sm->getBufferName(fid);

    llvm::SmallString<128> tempPath;
    std::error_code ec = llvm::sys::fs::createTemporaryFile(
        "glu-import", fileExtension, tempPath
    );
    if (ec) {
        _diagManager.error(
            importLoc, "Failed to create temporary file: " + ec.message()
        );
        return false;
    }

    auto compilerPath = llvm::sys::findProgramByName(compilerName);
    if (!compilerPath) {
        _diagManager.error(
            importLoc,
            "Could not find " + compilerName.str() + " to compile '"
                + sourcePath.str() + "': " + compilerPath.getError().message()
        );
        return false;
    }

    std::string outputArg;
    llvm::SmallVector<llvm::StringRef, 12> args;
    args.push_back(compilerName);
    args.append(compilerArgs.begin(), compilerArgs.end());
    args.push_back(sourcePath);
    if (outputFlag.ends_with("=")) {
        // Combined flag like -femit-llvm-ir=<path>
        outputArg = outputFlag.str() + tempPath.str().str();
        args.push_back(outputArg);
    } else {
        // Separate flag like -o <path>
        args.push_back(outputFlag);
        args.push_back(tempPath);
    }

    std::string errorMsg;
    int result = llvm::sys::ExecuteAndWait(
        *compilerPath, args, std::nullopt, {}, 0, 0, &errorMsg
    );
    if (result != 0) {
        std::string message
            = "Failed to compile source file: " + sourcePath.str();
        if (!errorMsg.empty()) {
            message += ": " + errorMsg;
        }
        _diagManager.error(importLoc, message);
        return false;
    }

    _generatedBitcodePaths[fid] = tempPath.str().str();
    return loadIRModuleFromPath(importLoc, fid, tempPath);
}

bool ImportManager::loadCSource(SourceLocation importLoc, FileID fid)
{
    llvm::StringRef targetTriple = _targetTriple;
    std::string targetTripleStorage;
    if (targetTriple.empty()) {
        targetTripleStorage = llvm::sys::getDefaultTargetTriple();
        targetTriple = targetTripleStorage;
    }

    llvm::SmallVector<llvm::StringRef, 8> args;
    args.push_back("-g");
    args.push_back("-c");
    args.push_back("-emit-llvm");
    if (!targetTriple.empty()) {
        args.push_back("-target");
        args.push_back(targetTriple);
    }

    return compileToIR(importLoc, fid, "clang", "bc", args);
}

bool ImportManager::loadCxxSource(SourceLocation importLoc, FileID fid)
{
    return loadCSource(importLoc, fid);
}

bool ImportManager::loadRustSource(SourceLocation importLoc, FileID fid)
{
    llvm::SmallVector<llvm::StringRef, 8> args;
    args.push_back("-g");
    args.push_back("--crate-type=lib");
    args.push_back("--emit=llvm-ir");
    if (!_targetTriple.empty()) {
        args.push_back("--target");
        args.push_back(_targetTriple);
    }

    return compileToIR(importLoc, fid, "rustc", "ll", args);
}

bool ImportManager::loadZigSource(SourceLocation importLoc, FileID fid)
{
    std::string targetArg;
    llvm::SmallVector<llvm::StringRef, 8> args;
    args.push_back("build-obj");
    args.push_back("-fllvm");
    args.push_back("-fno-strip");
    if (!_targetTriple.empty()) {
        targetArg = "-target=" + _targetTriple;
        args.push_back(targetArg);
    }

    return compileToIR(importLoc, fid, "zig", "ll", args, "-femit-llvm-ir=");
}

bool ImportManager::loadIRModuleFromPath(
    SourceLocation importLoc, FileID fid, llvm::StringRef path
)
{
    llvm::SMDiagnostic err;
    llvm::LLVMContext localContext;
    auto llvmModule = llvm::parseIRFile(path, err, localContext);

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
