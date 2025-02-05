#include "Instructions.hpp"
#include <gtest/gtest.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringRef.h>

/// @class MemberTest
/// @brief Test suite for the Member class and its usage in DenseMap.
class MemberTest : public ::testing::Test {
protected:
    glu::gil::Type parentType
        = glu::gil::Type(8, 8, false, nullptr); ///< Dummy type for testing.

    glu::gil::Member m1 = glu::gil::Member(
        "x", glu::gil::Type(4, 4, false, nullptr), &parentType
    ); ///< Member x.
    glu::gil::Member m2 = glu::gil::Member(
        "y", glu::gil::Type(8, 8, false, nullptr), &parentType
    ); ///< Member y.
    glu::gil::Member m3 = glu::gil::Member(
        "z", glu::gil::Type(4, 4, true, nullptr), &parentType
    ); ///< Member z.
    glu::gil::Member m4 = glu::gil::Member(
        "w", glu::gil::Type(16, 16, false, nullptr), &parentType
    ); ///< Member w.
};

/// @brief Test insertion and retrieval in a DenseMap.
TEST_F(MemberTest, InsertAndRetrieve)
{
    llvm::DenseMap<glu::gil::Member, int> memberMap;
    memberMap[m1] = 1;
    memberMap[m2] = 2;
    memberMap[m3] = 3;

    EXPECT_EQ(memberMap.find(m1) != memberMap.end(), true);
    EXPECT_EQ(memberMap.find(m2) != memberMap.end(), true);
    EXPECT_EQ(memberMap.find(m3) != memberMap.end(), true);
}

/// @brief Verify that a non-added element is not found.
TEST_F(MemberTest, NotFound)
{
    llvm::DenseMap<glu::gil::Member, int> memberMap;
    memberMap[m1] = 1;
    memberMap[m2] = 2;

    EXPECT_EQ(memberMap.find(m4) == memberMap.end(), true);
}

/// @brief Test special keys EmptyKey and TombstoneKey.
TEST_F(MemberTest, EmptyAndTombstoneKeys)
{
    llvm::DenseMap<glu::gil::Member, int> memberMap;
    glu::gil::Member emptyKey = glu::gil::Member::getEmptyKey();
    glu::gil::Member tombstoneKey = glu::gil::Member::getTombstoneKey();

    EXPECT_EQ(memberMap.find(emptyKey) == memberMap.end(), true);
    EXPECT_EQ(memberMap.find(tombstoneKey) == memberMap.end(), true);
}

/// @brief Tests equality and inequality operators between two Members.
TEST_F(MemberTest, EqualityAndInequality)
{
    glu::gil::Member m1_copy = m1;
    EXPECT_EQ(m1, m1_copy); ///< Expect m1 to be equal to its copy.
    EXPECT_NE(m1, m2); ///< Expect m1 to be different from m2.
}

/// @brief Tests the update of a Member element in a DenseMap.
TEST_F(MemberTest, UpdateValue)
{
    llvm::DenseMap<glu::gil::Member, int> memberMap;
    memberMap[m1] = 1; ///< Insert m1 with value 1.
    memberMap[m1] = 42; ///< Update m1's value to 42.

    EXPECT_EQ(memberMap[m1], 42); ///< Expect the updated value to be 42.
}

/// @brief Tests the erase functionality in a DenseMap for Member elements.
/// Verifies that an element can be erased from the map and that it's no longer
/// found.
TEST_F(MemberTest, EraseElement)
{
    llvm::DenseMap<glu::gil::Member, int> memberMap;
    memberMap[m1] = 1; ///< Insert m1 with value 1.

    memberMap.erase(m1); ///< Erase m1 from the map.
    EXPECT_EQ(
        memberMap.find(m1) == memberMap.end(), true
    ); ///< Expect m1 to be erased, and the map to not contain it anymore.
}
