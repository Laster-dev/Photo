﻿#include <Windows.h>

#include "pch.h"

#include "App.h"
#include "MainPage.h"

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nShowCmd)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    winrt::init_apartment(winrt::apartment_type::single_threaded);

    winrt::com_ptr<winrt::MileXamlControlsDemo::implementation::App> app =
        winrt::make_self<winrt::MileXamlControlsDemo::implementation::App>();

    winrt::MileXamlControlsDemo::MainPage XamlWindowContent =
        winrt::make<winrt::MileXamlControlsDemo::implementation::MainPage>();

    HWND WindowHandle = ::CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"Mile.Xaml.ContentWindow",
        L"MileXamlControlsDemo",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        0,
        CW_USEDEFAULT,
        0,
        nullptr,
        nullptr,
        hInstance,
        winrt::get_abi(XamlWindowContent));
    if (!WindowHandle)
    {
        return -1;
    }

    ::ShowWindow(WindowHandle, nShowCmd);
    ::UpdateWindow(WindowHandle);

    MSG Message;
    while (::GetMessageW(&Message, nullptr, 0, 0))
    {
        // Workaround for capturing Alt+F4 in applications with XAML Islands.
        // Reference: https://github.com/microsoft/microsoft-ui-xaml/issues/2408
        if (Message.message == WM_SYSKEYDOWN && Message.wParam == VK_F4)
        {
            ::SendMessageW(
                ::GetAncestor(Message.hwnd, GA_ROOT),
                Message.message,
                Message.wParam,
                Message.lParam);

            continue;
        }

        ::TranslateMessage(&Message);
        ::DispatchMessageW(&Message);
    }

    app->Close();

    return static_cast<int>(Message.wParam);
}
