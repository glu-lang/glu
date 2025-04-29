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

    // Source locations for testing
    glu::SourceLocation loc1 = glu::SourceLocation::invalid;
    glu::SourceLocation loc2 = glu::SourceLocation::invalid;
    glu::SourceLocation loc3 = glu::SourceLocation::invalid;

    void SetUp() override
    {
        // Reset SourceManager at the beginning of each test
        sm.reset();

        // Keep a copy of the buffers so they remain valid throughout the test
        sm.loadBuffer(
            llvm::MemoryBuffer::getMemBufferCopy(
                "func testFunc() {\n"
                "    let x = 42;\n"
                "    let y = x + 5;\n"
                "    return y;\n"
                "}\n"
            ),
            "test1.glu"
        );

        sm.loadBuffer(
            llvm::MemoryBuffer::getMemBufferCopy(
                "func secondFunc() {\n"
                "    let a = 10;\n"
                "    return a * 2;\n"
                "}\n"
            ),
            "test2.glu"
        );

        sm.loadBuffer(
            llvm::MemoryBuffer::getMemBufferCopy(
                "struct Point {\n"
                "    x: int,\n"
                "    y: int\n"
                "}\n"
            ),
            "test3.glu"
        );

        // Calculate positions in each file correctly
        loc1 = glu::SourceLocation(28);
        loc2 = glu::SourceLocation(67 + 22);
        loc3 = glu::SourceLocation(67 + 46 + 15);

        // Reset output for each test
        output.clear();
    }
};
// Test to verify that the error method works correctly with source location
TEST_F(DiagnosticTest, ErrorDiagnostic)
{
    diagnostics.error(loc1, "An error occurred");

    // Verify that the diagnostic was properly recorded
    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Error
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "An error occurred");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc1);

    // Print diagnostics to verify location info
    diagnostics.printAll(os);

    // Verify location information appears in output
    ASSERT_TRUE(output.find("test1.glu") != std::string::npos);

    // Verify that hasErrors returns true
    ASSERT_TRUE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, WarningDiagnostic)
{
    diagnostics.warning(loc1, "A warning");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Warning
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "A warning");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc1);

    ASSERT_FALSE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, NoteDiagnostic)
{
    diagnostics.note(loc1, "A note");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Note
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "A note");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc1);

    ASSERT_FALSE(diagnostics.hasErrors());
}

TEST_F(DiagnosticTest, FatalDiagnostic)
{
    diagnostics.fatal(loc1, "A fatal error");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Fatal
    );
    ASSERT_EQ(diagnostics.getMessages()[0].getMessage(), "A fatal error");
    ASSERT_EQ(diagnostics.getMessages()[0].getLocation(), loc1);

    ASSERT_TRUE(diagnostics.hasErrors());
}

// Test to verify the printAll method with diagnostics in different files
TEST_F(DiagnosticTest, PrintAllMultipleFiles)
{
    // Add multiple diagnostics in different files
    diagnostics.error(loc1, "Error in the first file");
    diagnostics.warning(loc2, "Warning in the second file");
    diagnostics.note(loc3, "Note in the third file");

    diagnostics.printAll(os);

    // Verify that the output contains the expected information
    ASSERT_TRUE(output.find("test1.glu") != std::string::npos);
    ASSERT_TRUE(output.find("test2.glu") != std::string::npos);
    ASSERT_TRUE(output.find("test3.glu") != std::string::npos);
    ASSERT_TRUE(output.find("Error in the first file") != std::string::npos);
    ASSERT_TRUE(output.find("Warning in the second file") != std::string::npos);
    ASSERT_TRUE(output.find("Note in the third file") != std::string::npos);
}

TEST_F(DiagnosticTest, InvalidLocation)
{
    diagnostics.error(glu::SourceLocation::invalid, "Error without location");

    ASSERT_EQ(diagnostics.getMessages().size(), 1);
    ASSERT_EQ(
        diagnostics.getMessages()[0].getSeverity(),
        glu::DiagnosticSeverity::Error
    );
    ASSERT_EQ(
        diagnostics.getMessages()[0].getMessage(), "Error without location"
    );
    ASSERT_TRUE(diagnostics.getMessages()[0].getLocation().isInvalid());

    ASSERT_TRUE(diagnostics.hasErrors());

    diagnostics.printAll(os);
}

TEST_F(DiagnosticTest, MultipleDiagnosticsOrder)
{
    // Add diagnostics in random order
    diagnostics.error(loc3, "Error in the third file");
    diagnostics.error(loc1, "Error in the first file");
    diagnostics.error(loc2, "Error in the second file");

    // Print all diagnostics
    diagnostics.printAll(os);

    // Verify that the output contains the errors
    size_t pos1 = output.find("test1.glu");
    size_t pos2 = output.find("test2.glu");
    size_t pos3 = output.find("test3.glu");

    ASSERT_NE(pos1, std::string::npos);
    ASSERT_NE(pos2, std::string::npos);
    ASSERT_NE(pos3, std::string::npos);

    // Check if the order is correct
    ASSERT_LT(pos1, pos2);
    ASSERT_LT(pos2, pos3);
}
