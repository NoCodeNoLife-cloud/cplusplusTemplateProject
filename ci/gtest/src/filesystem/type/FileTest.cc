/**
 * @file FileTest.cc
 * @brief Unit tests for the File class
 * @details Tests cover construction, existence checks, file operations (create, delete, rename, copy),
 *          permissions, file properties (size, time, space), path operations, and static utilities.
 */

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "filesystem/type/File.hpp"

using namespace common::filesystem::type;
namespace fs = std::filesystem;

class FileTest : public testing::Test
{
protected:
    fs::path tmpDir_;

    void SetUp() override
    {
        tmpDir_ = fs::temp_directory_path() / "FileTest";
        fs::remove_all(tmpDir_);
        fs::create_directories(tmpDir_);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(tmpDir_, ec);
    }

    fs::path createFile(const std::string& name, const std::string& content = {}) const
    {
        const auto path = tmpDir_ / name;
        std::ofstream ofs(path.string());
        ofs << content;
        ofs.close();
        return path;
    }
};

TEST_F(FileTest, ConstructFromString)
{
    const auto p = createFile("str.txt");
    const File f(p.string());
    EXPECT_EQ(f.getPath(), p.string());
}

TEST_F(FileTest, ConstructFromPath)
{
    const auto p = createFile("path.txt");
    const File f(p);
    EXPECT_EQ(f.getPath(), p.string());
}

TEST_F(FileTest, ConstructFromCString)
{
    const auto p = createFile("cstr.txt");
    const File f(p.string().c_str());
    EXPECT_EQ(f.getPath(), p.string());
}

TEST_F(FileTest, Exists)
{
    const auto p = createFile("exists.txt");
    const File f(p);
    EXPECT_TRUE(f.exists());
}

TEST_F(FileTest, ExistsNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_FALSE(f.exists());
}

TEST_F(FileTest, IsFile)
{
    const auto p = createFile("regular.txt");
    const File f(p);
    EXPECT_TRUE(f.isFile());
}

TEST_F(FileTest, IsFileOnDirectory)
{
    const File f(tmpDir_);
    EXPECT_FALSE(f.isFile());
}

TEST_F(FileTest, IsAbsolute)
{
    const File f(tmpDir_ / "abs.txt");
    EXPECT_TRUE(f.isAbsolute());
}

TEST_F(FileTest, IsAbsoluteRelative)
{
    const File f("relative.txt");
    EXPECT_FALSE(f.isAbsolute());
}

TEST_F(FileTest, IsHidden)
{
    const auto p = createFile(".hidden");
    const File f(p);
    EXPECT_TRUE(f.isHidden());
}

TEST_F(FileTest, IsHiddenRegular)
{
    const auto p = createFile("visible.txt");
    const File f(p);
    EXPECT_FALSE(f.isHidden());
}

TEST_F(FileTest, CanRead)
{
    const auto p = createFile("readable.txt");
    const File f(p);
    EXPECT_TRUE(f.canRead());
}

TEST_F(FileTest, CanReadNonExistent)
{
    const File f(tmpDir_ / "nope.txt");
    EXPECT_FALSE(f.canRead());
}

TEST_F(FileTest, CanWrite)
{
    const auto p = createFile("writable.txt");
    const File f(p);
    EXPECT_TRUE(f.canWrite());
}

TEST_F(FileTest, CanWriteNonExistent)
{
    const File f(tmpDir_ / "nowrite.txt");
    EXPECT_FALSE(f.canWrite());
}

TEST_F(FileTest, CreateNewFile)
{
    const auto p = tmpDir_ / "brand_new.txt";
    const File f(p);
    EXPECT_TRUE(f.createNewFile());
    EXPECT_TRUE(fs::exists(p));
}

TEST_F(FileTest, CreateNewFileAlreadyExists)
{
    const auto p = createFile("existing.txt");
    const File f(p);
    EXPECT_FALSE(f.createNewFile());
}

TEST_F(FileTest, DeleteFile)
{
    const auto p = createFile("todelete.txt");
    const File f(p);
    EXPECT_TRUE(f.deleteFile());
    EXPECT_FALSE(fs::exists(p));
}

TEST_F(FileTest, DeleteFileNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_FALSE(f.deleteFile());
}

TEST_F(FileTest, RenameTo)
{
    const auto src = createFile("rename_src.txt");
    const auto dst = tmpDir_ / "rename_dst.txt";
    const File f(src);
    const File dest(dst);
    EXPECT_TRUE(f.renameTo(dest));
    EXPECT_FALSE(fs::exists(src));
    EXPECT_TRUE(fs::exists(dst));
}

TEST_F(FileTest, CopyTo)
{
    const auto src = createFile("copy_src.txt", "hello");
    const auto dst = tmpDir_ / "copy_dst.txt";
    const File f(src);
    const File dest(dst);
    EXPECT_TRUE(f.copyTo(dest));
    EXPECT_TRUE(fs::exists(src));
    EXPECT_TRUE(fs::exists(dst));
}

TEST_F(FileTest, Length)
{
    const auto p = createFile("length_test.txt", "1234567890");
    const File f(p);
    EXPECT_EQ(f.length(), 10);
}

TEST_F(FileTest, LengthNonExistent)
{
    const File f(tmpDir_ / "nope.txt");
    EXPECT_EQ(f.length(), 0);
}

TEST_F(FileTest, LengthEmpty)
{
    const auto p = createFile("empty.txt");
    const File f(p);
    EXPECT_EQ(f.length(), 0);
}

TEST_F(FileTest, LastModified)
{
    const auto p = createFile("time_test.txt");
    const File f(p);
    EXPECT_GT(f.lastModified(), 0);
}

TEST_F(FileTest, LastModifiedNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_EQ(f.lastModified(), 0);
}

TEST_F(FileTest, SetLastModified)
{
    const auto p = createFile("settime.txt");
    const File f(p);
    EXPECT_TRUE(f.setLastModified(1000000000));
    const auto modified = f.lastModified();
    EXPECT_GE(modified, 999999998);
    EXPECT_LE(modified, 1000000002);
}

TEST_F(FileTest, GetAbsolutePath)
{
    const auto p = createFile("abs_path.txt");
    const File f("abs_path.txt"); // relative path
    const auto absPath = f.getAbsolutePath();
    EXPECT_FALSE(absPath.empty());
    EXPECT_NE(absPath, "abs_path.txt");
    EXPECT_TRUE(File(absPath).isAbsolute());
}

TEST_F(FileTest, GetAbsoluteFile)
{
    const auto p = createFile("abs_file.txt");
    const File f(p);
    const auto absFile = f.getAbsoluteFile();
    EXPECT_TRUE(absFile.isAbsolute());
}

TEST_F(FileTest, GetName)
{
    const auto p = createFile("myfile.txt");
    const File f(p);
    EXPECT_EQ(f.getName(), "myfile.txt");
}

TEST_F(FileTest, GetExtension)
{
    const auto p = createFile("ext.txt");
    const File f(p);
    EXPECT_EQ(f.getExtension(), ".txt");
}

TEST_F(FileTest, GetExtensionNoExtension)
{
    const auto p = createFile("noext");
    const File f(p);
    EXPECT_TRUE(f.getExtension().empty());
}

TEST_F(FileTest, GetParent)
{
    const File f(tmpDir_ / "child.txt");
    EXPECT_EQ(f.getParent(), tmpDir_.string());
}

TEST_F(FileTest, GetParentFile)
{
    const File f(tmpDir_ / "child.txt");
    const auto parent = f.getParentFile();
    EXPECT_EQ(parent.getPath(), tmpDir_.string());
}

TEST_F(FileTest, GetPath)
{
    const auto p = createFile("path_test.txt");
    const File f(p);
    EXPECT_EQ(f.getPath(), p.string());
}

TEST_F(FileTest, GetSizeString)
{
    const auto p = createFile("size_str.txt", std::string(1500, 'x'));
    const File f(p);
    const auto sizeStr = f.getSizeString();
    EXPECT_FALSE(sizeStr.empty());
    EXPECT_NE(sizeStr, "Unknown");
}

TEST_F(FileTest, GetTotalSpace)
{
    const auto p = createFile("space.txt");
    const File f(p);
    EXPECT_GT(f.getTotalSpace(), 0);
}

TEST_F(FileTest, GetUsableSpace)
{
    const auto p = createFile("uspace.txt");
    const File f(p);
    EXPECT_GT(f.getUsableSpace(), 0);
}

TEST_F(FileTest, HashCode)
{
    const File a(tmpDir_ / "hash.txt");
    const File b(tmpDir_ / "hash.txt");
    EXPECT_EQ(a.hashCode(), b.hashCode());
}

TEST_F(FileTest, ToURI)
{
    const auto p = createFile("uri_test.txt");
    const File f(p);
    const auto uri = f.toURI();
    EXPECT_TRUE(uri.starts_with("file://"));
    EXPECT_TRUE(uri.find(p.filename().string()) != std::string::npos);
}

TEST_F(FileTest, GetFileMD5)
{
    const auto p = createFile("md5_test.txt", "hello");
    const auto md5 = File::getFileMD5(p);
    EXPECT_EQ(md5.size(), 32);
    EXPECT_EQ(md5, "5d41402abc4b2a76b9719d911017c592");
}

TEST_F(FileTest, GetFileMD5Empty)
{
    const auto p = createFile("empty_md5.txt");
    const auto md5 = File::getFileMD5(p);
    EXPECT_EQ(md5.size(), 32);
}

TEST_F(FileTest, GetFileMD5NonExistent)
{
    EXPECT_THROW(File::getFileMD5(tmpDir_ / "nope.txt"), std::runtime_error);
}

TEST_F(FileTest, SetReadOnly)
{
    const auto p = createFile("readonly.txt", "data");
    const File f(p);
    EXPECT_TRUE(f.setReadOnly());
}

TEST_F(FileTest, PrintFilesWithDepthThrowsOnFile)
{
    const auto p = createFile("not_a_dir.txt");
    EXPECT_THROW(File::printFilesWithDepth(p), std::runtime_error);
}

TEST_F(FileTest, PrintFilesWithDepthThrowsOnNonExistent)
{
    EXPECT_THROW(File::printFilesWithDepth(tmpDir_ / "ghost"), std::runtime_error);
}
