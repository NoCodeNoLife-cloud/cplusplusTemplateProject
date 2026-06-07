/**
 * @file BmpImage.hpp
 * @brief BmpImage class declaration
 * @details This header defines the BmpImage class that provides functionality for
 *          creating, loading, manipulating, and saving 24-bit BMP images.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace common::filesystem
{
#pragma pack(push, 1)
    /// @brief BMP file header structure
    struct BitMapFileHeader
    {
        uint16_t bf_type_;
        uint32_t bf_size_;
        uint16_t bf_reserved1_;
        uint16_t bf_reserved2_;
        uint32_t bf_off_bits_;
    };

    /// @brief BMP info header structure
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

    /// @brief A class for creating and manipulating 24-bit BMP images
    /// @details This class allows creating BMP images from scratch or loading from files,
    ///          setting and getting pixel colors, and saving to files.
    class BmpImage
    {
    public:
        /// @brief Constructs a BmpImage with specified dimensions
        /// @param width The width of the image in pixels
        /// @param height The height of the image in pixels
        explicit BmpImage(int32_t width, int32_t height);

        /// @brief Constructs a BmpImage by loading from a file
        explicit BmpImage(const std::string& filename);

        /// @brief Sets the color of a pixel at the specified coordinates
        void setPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b);

        /// @brief Gets the color of a pixel at the specified coordinates
        /// @return true if the coordinates are valid, false otherwise
        [[nodiscard]] bool getPixel(int32_t x, int32_t y, uint8_t& r, uint8_t& g, uint8_t& b) const;

        /// @brief Saves the BMP image to a file
        [[nodiscard]] bool save(const std::string& filename) const;

        /// @brief Gets the width of the image
        [[nodiscard]] int32_t getWidth() const;

        /// @brief Gets the height of the image
        [[nodiscard]] int32_t getHeight() const;

    private:
        int32_t width_{};
        int32_t height_{};
        std::vector<uint8_t> pixels_{};

        /// @brief Loads a BMP image from a file
        void load(const std::string& filename);
    };
}
