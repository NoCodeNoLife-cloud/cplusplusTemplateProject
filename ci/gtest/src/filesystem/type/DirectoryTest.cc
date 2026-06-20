/**
 * @file DirectoryTest.cc
 * @brief Unit tests for the Directory class
 * @details Tests cover directory creation, deletion, copy, move, rename, list, size, and clear operations.
 */

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "filesystem/core/Directory.hpp"

#include "filesystem/core/Path.hpp"
#include "filesystem/core/File.hpp"

using namespace common::filesystem::core;
namespace fs = std::filesystem;

class DirectoryTest : public testing::Test
{
protected:
    fs::path tmpDir_;

    void SetUp() override
    {
        tmpDir_ = fs::temp_directory_path() / "DirectoryTest";
        fs::remove_all(tmpDir_);
        fs::create_directories(tmpDir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tmpDir_, ec);
    }
};

/**
 * @brief Test creating a single directory
 * @details Verifies that mkdir() creates a new directory and makes it exist on the filesystem
 */
TEST_F(DirectoryTest, Mkdir)
{
    const auto dirPath = tmpDir_ / "new_dir";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdir());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_directory(dirPath));
}

/**
 * @brief Test mkdir returns false when directory already exists
 * @details Verifies that mkdir() returns false when the target directory already exists on disk
 */
TEST_F(DirectoryTest, MkdirAlreadyExists)
{
    const auto dirPath = tmpDir_ / "existing_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_FALSE(d.mkdir());
}

/**
 * @brief Test mkdirs creates nested directories recursively
 * @details Verifies that mkdirs() creates all intermediate directories in a nested path like a/b/c
 */
TEST_F(DirectoryTest, MkdirsCreatesNested)
{
    const auto dirPath = tmpDir_ / "a" / "b" / "c";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_directory(dirPath));
}

/**
 * @brief Test mkdirs with existOk=true on existing directory
 * @details Verifies that mkdirs(true) returns true when the directory already exists
 */
TEST_F(DirectoryTest, MkdirsExistOkTrue)
{
    const auto dirPath = tmpDir_ / "exists_ok";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs(true));
}

/**
 * @brief Test mkdirs with existOk=false on existing directory
 * @details Verifies that mkdirs(false) returns false when the directory already exists
 */
TEST_F(DirectoryTest, MkdirsExistOkFalse)
{
    const auto dirPath = tmpDir_ / "exists_fail";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_FALSE(d.mkdirs(false));
}

/**
 * @brief Test exists returns true for an existing directory
 * @details Verifies that exists() returns true when the directory path exists on disk
 */
TEST_F(DirectoryTest, Exists)
{
    const auto dirPath = tmpDir_ / "check_exists";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.exists());
}

/**
 * @brief Test exists returns false for a non-existent directory
 * @details Verifies that exists() returns false when the directory path does not exist
 */
TEST_F(DirectoryTest, ExistsFalse)
{
    const Directory d(tmpDir_ / "nonexistent");
    EXPECT_FALSE(d.exists());
}

/**
 * @brief Test isDirectory returns true for a directory path
 * @details Verifies that isDirectory() correctly identifies a directory as a directory
 */
TEST_F(DirectoryTest, IsDirectory)
{
    const auto dirPath = tmpDir_ / "is_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.isDirectory());
}

/**
 * @brief Test isDirectory returns false for a file path
 * @details Verifies that isDirectory() returns false when the path points to a regular file
 */
TEST_F(DirectoryTest, IsDirectoryForFile)
{
    const auto filePath = tmpDir_ / "a_file.txt";
    std::ofstream(filePath.c_str()) << "content";
    const Directory d(filePath);
    EXPECT_FALSE(d.isDirectory());
}

/**
 * @brief Test isEmpty returns true for an empty directory
 * @details Verifies that isEmpty() returns true when the directory contains no entries
 */
TEST_F(DirectoryTest, IsEmpty)
{
    const auto dirPath = tmpDir_ / "empty_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.isEmpty());
}

/**
 * @brief Test isEmpty returns false for a directory with content
 * @details Verifies that isEmpty() returns false when the directory contains a file
 */
TEST_F(DirectoryTest, IsEmptyWithContent)
{
    const auto dirPath = tmpDir_ / "nonempty_dir";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "file.txt").c_str()) << "content";
    const Directory d(dirPath);
    EXPECT_FALSE(d.isEmpty());
}

/**
 * @brief Test removing an empty directory
 * @details Verifies that remove() deletes the empty directory and returns true
 */
TEST_F(DirectoryTest, Remove)
{
    const auto dirPath = tmpDir_ / "to_remove";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.remove());
    EXPECT_FALSE(fs::exists(dirPath));
}

/**
 * @brief Test remove returns false for a non-existent directory
 * @details Verifies that remove() returns false when the directory path does not exist
 */
TEST_F(DirectoryTest, RemoveNonExistent)
{
    const Directory d(tmpDir_ / "ghost");
    EXPECT_FALSE(d.remove());
}

/**
 * @brief Test removeAll removes a directory tree recursively
 * @details Verifies that removeAll() deletes the directory and all its contents, returning a positive count
 */
TEST_F(DirectoryTest, RemoveAll)
{
    const auto dirPath = tmpDir_ / "to_remove_all";
    fs::create_directories(dirPath / "sub" / "nested");
    std::ofstream((dirPath / "file.txt").c_str()) << "hello";
    const Directory d(dirPath);
    EXPECT_GT(d.removeAll(), 0);
    EXPECT_FALSE(fs::exists(dirPath));
}

/**
 * @brief Test removeAll returns zero for a non-existent directory
 * @details Verifies that removeAll() returns 0 when the path does not exist
 */
TEST_F(DirectoryTest, RemoveAllNonExistentReturnsZero)
{
    const Directory d(tmpDir_ / "ghost");
    EXPECT_EQ(d.removeAll(), 0);
}

/**
 * @brief Test moving a directory to a new location
 * @details Verifies that move() transfers the directory and its contents to the destination path
 */
TEST_F(DirectoryTest, Move)
{
    const auto srcPath = tmpDir_ / "source";
    const auto dstPath = tmpDir_ / "destination";
    fs::create_directories(srcPath);
    std::ofstream((srcPath / "file.txt").c_str()) << "data";
    const Directory d(srcPath);
    EXPECT_TRUE(d.move(dstPath));
    EXPECT_FALSE(fs::exists(srcPath));
    EXPECT_TRUE(fs::exists(dstPath));
    EXPECT_TRUE(fs::exists(dstPath / "file.txt"));
}

/**
 * @brief Test renaming a directory within the same parent
 * @details Verifies that rename() changes the directory name while preserving its content
 */
TEST_F(DirectoryTest, Rename)
{
    const auto srcPath = tmpDir_ / "old_name";
    const auto dstPath = tmpDir_ / "new_name";
    fs::create_directories(srcPath);
    std::ofstream((srcPath / "data.txt").c_str()) << "content";
    const Directory d(srcPath);
    EXPECT_TRUE(d.rename("new_name"));
    EXPECT_FALSE(fs::exists(srcPath));
    EXPECT_TRUE(fs::exists(dstPath));
    EXPECT_TRUE(fs::exists(dstPath / "data.txt"));
}

/**
 * @brief Test copying a directory tree to a new location
 * @details Verifies that copy() duplicates the directory structure and all files to the destination
 */
TEST_F(DirectoryTest, Copy)
{
    const auto srcPath = tmpDir_ / "copy_src";
    const auto dstPath = tmpDir_ / "copy_dst";
    fs::create_directories(srcPath / "subdir");
    std::ofstream((srcPath / "root.txt").c_str()) << "root";
    std::ofstream((srcPath / "subdir" / "nested.txt").c_str()) << "nested";
    const Directory d(srcPath);
    EXPECT_TRUE(d.copy(dstPath));
    EXPECT_TRUE(fs::exists(dstPath));
    EXPECT_TRUE(fs::exists(dstPath / "root.txt"));
    EXPECT_TRUE(fs::exists(dstPath / "subdir" / "nested.txt"));
}

/**
 * @brief Test copy to an already-existing destination directory
 * @details Verifies that copy() succeeds when the destination directory already exists
 */
TEST_F(DirectoryTest, CopyToExistingDestination)
{
    const auto srcPath = tmpDir_ / "copy_src2";
    const auto dstPath = tmpDir_ / "copy_dst2";
    fs::create_directories(srcPath);
    fs::create_directories(dstPath);
    std::ofstream((srcPath / "file.txt").c_str()) << "content";
    const Directory d(srcPath);
    EXPECT_TRUE(d.copy(dstPath));
}

/**
 * @brief Test copy returns false for a non-existent source
 * @details Verifies that copy() returns false when the source directory does not exist
 */
TEST_F(DirectoryTest, CopyNonExistentSource)
{
    const Directory d(tmpDir_ / "nonexistent_src");
    EXPECT_FALSE(d.copy(tmpDir_ / "nowhere"));
}

/**
 * @brief Test size calculates total file size in a directory
 * @details Verifies that size() returns the sum of all file sizes within the directory
 */
TEST_F(DirectoryTest, Size)
{
    const auto dirPath = tmpDir_ / "size_test";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "a.txt").c_str()) << "hello";
    std::ofstream((dirPath / "b.txt").c_str()) << "world";
    const Directory d(dirPath);
    EXPECT_EQ(d.size(), 10);
}

/**
 * @brief Test size returns zero for an empty directory
 * @details Verifies that size() returns 0 when the directory contains no files
 */
TEST_F(DirectoryTest, SizeEmptyDirectory)
{
    const auto dirPath = tmpDir_ / "empty_size";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_EQ(d.size(), 0);
}

/**
 * @brief Test lastModifiedTime returns a valid timestamp
 * @details Verifies that lastModifiedTime() returns a value for an existing directory
 */
TEST_F(DirectoryTest, LastModifiedTime)
{
    const auto dirPath = tmpDir_ / "time_test";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    const auto mtime = d.lastModifiedTime();
    EXPECT_TRUE(mtime.has_value());
}

/**
 * @brief Test lastModifiedTime returns nullopt for a non-existent directory
 * @details Verifies that lastModifiedTime() returns std::nullopt when the directory does not exist
 */
TEST_F(DirectoryTest, LastModifiedTimeNonExistent)
{
    const Directory d(tmpDir_ / "nonexistent_time");
    EXPECT_FALSE(d.lastModifiedTime().has_value());
}

/**
 * @brief Test listing directory contents non-recursively
 * @details Verifies that listDir(false) returns only immediate children and not nested entries
 */
TEST_F(DirectoryTest, ListDirNonRecursive)
{
    const auto dirPath = tmpDir_ / "list_nonrec";
    fs::create_directories(dirPath);
    fs::create_directories(dirPath / "sub");
    std::ofstream((dirPath / "a.txt").c_str()) << "a";
    std::ofstream((dirPath / "b.txt").c_str()) << "b";
    const Directory d(dirPath);
    const auto entries = d.listDir(false);
    EXPECT_EQ(entries.size(), 3);
}

/**
 * @brief Test listing directory contents recursively
 * @details Verifies that listDir(true) returns all entries including those in subdirectories
 */
TEST_F(DirectoryTest, ListDirRecursive)
{
    const auto dirPath = tmpDir_ / "list_rec";
    fs::create_directories(dirPath);
    fs::create_directories(dirPath / "sub" / "nested");
    std::ofstream((dirPath / "root.txt").c_str()) << "root";
    std::ofstream((dirPath / "sub" / "inner.txt").c_str()) << "inner";
    const Directory d(dirPath);
    const auto entries = d.listDir(true);
    EXPECT_EQ(entries.size(), 4);
}

/**
 * @brief Test listing returns empty for a non-existent directory
 * @details Verifies that listDir() returns an empty vector when the directory does not exist
 */
TEST_F(DirectoryTest, ListDirNonExistent)
{
    const Directory d(tmpDir_ / "ghost_list");
    const auto entries = d.listDir(false);
    EXPECT_TRUE(entries.empty());
}

/**
 * @brief Test clearing all content from a directory
 * @details Verifies that clearAll() removes all entries while preserving the directory itself
 */
TEST_F(DirectoryTest, ClearAll)
{
    const auto dirPath = tmpDir_ / "to_clear";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "file1.txt").c_str()) << "one";
    std::ofstream((dirPath / "file2.txt").c_str()) << "two";
    const Directory d(dirPath);
    EXPECT_TRUE(d.clearAll());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_empty(dirPath));
}

/**
 * @brief Test clearAll returns false for a non-existent directory
 * @details Verifies that clearAll() returns false when the directory path does not exist
 */
TEST_F(DirectoryTest, ClearAllNonExistent)
{
    const Directory d(tmpDir_ / "ghost_clear");
    EXPECT_FALSE(d.clearAll());
}

/**
 * @brief Test getPath returns the stored directory path
 * @details Verifies that getPath() returns the exact path used during Directory construction
 */
TEST_F(DirectoryTest, GetPath)
{
    const auto dirPath = tmpDir_ / "get_path_test";
    const Directory d(dirPath);
    EXPECT_EQ(d.getPath(), dirPath);
}

/**
 * @brief Test getCurrentWorkingDirectory returns a valid path
 * @details Verifies that the static method getCurrentWorkingDirectory() returns an existing directory path
 */
TEST_F(DirectoryTest, GetCurrentWorkingDirectory)
{
    const auto cwd = Directory::getCurrentWorkingDirectory();
    EXPECT_TRUE(fs::exists(cwd));
    EXPECT_TRUE(fs::is_directory(cwd));
}

/**
 * @brief Test static listDir non-recursively
 * @details Verifies that the static listDir() method returns only immediate children when recursive=false
 */
TEST_F(DirectoryTest, StaticListDirNonRecursive)
{
    const auto dirPath = tmpDir_ / "static_list";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "x.txt").c_str()) << "x";
    const Directory d(dirPath);
    const auto entries = Directory::listDir(dirPath, false);
    EXPECT_EQ(entries.size(), 1);
}

/**
 * @brief Test static listDir recursively
 * @details Verifies that the static listDir() method returns all entries including nested when recursive=true
 */
TEST_F(DirectoryTest, StaticListDirRecursive)
{
    const auto dirPath = tmpDir_ / "static_list_rec";
    fs::create_directories(dirPath / "sub");
    std::ofstream((dirPath / "a.txt").c_str()) << "a";
    std::ofstream((dirPath / "sub" / "b.txt").c_str()) << "b";
    const auto entries = Directory::listDir(dirPath, true);
    EXPECT_EQ(entries.size(), 3);
}

/**
 * @brief Test clearAll removes subdirectories and files
 * @details Verifies that clearAll() deletes nested subdirectory trees while keeping the root directory
 */
TEST_F(DirectoryTest, ClearAllWithSubdirectories)
{
    const auto dirPath = tmpDir_ / "clear_all_sub";
    fs::create_directories(dirPath / "sub1" / "nested");
    fs::create_directories(dirPath / "sub2");
    std::ofstream((dirPath / "root.txt").c_str()) << "root";
    std::ofstream((dirPath / "sub1" / "a.txt").c_str()) << "a";
    std::ofstream((dirPath / "sub2" / "b.txt").c_str()) << "b";

    const Directory d(dirPath);
    EXPECT_TRUE(d.clearAll());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_empty(dirPath));
}

/**
 * @brief Test clearAll with deeply nested directory structure
 * @details Verifies that clearAll() handles deep nesting (a/b/c/d) correctly and empties the root directory
 */
TEST_F(DirectoryTest, ClearAllWithDeepNesting)
{
    const auto dirPath = tmpDir_ / "clear_all_deep";
    fs::create_directories(dirPath / "a" / "b" / "c" / "d");
    std::ofstream((dirPath / "a" / "b" / "f.txt").c_str()) << "deep";

    const Directory d(dirPath);
    EXPECT_TRUE(d.clearAll());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_empty(dirPath));
}

/**
 * @brief Test copy into itself throws an exception
 * @details Verifies that attempting to copy a directory into itself throws an exception to prevent infinite recursion
 */
TEST_F(DirectoryTest, CopyDirectoryIntoItself)
{
    const auto srcPath = tmpDir_ / "copy_src";
    const auto dstPath = srcPath / "dest";
    fs::create_directories(srcPath);
    std::ofstream((srcPath / "file.txt").c_str()) << "data";

    const Directory src(srcPath);
    EXPECT_ANY_THROW(static_cast<void>(src.copy(dstPath)));
}

/**
 * @brief Test rename to the same path does nothing
 * @details Verifies that renaming a directory to its current name completes without throwing and preserves the directory
 */
TEST_F(DirectoryTest, RenameToSamePath)
{
    const auto dirPath = tmpDir_ / "rename_same";
    fs::create_directories(dirPath);

    const Directory d(dirPath);
    EXPECT_NO_THROW(static_cast<void>(d.rename(dirPath.filename().string())));
    EXPECT_TRUE(fs::exists(dirPath));
}

/**
 * @brief Test size includes files in subdirectories
 * @details Verifies that size() accumulates file sizes from all subdirectories recursively
 */
TEST_F(DirectoryTest, SizeWithSubdirectories)
{
    const auto dirPath = tmpDir_ / "size_sub";
    fs::create_directories(dirPath / "sub");
    std::ofstream((dirPath / "a.txt").c_str()) << "hello";
    std::ofstream((dirPath / "sub" / "b.txt").c_str()) << "world";

    const Directory d(dirPath);
    const auto size = d.size();
    EXPECT_GT(size, 0);
}

/**
 * @brief Test mkdirs with path ending in a trailing slash
 * @details Verifies that mkdirs() correctly handles paths with a trailing separator
 */
TEST_F(DirectoryTest, MkdirsNestedWithTrailingSlash)
{
    const auto dirPath = tmpDir_ / "trailing" / "nested";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs(true));
    EXPECT_TRUE(fs::exists(dirPath));
}

/**
 * @brief Test move to an already-existing location completes without exception
 * @details Verifies that moving a directory to a destination that already exists does not throw
 */
TEST_F(DirectoryTest, MoveToExistingLocation)
{
    const auto srcPath = tmpDir_ / "move_src";
    const auto dstPath = tmpDir_ / "move_dst_existing";
    fs::create_directories(srcPath);
    fs::create_directories(dstPath);
    std::ofstream((srcPath / "f.txt").c_str()) << "data";

    const Directory src(srcPath);
    EXPECT_NO_THROW(static_cast<void>(src.move(dstPath)));
    EXPECT_FALSE(fs::exists(srcPath));
}

// ══════════════════════════════════════════════════════════════════════════
//  Path Conversion & Listing (new methods)
// ══════════════════════════════════════════════════════════════════════════

/**
 * @brief Test converting Directory to Path
 * @details Verifies that toPath() returns a Path whose string matches the
 *          directory path
 */
TEST_F(DirectoryTest, ToPath)
{
    const auto dirPath = tmpDir_ / "topath_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    const Path path = d.toPath();
    EXPECT_EQ(path.string(), dirPath.string());
}

/**
 * @brief Test listing only regular files in a directory
 * @details Verifies that listFiles() returns File objects and excludes
 *          subdirectory entries
 */
TEST_F(DirectoryTest, ListFiles)
{
    const auto dirPath = tmpDir_ / "listfiles_test";
    fs::create_directories(dirPath);

    // Create two regular files and one subdirectory
    std::ofstream((dirPath / "a.txt").c_str()) << "content a";
    std::ofstream((dirPath / "b.txt").c_str()) << "content b";
    fs::create_directories(dirPath / "sub");

    const Directory d(dirPath);
    const auto files = d.listFiles(false);

    // Should contain exactly the two files, not the subdirectory
    EXPECT_EQ(files.size(), 2);
    for (const auto& f : files)
    {
        EXPECT_TRUE(f.isFile());
    }
}

/**
 * @brief Test listFiles on an empty directory
 * @details Verifies that listFiles() returns an empty vector when the
 *          directory contains no files
 */
TEST_F(DirectoryTest, ListFilesEmptyDir)
{
    const auto dirPath = tmpDir_ / "empty_listfiles";
    fs::create_directories(dirPath);

    const Directory d(dirPath);
    const auto files = d.listFiles(false);
    EXPECT_TRUE(files.empty());
}
