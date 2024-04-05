
#include <windows.h>
#include <gdiplus.h>
#include <iostream>

// 使用Gdiplus命名空间
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

class ImageInfo {
public:
    ImageInfo(const std::wstring& filePath) : filePath_(filePath), width_(0), height_(0) {
        GdiplusStartupInput gdiplusStartupInput;
        GdiplusStartup(&gdiplusToken_, &gdiplusStartupInput, NULL);

        // 可以选择在构造函数中调用GetDimensions，这样对象创建后立即加载尺寸
        UINT tempWidth, tempHeight;
        if (GetDimensions(tempWidth, tempHeight)) {
            width_ = tempWidth;
            height_ = tempHeight;
        }
    }

    ~ImageInfo() {
        GdiplusShutdown(gdiplusToken_);
    }

    std::wstring GetFileName() const {
        return filePath_;
    }

    ULONGLONG GetFileSize() const {
        WIN32_FILE_ATTRIBUTE_DATA fileInformation;
        if (GetFileAttributesEx(filePath_.c_str(), GetFileExInfoStandard, &fileInformation)) {
            LARGE_INTEGER size;
            size.HighPart = fileInformation.nFileSizeHigh;
            size.LowPart = fileInformation.nFileSizeLow;
            return size.QuadPart;
        }
        return 0;
    }

    bool GetDimensions(UINT& width, UINT& height) const {
        if (width_ > 0 && height_ > 0) {
            width = width_;
            height = height_;
            return true;
        }

        Gdiplus::Image image(filePath_.c_str());
        if (image.GetLastStatus() == Ok) {
            width_ = image.GetWidth();
            height_ = image.GetHeight();
            width = width_;
            height = height_;
            return true;
        }
        return false;
    }

    UINT GetWidth() const {
        return width_;
    }

    UINT GetHeight() const {
        return height_;
    }

private:
    std::wstring filePath_;
    mutable UINT width_, height_; // 添加mutable关键字允许在const成员函数中修改这些变量
    ULONG_PTR gdiplusToken_;
};
