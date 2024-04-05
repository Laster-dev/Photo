#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"
#include "winrt/Windows.UI.Xaml.Controls.h"
#include "winrt/Windows.UI.Xaml.h"
#include <winrt/Windows.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.Foundation.h>
#include <fstream>
#include <iostream>

#include <Windows.h>



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
int i = 0;

void winrt::MileXamlBlankApp::implementation::MainPage::Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    auto button = this->FindName(L"Buttonnn").as<Windows::UI::Xaml::Controls::Button>();
    if (button)
    {
        button.Content(box_value(L"..0..00.."));
    }
}


void winrt::MileXamlBlankApp::implementation::MainPage::inputButton_Click_1(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    try {
        Controls::ContentDialog messageDialog = Controls::ContentDialog();
        messageDialog.Title(box_value(L"0"));
        messageDialog.Content(box_value(L"1。"));
        messageDialog.CloseButtonText(L"0");
        messageDialog.XamlRoot(this->XamlRoot());
        auto result = messageDialog.ShowAsync();
    }
    catch(EXCEPINFO e){
        
    }
    
}

bool fileExists(LPWSTR filename) {
    DWORD attributes = GetFileAttributesW(filename);
    return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}


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
std::wstring GetMainImageSourceUri(const winrt::Windows::UI::Xaml::Controls::Image& mainImage) {
    auto imageSource = mainImage.Source();
    auto bitmapImage = imageSource.as<winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage>();

    if (bitmapImage) {
        auto uriSource = bitmapImage.UriSource();
        return uriSource.ToString().c_str();
    }

    return std::wstring();
}

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
