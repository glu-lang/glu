#include "Basic/Diagnostic.hpp"
#include "Basic/SourceLocation.hpp"
#include "Basic/SourceManager.hpp"

#include <gtest/gtest.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>

class DiagnosticTest : public ::testing::Test {
protected:
    glu::SourceManager sm;
    glu::DiagnosticManager diagnostics { sm };
    std::string output;
    llvm::raw_string_ostream os { output };
    glu::SourceLocation loc = glu::SourceLocation::invalid;

    void SetUp() override
    {
        // Default buffer
        std::unique_ptr<llvm::MemoryBuffer> buf(
            llvm::MemoryBuffer::getMemBufferCopy(
                "func testFunc() {\n"
                "    let x = 42;\n"
                "    let y = x + 5;\n"
                "    return y;\n"
                "}\n"
            )
        );
        sm.loadBuffer(std::move(buf), "test.glu");

        // Test position (line 3, column 13)
        loc = glu::SourceLocation(28); // After "x + " on line 3

        output.clear();
    }
};

TEST_F(DiagnosticTest, ErrorDiagnostic)
{
    diagnostics.error(loc, "One error occured");

    // Check if diagnostic is correctly set.
    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Error
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "One error occured");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc);

    // Check if hasErrors is true
    ASSERT_TRUE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, WarningDiagnostic)
{
    diagnostics.warning(loc, "Warning");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Warning
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "Warning");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc);

    ASSERT_FALSE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, NoteDiagnostic)
{
    diagnostics.note(loc, "Note");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Note
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "Note");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc);

    ASSERT_FALSE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, FatalDiagnostic)
{
    diagnostics.fatal(loc, "Fatal error");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Fatal
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "Fatal error");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc);

    ASSERT_TRUE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, PrintAll)
{
    diagnostics.error(loc, "First error");
    diagnostics.warning(glu::SourceLocation(15), "Warning here");
    diagnostics.note(glu::SourceLocation(40), "Note for informations");

    diagnostics.printAll(os);

    ASSERT_TRUE(output.find("First error") != std::string::npos);
    ASSERT_TRUE(output.find("Warning here") != std::string::npos);
    ASSERT_TRUE(output.find("Note for informations") != std::string::npos);
    ASSERT_TRUE(output.find("error:") != std::string::npos);
    ASSERT_TRUE(output.find("warning:") != std::string::npos);
    ASSERT_TRUE(output.find("note:") != std::string::npos);
    ASSERT_TRUE(output.find("1 error(s)") != std::string::npos);
    ASSERT_TRUE(output.find("1 warning(s)") != std::string::npos);
    ASSERT_TRUE(output.find("1 note(s)") != std::string::npos);
}

TEST_F(DiagnosticTest, InvalidLocation)
{
    diagnostics.error(glu::SourceLocation::invalid, "Without localization");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Error
    );
    ASSERT_EQ(
        diagnostics.getMessages()[0].getMessage(), "Without localization"
    );
    ASSERT_TRUE(diagnostics.getMessages()[0].getLocation().isInvalid());

    ASSERT_TRUE(diagnostics.hasErrors());

    diagnostics.printAll(os);
}

TEST_F(DiagnosticTest, MultipleDiagnosticsOrder)
{
    // Add many diagnostics in different order
    diagnostics.error(glu::SourceLocation(40), "Error line 4");
    diagnostics.error(glu::SourceLocation(15), "Error line 2");
    diagnostics.error(glu::SourceLocation(28), "Error line 3");

    // Print all diagnostics
    diagnostics.printAll(os);

    // Check if the output contains the expected messages
    size_t pos1 = output.find("Error line 2");
    size_t pos2 = output.find("Error line 3");
    size_t pos3 = output.find("Error line 4");

    ASSERT_NE(pos1, std::string::npos);
    ASSERT_NE(pos2, std::string::npos);
    ASSERT_NE(pos3, std::string::npos);

    // Check if the order is correct
    ASSERT_LT(pos1, pos2);
    ASSERT_LT(pos2, pos3);
}
