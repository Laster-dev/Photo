#include <Windows.h>

#include "pch.h"

#include "App.h"
#include "MainPage.h"

// 程序的主入口点
int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,         // 当前实例句柄
    _In_opt_ HINSTANCE hPrevInstance, // 上一个实例句柄（在现代Windows应用中不再使用）
    _In_ LPWSTR lpCmdLine,            // 命令行参数
    _In_ int nShowCmd)                // 窗口显示状态
{
    UNREFERENCED_PARAMETER(hPrevInstance); // 标记为未使用，避免编译器警告
    UNREFERENCED_PARAMETER(lpCmdLine);     // 标记为未使用，避免编译器警告

    // 初始化 COM 库为单线程
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    // 创建 App 类的实例
    winrt::com_ptr<winrt::MileXamlBlankApp::implementation::App> app =
        winrt::make_self<winrt::MileXamlBlankApp::implementation::App>();

    // 创建 MainPage 类的实例，用于展示 XAML 内容
    winrt::MileXamlBlankApp::MainPage XamlWindowContent =
        winrt::make<winrt::MileXamlBlankApp::implementation::MainPage>();

    // 创建一个窗口，使用扩展样式，指定类名、窗口标题和其他参数
    HWND WindowHandle = ::CreateWindowExW(
        WS_EX_CLIENTEDGE,                     // 扩展窗口样式，为窗口边缘添加阴影
        L"Mile.Xaml.ContentWindow",           // 窗口类名称
        L"Photo",                             // 窗口标题
        WS_OVERLAPPEDWINDOW,                  // 窗口样式，支持最小化、最大化和关闭按钮
        CW_USEDEFAULT,                        // 窗口的初始x位置（系统选择）
        0,                                    // 窗口的初始y位置
        1200,                                  // 窗口宽度
        800,                                  // 窗口高度
        nullptr,                              // 父窗口句柄
        nullptr,                              // 菜单句柄
        hInstance,                            // 应用程序实例句柄
        winrt::get_abi(XamlWindowContent));   // 传递给窗口的额外数据

    // 检查窗口是否成功创建
    if (!WindowHandle)
    {
        return -1; // 创建失败，退出程序
    }

    // 显示窗口并更新其客户区
    ::ShowWindow(WindowHandle, nShowCmd);
    ::UpdateWindow(WindowHandle);

    // 消息循环，处理窗口消息
    MSG Message;
    while (::GetMessageW(&Message, nullptr, 0, 0))
    {
        // 特殊处理：捕获Alt+F4，确保应用程序可以正确响应
        if (Message.message == WM_SYSKEYDOWN && Message.wParam == VK_F4)
        {
            // 将消息发送到顶级窗口，以标准方式处理Alt+F4
            ::SendMessageW(
                ::GetAncestor(Message.hwnd, GA_ROOT),
                Message.message,
                Message.wParam,
                Message.lParam);

            continue;
        }

        // 翻译并分发消息给窗口过程
        ::TranslateMessage(&Message);
        ::DispatchMessageW(&Message);
    }

    // 在消息循环结束后清理
    app->Close();

    // 返回消息循环的退出值
    return static_cast<int>(Message.wParam);
}
