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

#include "filesystem/type/Directory.hpp"

using namespace common::filesystem::type;
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

TEST_F(DirectoryTest, Mkdir)
{
    const auto dirPath = tmpDir_ / "new_dir";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdir());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_directory(dirPath));
}

TEST_F(DirectoryTest, MkdirAlreadyExists)
{
    const auto dirPath = tmpDir_ / "existing_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_FALSE(d.mkdir());
}

TEST_F(DirectoryTest, MkdirsCreatesNested)
{
    const auto dirPath = tmpDir_ / "a" / "b" / "c";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs());
    EXPECT_TRUE(fs::exists(dirPath));
    EXPECT_TRUE(fs::is_directory(dirPath));
}

TEST_F(DirectoryTest, MkdirsExistOkTrue)
{
    const auto dirPath = tmpDir_ / "exists_ok";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs(true));
}

TEST_F(DirectoryTest, MkdirsExistOkFalse)
{
    const auto dirPath = tmpDir_ / "exists_fail";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_FALSE(d.mkdirs(false));
}

TEST_F(DirectoryTest, Exists)
{
    const auto dirPath = tmpDir_ / "check_exists";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.exists());
}

TEST_F(DirectoryTest, ExistsFalse)
{
    const Directory d(tmpDir_ / "nonexistent");
    EXPECT_FALSE(d.exists());
}

TEST_F(DirectoryTest, IsDirectory)
{
    const auto dirPath = tmpDir_ / "is_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.isDirectory());
}

TEST_F(DirectoryTest, IsDirectoryForFile)
{
    const auto filePath = tmpDir_ / "a_file.txt";
    std::ofstream(filePath.c_str()) << "content";
    const Directory d(filePath);
    EXPECT_FALSE(d.isDirectory());
}

TEST_F(DirectoryTest, IsEmpty)
{
    const auto dirPath = tmpDir_ / "empty_dir";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.isEmpty());
}

TEST_F(DirectoryTest, IsEmptyWithContent)
{
    const auto dirPath = tmpDir_ / "nonempty_dir";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "file.txt").c_str()) << "content";
    const Directory d(dirPath);
    EXPECT_FALSE(d.isEmpty());
}

TEST_F(DirectoryTest, Remove)
{
    const auto dirPath = tmpDir_ / "to_remove";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_TRUE(d.remove());
    EXPECT_FALSE(fs::exists(dirPath));
}

TEST_F(DirectoryTest, RemoveNonExistent)
{
    const Directory d(tmpDir_ / "ghost");
    EXPECT_FALSE(d.remove());
}

TEST_F(DirectoryTest, RemoveAll)
{
    const auto dirPath = tmpDir_ / "to_remove_all";
    fs::create_directories(dirPath / "sub" / "nested");
    std::ofstream((dirPath / "file.txt").c_str()) << "hello";
    const Directory d(dirPath);
    EXPECT_GT(d.removeAll(), 0);
    EXPECT_FALSE(fs::exists(dirPath));
}

TEST_F(DirectoryTest, RemoveAllNonExistentReturnsZero)
{
    const Directory d(tmpDir_ / "ghost");
    EXPECT_EQ(d.removeAll(), 0);
}

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

TEST_F(DirectoryTest, CopyNonExistentSource)
{
    const Directory d(tmpDir_ / "nonexistent_src");
    EXPECT_FALSE(d.copy(tmpDir_ / "nowhere"));
}

TEST_F(DirectoryTest, Size)
{
    const auto dirPath = tmpDir_ / "size_test";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "a.txt").c_str()) << "hello";
    std::ofstream((dirPath / "b.txt").c_str()) << "world";
    const Directory d(dirPath);
    EXPECT_EQ(d.size(), 10);
}

TEST_F(DirectoryTest, SizeEmptyDirectory)
{
    const auto dirPath = tmpDir_ / "empty_size";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    EXPECT_EQ(d.size(), 0);
}

TEST_F(DirectoryTest, LastModifiedTime)
{
    const auto dirPath = tmpDir_ / "time_test";
    fs::create_directories(dirPath);
    const Directory d(dirPath);
    const auto mtime = d.lastModifiedTime();
    EXPECT_TRUE(mtime.has_value());
}

TEST_F(DirectoryTest, LastModifiedTimeNonExistent)
{
    const Directory d(tmpDir_ / "nonexistent_time");
    EXPECT_FALSE(d.lastModifiedTime().has_value());
}

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

TEST_F(DirectoryTest, ListDirNonExistent)
{
    const Directory d(tmpDir_ / "ghost_list");
    const auto entries = d.listDir(false);
    EXPECT_TRUE(entries.empty());
}

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

TEST_F(DirectoryTest, ClearAllNonExistent)
{
    const Directory d(tmpDir_ / "ghost_clear");
    EXPECT_FALSE(d.clearAll());
}

TEST_F(DirectoryTest, GetPath)
{
    const auto dirPath = tmpDir_ / "get_path_test";
    const Directory d(dirPath);
    EXPECT_EQ(d.getPath(), dirPath);
}

TEST_F(DirectoryTest, GetCurrentWorkingDirectory)
{
    const auto cwd = Directory::getCurrentWorkingDirectory();
    EXPECT_TRUE(fs::exists(cwd));
    EXPECT_TRUE(fs::is_directory(cwd));
}

TEST_F(DirectoryTest, StaticListDirNonRecursive)
{
    const auto dirPath = tmpDir_ / "static_list";
    fs::create_directories(dirPath);
    std::ofstream((dirPath / "x.txt").c_str()) << "x";
    const Directory d(dirPath);
    const auto entries = Directory::listDir(dirPath, false);
    EXPECT_EQ(entries.size(), 1);
}

TEST_F(DirectoryTest, StaticListDirRecursive)
{
    const auto dirPath = tmpDir_ / "static_list_rec";
    fs::create_directories(dirPath / "sub");
    std::ofstream((dirPath / "a.txt").c_str()) << "a";
    std::ofstream((dirPath / "sub" / "b.txt").c_str()) << "b";
    const auto entries = Directory::listDir(dirPath, true);
    EXPECT_EQ(entries.size(), 3);
}

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

TEST_F(DirectoryTest, CopyDirectoryIntoItself)
{
    const auto srcPath = tmpDir_ / "copy_src";
    const auto dstPath = srcPath / "dest";
    fs::create_directories(srcPath);
    std::ofstream((srcPath / "file.txt").c_str()) << "data";

    const Directory src(srcPath);
    EXPECT_ANY_THROW(static_cast<void>(src.copy(dstPath)));
}

TEST_F(DirectoryTest, RenameToSamePath)
{
    const auto dirPath = tmpDir_ / "rename_same";
    fs::create_directories(dirPath);

    const Directory d(dirPath);
    EXPECT_NO_THROW(d.rename(dirPath.filename().string()));
    EXPECT_TRUE(fs::exists(dirPath));
}

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

TEST_F(DirectoryTest, MkdirsNestedWithTrailingSlash)
{
    const auto dirPath = tmpDir_ / "trailing" / "nested";
    const Directory d(dirPath);
    EXPECT_TRUE(d.mkdirs(true));
    EXPECT_TRUE(fs::exists(dirPath));
}

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
