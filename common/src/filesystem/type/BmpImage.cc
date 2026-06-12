/**
 * @file BmpImage.cc
 * @brief BmpImage class implementation
 * @details This file contains the implementation of the BmpImage class methods.
 *          BMP file format structures are kept here as internal implementation details.
 */

#include "filesystem/type/BmpImage.hpp"

#include <fstream>
#include <stdexcept>
#include <vector>
#include <fmt/format.h>

namespace common::filesystem::type
{
    namespace
    {
#pragma pack(push, 1)
        struct BitMapFileHeader
        {
            uint16_t bf_type_;
            uint32_t bf_size_;
            uint16_t bf_reserved1_;
            uint16_t bf_reserved2_;
            uint32_t bf_off_bits_;
        };

        struct BitmapInfoHeader
        {
            uint32_t bi_size_;
            int32_t bi_width_;
            int32_t bi_height_;
            uint16_t bi_planes_;
            uint16_t bi_bit_count_;
            uint32_t bi_compression_;
            uint32_t bi_size_image_;
            int32_t bi_x_pels_per_meter_;
            int32_t bi_y_pels_per_meter_;
            uint32_t bi_clr_used_;
            uint32_t bi_clr_important_;
        };
#pragma pack(pop)
    }


    BmpImage::BmpImage(const int32_t width, const int32_t height) : width_(width), height_(height)
    {
        if (width <= 0 || height <= 0)
        {
            throw std::invalid_argument("Invalid image dimensions");
        }
        pixels_.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 3, 0);
    }

    BmpImage::BmpImage(const std::string& filename)
    {
        load(filename);
    }

    void BmpImage::setPixel(const int32_t x, const int32_t y, const uint8_t r, const uint8_t g, const uint8_t b)
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
        {
            return;
        }
        const int32_t invertedY = height_ - 1 - y;
        const size_t index = static_cast<size_t>(invertedY) * static_cast<size_t>(width_) * 3 + static_cast<size_t>(x) * 3;
        pixels_[index] = b;
        pixels_[index + 1] = g;
        pixels_[index + 2] = r;
    }

    bool BmpImage::getPixel(const int32_t x, const int32_t y, uint8_t& r, uint8_t& g, uint8_t& b) const
    {
        if (x < 0 || x >= width_ || y < 0 || y >= height_)
        {
            return false;
        }
        const int32_t invertedY = height_ - 1 - y;
        const size_t index = static_cast<size_t>(invertedY) * static_cast<size_t>(width_) * 3 + static_cast<size_t>(x) * 3;
        b = pixels_[index];
        g = pixels_[index + 1];
        r = pixels_[index + 2];
        return true;
    }

    bool BmpImage::save(const std::string& filename) const
    {
        try
        {
            const int32_t rowSize = (width_ * 3 + 3) & ~3;
            const int32_t pixelDataSize = rowSize * height_;
            const uint64_t fileSize = sizeof(BitMapFileHeader) + sizeof(BitmapInfoHeader) +
                static_cast<uint64_t>(pixelDataSize);

            BitMapFileHeader fileHeader{};
            fileHeader.bf_type_ = 0x4D42;
            fileHeader.bf_size_ = static_cast<uint32_t>(fileSize);
            fileHeader.bf_off_bits_ = sizeof(BitMapFileHeader) + sizeof(BitmapInfoHeader);

            BitmapInfoHeader infoHeader{};
            infoHeader.bi_size_ = sizeof(BitmapInfoHeader);
            infoHeader.bi_width_ = width_;
            infoHeader.bi_height_ = height_;
            infoHeader.bi_planes_ = 1;
            infoHeader.bi_bit_count_ = 24;
            infoHeader.bi_size_image_ = pixelDataSize;

            std::ofstream file(filename, std::ios::binary | std::ios::trunc);
            if (!file)
            {
                return false;
            }

            file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
            file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

            constexpr char padding[3] = {};
            const auto rowStride = static_cast<size_t>(width_) * 3;
            for (int32_t y = height_ - 1; y >= 0; --y)
            {
                const auto rowStart = static_cast<size_t>(y) * rowStride;
                file.write(reinterpret_cast<const char*>(&pixels_[rowStart]),
                           static_cast<std::streamsize>(rowStride));
                file.write(padding, rowSize - static_cast<int32_t>(rowStride));
            }
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    int32_t BmpImage::getWidth() const
    {
        return width_;
    }

    int32_t BmpImage::getHeight() const
    {
        return height_;
    }

    void BmpImage::load(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        BitMapFileHeader fileHeader{};
        BitmapInfoHeader infoHeader{};

        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
        if (file.gcount() != sizeof(fileHeader) || fileHeader.bf_type_ != 0x4D42)
        {
            throw std::runtime_error("Invalid BMP file: " + filename);
        }

        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));
        if (file.gcount() != sizeof(infoHeader) || infoHeader.bi_bit_count_ != 24)
        {
            throw std::runtime_error("Unsupported BMP format (only 24-bit BMP is supported): " + filename);
        }

        width_ = infoHeader.bi_width_;
        height_ = infoHeader.bi_height_;

        if (width_ <= 0 || height_ <= 0)
        {
            throw std::runtime_error("Invalid BMP dimensions: " + filename);
        }

        const auto rowStride = static_cast<size_t>(width_) * 3;
        pixels_.resize(rowStride * static_cast<size_t>(height_));

        const int32_t rowSize = (width_ * 3 + 3) & ~3;
        const auto padding = static_cast<size_t>(rowSize - width_ * 3);

        file.seekg(fileHeader.bf_off_bits_);

        for (int32_t y = height_ - 1; y >= 0; --y)
        {
            auto* rowPtr = &pixels_[y * rowStride];
            file.read(reinterpret_cast<char*>(rowPtr), static_cast<std::streamsize>(rowStride));
            if (file.gcount() != static_cast<std::streamsize>(rowStride))
            {
                throw std::runtime_error("Error reading pixel data from file: " + filename);
            }

            if (padding > 0)
            {
                file.ignore(static_cast<std::streamsize>(padding));
            }
        }
    }
}
