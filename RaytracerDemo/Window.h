#pragma once

#include "../RaytracerLib/Common.h"

#include <string>



class Window
{
private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND mHandle;
    HDC mDC;
    HINSTANCE mInstance;
    wchar_t mWndClass[48];

    bool mClosed;
    bool mFullscreen;
    bool mInvisible;
    Uint32 mWidth;
    Uint32 mHeight;
    int mLeft;
    int mTop;
    std::string mTitle;

    /// input recorded since last @p ProcessMessages() method call
    bool mMouseButtons[3];
    int mMousePos[2];
    int mMouseWheelDelta;
    bool mKeys[256];

    void LostFocus();
    void MouseDown(Uint32 button, int x, int y);
    void MouseUp(Uint32 button);
    void MouseMove(int x, int y);

    Window(const Window&);
    Window& operator= (const Window&);

public:
    Window();
    ~Window();

    /**
     * Initializes Window connection to the system.
     *
     * @return True on success, false on failure
     */
    bool Init();

    /**
     * Creates and opens a Window, if it isn't opened already.
     *
     * @return True on success, false on failure
     */
    bool Open();

    /**
     * Closes a Window which is already opened.
     *
     * @return True on success, false on failure
     *
     * @remarks This function will free any Window-related resources on the system. Should
     * the window still be valid, but invisible for the user, a @p SetInvisible function
     * should be used.
     */
    bool Close();

    /**
     * Acquire system-specific Window handle
     */
    void* GetHandle() const;

    /**
     * Get Window's size
     */
    void GetSize(Uint32& width, Uint32& height) const;

    /**
     * Get Window's current aspect ratio.
     */
    float GetAspectRatio() const;

    /**
     * Check if window is currently in Fullscreen mode.
     */
    bool GetFullscreenMode() const;

    /**
     * Acquire how much did the mouse wheel scroll.
     */
    int GetMouseWheelDelta() const;

    /**
     * Acquire current mouse position relative to window.
     */
    void GetMousePosition(int& x, int& y) const;

    /**
     * Check if a mouse button is pressed
     */
    bool IsMouseButtonDown(Uint32 button) const;

    /**
     * Check if a key is pressed.
     */
    bool IsKeyPressed(Uint32 key) const;

    /**
     * Set window's new size
     */
    void SetSize(Uint32 width, Uint32 height);

    /**
     * Enable or disable fullscreen
     */
    void SetFullscreenMode(bool enabled);

    /**
     * Make window invisible or visible
     */
    void SetInvisible(bool invisible);

    /**
     * Set window's title
     */
    void SetTitle(const char* title);

    /**
     * Process messages from Window system.
     */
    void ProcessMessages();

    /**
     * Is the window closed?
     */
    bool IsClosed() const;

    /**
     * Is the window focused on the system?
     */
    bool HasFocus() const;

    // callbacks, overridden by inheritance
    virtual void OnClose();
    virtual void OnResize(Uint32 width, Uint32 height);
    virtual void OnKeyPress(Uint32 key);
    virtual void OnCharTyped(const char* charUTF8);
    virtual void OnScroll(int delta);
    virtual void OnMouseDown(Uint32 button, int x, int y);
    virtual void OnMouseMove(int x, int y, int deltaX, int deltaY);
    virtual void OnMouseUp(Uint32 button);
};