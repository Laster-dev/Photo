#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "winrt/Windows.UI.Xaml.Controls.h"
#include "winrt/Windows.UI.Xaml.h"
#include <winrt/Windows.UI.Xaml.Media.Animation.h>
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Input.h>

#include <winrt/Windows.UI.Input.h> // 确保包括了这个头文件
#include <winrt/Windows.Foundation.h>
#include <fstream>
#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <ImageFile.h>


using namespace winrt;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace winrt::Windows::Foundation;

namespace winrt::MileXamlBlankApp::implementation
{
    void MainPage::InitializeComponent()
    {
        MainPageT::InitializeComponent();
        
    }

    int32_t MainPage::MyProperty()
    {
        throw hresult_not_implemented();
    }

    void MainPage::MyProperty(int32_t /* value */)
    {
        throw hresult_not_implemented();
    }

    void MainPage::ClickHandler(IInspectable const&, RoutedEventArgs const&)
    {
        //Button().Content(box_value(L"Clicked"));
    }
}
bool fileExists(LPWSTR filename) {
    DWORD attributes = GetFileAttributesW(filename);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

///窗口加载
void winrt::MileXamlBlankApp::implementation::MainPage::Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    LPWSTR* argv;
    int argc;
    LPWSTR Filepath = NULL;
    LPWSTR commandLine = GetCommandLineW();
    argv = CommandLineToArgvW(commandLine, &argc);
    if (nullptr != argv && argc > 1) {
        if (fileExists(argv[1])) {
            Filepath = argv[1];
        }
        LocalFree(argv);
    }
    if (Filepath != NULL) {
        winrt::hstring path(Filepath);
        winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bitmapImage;
        bitmapImage.UriSource(winrt::Windows::Foundation::Uri(path));

        MainImage().Source(bitmapImage);
    }
}
///获取Image控件URL
std::wstring GetMainImageSourceUri(const winrt::Windows::UI::Xaml::Controls::Image& mainImage) {
    auto imageSource = mainImage.Source();
    auto bitmapImage = imageSource.as<winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage>();

    if (bitmapImage) {
        auto uriSource = bitmapImage.UriSource();
        return uriSource.ToString().c_str();
    }

    return std::wstring();
}

///显示文件信息
winrt::fire_and_forget ShowImageInfoDialog(const ImageInfo& imgInfo, const UIElement& owner) {
    std::wstring message = L"文件名: " + imgInfo.GetFileName() +
        L"\n文件大小: " + std::to_wstring(imgInfo.GetFileSize()) + L" bytes" +
        L"\n宽度: " + std::to_wstring(imgInfo.GetWidth()) +
        L", 高度: " + std::to_wstring(imgInfo.GetHeight());


    TextBlock textBlock;
    textBlock.Text(message);
    textBlock.TextWrapping(TextWrapping::Wrap);


    ContentDialog contentDialog;
    contentDialog.Title(box_value(L"图片信息"));
    contentDialog.Content(textBlock);
    contentDialog.CloseButtonText(L"关闭");


    if (auto control = owner.try_as<Controls::Control>()) {
        contentDialog.XamlRoot(control.XamlRoot());
    }


    co_await contentDialog.ShowAsync();
}

///文件信息
void winrt::MileXamlBlankApp::implementation::MainPage::InfoButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{

    std::wstring filename = GetMainImageSourceUri(MainImage());

    // 检查是否以"file:///"开头
    if (filename.substr(0, 8) == L"file:///") {
        // 移除"file:///"，并替换所有的"/"为"\\"
        filename = filename.substr(8);
        std::replace(filename.begin(), filename.end(), L'/', L'\\');
    }

    ImageInfo imgInfo(filename);
    auto control = this->try_as<Windows::UI::Xaml::Controls::Control>();

    ShowImageInfoDialog(imgInfo, *this); // 传递this指针作为显示的窗口
}

///打开文件
void winrt::MileXamlBlankApp::implementation::MainPage::OpenButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    OPENFILENAMEW ofn; // 使用宽字符版本的结构体
    WCHAR szFile[260]; // 使用WCHAR类型定义文件路径数组

    // 初始化OPENFILENAMEW结构体
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL; // 如果你有窗口句柄，请替换NULL
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"所有图片文件\0*.bmp;*.jpg;*.jpeg;*.png;*.tif;*.gif;*.pcx;*.tga;*.exif;*.fpx;*.svg;*.psd;*.cdr;*.pcd;*.dxf;*.ufo;*.eps;*.ai;*.raw;*.wmf;*.webp;*.avif;*.apng\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        if (szFile != NULL) {
            winrt::hstring path(szFile);
            winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage bitmapImage;
            bitmapImage.UriSource(winrt::Windows::Foundation::Uri(path));

            auto imageControl = this->FindName(L"MainImage").as<winrt::Windows::UI::Xaml::Controls::Image>();

            auto currentTransform = imageControl.RenderTransform().as<winrt::Windows::UI::Xaml::Media::RotateTransform>();

            double currentAngle = 0;
            if (currentTransform) {
                currentAngle = currentTransform.Angle();
            }

            double newAngle = 0;

            auto rotateTransform = winrt::Windows::UI::Xaml::Media::RotateTransform();
            rotateTransform.Angle(newAngle);

            rotateTransform.CenterX(imageControl.ActualWidth() / 2);
            rotateTransform.CenterY(imageControl.ActualHeight() / 2);

            imageControl.RenderTransform(rotateTransform);
            MainImage().Source(bitmapImage);
        }
    }
}

///删除文件
void winrt::MileXamlBlankApp::implementation::MainPage::DelButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    // 创建 ContentDialog
    ContentDialog contentDialog;
    contentDialog.Title(box_value(L"提示"));
    contentDialog.CloseButtonText(L"取消");
    contentDialog.PrimaryButtonText(L"确认删除");
    // 添加 TextBlock 到 Content
    TextBlock textBlock;
    textBlock.Text(L"是否删除文件？");
    textBlock.TextWrapping(TextWrapping::Wrap);
    contentDialog.Content(textBlock);
    contentDialog.XamlRoot(this->XamlRoot());
    // 显示 ContentDialog 并等待结果
    auto asyncOperation = contentDialog.ShowAsync();

    // 处理对话框关闭后的操作
    asyncOperation.Completed([=](const IAsyncOperation<ContentDialogResult>& asyncInfo, AsyncStatus asyncStatus) {
        ContentDialogResult result = asyncInfo.GetResults();
        if (result == ContentDialogResult::Primary) {
            std::wstring filename = GetMainImageSourceUri(MainImage());
            // 检查是否以"file:///"开头
            if (filename.substr(0, 8) == L"file:///") {
                // 移除"file:///"，并替换所有的"/"为"\\"
                filename = filename.substr(8);
                std::replace(filename.begin(), filename.end(), L'/', L'\\');
            }
            // 将wstring转换为LPCWSTR
            LPCWSTR lpFileName = filename.c_str();
            // 尝试删除文件
            MainImage().Source(nullptr); // 先取消显示
            if (DeleteFileW(lpFileName)) {
                std::wcout << L"文件删除成功：" << filename << std::endl;
            }
            else {
                DWORD error = GetLastError();
                std::wstring errorMessage;
                if (error == ERROR_FILE_NOT_FOUND) {
                    errorMessage = L"文件不存在：" + filename;
                }
                else {
                    errorMessage = L"文件删除失败：" + filename + L", 错误码：" + std::to_wstring(error);
                }
                ContentDialog errorDialog;
                errorDialog.Title(box_value(L"错误"));
                errorDialog.Content(box_value(errorMessage));
                errorDialog.CloseButtonText(L"关闭");
                errorDialog.XamlRoot(this->XamlRoot());
                errorDialog.ShowAsync();

            }
        }
        });
}



void winrt::MileXamlBlankApp::implementation::MainPage::RotateButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    auto imageControl = this->FindName(L"MainImage").as<winrt::Windows::UI::Xaml::Controls::Image>();

    auto currentTransform = imageControl.RenderTransform().as<winrt::Windows::UI::Xaml::Media::RotateTransform>();

    double currentAngle = 0;
    if (currentTransform) {
        currentAngle = currentTransform.Angle();
    }

    double newAngle = std::fmod(currentAngle + 90, 360); // 确保角度值在0到359之间

    auto rotateTransform = winrt::Windows::UI::Xaml::Media::RotateTransform();
    rotateTransform.Angle(newAngle);

    // 设置旋转中心为图片中心
    rotateTransform.CenterX(imageControl.ActualWidth() / 2);
    rotateTransform.CenterY(imageControl.ActualHeight() / 2);

    imageControl.RenderTransform(rotateTransform);

}







void winrt::MileXamlBlankApp::implementation::MainPage::ZoomIn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    // 获取ScaleTransform对象并设置放大倍数
    //winrt::Windows::UI::Xaml::Media::CompositeTransform transformnew;
    //auto transform = ImageTransform().as<winrt::Windows::UI::Xaml::Media::CompositeTransform>();
    //transformnew.ScaleX(transform.ScaleX() * 1.1);
    //transformnew.ScaleY(transform.ScaleY() * 1.1);
    // 将ScaleTransform应用到图像控件
    //MainImage().RenderTransform(transform);
}
