#include "Basic/Diagnostic.hpp"

namespace glu {

void Diagnostic::addDiagnostic(
    DiagnosticSeverity severity, SourceLocation loc, llvm::StringRef message
)
{
    _messages.emplace_back(severity, loc, message.str());

    // Set the error flag if this is an error or fatal error
    _hasErrors = severity >= DiagnosticSeverity::Error;

    // Print the diagnostic immediately for better debugging experience
    printDiagnostic(llvm::errs(), _messages.back());
}

void Diagnostic::error(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Error, loc, message);
}

void Diagnostic::warning(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Warning, loc, message);
}

void Diagnostic::note(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Note, loc, message);
}

void Diagnostic::fatal(SourceLocation loc, llvm::StringRef message)
{
    addDiagnostic(DiagnosticSeverity::Fatal, loc, message);
}

void Diagnostic::printDiagnostic(
    llvm::raw_ostream &os, DiagnosticMessage const &msg
) const
{
    // Don't print anything if location is invalid
    if (!msg.getLocation().isValid()) {
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

    auto lineStart = _sourceManager.getLineStart(msg.getLocation()).getOffset();
    auto lineEnd = _sourceManager.getLineEnd(msg.getLocation()).getOffset();
    auto column = _sourceManager.getSpellingColumnNumber(msg.getLocation());

    // Print the line of source code
    os << _sourceManager.getCharacterDataInStringRef(msg.getLocation())
              .slice(lineStart, lineEnd)
              .str()
       << "\n";

    // Print a caret (^) pointing to the specific column
    os.indent(column - 1);
    os << "^\n";
}

void Diagnostic::printAll(llvm::raw_ostream &os) const
{
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
