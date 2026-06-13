/**
 * @file BmpImage.hpp
 * @brief BMP image file reader — header parsing and pixel data access
 * @description Reads BMP (Bitmap) image files, parsing the file header,
 *          DIB header, and pixel data.  Supports 24-bit and 32-bit BMP
 *          formats.  Provides pixel access by coordinate and basic image
 *          metadata (width, height, bit depth).
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace common::filesystem::type
{
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
