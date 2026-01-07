#ifndef GLU_BASIC_DIAGNOSTIC_HPP
#define GLU_BASIC_DIAGNOSTIC_HPP

#include "SourceLocation.hpp"
#include "SourceManager.hpp"
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/raw_ostream.h>
#include <string>

namespace glu {

/// @brief Level of severity for diagnostic messages.
enum class DiagnosticSeverity {
    Note, ///< Informational message
    Warning, ///< Warning that doesn't prevent compilation
    Error, ///< Error that prevents compilation
    Fatal ///< Fatal error that stops compilation immediately
};

/// @brief A diagnostic message with location and severity information.
class Diagnostic {
    DiagnosticSeverity _severity;
    SourceLocation _location;
    std::string _message;
    std::unique_ptr<Diagnostic> _note;

public:
    /// @brief Construct a diagnostic message.
    /// @param severity The severity level of the diagnostic.
    /// @param location The source location where the diagnostic occurred.
    /// @param message The message text.
    Diagnostic(
        DiagnosticSeverity severity, SourceLocation location,
        llvm::Twine const &message, std::unique_ptr<Diagnostic> note = nullptr
    )
        : _severity(severity)
        , _location(location)
        , _message(message.str())
        , _note(std::move(note))
    {
    }

    /// @return The severity level of this diagnostic.
    DiagnosticSeverity getSeverity() const { return _severity; }

    /// @return The source location where the diagnostic occurred.
    SourceLocation getLocation() const { return _location; }

    /// @return The message text.
    std::string const &getMessage() const { return _message; }

    /// @return An optional note associated with this diagnostic.
    Diagnostic const *getNote() const { return _note.get(); }

    /// @brief Sets an associated note for this diagnostic.
    /// @param note The note to associate with this diagnostic.
    void addNote(std::unique_ptr<Diagnostic> note)
    {
        if (_note) {
            // If there's already a note, chain the new note to the existing one
            _note->addNote(std::move(note));
        } else {
            _note = std::move(note);
        }
    }
};

/// @class DiagnosticManager
/// @brief Class for reporting and collecting diagnostics during compilation.
class DiagnosticManager {
    SourceManager &_sourceManager;
    llvm::SmallVector<Diagnostic, 8> _messages;
    bool _hasErrors = false;

public:
    /// @brief Constructs a DiagnosticManager instance.
    /// @param sourceManager The source manager to use for location information.
    explicit DiagnosticManager(SourceManager &sourceManager)
        : _sourceManager(sourceManager)
    {
    }

    /// @brief Reports an error at the specified source location.
    /// @param loc The source location where the error occurred.
    /// @param message The error message.
    void error(SourceLocation loc, llvm::Twine const &message);

    /// @brief Reports a warning at the specified source location.
    /// @param loc The source location where the warning occurred.
    /// @param message The warning message.
    void warning(SourceLocation loc, llvm::Twine const &message);

    /// @brief Reports an informational note at the specified source location.
    /// @param loc The source location where the note is relevant.
    /// @param message The note message.
    void note(SourceLocation loc, llvm::Twine const &message);

    /// @brief Reports a fatal error and stops compilation.
    /// @param loc The source location where the fatal error occurred.
    /// @param message The fatal error message.
    void fatal(SourceLocation loc, llvm::Twine const &message);

    /// @brief Prints all collected diagnostics to the specified output stream.
    /// @param os The output stream where diagnostics will be printed.
    void printAll(llvm::raw_ostream &os = llvm::errs());

    /// @brief Returns whether any errors have been reported.
    /// @return True if any errors have been reported, false otherwise.
    bool hasErrors() const { return _hasErrors; }

    /// @brief Returns all collected diagnostic messages.
    /// @return A vector of all diagnostic messages.
    llvm::SmallVector<Diagnostic, 8> const &getMessages() const
    {
        return _messages;
    }

private:
    /// @brief Adds a diagnostic message to the collection.
    /// @param severity The severity level of the diagnostic.
    /// @param loc The source location where the diagnostic occurred.
    /// @param message The message text.
    void addDiagnostic(
        DiagnosticSeverity severity, SourceLocation loc,
        llvm::Twine const &message, std::unique_ptr<Diagnostic> note = nullptr
    );

    /// @brief Formats and prints a single diagnostic message.
    /// @param os The output stream where the diagnostic will be printed.
    /// @param msg The diagnostic message to print.
    void printDiagnostic(llvm::raw_ostream &os, Diagnostic const &msg) const;
};

} // namespace glu

#endif // GLU_BASIC_DIAGNOSTIC_HPP
