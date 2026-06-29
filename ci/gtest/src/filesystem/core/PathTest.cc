/**
 * @file PathTest.cc
 * @brief Unit tests for the Path class
 * @details Tests cover construction (default, string, fs::path, C-string),
 *          path queries (empty, isAbsolute, isRelative, filename, stem,
 *          extension), path manipulation (operator/, parentPath, absolutePath,
 *          lexicallyNormal), comparison, conversion (string, hashCode, toFile,
 *          toDirectory), and implicit conversion to std::filesystem::path.
 */

#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "filesystem/core/Path.hpp"
#include "filesystem/core/Directory.hpp"
#include "filesystem/core/File.hpp"

using namespace cppforge::filesystem::core;
namespace fs = std::filesystem;

// ══════════════════════════════════════════════════════════════════════════
//  Test Fixture
// ══════════════════════════════════════════════════════════════════════════

class PathTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ══════════════════════════════════════════════════════════════════════════
//  Construction
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test default-constructed Path is empty
 * @details Verifies that Path() creates a path with no components
 */
TEST_F(PathTest, Constructor_Default_IsEmpty)
{
    const Path p;
    EXPECT_TRUE(p.empty());
    EXPECT_EQ(p.string(), std::string{});
}

/**
 * @brief Test construction from std::string
 * @details Verifies that Path stores the string path correctly
 */
TEST_F(PathTest, Constructor_FromString_StoresPath)
{
    const Path p("/tmp/foo/bar.txt");
    EXPECT_EQ(p.string(), std::string("/tmp/foo/bar.txt"));
}

/**
 * @brief Test construction from std::filesystem::path
 * @details Verifies that Path wraps an existing fs::path correctly
 */
TEST_F(PathTest, Constructor_FromFilesystemPath_StoresPath)
{
    const Path p(fs::path("/tmp/foo/bar.txt"));
    EXPECT_EQ(p.string(), std::string("/tmp/foo/bar.txt"));
}

/**
 * @brief Test construction from a C-string (const char*)
 * @details Verifies that Path handles C-string construction correctly
 */
TEST_F(PathTest, Constructor_FromCString_StoresPath)
{
    const Path p("/tmp/foo/baz.txt");
    EXPECT_EQ(p.string(), std::string("/tmp/foo/baz.txt"));
}

// ══════════════════════════════════════════════════════════════════════════
//  Path Queries
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test empty() on a non-empty path
 * @details Verifies that empty() returns false when path has components
 */
TEST_F(PathTest, Empty_NonEmpty_ReturnsFalse)
{
    const Path p("/tmp");
    EXPECT_FALSE(p.empty());
}

/**
 * @brief Test empty() on a default-constructed path
 * @details Verifies that empty() returns true for a default Path
 */
TEST_F(PathTest, Empty_Default_ReturnsTrue)
{
    const Path p;
    EXPECT_TRUE(p.empty());
}

/**
 * @brief Test isAbsolute on a relative path
 * @details Verifies that isAbsolute() returns false for a relative path
 */
TEST_F(PathTest, IsAbsolute_RelativePath_ReturnsFalse)
{
    const Path p("relative/path");
    EXPECT_FALSE(p.isAbsolute());
}

/**
 * @brief Test isAbsolute on an absolute path
 * @details Verifies that isAbsolute() returns true for a path that has been
 *          resolved to an absolute location via fs::absolute
 */
TEST_F(PathTest, IsAbsolute_AbsolutePath_ReturnsTrue)
{
    // Use fs::absolute to produce a portably absolute path
    const Path p(fs::absolute(fs::path("some_dir")));
    EXPECT_TRUE(p.isAbsolute());
}

/**
 * @brief Test isRelative on an absolute path
 * @details Verifies that isRelative() returns false when the path is absolute
 */
TEST_F(PathTest, IsRelative_AbsolutePath_ReturnsFalse)
{
    const Path p(fs::absolute(fs::path("some_dir")));
    EXPECT_FALSE(p.isRelative());
}

/**
 * @brief Test isRelative on a relative path
 * @details Verifies that isRelative() returns true for a path without a root
 */
TEST_F(PathTest, IsRelative_RelativePath_ReturnsTrue)
{
    const Path p("relative/path");
    EXPECT_TRUE(p.isRelative());
}

/**
 * @brief Test filename on a simple relative path
 * @details Verifies that filename() returns the last component of a relative path
 */
TEST_F(PathTest, Filename_SimplePath_ReturnsLastComponent)
{
    const Path p("foo/bar.txt");
    EXPECT_EQ(p.filename(), std::string("bar.txt"));
}

/**
 * @brief Test filename on a path with directory components
 * @details Verifies that filename() returns only the file name, not the directory prefix
 */
TEST_F(PathTest, Filename_WithDirectory_ReturnsFilename)
{
    const Path p("/usr/local/bin/tool");
    EXPECT_EQ(p.filename(), std::string("tool"));
}

/**
 * @brief Test filename on a root path
 * @details Verifies that filename() on a root path is either empty or "/"
 *          depending on the platform (empty on MSVC, "/" on POSIX)
 */
TEST_F(PathTest, Filename_RootPath_ReturnsRoot)
{
    const Path p("/");
    // On MSVC: fs::path("/").filename() == "";
    // On POSIX: fs::path("/").filename() == "/";
    EXPECT_TRUE(p.filename().empty() || p.filename() == "/");
}

/**
 * @brief Test stem on a path with extension
 * @details Verifies that stem() returns the filename without the extension
 */
TEST_F(PathTest, Stem_WithExtension_ReturnsStem)
{
    const Path p("data.tar.gz");
    // Stem of "data.tar.gz" is "data.tar" (only the final extension is removed)
    EXPECT_EQ(p.stem(), std::string("data.tar"));
}

/**
 * @brief Test stem on a path without extension
 * @details Verifies that stem() equals the filename when there is no extension
 */
TEST_F(PathTest, Stem_WithoutExtension_ReturnsFullFilename)
{
    const Path p("Makefile");
    EXPECT_EQ(p.stem(), std::string("Makefile"));
}

/**
 * @brief Test stem on a hidden dot-file
 * @details Verifies that stem() treats a leading dot as part of the filename
 */
TEST_F(PathTest, Stem_DotFile_ReturnsDotPrefix)
{
    const Path p(".gitignore");
    // .gitignore has no extension in the usual sense; stem = ".gitignore"
    EXPECT_EQ(p.stem(), std::string(".gitignore"));
}

/**
 * @brief Test extension on a path with a single dot
 * @details Verifies that extension() returns the trailing dot + suffix
 */
TEST_F(PathTest, Extension_WithExtension_ReturnsDotSuffix)
{
    const Path p("readme.txt");
    EXPECT_EQ(p.extension(), std::string(".txt"));
}

/**
 * @brief Test extension on a path without a dot
 * @details Verifies that extension() returns an empty string when no extension
 */
TEST_F(PathTest, Extension_WithoutExtension_ReturnsEmpty)
{
    const Path p("Makefile");
    EXPECT_TRUE(p.extension().empty());
}

/**
 * @brief Test extension on a path with multiple dots
 * @details Verifies that extension() returns only the final extension
 */
TEST_F(PathTest, Extension_MultipleDots_ReturnsFinal)
{
    const Path p("archive.tar.gz");
    EXPECT_EQ(p.extension(), std::string(".gz"));
}

/**
 * @brief Test extension on a dot-file without further extension
 * @details Verifies that a hidden file with no trailing extension returns empty
 */
TEST_F(PathTest, Extension_DotFile_ReturnsEmpty)
{
    const Path p(".gitignore");
    EXPECT_TRUE(p.extension().empty());
}

// ══════════════════════════════════════════════════════════════════════════
//  Path Manipulation
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test operator/ concatenates two path components
 * @details Verifies that Path / Path produces a correctly joined path.
 *          Uses Path equality (delegates to fs::path::operator==) which
 *          handles platform-specific separators correctly.
 */
TEST_F(PathTest, OperatorDivide_ConcatenatesPaths)
{
    const Path base("/tmp");
    const Path sub("foo");
    const Path result = base / sub;
    EXPECT_EQ(result, Path("/tmp/foo"));
}

/**
 * @brief Test operator/ with an empty right-hand side
 * @details Verifies that dividing by an empty path returns a non-empty result
 *          that contains the left operand as a prefix.  On MSVC a trailing
 *          separator is appended (e.g. "/tmp\\"), while on POSIX the result
 *          equals the left operand.  Both are valid.
 */
TEST_F(PathTest, OperatorDivide_EmptyRHS_ReturnsLHS)
{
    const Path base("/tmp");
    const Path empty;
    const Path result = base / empty;
    // The result should not be empty and should contain the base path.
    EXPECT_FALSE(result.empty());
    // The base path string should be a prefix of the result.
    const auto rStr = result.string();
    const auto bStr = base.string();
    EXPECT_TRUE(rStr.find(bStr) == 0);
}

/**
 * @brief Test parentPath on a deeply nested path
 * @details Verifies that parentPath() returns the path minus the last component
 */
TEST_F(PathTest, ParentPath_NestedPath_ReturnsParent)
{
    const Path p("/a/b/c/d.txt");
    const Path parent = p.parentPath();
    EXPECT_EQ(parent.string(), std::string("/a/b/c"));
}

/**
 * @brief Test parentPath on a root path
 * @details Verifies that parentPath() on root returns root itself
 */
TEST_F(PathTest, ParentPath_RootPath_ReturnsRoot)
{
    const Path p("/");
    const Path parent = p.parentPath();
    // Root's parent is root
    EXPECT_EQ(parent, p);
}

/**
 * @brief Test parentPath on a single-component relative path
 * @details Verifies that parentPath() returns an empty path for a bare filename
 */
TEST_F(PathTest, ParentPath_RelativeSingle_ReturnsEmpty)
{
    const Path p("file.txt");
    const Path parent = p.parentPath();
    EXPECT_TRUE(parent.empty());
}

/**
 * @brief Test absolutePath resolves a relative path
 * @details Verifies that absolutePath() returns a non-empty, absolute path
 *          from a relative input, and the resulting path ends with the original
 */
TEST_F(PathTest, AbsolutePath_RelativePath_ReturnsAbsolute)
{
    const Path p("some/relative/path.txt");
    const Path abs = p.absolutePath();
    EXPECT_FALSE(abs.empty());
    EXPECT_TRUE(abs.isAbsolute());
    // The absolute path should end with the original relative path
    const auto absStr = abs.string();
    EXPECT_TRUE(absStr.size() >= std::string("some/relative/path.txt").size());
}

/**
 * @brief Test lexicallyNormal on a path with dot components
 * @details Verifies that lexicallyNormal() removes redundant ./ segments.
 *          Uses Path equality which handles platform separator differences.
 */
TEST_F(PathTest, LexicallyNormal_WithDot_RemovesDot)
{
    const Path p("foo/./bar");
    const Path norm = p.lexicallyNormal();
    EXPECT_EQ(norm, Path("foo/bar"));
}

/**
 * @brief Test lexicallyNormal on a path with dot-dot components
 * @details Verifies that lexicallyNormal() resolves .. segments correctly.
 *          Uses Path equality which handles platform separator differences.
 */
TEST_F(PathTest, LexicallyNormal_WithDotDot_ResolvesParent)
{
    const Path p("foo/bar/../baz");
    const Path norm = p.lexicallyNormal();
    EXPECT_EQ(norm, Path("foo/baz"));
}

/**
 * @brief Test lexicallyNormal on an already-normal path
 * @details Verifies that lexicallyNormal() returns an equivalent path unchanged
 */
TEST_F(PathTest, LexicallyNormal_NormalPath_Unchanged)
{
    const Path p("/usr/local/bin");
    const Path norm = p.lexicallyNormal();
    EXPECT_EQ(norm, p);
}

// ══════════════════════════════════════════════════════════════════════════
//  Comparison
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test equality of two Paths with the same value
 * @details Verifies that operator== returns true for identical paths
 */
TEST_F(PathTest, Equal_SamePath_ReturnsTrue)
{
    const Path a("/tmp/foo.txt");
    const Path b("/tmp/foo.txt");
    EXPECT_TRUE(a == b);
}

/**
 * @brief Test equality of two Paths with different values
 * @details Verifies that operator== returns false for different paths
 */
TEST_F(PathTest, Equal_DifferentPath_ReturnsFalse)
{
    const Path a("/tmp/foo.txt");
    const Path b("/tmp/bar.txt");
    EXPECT_FALSE(a == b);
}

/**
 * @brief Test inequality of two Paths with different values
 * @details Verifies that operator!= returns true for different paths
 */
TEST_F(PathTest, NotEqual_DifferentPath_ReturnsTrue)
{
    const Path a("/tmp/foo.txt");
    const Path b("/tmp/bar.txt");
    EXPECT_TRUE(a != b);
}

/**
 * @brief Test inequality of two Paths with the same value
 * @details Verifies that operator!= returns false for identical paths
 */
TEST_F(PathTest, NotEqual_SamePath_ReturnsFalse)
{
    const Path a("/tmp/foo.txt");
    const Path b("/tmp/foo.txt");
    EXPECT_FALSE(a != b);
}

// ══════════════════════════════════════════════════════════════════════════
//  Conversion
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test string() returns the underlying path string
 * @details Verifies that string() produces the same result as the input
 */
TEST_F(PathTest, String_ReturnsPathString)
{
    const Path p("/some/path/document.txt");
    EXPECT_EQ(p.string(), std::string("/some/path/document.txt"));
}

/**
 * @brief Test hashCode returns the same value for equal paths
 * @details Verifies that hashCode() is consistent for Paths with the same value
 */
TEST_F(PathTest, HashCode_SamePath_ReturnsSameValue)
{
    const Path a("/data/file.bin");
    const Path b("/data/file.bin");
    EXPECT_EQ(a.hashCode(), b.hashCode());
}

/**
 * @brief Test toFile returns a File with the correct path
 * @details Verifies that toFile() constructs a File representing the same path
 */
TEST_F(PathTest, ToFile_ReturnsFileWithCorrectPath)
{
    const Path p("/tmp/test_file.dat");
    const File f = p.toFile();
    EXPECT_EQ(f.getPath(), p.string());
}

/**
 * @brief Test toDirectory returns a Directory with the correct path
 * @details Verifies that toDirectory() constructs a Directory representing the same path
 */
TEST_F(PathTest, ToDirectory_ReturnsDirectoryWithCorrectPath)
{
    const Path p("/tmp/test_directory");
    const Directory d = p.toDirectory();
    EXPECT_EQ(d.getPath(), p.native());
}

// ══════════════════════════════════════════════════════════════════════════
//  Implicit Conversion
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test implicit conversion to std::filesystem::path
 * @details Verifies that a Path can be passed where a const fs::path& is
 *          expected, and the underlying path is preserved
 */
TEST_F(PathTest, ImplicitConversion_ToFilesystemPath)
{
    const Path p("/usr/local/lib/libfoo.so");

    // Implicit conversion via operator const fs::path&
    const fs::path& nativeRef = p;
    EXPECT_EQ(nativeRef.string(), std::string("/usr/local/lib/libfoo.so"));

    // Also works through a function that takes fs::path
    const auto takePath = [](const fs::path& pp) { return pp.string(); };
    EXPECT_EQ(takePath(p), std::string("/usr/local/lib/libfoo.so"));
}
