#include "engine/graphics/Image.h"

#define NOMINMAX
#include <Windows.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <SDL/SDL.h>

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace engine {
namespace {

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

} // namespace

Image::Image(int width, int height, Color fill)
    : m_width(std::max(width, 0)), m_height(std::max(height, 0)) {
    m_pixels.resize(static_cast<std::size_t>(m_width) * m_height * 4);
    FillRect(0, 0, m_width, m_height, fill);
}

Image Image::LoadFromFile(const std::filesystem::path& path) {
    ComApartment comApartment;
    std::filesystem::path resolvedPath = path;
    if (path.is_relative() && !std::filesystem::exists(path)) {
        // Packaged games may be launched from a different working directory.
        if (char* basePath = SDL_GetBasePath()) {
            resolvedPath = std::filesystem::path(basePath) / path;
            SDL_free(basePath);
        }
    }

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
