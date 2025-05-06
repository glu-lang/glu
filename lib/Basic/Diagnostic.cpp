#include "Basic/Diagnostic.hpp"

namespace glu {

void DiagnosticManager::addDiagnostic(
    DiagnosticSeverity severity, SourceLocation loc, llvm::StringRef message
)
{
    _messages.emplace_back(severity, loc, message.str());

    // Set the error flag if this is an error or fatal error
    if (not _hasErrors)
        _hasErrors = severity >= DiagnosticSeverity::Error;

#if VERBOSE
    // Print the diagnostic immediately for better debugging experience
    printDiagnostic(llvm::errs(), _messages.back());
#endif
}

void DiagnosticManager::error(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Error, loc, message);
}

void DiagnosticManager::warning(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Warning, loc, message);
}

void DiagnosticManager::note(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Note, loc, message);
}

void DiagnosticManager::fatal(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Fatal, loc, message);
}

void DiagnosticManager::printDiagnostic(
    llvm::raw_ostream &os, Diagnostic const &msg
) const
{
    // Don't print anything if location is invalid
    if (not msg.getLocation().isValid()) {
        return;
    }

    // Format the location information
    os << _sourceManager.getBufferName(msg.getLocation()) << ":"
       << _sourceManager.getSpellingLineNumber(msg.getLocation()) << ":"
       << _sourceManager.getSpellingColumnNumber(msg.getLocation()) << ": ";

    // Format the severity prefix with colors if supported
    switch (msg.getSeverity()) {
    case DiagnosticSeverity::Note: os << "note: "; break;
    case DiagnosticSeverity::Warning: os << "warning: "; break;
    case DiagnosticSeverity::Error: os << "error: "; break;
    case DiagnosticSeverity::Fatal: os << "fatal error: "; break;
    }

    // Print the actual message
    os << msg.getMessage() << "\n";

    if (_sourceManager.getCharacterData(msg.getLocation()) == nullptr) {
        return;
    }

    // Get column and line information
    unsigned column = _sourceManager.getSpellingColumnNumber(msg.getLocation());
    llvm::StringRef line = _sourceManager.getLine(msg.getLocation());

    if (line.empty()) {
        return;
    }

    // Print the line of source code
    os << line << "\n";

    // Print a caret (^) pointing to the specific column with proper indentation
    // Make sure the column is 1-based and we need 0-based for indentation
    if (column > 0) {
        os.indent(column - 1);
        os << "^\n";
    } else {
        os << "^\n"; // Fallback if column is 0
    }
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
