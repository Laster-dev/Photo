#include "pch.h"

#include "ImageFile.h"
#include "MainPage.h"
#include "MainPage.g.cpp"





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


///在Image显示内存中的图片
winrt::Windows::Foundation::IAsyncAction winrt::MileXamlBlankApp::implementation::MainPage::DisplayImageFromMemory(std::vector<uint8_t> const& imageData, winrt::Windows::UI::Xaml::Controls::Image imageControl) {
    try {
        auto stream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
        auto writer = winrt::Windows::Storage::Streams::DataWriter(stream.GetOutputStreamAt(0));

        writer.WriteBytes(imageData);
        co_await writer.StoreAsync();
        writer.Close();
        stream.Seek(0);

        auto bitmapImage = winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage();


        bitmapImage.ImageOpened([imageControl, &bitmapImage, this](auto&&, winrt::Windows::UI::Xaml::RoutedEventArgs const&) {
            // 在图片加载完毕后设置 Image 控件的宽度和高度
            auto dispatcher = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();
            dispatcher.TryEnqueue([imageControl, &bitmapImage, this]() {
                // 假设 this->ImageGrid 是包含 Image 控件的 Grid
                auto gridWidth = this->ImageGrid().ActualWidth();
                auto gridHeight = this->ImageGrid().ActualHeight();

                HelpTextBlock().Visibility(Windows::UI::Xaml::Visibility::Collapsed);//隐藏提示

                // 计算应当设置的宽度和高度，以适应 Grid 的大小
                double setImageWidth, setImageHeight;
                if (bitmapImage.PixelWidth() > gridWidth || bitmapImage.PixelHeight() > gridHeight) {
                    // 图片尺寸大于 Grid，调整图片大小以适应 Grid
                    setImageWidth = gridWidth;
                    setImageHeight = gridHeight;
                }
                else {
                    // 图片尺寸小于或等于 Grid，使用原始图片尺寸
                    setImageWidth = bitmapImage.PixelWidth();
                    setImageHeight = bitmapImage.PixelHeight();
                }

                // 设置 Image 控件的宽度和高度
                imageControl.Width(setImageWidth);
                imageControl.Height(setImageHeight);
                });
            });


        co_await bitmapImage.SetSourceAsync(stream);

        // 获取当前线程的 DispatcherQueue，以确保UI操作在UI线程上执行
        auto dispatcher = winrt::Windows::System::DispatcherQueue::GetForCurrentThread();

        bool isEnqueued = dispatcher.TryEnqueue([imageControl, bitmapImage]() {
            imageControl.Source(bitmapImage);
            });
    }
    catch (const winrt::hresult_error& e) {
        //MessageBoxA(0, "Failed to display image from memory.", 0, 0);
    }
}

///cv::Mat到BitmapImage
winrt::Windows::Foundation::IAsyncOperation<winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage> MatToBitmapImage(cv::Mat const& image) {
    auto stream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();

    // 转换cv::Mat到BGRA格式（适合Windows显示）
    cv::Mat buffer;
    cv::cvtColor(image, buffer, cv::COLOR_BGR2BGRA);

    // 写入图像数据到流
    std::vector<uchar> data(buffer.ptr(), buffer.ptr() + buffer.total() * buffer.elemSize());
    auto writer = winrt::Windows::Storage::Streams::DataWriter(stream.GetOutputStreamAt(0));
    writer.WriteBytes(data);
    co_await writer.StoreAsync();
    co_await writer.FlushAsync();
    writer.Close();

    // 使用流创建BitmapImage
    auto bitmapImage = winrt::Windows::UI::Xaml::Media::Imaging::BitmapImage();
    co_await bitmapImage.SetSourceAsync(stream);
    co_return bitmapImage;
}
///窗口加载
winrt::Windows::Foundation::IAsyncAction winrt::MileXamlBlankApp::implementation::MainPage::Page_Loaded(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    LPWSTR* argv;
    int argc;
    LPWSTR Filepath = NULL;
    LPWSTR commandLine = GetCommandLineW();
    argv = CommandLineToArgvW(commandLine, &argc);
    if (nullptr != argv && argc > 1) {
        if (fs::exists(argv[1])) { // 使用 std::filesystem::exists 来检查文件是否存在
            Filepath = argv[1];
        }
        LocalFree(argv);
    }

    if (Filepath != NULL) {
        // 转换 LPWSTR 到 std::filesystem::path
        fs::path filepath(Filepath);

        // 首先检查路径是否存在
        if (!fs::exists(filepath)) {
            MessageBoxA(0, "The path does not exist.", 0, 0);
            //return; // 提前返回，因为没有要处理的文件
        }

        // 检查给定路径是文件夹还是文件
        if (fs::is_directory(filepath)) {
            MessageBoxA(0, "The path is a directory, not a file.", 0, 0);
           // return; // 提前返回，因为给定路径是一个目录
        }
        else if (!fs::is_regular_file(filepath)) {
            MessageBoxA(0, "The path is not a regular file.", 0, 0);
            //return; // 提前返回，因为给定路径不是一个正常的文件
        }

        // 使用 std::ifstream 打开文件
        std::ifstream file(filepath, std::ios::binary);

        if (!file.is_open()) {
            MessageBoxA(0, "Failed to open file.", 0, 0);
            // 转换 std::filesystem::path 到 std::wstring，然后再到 LPWSTR，以便使用 MessageBoxW
            std::wstring filepath_wstr = filepath.wstring();
            MessageBoxW(0, filepath_wstr.c_str(), 0, 0);
        }
        else {
            // 文件成功打开，读取文件到 std::vector<uint8_t>
            myImageData = std::vector<uint8_t>((std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>());
            file.close();
            co_await DisplayImageFromMemory(myImageData, MainImage());
        }
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

///文件大小计算
std::wstring FormatFileSize(size_t bytes) {
    const size_t conversion = 1024;
    std::wstringstream ss;
    if (bytes < conversion) {
        ss << bytes << L" B";
    }
    else if (bytes < conversion * conversion) {
        ss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / conversion << L" KB";
    }
    else if (bytes < conversion * conversion * conversion) {
        ss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / (conversion * conversion) << L" MB";
    }
    else {
        ss << std::fixed << std::setprecision(2) << static_cast<double>(bytes) / (conversion * conversion * conversion) << L" GB";
    }
    return ss.str();
}
///获取图片信息
std::wstring GetImageInfo(const std::vector<uint8_t>& imageData) {
    // 将图片数据从 std::vector 转换为 cv::Mat
    cv::Mat image = cv::imdecode(cv::Mat(imageData), cv::IMREAD_UNCHANGED);

    // 获取图片信息
    std::wstring info = L"图片信息：\n";
    info += L"宽度: " + std::to_wstring(image.cols) + L" 像素\n";
    info += L"高度: " + std::to_wstring(image.rows) + L" 像素\n";
    info += L"通道数: " + std::to_wstring(image.channels()) + L"\n";

    // 深度信息
    std::wstring depthInfo;
    switch (image.depth()) {
    case CV_8U: depthInfo = L"8 位无符号"; break;
    case CV_8S: depthInfo = L"8 位有符号"; break;
    case CV_16U: depthInfo = L"16 位无符号"; break;
    case CV_16S: depthInfo = L"16 位有符号"; break;
    case CV_32S: depthInfo = L"32 位整型"; break;
    case CV_32F: depthInfo = L"32 位浮点"; break;
    case CV_64F: depthInfo = L"64 位浮点"; break;
    default: depthInfo = L"未知"; break;
    }
    info += L"深度: " + depthInfo + L"\n";

    // 图片类型
    std::wstring typeInfo;
    if (image.type() == CV_8UC1) typeInfo = L"灰度图";
    else if (image.type() == CV_8UC3) typeInfo = L"彩色图";
    else typeInfo = L"其他类型";
    info += L"类型: " + typeInfo + L"\n";

    // 文件大小
    info += L"文件大小: " + FormatFileSize(imageData.size()) + L"\n";

    return info;
}
///文件信息
void winrt::MileXamlBlankApp::implementation::MainPage::InfoButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{

    auto imageInfo = GetImageInfo(myImageData);
    // 创建 TextBlock 显示图片信息
    TextBlock textBlock{};
    textBlock.Text(imageInfo);
    textBlock.TextWrapping(TextWrapping::Wrap);

    // 创建 ContentDialog 并设置内容
    ContentDialog dialog{};
    dialog.Content(textBlock);
    dialog.CloseButtonText(L"Close");
    dialog.XamlRoot(this->XamlRoot());
    dialog.ShowAsync();
}

///打开文件
winrt::Windows::Foundation::IAsyncAction winrt::MileXamlBlankApp::implementation::MainPage::OpenButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    OPENFILENAMEW ofn; // 使用宽字符版本的结构体
    WCHAR szFile[260]; // 使用WCHAR类型定义文件路径数组

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = L'\0';
    ofn.nMaxFile = sizeof(szFile) / sizeof(WCHAR);
    ofn.lpstrFilter = L"所有图片文件\0*.bmp;*.jpg;*.jpeg;*.ico;*.icon;*.png;*.tif;*.gif;*.pcx;*.tga;*.exif;*.fpx;*.svg;*.psd;*.cdr;*.pcd;*.dxf;*.ufo;*.eps;*.ai;*.raw;*.wmf;*.webp;*.avif;*.apng\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn) == TRUE) {
        if (szFile != NULL) {
            std::ifstream file(szFile, std::ios::binary);
            // 检查文件是否成功打开
            if (!file.is_open()) {
                MessageBoxA(0, 0, "Failed to open file.", 0);
                co_return;
            }
            myImageData = std::vector<uint8_t>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            file.close();
            // 异步显示图像，避免使用.get()
            ImageTranslateTransform().X(0);
            ImageTranslateTransform().Y(0);
            ImageScaleTransform().ScaleX(1);
            ImageScaleTransform().ScaleY(1);
            co_await DisplayImageFromMemory(myImageData, MainImage());
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

///旋转图片
std::vector<uint8_t> RotateEncodedImage(const std::vector<uint8_t>& encodedImage, int angle) {
    // 步骤 1: 从内存中解码图像数据
    cv::Mat image = cv::imdecode(cv::Mat(encodedImage), cv::IMREAD_COLOR);

    // 步骤 2: 旋转图像
    cv::Mat rotatedImage;
    switch (angle % 360) {
    case 90:
        cv::rotate(image, rotatedImage, cv::ROTATE_90_CLOCKWISE);
        break;
    case 180:
        cv::rotate(image, rotatedImage, cv::ROTATE_180);
        break;
    case 270:
        cv::rotate(image, rotatedImage, cv::ROTATE_90_COUNTERCLOCKWISE);
        break;
    default:
        // 如果提供了非90、180、270度的角度，直接返回原图像数据
        return encodedImage;
    }

    // 步骤 3: 将旋转后的图像重新编码
    std::vector<uint8_t> rotatedEncodedImage;
    cv::imencode(".jpg", rotatedImage, rotatedEncodedImage);

    return rotatedEncodedImage;
}
///旋转按钮
winrt::Windows::Foundation::IAsyncAction winrt::MileXamlBlankApp::implementation::MainPage::RotateButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    myImageData = RotateEncodedImage(myImageData, 90);
    co_await DisplayImageFromMemory(myImageData, MainImage());
}

///放大
void winrt::MileXamlBlankApp::implementation::MainPage::ZoomIn_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    auto scaleFactor = 1.1;
    // 更新缩放级别
    auto newScaleX = ImageScaleTransform().ScaleX() * scaleFactor;
    auto newScaleY = ImageScaleTransform().ScaleY() * scaleFactor;

    // 限制缩放级别，防止无限放大或缩小到看不见
    newScaleX = std::max(0.1, std::min(newScaleX, 10.0));
    newScaleY = std::max(0.1, std::min(newScaleY, 10.0));

    ImageScaleTransform().ScaleX(newScaleX);
    ImageScaleTransform().ScaleY(newScaleY);

}
void winrt::MileXamlBlankApp::implementation::MainPage::ZoomOut_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{
    auto scaleFactor =0.9;
    // 更新缩放级别
    auto newScaleX = ImageScaleTransform().ScaleX() * scaleFactor;
    auto newScaleY = ImageScaleTransform().ScaleY() * scaleFactor;

    // 限制缩放级别，防止无限放大或缩小到看不见
    newScaleX = std::max(0.1, std::min(newScaleX, 10.0));
    newScaleY = std::max(0.1, std::min(newScaleY, 10.0));

    ImageScaleTransform().ScaleX(newScaleX);
    ImageScaleTransform().ScaleY(newScaleY);
}
///窗口大小改变
winrt::Windows::Foundation::IAsyncAction winrt::MileXamlBlankApp::implementation::MainPage::ImageGrid_SizeChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::SizeChangedEventArgs const& e)
{
    co_await DisplayImageFromMemory(myImageData, MainImage());
}

///滚轮缩放
void winrt::MileXamlBlankApp::implementation::MainPage::MainImage_PointerWheelChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
{
    auto delta = e.GetCurrentPoint(nullptr).Properties().MouseWheelDelta();
    double scaleFactor = delta > 0 ? 1.2 : 0.8; // 向上滚动放大，向下滚动缩小

    // 更新缩放级别
    auto newScaleX = ImageScaleTransform().ScaleX() * scaleFactor;
    auto newScaleY = ImageScaleTransform().ScaleY() * scaleFactor;

    // 限制缩放级别，防止无限放大或缩小到看不见
    newScaleX = std::max(0.1, std::min(newScaleX, 10.0));
    newScaleY = std::max(0.1, std::min(newScaleY, 10.0));

    ImageScaleTransform().ScaleX(newScaleX);
    ImageScaleTransform().ScaleY(newScaleY);

    e.Handled(true);
}

POINT cursorPosold;
POINT cursorPosonew;
void winrt::MileXamlBlankApp::implementation::MainPage::ImageGrid_PointerMoved(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::Input::PointerRoutedEventArgs const& e)
{
    auto pointer = e.Pointer();
    if (pointer.PointerDeviceType() == Windows::Devices::Input::PointerDeviceType::Mouse)
    {
        auto properties = e.GetCurrentPoint(nullptr).Properties();
        if (properties.IsLeftButtonPressed())
        {
            auto currentPoint = e.GetCurrentPoint(sender.as<winrt::Windows::UI::Xaml::UIElement>()).Position();

            // 首次移动时，记录初始位置
            if (cursorPosold.x == 0 && cursorPosold.y == 0)
            {
                GetCursorPos(&cursorPosold);//初始化记录点
                return; // 直到下一次移动事件再开始处理
            }
            GetCursorPos(&cursorPosonew);//获取鼠标位置
            // 计算自上次以来的移动距离
            int Xmove = cursorPosonew.x - cursorPosold.x;
            int Ymove = cursorPosonew.y - cursorPosold.y;

            double IX = ImageTranslateTransform().X();
            double IY = ImageTranslateTransform().Y();
            // 更新图像的位置
            ImageTranslateTransform().X(IX + Xmove);
            ImageTranslateTransform().Y(IY+Ymove);
            GetCursorPos(&cursorPosold);//更新记录点
        }
        else
        {

            cursorPosold.x = 0;
            cursorPosold.y = 0;
        }
    }
}



