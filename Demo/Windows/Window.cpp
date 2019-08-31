#include "PCH.h"
#include "../Window.h"

#include "../Core/Utils/Logger.h"
#include "../Core/Utils/Bitmap.h"

namespace {

const DWORD gWindowedExStyle = WS_EX_WINDOWEDGE;
const DWORD gWindowedStyle = WS_OVERLAPPEDWINDOW;

WPARAM MapLeftRightSpecialKey(WPARAM wParam, LPARAM lParam)
{
    WPARAM newKey;

    // 16-23 bits are scan code needed for left/right distinguishment
    UINT scanCode = (lParam & 0x00FF0000) >> 16;
    // 24th bit is an "extended" bit, set to true if right control/alt are pressed
    UINT extended = (lParam & 0x01000000);

    switch (wParam)
    {
    case VK_SHIFT:
        newKey = MapVirtualKey(scanCode, MAPVK_VSC_TO_VK_EX);
        if (newKey == 0)
            // MapVirtualKey failed to map scan code to vkey, fallback to old value
            newKey = wParam;
        break;
    case VK_CONTROL:
        newKey = extended ? VK_RCONTROL : VK_LCONTROL;
        break;
    case VK_MENU:
        newKey = extended ? VK_RMENU : VK_LMENU;
        break;
    default:
        return wParam;
    }

    return newKey;
}

bool UTF8ToUTF16(const std::string& in, std::wstring& out)
{
    // TODO: consider dynamic allocation
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    size_t inChars;
    HRESULT hr = ::StringCchLengthA(in.c_str(), INT_MAX - 1, &inChars);
    if (FAILED(hr))
        return false;

    ++inChars;

    int result = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in.c_str(),
                                       static_cast<int>(inChars), buffer, bufferSize);
    if (result == 0)
        return false;

    out = buffer;
    return true;
}

bool UTF16ToUTF8(const std::wstring& in, std::string& out)
{
    if (in.empty())
    {
        out.clear();
        return true;
    }

    int numChars = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), static_cast<int>(in.length()), nullptr, 0, 0, 0);
    if (numChars <= 0)
    {
        // conversion failed
        return false;
    }

    std::vector<char> buffer;
    buffer.resize(numChars);

    int wideChars = ::WideCharToMultiByte(CP_UTF8, 0, in.c_str(), static_cast<int>(in.length()), buffer.data(), numChars, 0, 0);
    if (wideChars <= 0)
    {
        // conversion failed
        return false;
    }

    buffer.push_back(0);
    out = buffer.data();
    return true;
}

} // namespace


Window::Window()
    : mHandle(0)
    , mInstance(0)
    , mClosed(true)
    , mWidth(200)
    , mHeight(200)
    , mLeft(10)
    , mTop(10)
    , mInvisible(false)
    , mTitle("Window")
    , mMouseWheelDelta(0)
{
    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < 256; i++)
        mKeys[i] = false;

    mMousePos[0] = mMousePos[1] = -1;
}

Window::~Window()
{
    Close();
    UnregisterClass(mWndClass, mInstance);
}

void Window::SetSize(Uint32 width, Uint32 height)
{
    mWidth = width;
    mHeight = height;

    if (!mClosed)
    {
        RECT windowRect;
        windowRect.left = (long)mLeft;
        windowRect.right = (long)(mWidth + mLeft);
        windowRect.top = (long)mTop;
        windowRect.bottom = (long)(mHeight + mTop);
        AdjustWindowRectEx(&windowRect, gWindowedStyle, FALSE, gWindowedExStyle);

        MoveWindow(mHandle, windowRect.left, windowRect.top, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, FALSE);
    }
}

void Window::SetTitle(const char* title)
{
    mTitle = title;
    if (!mClosed)
    {
        std::wstring wideTitle;
        if (UTF8ToUTF16(title, wideTitle))
            SetWindowText(mHandle, wideTitle.c_str());
    }
}

void Window::SetInvisible(bool invisible)
{
    mInvisible = invisible;

    if (mHandle)
    {
        if (mInvisible)
            ShowWindow(mHandle, SW_SHOW);
        else
            ShowWindow(mHandle, SW_HIDE);
    }
}

bool Window::Init()
{
    swprintf_s(mWndClass, L"%ws_%p", L"RaytracerDemo_WndClass", this);

    mInstance = GetModuleHandle(0);
    if (!mInstance)
    {
        RT_LOG_ERROR("Failed to acquire Instance from WinAPI");
        return false;
    }

    WNDCLASSEX wcex;
    wcex.cbSize        = sizeof(WNDCLASSEX);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = mInstance;
    wcex.hIcon         = 0; // LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GAME));
    wcex.hIconSm       = 0; // LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = 0; // (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = 0;
    wcex.lpszClassName = mWndClass;

    if (!RegisterClassEx(&wcex))
    {
        RT_LOG_ERROR("Failed to register Window");
        return false;
    }

    return true;
}

bool Window::Open()
{
    if (!mClosed)
        return false;
    std::wstring wideTitle;
    if (!UTF8ToUTF16(mTitle, wideTitle))
        return false;

    RECT windowRect;
    windowRect.left = (long)mLeft;
    windowRect.right = (long)(mWidth + mLeft);
    windowRect.top = (long)mTop;
    windowRect.bottom = (long)(mHeight + mTop);
    AdjustWindowRectEx(&windowRect, gWindowedStyle, FALSE, gWindowedExStyle);

    mLeft = -windowRect.left;
    mTop = -windowRect.top;

    mHandle = CreateWindowEx(gWindowedExStyle, mWndClass, wideTitle.c_str(), gWindowedStyle,
                             mTop, mTop, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
                             nullptr, nullptr, mInstance, nullptr);

    if (!mHandle)
        return false;

    SetWindowLongPtr(mHandle, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowText(mHandle, wideTitle.c_str());

    if (!mInvisible)
        ShowWindow(mHandle, SW_SHOW);

    DragAcceptFiles(mHandle, TRUE);
    UpdateWindow(mHandle);
    SetFocus(mHandle);

    mDC = GetDC(mHandle);

    mClosed = false;
    return true;
}

bool Window::DrawPixels(const rt::Bitmap& bitmap)
{
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = bitmap.GetWidth();
    bmi.bmiHeader.biHeight = -static_cast<Int32>(bitmap.GetHeight()); // flip the image
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = bitmap.GetStride() * bitmap.GetHeight();
    bmi.bmiHeader.biXPelsPerMeter = 1;
    bmi.bmiHeader.biYPelsPerMeter = 1;

    if (bitmap.GetFormat() == rt::Bitmap::Format::B8G8R8A8_UNorm)
    {
        bmi.bmiHeader.biBitCount = 32;
    }
    else if (bitmap.GetFormat() == rt::Bitmap::Format::B8G8R8_UNorm)
    {
        bmi.bmiHeader.biBitCount = 24;
    }
    else
    {
        return false;
    }

    if (0 == SetDIBitsToDevice(mDC, 0, 0, mWidth, mHeight, 0, 0, 0, mHeight, bitmap.GetData(), &bmi, DIB_RGB_COLORS))
    {
        RT_LOG_ERROR("Paint failed, error code: %u", GetLastError());
        return false;
    }

    return true;
}

bool Window::Close()
{
    if (mClosed)
        return false;

    if (!mInvisible)
        ShowWindow(mHandle, SW_HIDE);

    DestroyWindow(mHandle);
    mClosed = true;
    return true;
}


void Window::MouseDown(MouseButton button, int x, int y)
{
    SetCapture(mHandle);
    mMouseButtons[(int)button] = true;
    mMousePos[0] = x;
    mMousePos[1] = y;

    OnMouseDown(button, x, y);
}

void Window::MouseUp(MouseButton button)
{
    mMouseButtons[(int)button] = false;

    bool ButtonsReleased = true;
    for (int i = 0; i < 3; i++)
        if (mMouseButtons[i])
            ButtonsReleased = false;

    if (ButtonsReleased)
        ReleaseCapture();

    OnMouseUp(button);
}

void Window::MouseMove(int x, int y)
{
    OnMouseMove(x, y, x - mMousePos[0], y - mMousePos[1]);
    mMousePos[0] = x;
    mMousePos[1] = y;
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Window* window = (Window*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!window)
        return DefWindowProc(hWnd, message, wParam, lParam);

    switch (message)
    {
        case WM_SYSCOMMAND:
        {
            switch (wParam)
            {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
            }
            break;
        }

        case WM_CLOSE:
        {
            window->Close();
            window->OnClose();
            return 0;
        }

        case WM_SIZE:
        {
            if (wParam != SIZE_MINIMIZED)
            {
                window->mWidth = LOWORD(lParam);
                window->mHeight = HIWORD(lParam);
                window->OnResize(window->mWidth, window->mHeight);
            }

            return 0;
        }

        case WM_KEYDOWN:
        {
            wParam = MapLeftRightSpecialKey(wParam, lParam);
            window->mKeys[wParam] = true;
            window->OnKeyPress(static_cast<KeyCode>(wParam));
            return 0;
        }

        case WM_KEYUP:
        {
            wParam = MapLeftRightSpecialKey(wParam, lParam);
            window->mKeys[wParam] = false;
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            window->mMouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            window->OnScroll(window->mMouseWheelDelta);
            return 0;
        }

        // MOUSE
        case WM_LBUTTONDOWN:
        {
            window->MouseDown(MouseButton::Left, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_LBUTTONUP:
        {
            window->MouseUp(MouseButton::Left);
            return 0;
        }


        case WM_MBUTTONDOWN:
        {
            window->MouseDown(MouseButton::Middle, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_MBUTTONUP:
        {
            window->MouseUp(MouseButton::Middle);
            return 0;
        }


        case WM_RBUTTONDOWN:
        {
            window->MouseDown(MouseButton::Right, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_RBUTTONUP:
        {
            window->MouseUp(MouseButton::Right);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            window->MouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        }

        case WM_ACTIVATE:
        {
            if (wParam == WA_INACTIVE)
                window->LostFocus();
            return 0;
        }

        case WM_CHAR:
        {
            wchar_t input[] = { static_cast<wchar_t>(wParam), 0 };
            const int bufferSize = 8;
            char buffer[bufferSize];

            // convert UTF-16 char to UTF-8
            int result = ::WideCharToMultiByte(CP_UTF8, 0, input, 1, buffer, bufferSize, 0, 0);
            if (result > 0)
            {
                buffer[result] = '\0';
                window->OnCharTyped(buffer);
            }

            return 0;
        }

        case WM_DROPFILES:
        {
            const Uint32 bufferSize = 1024;
            wchar_t buffer[bufferSize] = { 0 };
            HDROP hDropInfo = (HDROP)wParam;
            if (0 != ::DragQueryFile(hDropInfo, 0, buffer, bufferSize))
            {
                std::string filePath;
                if (UTF16ToUTF8(std::wstring(buffer), filePath))
                {
                    RT_LOG_INFO("File dropped: '%s'", filePath.c_str());
                    window->OnFileDrop(filePath);
                }
                else
                {
                    RT_LOG_ERROR("Failed to convert UTF-16 file path to UTF-8 string");
                }
                return 0;
            }

            break;
        }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

void Window::LostFocus()
{
    MouseUp(MouseButton::Left);
    MouseUp(MouseButton::Middle);
    MouseUp(MouseButton::Right);

    for (int i = 0; i < 256; i++)
        mKeys[i] = false;
}

bool Window::IsClosed() const
{
    return mClosed;
}

bool Window::HasFocus() const
{
    return GetActiveWindow() == mHandle;
}

void* Window::GetHandle() const
{
    return static_cast<void*>(mHandle);
}

void Window::GetSize(Uint32& Width, Uint32& Height) const
{
    Width = mWidth;
    Height = mHeight;
}

void Window::ProcessMessages()
{
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
        {
            Close();
        }
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
}

void Window::OnClose()
{
}

void Window::OnResize(Uint32, Uint32)
{
}

void Window::OnKeyPress(KeyCode)
{
}

void Window::OnCharTyped(const char*)
{
}

void Window::OnScroll(int)
{
}

void Window::OnMouseDown(MouseButton, int, int)
{
}

void Window::OnMouseMove(int, int, int, int)
{
}

void Window::OnMouseUp(MouseButton)
{
}

void Window::OnFileDrop(const std::string&)
{
}

int Window::GetMouseWheelDelta() const
{
    return mMouseWheelDelta;
}

void Window::GetMousePosition(int& x, int& y) const
{
    x = mMousePos[0];
    y = mMousePos[1];
}

bool Window::IsMouseButtonDown(MouseButton button) const
{
    return mMouseButtons[(int)button];
}

bool Window::IsKeyPressed(KeyCode key) const
{
    unsigned int k = static_cast<unsigned int>(key);
    if (k <= 256)
        return mKeys[k];
    return false;
}