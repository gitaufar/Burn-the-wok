#include "engine/graphics/Image.h"
#include "engine/core/AssetPath.h"

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#else
#include <png.h>
#endif
#include <SDL.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace engine {
namespace {

#ifdef _WIN32
class ComApartment {
public:
    ComApartment() : m_result(CoInitializeEx(nullptr, COINIT_MULTITHREADED)) {
        if (FAILED(m_result) && m_result != RPC_E_CHANGED_MODE) {
            throw std::runtime_error("Image::LoadFromFile: COM initialization failed");
        }
    }

    ~ComApartment() {
        if (SUCCEEDED(m_result)) {
            CoUninitialize();
        }
    }

private:
    HRESULT m_result;
};

void CheckResult(HRESULT result, const char* message) {
    if (FAILED(result)) {
        throw std::runtime_error(message);
    }
}

#else
// Free decoder state even if pixel allocation or decoding throws.
struct PngImage {
    png_image value{};
    PngImage() { value.version = PNG_IMAGE_VERSION; }
    ~PngImage() { png_image_free(&value); }
    PngImage(const PngImage&) = delete;
    PngImage& operator=(const PngImage&) = delete;
};
#endif

} // namespace

Image::Image(int width, int height, Color fill)
    : m_width(std::max(width, 0)), m_height(std::max(height, 0)) {
    m_pixels.resize(static_cast<std::size_t>(m_width) * m_height * 4);
    FillRect(0, 0, m_width, m_height, fill);
}

Image Image::LoadFromFile(const std::filesystem::path& path) {
    const auto resolvedPath = ResolveAssetPath(path);

#ifdef _WIN32
    ComApartment comApartment;
    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    CheckResult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(factory.GetAddressOf())),
                "Image::LoadFromFile: cannot create WIC factory");

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    CheckResult(factory->CreateDecoderFromFilename(resolvedPath.c_str(), nullptr, GENERIC_READ,
                                                   WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf()),
                "Image::LoadFromFile: cannot open image");

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    CheckResult(decoder->GetFrame(0, frame.GetAddressOf()),
                "Image::LoadFromFile: cannot read image frame");

    UINT width = 0;
    UINT height = 0;
    CheckResult(frame->GetSize(&width, &height),
                "Image::LoadFromFile: cannot read image dimensions");

    const std::uint64_t stride = static_cast<std::uint64_t>(width) * 4;
    const std::uint64_t bufferSize = stride * height;
    if (width == 0 || height == 0 || width > static_cast<UINT>(std::numeric_limits<int>::max()) ||
        height > static_cast<UINT>(std::numeric_limits<int>::max()) ||
        stride > std::numeric_limits<UINT>::max() || bufferSize > std::numeric_limits<UINT>::max()) {
        throw std::runtime_error("Image::LoadFromFile: image dimensions are unsupported");
    }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    CheckResult(factory->CreateFormatConverter(converter.GetAddressOf()),
                "Image::LoadFromFile: cannot create pixel converter");
    CheckResult(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA,
                                      WICBitmapDitherTypeNone, nullptr, 0.0,
                                      WICBitmapPaletteTypeCustom),
                "Image::LoadFromFile: cannot convert image pixels");

    Image image(static_cast<int>(width), static_cast<int>(height));
    CheckResult(converter->CopyPixels(nullptr, static_cast<UINT>(stride),
                                      static_cast<UINT>(bufferSize), image.m_pixels.data()),
                "Image::LoadFromFile: cannot copy image pixels");
    return image;
#else
    PngImage png;
    const std::string filename = resolvedPath.string();
    if (!png_image_begin_read_from_file(&png.value, filename.c_str())) {
        throw std::runtime_error("Image::LoadFromFile: " + filename + ": " + png.value.message);
    }
    const std::uint64_t bufferSize = static_cast<std::uint64_t>(png.value.width) * png.value.height * 4;
    if (png.value.width == 0 || png.value.height == 0 ||
        png.value.width > static_cast<png_uint_32>(std::numeric_limits<int>::max() / 4) ||
        png.value.height > static_cast<png_uint_32>(std::numeric_limits<int>::max()) ||
        bufferSize > std::numeric_limits<std::size_t>::max()) {
        throw std::runtime_error("Image::LoadFromFile: image dimensions are unsupported");
    }
    png.value.format = PNG_FORMAT_RGBA;
    Image image(static_cast<int>(png.value.width), static_cast<int>(png.value.height));
    if (!png_image_finish_read(&png.value, nullptr, image.m_pixels.data(), 0, nullptr)) {
        throw std::runtime_error("Image::LoadFromFile: " + filename + ": " + png.value.message);
    }
    return image;
#endif
}

Rect Image::GetContentBounds(std::uint8_t minAlpha) const {
    int left = m_width;
    int top = m_height;
    int right = 0;
    int bottom = 0;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (GetPixel(x, y).a >= minAlpha) {
                left = std::min(left, x);
                top = std::min(top, y);
                right = std::max(right, x + 1);
                bottom = std::max(bottom, y + 1);
            }
        }
    }
    if (right <= left || bottom <= top) {
        return {0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height)};
    }
    return {static_cast<float>(left), static_cast<float>(top),
            static_cast<float>(right - left), static_cast<float>(bottom - top)};
}

void Image::SetPixel(int x, int y, Color color) {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return;
    }
    const std::size_t i = (static_cast<std::size_t>(y) * m_width + x) * 4;
    m_pixels[i + 0] = color.r;
    m_pixels[i + 1] = color.g;
    m_pixels[i + 2] = color.b;
    m_pixels[i + 3] = color.a;
}

Color Image::GetPixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= m_width || y >= m_height) {
        return Color::Transparent();
    }
    const std::size_t i = (static_cast<std::size_t>(y) * m_width + x) * 4;
    return {m_pixels[i + 0], m_pixels[i + 1], m_pixels[i + 2], m_pixels[i + 3]};
}

void Image::FillRect(int x, int y, int width, int height, Color color) {
    const int x0 = std::max(x, 0);
    const int y0 = std::max(y, 0);
    const int x1 = std::min(x + width, m_width);
    const int y1 = std::min(y + height, m_height);
    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            SetPixel(px, py, color);
        }
    }
}

} // namespace engine
