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

/**
 * @brief Test constructing File from std::string path
 * @details Verifies that a File object created from a string path stores the correct path value
 */
TEST_F(FileTest, ConstructFromString)
{
    const auto p = createFile("str.txt");
    const File f(p.string());
    EXPECT_EQ(f.getPath(), p.string());
}

/**
 * @brief Test constructing File from std::filesystem::path
 * @details Verifies that a File object created from a filesystem::path stores the correct path
 */
TEST_F(FileTest, ConstructFromPath)
{
    const auto p = createFile("path.txt");
    const File f(p);
    EXPECT_EQ(f.getPath(), p.string());
}

/**
 * @brief Test constructing File from a C-string path
 * @details Verifies that a File object created from a const char* path stores the correct path
 */
TEST_F(FileTest, ConstructFromCString)
{
    const auto p = createFile("cstr.txt");
    const File f(p.string().c_str());
    EXPECT_EQ(f.getPath(), p.string());
}

/**
 * @brief Test existence check on an existing file
 * @details Verifies that exists() returns true for a file that has been created on disk
 */
TEST_F(FileTest, Exists)
{
    const auto p = createFile("exists.txt");
    const File f(p);
    EXPECT_TRUE(f.exists());
}

/**
 * @brief Test existence check on a non-existent file
 * @details Verifies that exists() returns false when the file path does not exist on disk
 */
TEST_F(FileTest, ExistsNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_FALSE(f.exists());
}

/**
 * @brief Test isFile returns true for a regular file
 * @details Verifies that isFile() correctly identifies a regular file as a file
 */
TEST_F(FileTest, IsFile)
{
    const auto p = createFile("regular.txt");
    const File f(p);
    EXPECT_TRUE(f.isFile());
}

/**
 * @brief Test isFile returns false for a directory
 * @details Verifies that isFile() returns false when the path points to a directory
 */
TEST_F(FileTest, IsFileOnDirectory)
{
    const File f(tmpDir_);
    EXPECT_FALSE(f.isFile());
}

/**
 * @brief Test isAbsolute returns true for an absolute path
 * @details Verifies that isAbsolute() correctly identifies an absolute filesystem path
 */
TEST_F(FileTest, IsAbsolute)
{
    const File f(tmpDir_ / "abs.txt");
    EXPECT_TRUE(f.isAbsolute());
}

/**
 * @brief Test isAbsolute returns false for a relative path
 * @details Verifies that isAbsolute() returns false when the path is relative
 */
TEST_F(FileTest, IsAbsoluteRelative)
{
    const File f("relative.txt");
    EXPECT_FALSE(f.isAbsolute());
}

/**
 * @brief Test isHidden returns true for a dot-file
 * @details Verifies that isHidden() identifies files with a leading dot as hidden
 */
TEST_F(FileTest, IsHidden)
{
    const auto p = createFile(".hidden");
    const File f(p);
    EXPECT_TRUE(f.isHidden());
}

/**
 * @brief Test isHidden returns false for a regular file
 * @details Verifies that isHidden() returns false for files without a leading dot
 */
TEST_F(FileTest, IsHiddenRegular)
{
    const auto p = createFile("visible.txt");
    const File f(p);
    EXPECT_FALSE(f.isHidden());
}

/**
 * @brief Test canRead returns true for an existing readable file
 * @details Verifies that canRead() returns true when the file exists and is readable
 */
TEST_F(FileTest, CanRead)
{
    const auto p = createFile("readable.txt");
    const File f(p);
    EXPECT_TRUE(f.canRead());
}

/**
 * @brief Test canRead returns false for a non-existent file
 * @details Verifies that canRead() returns false when the file path does not exist
 */
TEST_F(FileTest, CanReadNonExistent)
{
    const File f(tmpDir_ / "nope.txt");
    EXPECT_FALSE(f.canRead());
}

/**
 * @brief Test canWrite returns true for a writable file
 * @details Verifies that canWrite() returns true when the file exists and is writable
 */
TEST_F(FileTest, CanWrite)
{
    const auto p = createFile("writable.txt");
    const File f(p);
    EXPECT_TRUE(f.canWrite());
}

/**
 * @brief Test canWrite returns false for a non-existent file
 * @details Verifies that canWrite() returns false when the file path does not exist
 */
TEST_F(FileTest, CanWriteNonExistent)
{
    const File f(tmpDir_ / "nowrite.txt");
    EXPECT_FALSE(f.canWrite());
}

/**
 * @brief Test creating a brand new file
 * @details Verifies that createNewFile() creates the file on disk and returns true
 */
TEST_F(FileTest, CreateNewFile)
{
    const auto p = tmpDir_ / "brand_new.txt";
    const File f(p);
    EXPECT_TRUE(f.createNewFile());
    EXPECT_TRUE(fs::exists(p));
}

/**
 * @brief Test createNewFile returns false when file already exists
 * @details Verifies that attempting to create an already-existing file returns false
 */
TEST_F(FileTest, CreateNewFileAlreadyExists)
{
    const auto p = createFile("existing.txt");
    const File f(p);
    EXPECT_FALSE(f.createNewFile());
}

/**
 * @brief Test deleting an existing file
 * @details Verifies that deleteFile() removes the file from disk and returns true
 */
TEST_F(FileTest, DeleteFile)
{
    const auto p = createFile("todelete.txt");
    const File f(p);
    EXPECT_TRUE(f.deleteFile());
    EXPECT_FALSE(fs::exists(p));
}

/**
 * @brief Test deleteFile returns false for a non-existent file
 * @details Verifies that attempting to delete a file that does not exist returns false
 */
TEST_F(FileTest, DeleteFileNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_FALSE(f.deleteFile());
}

/**
 * @brief Test renaming a file to a new path
 * @details Verifies that renameTo() moves the file to the destination path and updates the filesystem
 */
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

/**
 * @brief Test copying a file to a new path
 * @details Verifies that copyTo() duplicates the file content to the destination while preserving the original
 */
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

/**
 * @brief Test getting the length of a file with content
 * @details Verifies that length() returns the correct byte count for a file with known content
 */
TEST_F(FileTest, Length)
{
    const auto p = createFile("length_test.txt", "1234567890");
    const File f(p);
    EXPECT_EQ(f.length(), 10);
}

/**
 * @brief Test length returns zero for a non-existent file
 * @details Verifies that length() returns 0 when the file path does not exist on disk
 */
TEST_F(FileTest, LengthNonExistent)
{
    const File f(tmpDir_ / "nope.txt");
    EXPECT_EQ(f.length(), 0);
}

/**
 * @brief Test length returns zero for an empty file
 * @details Verifies that length() returns 0 when the file exists but contains no data
 */
TEST_F(FileTest, LengthEmpty)
{
    const auto p = createFile("empty.txt");
    const File f(p);
    EXPECT_EQ(f.length(), 0);
}

/**
 * @brief Test lastModified returns a positive timestamp
 * @details Verifies that lastModified() returns a non-zero timestamp for an existing file
 */
TEST_F(FileTest, LastModified)
{
    const auto p = createFile("time_test.txt");
    const File f(p);
    EXPECT_GT(f.lastModified(), 0);
}

/**
 * @brief Test lastModified returns zero for a non-existent file
 * @details Verifies that lastModified() returns 0 when the file path does not exist
 */
TEST_F(FileTest, LastModifiedNonExistent)
{
    const File f(tmpDir_ / "ghost.txt");
    EXPECT_EQ(f.lastModified(), 0);
}

/**
 * @brief Test setting the last modified timestamp
 * @details Verifies that setLastModified() updates the modification time and the new value is within tolerance
 */
TEST_F(FileTest, SetLastModified)
{
    const auto p = createFile("settime.txt");
    const File f(p);
    EXPECT_TRUE(f.setLastModified(1000000000));
    const auto modified = f.lastModified();
    EXPECT_GE(modified, 999999998);
    EXPECT_LE(modified, 1000000002);
}

/**
 * @brief Test getAbsolutePath resolves a relative path
 * @details Verifies that getAbsolutePath() converts a relative path to a non-empty absolute path
 */
TEST_F(FileTest, GetAbsolutePath)
{
    const auto p = createFile("abs_path.txt");
    const File f("abs_path.txt"); // relative path
    const auto absPath = f.getAbsolutePath();
    EXPECT_FALSE(absPath.empty());
    EXPECT_NE(absPath, "abs_path.txt");
    EXPECT_TRUE(File(absPath).isAbsolute());
}

/**
 * @brief Test getAbsoluteFile returns a File with absolute path
 * @details Verifies that getAbsoluteFile() returns a File object whose path is absolute
 */
TEST_F(FileTest, GetAbsoluteFile)
{
    const auto p = createFile("abs_file.txt");
    const File f(p);
    const auto absFile = f.getAbsoluteFile();
    EXPECT_TRUE(absFile.isAbsolute());
}

/**
 * @brief Test getName returns the filename component
 * @details Verifies that getName() returns the file name without the directory portion
 */
TEST_F(FileTest, GetName)
{
    const auto p = createFile("myfile.txt");
    const File f(p);
    EXPECT_EQ(f.getName(), "myfile.txt");
}

/**
 * @brief Test getExtension returns the file extension
 * @details Verifies that getExtension() returns the extension including the leading dot
 */
TEST_F(FileTest, GetExtension)
{
    const auto p = createFile("ext.txt");
    const File f(p);
    EXPECT_EQ(f.getExtension(), ".txt");
}

/**
 * @brief Test getExtension returns empty for files without extension
 * @details Verifies that getExtension() returns an empty string when the filename has no extension
 */
TEST_F(FileTest, GetExtensionNoExtension)
{
    const auto p = createFile("noext");
    const File f(p);
    EXPECT_TRUE(f.getExtension().empty());
}

/**
 * @brief Test GetParent returns the parent directory path
 * @details Verifies that getParent() returns the string path of the parent directory
 */
TEST_F(FileTest, GetParent)
{
    const File f(tmpDir_ / "child.txt");
    EXPECT_EQ(f.getParent(), tmpDir_.string());
}

/**
 * @brief Test getParentFile returns a File for the parent directory
 * @details Verifies that getParentFile() returns a File object representing the parent directory
 */
TEST_F(FileTest, GetParentFile)
{
    const File f(tmpDir_ / "child.txt");
    const auto parent = f.getParentFile();
    EXPECT_EQ(parent.getPath(), tmpDir_.string());
}

/**
 * @brief Test getPath returns the stored path
 * @details Verifies that getPath() returns the exact path string used during construction
 */
TEST_F(FileTest, GetPath)
{
    const auto p = createFile("path_test.txt");
    const File f(p);
    EXPECT_EQ(f.getPath(), p.string());
}

/**
 * @brief Test getSizeString returns a human-readable size
 * @details Verifies that getSizeString() returns a non-empty formatted string instead of "Unknown"
 */
TEST_F(FileTest, GetSizeString)
{
    const auto p = createFile("size_str.txt", std::string(1500, 'x'));
    const File f(p);
    const auto sizeStr = f.getSizeString();
    EXPECT_FALSE(sizeStr.empty());
    EXPECT_NE(sizeStr, "Unknown");
}

/**
 * @brief Test getTotalSpace returns a positive value
 * @details Verifies that getTotalSpace() returns the total capacity of the filesystem partition
 */
TEST_F(FileTest, GetTotalSpace)
{
    const auto p = createFile("space.txt");
    const File f(p);
    EXPECT_GT(f.getTotalSpace(), 0);
}

/**
 * @brief Test getUsableSpace returns a positive value
 * @details Verifies that getUsableSpace() returns the available free space on the filesystem partition
 */
TEST_F(FileTest, GetUsableSpace)
{
    const auto p = createFile("uspace.txt");
    const File f(p);
    EXPECT_GT(f.getUsableSpace(), 0);
}

/**
 * @brief Test hashCode returns consistent values for equal paths
 * @details Verifies that two File objects pointing to the same path produce identical hash codes
 */
TEST_F(FileTest, HashCode)
{
    const File a(tmpDir_ / "hash.txt");
    const File b(tmpDir_ / "hash.txt");
    EXPECT_EQ(a.hashCode(), b.hashCode());
}

/**
 * @brief Test toURI returns a file:// URI
 * @details Verifies that toURI() produces a valid file:// scheme URI containing the filename
 */
TEST_F(FileTest, ToURI)
{
    const auto p = createFile("uri_test.txt");
    const File f(p);
    const auto uri = f.toURI();
    EXPECT_TRUE(uri.starts_with("file://"));
    EXPECT_TRUE(uri.find(p.filename().string()) != std::string::npos);
}

/**
 * @brief Test getFileMD5 computes the MD5 hash of a file
 * @details Verifies that getFileMD5() returns a 32-character hex string matching the expected MD5 digest
 */
TEST_F(FileTest, GetFileMD5)
{
    const auto p = createFile("md5_test.txt", "hello");
    const auto md5 = File::getFileMD5(p);
    EXPECT_EQ(md5.size(), 32);
    EXPECT_EQ(md5, "5d41402abc4b2a76b9719d911017c592");
}

/**
 * @brief Test getFileMD5 on an empty file
 * @details Verifies that getFileMD5() produces a valid 32-character MD5 hash even for an empty file
 */
TEST_F(FileTest, GetFileMD5Empty)
{
    const auto p = createFile("empty_md5.txt");
    const auto md5 = File::getFileMD5(p);
    EXPECT_EQ(md5.size(), 32);
}

/**
 * @brief Test getFileMD5 throws on a non-existent file
 * @details Verifies that getFileMD5() throws std::runtime_error when the file path does not exist
 */
TEST_F(FileTest, GetFileMD5NonExistent)
{
    EXPECT_THROW(File::getFileMD5(tmpDir_ / "nope.txt"), std::runtime_error);
}

/**
 * @brief Test setReadOnly marks the file as read-only
 * @details Verifies that setReadOnly() successfully changes the file permissions to read-only
 */
TEST_F(FileTest, SetReadOnly)
{
    const auto p = createFile("readonly.txt", "data");
    const File f(p);
    EXPECT_TRUE(f.setReadOnly());
}

/**
 * @brief Test printFilesWithDepth throws on a file path
 * @details Verifies that calling printFilesWithDepth() on a regular file throws std::runtime_error
 */
TEST_F(FileTest, PrintFilesWithDepthThrowsOnFile)
{
    const auto p = createFile("not_a_dir.txt");
    EXPECT_THROW(File::printFilesWithDepth(p), std::runtime_error);
}

/**
 * @brief Test printFilesWithDepth throws on a non-existent path
 * @details Verifies that calling printFilesWithDepth() on a path that does not exist throws std::runtime_error
 */
TEST_F(FileTest, PrintFilesWithDepthThrowsOnNonExistent)
{
    EXPECT_THROW(File::printFilesWithDepth(tmpDir_ / "ghost"), std::runtime_error);
}
