#include "ImportHandler.hpp"
#include "ImportManager.hpp"
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
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/TargetParser/Host.h>

namespace glu::sema {

struct AutoImportConfig {
    llvm::StringRef sourceFile;
    llvm::StringRef moduleName;
    llvm::StringRef outputIRFile;
    llvm::StringRef outputLinkerFile;
};

struct AutoImportTemplateArg {
    llvm::StringRef content;
    enum Kind {
        String,
        SourceFile,
        ModuleName,
        OutputIRFile,
        OutputLinkerFile
    } kind;

    AutoImportTemplateArg(char const *content)
        : content(content), kind(Kind::String)
    {
    }

    AutoImportTemplateArg(Kind kind) : kind(kind) { }

    llvm::StringRef resolve(AutoImportConfig const &config) const
    {
        switch (kind) {
        case Kind::String: return content;
        case Kind::SourceFile: return config.sourceFile;
        case Kind::ModuleName: return config.moduleName;
        case Kind::OutputIRFile: return config.outputIRFile;
        case Kind::OutputLinkerFile: return config.outputLinkerFile;
        }
        llvm_unreachable("Unknown AutoImportTemplateArg kind");
    }
};

static AutoImportTemplateArg CLANG_TEMPLATE[]
    = { "clang",
        "-g",
        "-c",
        "-emit-llvm",
        AutoImportTemplateArg::SourceFile,
        "-o",
        AutoImportTemplateArg::OutputIRFile };
static AutoImportTemplateArg RUST_TEMPLATE[]
    = { "rustc",
        "-g",
        "--crate-type=staticlib",
        "--emit",
        "llvm-ir=",
        AutoImportTemplateArg::OutputIRFile,
        "--emit",
        "link=",
        AutoImportTemplateArg::OutputLinkerFile,
        AutoImportTemplateArg::SourceFile };
static AutoImportTemplateArg ZIG_TEMPLATE[]
    = { "zig",
        "build-obj",
        "-fllvm",
        "-fno-strip",
        AutoImportTemplateArg::SourceFile,
        "-femit-llvm-ir=",
        AutoImportTemplateArg::OutputIRFile };
static AutoImportTemplateArg SWIFT_TEMPLATE[]
    = { "swiftc",
        "-parse-as-library",
        "-emit-ir",
        "-g",
        "-gdwarf-types",
        "-module-name",
        AutoImportTemplateArg::ModuleName,
        AutoImportTemplateArg::SourceFile,
        "-o",
        AutoImportTemplateArg::OutputIRFile };
static AutoImportTemplateArg D_TEMPLATE[]
    = { "ldc2",
        "-c",
        "--output-bc",
        "-g",
        AutoImportTemplateArg::SourceFile,
        "-of=",
        AutoImportTemplateArg::OutputIRFile };

bool ImportManager::compileToIR(
    SourceLocation importLoc, FileID fid,
    llvm::ArrayRef<AutoImportTemplateArg> templateArgs
)
{
    auto cachedPath = _generatedBitcodePaths.find(fid);
    if (cachedPath != _generatedBitcodePaths.end()) {
        return loadIRModuleFromPath(importLoc, fid, cachedPath->second);
    }

    auto *sm = _context.getSourceManager();
    llvm::StringRef sourcePath = sm->getBufferName(fid);

    llvm::SmallString<128> tempPath;
    std::error_code ec
        = llvm::sys::fs::createTemporaryFile("glu-import", "ll", tempPath);
    if (ec) {
        _diagManager.error(
            importLoc, "Failed to create temporary file: " + ec.message()
        );
        return false;
    }

    assert(
        !templateArgs.empty()
        && templateArgs[0].kind == AutoImportTemplateArg::Kind::String
    );
    auto compilerName = templateArgs[0].content;
    auto compilerPath = llvm::sys::findProgramByName(compilerName);
    if (!compilerPath) {
        _diagManager.error(
            importLoc,
            "Could not find " + compilerName.str() + " to compile '"
                + sourcePath.str() + "': " + compilerPath.getError().message()
        );
        return false;
    }

    AutoImportConfig config = {
        sourcePath,
        llvm::sys::path::stem(llvm::sys::path::filename(sourcePath)),
        tempPath.str(),
        "" // lazy initialization
    };
    llvm::SmallVector<llvm::StringRef, 12> compilerArgs;
    for (auto &arg : templateArgs) {
        if (arg.kind == AutoImportTemplateArg::Kind::OutputLinkerFile
            && config.outputLinkerFile.empty()) {
            // Create temporary file for linker output if needed
            llvm::SmallString<128> linkerTempPath;
            std::error_code lec = llvm::sys::fs::createTemporaryFile(
                "glu-import-linker", "a", linkerTempPath
            );
            if (lec) {
                _diagManager.error(
                    importLoc,
                    "Failed to create temporary file for linker output: "
                        + lec.message()
                );
                return false;
            }
            config.outputLinkerFile = linkerTempPath.str();
            _generatedObjectPaths[fid] = config.outputLinkerFile;
        }
        compilerArgs.push_back(arg.resolve(config));
    }

    llvm::SmallVector<std::string> _retain;
    // Handle arguments ending with '=' that need to be merged with the next arg
    llvm::SmallVector<llvm::StringRef, 12> args;
    for (std::size_t i = 0; i < compilerArgs.size(); ++i) {
        auto arg = compilerArgs[i];
        if (arg.ends_with("=") && i + 1 < compilerArgs.size()) {
            _retain.push_back((arg + compilerArgs[i + 1]).str());
            args.push_back(_retain.back());
            ++i; // skip next arg
        } else {
            args.push_back(arg);
        }
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
    return compileToIR(importLoc, fid, CLANG_TEMPLATE);
}

bool ImportManager::loadCxxSource(SourceLocation importLoc, FileID fid)
{
    return compileToIR(importLoc, fid, CLANG_TEMPLATE);
}

bool ImportManager::loadRustSource(SourceLocation importLoc, FileID fid)
{
    return compileToIR(importLoc, fid, RUST_TEMPLATE);
}

bool ImportManager::loadZigSource(SourceLocation importLoc, FileID fid)
{
    return compileToIR(importLoc, fid, ZIG_TEMPLATE);
}

bool ImportManager::loadSwiftSource(SourceLocation importLoc, FileID fid)
{
    return compileToIR(importLoc, fid, SWIFT_TEMPLATE);
}

bool ImportManager::loadDSource(SourceLocation importLoc, FileID fid)
{
    return compileToIR(importLoc, fid, D_TEMPLATE);
}

} // namespace glu::sema
