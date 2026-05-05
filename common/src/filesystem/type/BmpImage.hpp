#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace common::filesystem {
#pragma pack(push, 1)
/// @brief BMP file header structure
/// @details This structure represents the BMP file header which contains
///          information about the BMP file format and layout
struct BitMapFileHeader {
    /// @brief BMP file type, should be 'BM'
    uint16_t bf_type_;
    /// @brief Size of the BMP file in bytes
    uint32_t bf_size_;
    /// @brief Reserved field (must be zero)
    uint16_t bf_reserved1_;
    /// @brief Reserved field (must be zero)
    uint16_t bf_reserved2_;
    /// @brief Offset to the start of image data
    uint32_t bf_off_bits_;
};

/// @brief BMP info header structure
/// @details This structure contains information about the dimensions and color format of a BMP image
struct BitmapInfoHeader {
    /// @brief Size of this header (40 bytes)
    uint32_t bi_size_;
    /// @brief Width of the bitmap in pixels
    int32_t bi_width_;
    /// @brief Height of the bitmap in pixels
    int32_t bi_height_;
    /// @brief Number of color planes (must be 1)
    uint16_t bi_planes_;
    /// @brief Number of bits per pixel (1, 4, 8, 16, 24, or 32)
    uint16_t bi_bit_count_;
    /// @brief Compression method used (0 = none, 1 = RLE-8, 2 = RLE-4)
    uint32_t bi_compression_;
    /// @brief Size of the image data in bytes (maybe 0 for uncompressed images)
    uint32_t bi_size_image_;
    /// @brief Horizontal resolution in pixels per meter
    int32_t bi_x_pels_per_meter_;
    /// @brief Vertical resolution in pixels per meter
    int32_t bi_y_pels_per_meter_;
    /// @brief Number of colors in the color palette (0 = default)
    uint32_t bi_clr_used_;
    /// @brief Number of important colors (0 = all are important)
    uint32_t bi_clr_important_;
};
#pragma pack(pop)

/// @brief A class for creating and manipulating BMP images
/// @details This class allows creating BMP images, setting pixel colors, and saving to files
class BmpImage {
public:
    /// @brief Constructs a BmpImage with specified dimensions
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @throws std::invalid_argument if width or height is non-positive
    explicit BmpImage(int32_t width, int32_t height);

    /// @brief Constructs a BmpImage by loading from a file
    /// @param filename The name of the file to load the image from
    /// @throws std::runtime_error if the file cannot be loaded or is not a valid BMP
    explicit BmpImage(const std::string& filename);

    /// @brief Sets the color of a pixel at the specified coordinates
    /// @param x The x-coordinate of the pixel
    /// @param y The y-coordinate of the pixel
    /// @param r The red component of the color (0-255)
    /// @param g The green component of the color (0-255)
    /// @param b The blue component of the color (0-255)
    auto setPixel(int32_t x, int32_t y, uint8_t r, uint8_t g, uint8_t b) noexcept -> void;

    /// @brief Gets the color of a pixel at the specified coordinates
    /// @param x The x-coordinate of the pixel
    /// @param y The y-coordinate of the pixel
    /// @param r Reference to store the red component of the color (0-255)
    /// @param g Reference to store the green component of the color (0-255)
    /// @param b Reference to store the blue component of the color (0-255)
    /// @return true if the coordinates are valid, false otherwise
    [[nodiscard]] auto getPixel(int32_t x, int32_t y, uint8_t& r, uint8_t& g, uint8_t& b) const noexcept -> bool;

    /// @brief Saves the BMP image to a file
    /// @param filename The name of the file to save the image to
    /// @throws std::runtime_error if the file cannot be created
    auto save(const std::string& filename) const -> void;

    /// @brief Gets the width of the image
    /// @return The width of the image in pixels
    [[nodiscard]] auto getWidth() const noexcept -> int32_t;

    /// @brief Gets the height of the image
    /// @return The height of the image in pixels
    [[nodiscard]] auto getHeight() const noexcept -> int32_t;

private:
    int32_t width_{};
    int32_t height_{};
    std::vector<uint8_t> pixels_{};

    /// @brief Loads a BMP image from a file
    /// @param filename The name of the file to load the image from
    /// @throws std::runtime_error if the file cannot be loaded or is not a valid BMP
    auto load(const std::string& filename) -> void;
};
}