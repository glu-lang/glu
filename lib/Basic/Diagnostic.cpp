#include "Basic/Diagnostic.hpp"

#include <llvm/Support/WithColor.h>

namespace glu {

void DiagnosticManager::addDiagnostic(
    DiagnosticSeverity severity, SourceLocation loc, llvm::Twine const &message,
    std::unique_ptr<Diagnostic> note
)
{
    _messages.emplace_back(severity, loc, message.str(), std::move(note));

    // Set the error flag if this is an error or fatal error
    if (not _hasErrors)
        _hasErrors = severity >= DiagnosticSeverity::Error;

#if VERBOSE
    // Print the diagnostic immediately for better debugging experience
    printDiagnostic(llvm::errs(), _messages.back());
#endif
}

void DiagnosticManager::error(
    SourceLocation loc, llvm::Twine const &message,
    std::unique_ptr<Diagnostic> note
)
{
    addDiagnostic(
        DiagnosticSeverity::Error, loc, std::move(message), std::move(note)
    );
}

void DiagnosticManager::warning(
    SourceLocation loc, llvm::Twine const &message,
    std::unique_ptr<Diagnostic> note
)
{
    addDiagnostic(
        DiagnosticSeverity::Warning, loc, std::move(message), std::move(note)
    );
}

void DiagnosticManager::note(
    SourceLocation loc, llvm::Twine const &message,
    std::unique_ptr<Diagnostic> note
)
{
    addDiagnostic(
        DiagnosticSeverity::Note, loc, std::move(message), std::move(note)
    );
}

void DiagnosticManager::fatal(
    SourceLocation loc, llvm::Twine const &message,
    std::unique_ptr<Diagnostic> note
)
{
    addDiagnostic(
        DiagnosticSeverity::Fatal, loc, std::move(message), std::move(note)
    );
}

void DiagnosticManager::printDiagnostic(
    llvm::raw_ostream &os, Diagnostic const &msg
) const
{
    // Don't print anything if location is invalid
    if (msg.getLocation().isValid()) {
        llvm::WithColor(os, llvm::raw_ostream::SAVEDCOLOR, true)
            << _sourceManager.getBufferName(msg.getLocation()) << ":"
            << _sourceManager.getSpellingLineNumber(msg.getLocation()) << ":"
            << _sourceManager.getSpellingColumnNumber(msg.getLocation())
            << ": ";
    }

    // Format the severity prefix with colors if supported
    switch (msg.getSeverity()) {
    case DiagnosticSeverity::Note: llvm::WithColor::note(os); break;
    case DiagnosticSeverity::Warning: llvm::WithColor::warning(os); break;
    case DiagnosticSeverity::Error: llvm::WithColor::error(os); break;
    case DiagnosticSeverity::Fatal:
        llvm::WithColor(os, llvm::HighlightColor::Error).get()
            << "fatal error: ";
        break;
    }

    // Print the actual message in bold
    llvm::WithColor(os, llvm::raw_ostream::SAVEDCOLOR, true)
        << msg.getMessage() << "\n";

    if (_sourceManager.getCharacterData(msg.getLocation()) == nullptr) {
        return;
    }

    // Get column and line information
    unsigned column = _sourceManager.getSpellingColumnNumber(msg.getLocation());
    llvm::StringRef line = _sourceManager.getLine(msg.getLocation());

    if (line.empty()) {
        return;
    }

    // Print the line of source code with line number
    unsigned lineNumber
        = _sourceManager.getSpellingLineNumber(msg.getLocation());
    os << llvm::format("%5u | ", lineNumber) << line << "\n";

    // Print a caret (^) pointing to the specific column with proper indentation
    // Account for the line number prefix (5 digits + " | " = 8 characters)
    os << "      | ";
    if (column > 0) {
        os.indent(column - 1);
        os << "^\n";
    } else {
        os << "^\n"; // Fallback if column is 0
    }

    if (msg.getNote() != nullptr)
        printDiagnostic(os, *msg.getNote());
}

void DiagnosticManager::printAll(llvm::raw_ostream &os)
{
    std::sort(
        _messages.begin(), _messages.end(),
        [this](Diagnostic const &a, Diagnostic const &b) {
            auto aFileName = _sourceManager.getBufferName(a.getLocation());
            auto bFileName = _sourceManager.getBufferName(b.getLocation());

            if (aFileName != bFileName) {
                return aFileName < bFileName;
            }

            auto aLineNumber
                = _sourceManager.getSpellingLineNumber(a.getLocation());
            auto bLineNumber
                = _sourceManager.getSpellingLineNumber(b.getLocation());
            if (aLineNumber != bLineNumber) {
                return aLineNumber < bLineNumber;
            }

            return _sourceManager.getSpellingColumnNumber(a.getLocation())
                < _sourceManager.getSpellingColumnNumber(b.getLocation());
        }
    );

    for (auto const &msg : _messages) {
        printDiagnostic(os, msg);
    }

    // Print summary counts
    size_t errorCount = 0;
    size_t warningCount = 0;
    size_t noteCount = 0;

    for (auto const &msg : _messages) {
        switch (msg.getSeverity()) {
        case DiagnosticSeverity::Note: ++noteCount; break;
        case DiagnosticSeverity::Warning: ++warningCount; break;
        case DiagnosticSeverity::Error:
        case DiagnosticSeverity::Fatal: ++errorCount; break;
        }
    }

    if (errorCount > 0 || warningCount > 0 || noteCount > 0) {
        os << "Compilation finished with:\n";
        if (errorCount > 0) {
            os << errorCount << " error(s)\n";
        }
        if (warningCount > 0) {
            os << warningCount << " warning(s)\n";
        }
        if (noteCount > 0) {
            os << noteCount << " note(s)\n";
        }
    } else {
        os << "Compilation finished successfully.\n";
    }
}

} // namespace glu
