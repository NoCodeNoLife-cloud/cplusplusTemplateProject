#include "src/filesystem/type/BmpImage.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <fstream>
#include <stdexcept>
#include <vector>

namespace common::filesystem {
    BmpImage::BmpImage(const int32_t width, const int32_t height) : width_(width), height_(height) {
        if (width <= 0 || height <= 0) {
            DLOG(ERROR) << fmt::format("BmpImage initialization failed - invalid dimensions: width={}, height={}", width, height);
            throw std::invalid_argument("Invalid image dimensions");
        }
        pixels_.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 3, 0);
        DLOG(INFO) << fmt::format("BmpImage created - width={}, height={}, total_pixels={}", width, height, width * height);
    }

    BmpImage::BmpImage(const std::string &filename) {
        load(filename);
    }

    auto BmpImage::setPixel(const int32_t x, const int32_t y, const uint8_t r, const uint8_t g, const uint8_t b) noexcept -> void {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            return;
        }
        const int32_t invertedY = height_ - 1 - y;
        const size_t index = static_cast<size_t>(invertedY * width_ + x) * 3;
        pixels_[index] = b;
        pixels_[index + 1] = g;
        pixels_[index + 2] = r;
    }

    auto BmpImage::getPixel(const int32_t x, const int32_t y, uint8_t &r, uint8_t &g, uint8_t &b) const noexcept -> bool {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) {
            return false;
        }
        const int32_t invertedY = height_ - 1 - y;
        const size_t index = static_cast<size_t>(invertedY * width_ + x) * 3;
        b = pixels_[index];
        g = pixels_[index + 1];
        r = pixels_[index + 2];
        return true;
    }

    auto BmpImage::save(const std::string &filename) const -> void {
        DLOG(INFO) << fmt::format("BmpImage save - saving to file: {}", filename);
        const int32_t rowSize = width_ * 3 + 3 & ~3;
        const int32_t pixelDataSize = rowSize * height_;
        const uint64_t fileSize = sizeof(BitMapFileHeader) + sizeof(BitmapInfoHeader) + static_cast<uint64_t>(pixelDataSize);
        BitMapFileHeader fileHeader{};
        fileHeader.bf_type_ = 0x4D42;
        fileHeader.bf_size_ = fileSize;
        fileHeader.bf_off_bits_ = sizeof(BitMapFileHeader) + sizeof(BitmapInfoHeader);
        BitmapInfoHeader infoHeader{};
        infoHeader.bi_size_ = sizeof(BitmapInfoHeader);
        infoHeader.bi_width_ = width_;
        infoHeader.bi_height_ = height_;
        infoHeader.bi_planes_ = 1;
        infoHeader.bi_bit_count_ = 24;
        infoHeader.bi_compression_ = 0;
        infoHeader.bi_size_image_ = pixelDataSize;
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file) {
            DLOG(ERROR) << fmt::format("BmpImage save failed - cannot create file: {}", filename);
            throw std::runtime_error("can't create file: " + filename);
        }
        file.write(reinterpret_cast<const char *>(&fileHeader), sizeof(fileHeader));
        file.write(reinterpret_cast<const char *>(&infoHeader), sizeof(infoHeader));
        constexpr char padding[3] = {};
        for (int32_t y = 0; y < height_; ++y) {
            const auto rowStart = static_cast<size_t>(y * width_ * 3);
            file.write(reinterpret_cast<const char *>(&pixels_[rowStart]), width_ * 3);
            file.write(padding, rowSize - width_ * 3);
        }
        DLOG(INFO) << fmt::format("BmpImage saved successfully - file_size={} bytes", fileSize);
    }

    auto BmpImage::getWidth() const noexcept -> int32_t {
        return width_;
    }

    auto BmpImage::getHeight() const noexcept -> int32_t {
        return height_;
    }

    auto BmpImage::load(const std::string &filename) -> void {
        DLOG(INFO) << fmt::format("BmpImage load - loading from file: {}", filename);
        std::ifstream file(filename, std::ios::binary);
        if (!file) {
            DLOG(ERROR) << fmt::format("BmpImage load failed - cannot open file: {}", filename);
            throw std::runtime_error("Cannot open file: " + filename);
        }

        BitMapFileHeader fileHeader{};
        BitmapInfoHeader infoHeader{};

        file.read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader));
        if (file.gcount() != sizeof(fileHeader) || fileHeader.bf_type_ != 0x4D42) // Check 'BM' signature
        {
            DLOG(ERROR) << fmt::format("BmpImage load failed - invalid BMP file signature: {}", filename);
            throw std::runtime_error("Invalid BMP file: " + filename);
        }

        file.read(reinterpret_cast<char *>(&infoHeader), sizeof(infoHeader));
        if (file.gcount() != sizeof(infoHeader) || infoHeader.bi_bit_count_ != 24) // Only support 24-bit BMP
        {
            DLOG(ERROR) << fmt::format("BmpImage load failed - unsupported BMP format (bit_count={}): {}", infoHeader.bi_bit_count_, filename);
            throw std::runtime_error("Unsupported BMP format (only 24-bit BMP is supported): " + filename);
        }

        width_ = infoHeader.bi_width_;
        height_ = infoHeader.bi_height_;
        DLOG(INFO) << fmt::format("BmpImage loaded - width={}, height={}", width_, height_);

        // Resize pixel data
        pixels_.resize(static_cast<size_t>(width_) * static_cast<size_t>(height_) * 3);

        // Calculate row size (must be multiple of 4 bytes)
        const int32_t rowSize = width_ * 3 + 3 & ~3;
        const auto padding = static_cast<size_t>(rowSize - width_ * 3);

        // Seek to the beginning of pixel data
        file.seekg(fileHeader.bf_off_bits_);

        // Read pixel data row by row (BMP stores rows bottom-to-top)
        std::vector<char> rowBuffer(static_cast<size_t>(width_ * 3));
        for (int32_t y = height_ - 1; y >= 0; --y) {
            file.read(rowBuffer.data(), static_cast<std::streamsize>(rowBuffer.size()));
            if (file.gcount() != static_cast<std::streamsize>(rowBuffer.size())) {
                DLOG(ERROR) << fmt::format("BmpImage load failed - error reading pixel data at row {}: {}", y, filename);
                throw std::runtime_error("Error reading pixel data from file: " + filename);
            }

            const auto rowIndex = static_cast<size_t>(y * width_ * 3);

            // Copy pixels (BGR to RGB)
            for (int32_t x = 0; x < width_; ++x) {
                const auto srcIndex = static_cast<size_t>(x * 3);
                const size_t dstIndex = rowIndex + srcIndex;
                pixels_[dstIndex] = static_cast<uint8_t>(rowBuffer[srcIndex + 2]); // R
                pixels_[dstIndex + 1] = static_cast<uint8_t>(rowBuffer[srcIndex + 1]); // G
                pixels_[dstIndex + 2] = static_cast<uint8_t>(rowBuffer[srcIndex]); // B
            }

            // Skip padding bytes
            if (padding > 0) {
                file.ignore(static_cast<std::streamsize>(padding));
            }
        }
        DLOG(INFO) << fmt::format("BmpImage loaded successfully - total_pixels={}", width_ * height_);
    }
}
