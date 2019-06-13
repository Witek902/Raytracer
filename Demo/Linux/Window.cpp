#include "PCH.h"

#include "../Window.h"
#include "../Core/Utils/Logger.h"
#include "../Core/Utils/Bitmap.h"

namespace {

const char* TranslateErrorCodeToStr(int err)
{
    // these error codes base on XErrorDB, available on /usr/share/X11/XErrorDB
    switch(err)
    {
    case 0: return "OK";
    case 1: return "BadRequest";
    case 2: return "BadValue";
    case 3: return "BadWindow";
    case 4: return "BadPixmap";
    case 5: return "BadAtom";
    case 6: return "BadCursor";
    case 7: return "BadFont";
    case 8: return "BadMatch";
    case 9: return "BadDrawable";
    case 10: return "BadAccess";
    case 11: return "BadAlloc";
    case 12: return "BadColor";
    case 13: return "BadGC";
    case 14: return "BadIDChoice";
    case 15: return "BadName";
    case 16: return "BadLength";
    case 17: return "BadImplementation";
    default: return "Unknown";
    }
}

} // namespace


Window::Window()
    : mConnection(nullptr)
    , mWindow(0)
    , mScreen(nullptr)
    , mDeleteReply(nullptr)
    , mConnScreen(0)
    , mGraphicsContext(0u)
    , mClosed(true)
    , mInvisible(false)
    , mWidth(400)
    , mHeight(400)
    , mLeft(20)
    , mTop(20)
    , mTitle("Window")
{
    for (int i = 0; i < 3; i++)
        mMouseButtons[i] = false;

    for (int i = 0; i < 255; i++)
        mKeys[i] = false;
}

Window::~Window()
{
    Close();
    free(mDeleteReply);

    if (mConnection)
    {
        xcb_set_screen_saver(mConnection, -1, 0, XCB_BLANKING_NOT_PREFERRED, XCB_EXPOSURES_ALLOWED);
        xcb_destroy_window(mConnection, mWindow);
        xcb_flush(mConnection);
        xcb_disconnect(mConnection);
    }
}

bool Window::Init()
{
    mConnection = xcb_connect(nullptr, &mConnScreen);
    if (xcb_connection_has_error(mConnection))
    {
        RT_LOG_ERROR("Failed to connect to X server!");
        return false;
    }

    // acquire current screen
    const xcb_setup_t* xcbSetup;
    xcb_screen_iterator_t xcbIt;

    xcbSetup = xcb_get_setup(mConnection);
    xcbIt = xcb_setup_roots_iterator(xcbSetup);
    while (mConnScreen-- > 0)
        xcb_screen_next(&xcbIt);
    mScreen = xcbIt.data;

    xcb_set_screen_saver(mConnection, 0, 0, XCB_BLANKING_NOT_PREFERRED, XCB_EXPOSURES_ALLOWED);
    return true;
}

void Window::SetSize(Uint32 width, Uint32 height)
{
    mWidth = width;
    mHeight = height;

    if (!mClosed)
    {
        const Uint32 newSize[] = { mWidth, mHeight };
        xcb_configure_window(mConnection, mWindow, XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT, newSize);
    }
}

void Window::SetTitle(const char* title)
{
    mTitle = title;
    if (!mClosed)
    {
        xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, mTitle.length(), mTitle.data());
    }
}

void Window::SetInvisible(bool invisible)
{
    mInvisible = invisible;

    if (mWindow)
    {
        if (mInvisible)
            xcb_unmap_window(mConnection, mWindow);
        else
            xcb_map_window(mConnection, mWindow);
    }
}

bool Window::Open()
{
    if (!mClosed)
        return false;

    // generate our window ID
    mWindow = xcb_generate_id(mConnection);

    xcb_colormap_t colormap = xcb_generate_id(mConnection);
    xcb_create_colormap(mConnection, XCB_COLORMAP_ALLOC_NONE, colormap, mScreen->root,
                        mScreen->root_visual);

    // set some settings (these match the old ones)
    Uint32 value_mask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
    Uint32 value_list[] = {
        XCB_EVENT_MASK_BUTTON_1_MOTION | XCB_EVENT_MASK_BUTTON_2_MOTION | XCB_EVENT_MASK_BUTTON_PRESS |
        XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_FOCUS_CHANGE |
        XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
        XCB_EVENT_MASK_STRUCTURE_NOTIFY,
        colormap
    };

    xcb_void_cookie_t cookie = xcb_create_window_checked(mConnection, mScreen->root_depth, mWindow,
                                                      mScreen->root, mLeft, mTop, mWidth, mHeight,
                                                      1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                                                      mScreen->root_visual, value_mask, value_list);

    xcb_generic_error_t* err = xcb_request_check(mConnection, cookie);
    if (err)
    {
        RT_LOG_ERROR("Failed to create a window: X11 protocol error %d (%s)", err->error_code, TranslateErrorCodeToStr(err->error_code));
        free(err);
        return false;
    }

    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, mTitle.length(), mTitle.data());

    // pre-initialize delete atom to be later on notified about Window being destroyed
    xcb_intern_atom_cookie_t deleteCookie = xcb_intern_atom(mConnection, 1, 16, "WM_DELETE_WINDOW");
    mDeleteReply = xcb_intern_atom_reply(mConnection, deleteCookie, nullptr);

    // get access to WM_PROTOCOLS atom
    xcb_intern_atom_cookie_t wmProtCookie = xcb_intern_atom(mConnection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* wmProtReply = xcb_intern_atom_reply(mConnection, wmProtCookie, nullptr);

    // notify X that we want to be part of WM_DELETE_WINDOW communication
    xcb_change_property(mConnection, XCB_PROP_MODE_REPLACE, mWindow, wmProtReply->atom, 4, 32, 1, &mDeleteReply->atom);
    free(wmProtReply);

    if (!mInvisible)
        xcb_map_window(mConnection, mWindow);

    const int32_t gcValueMask = XCB_GC_BACKGROUND | XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
    const uint32_t gcValue[] = { mScreen->white_pixel, mScreen->black_pixel, 0 };

    mGraphicsContext = xcb_generate_id(mConnection);
    cookie = xcb_create_gc(mConnection, mGraphicsContext, mWindow, gcValueMask, gcValue);
    err = xcb_request_check(mConnection, cookie);
    if (err)
    {
        RT_LOG_ERROR("Failed to create graphics context: X11 protocol error: %s", TranslateErrorCodeToStr(err->error_code));
        free(err);
        return false;
    }

    mClosed = false;
    return true;
}

bool Window::Close()
{
    if (mClosed)
    {
        return false;
    }

    if (mGraphicsContext)
    {
        xcb_free_gc(mConnection, mGraphicsContext);
        mGraphicsContext = 0;
    }

    if (!mInvisible)
    {
        xcb_unmap_window(mConnection, mWindow);
    }

    mClosed = true;
    return true;
}

void Window::MouseDown(MouseButton button, int x, int y)
{
    xcb_grab_pointer_cookie_t grab = xcb_grab_pointer(mConnection, 1, mWindow, 0,
                                                      XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
                                                      mWindow, XCB_NONE, XCB_CURRENT_TIME);
    xcb_grab_pointer_reply_t* grabReply = xcb_grab_pointer_reply(mConnection, grab, nullptr);
    free(grabReply);

    mMouseButtons[(int)button] = true;
    mMousePos[(int)button] = x;
    mMousePos[(int)button] = y;

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
        xcb_ungrab_pointer(mConnection, XCB_CURRENT_TIME);

    OnMouseUp(button);
}

void Window::MouseMove(int x, int y)
{
    OnMouseMove(x, y, x - mMousePos[0], y - mMousePos[1]);
    mMousePos[0] = x;
    mMousePos[1] = y;
}

void Window::ProcessMessages()
{
    xcb_flush(mConnection);

    xcb_generic_event_t* event;

    while ((event = xcb_poll_for_event(mConnection)))
    {
        // I have no idea why (and XCB reference won't tell me why as well),
        // but without & ~0x80 ClientMessage event is not received
        switch (event->response_type & ~0x80)
        {

            case XCB_CLIENT_MESSAGE:
            {
                xcb_client_message_event_t* cm = reinterpret_cast<xcb_client_message_event_t*>(event);

                // close handling
                if (static_cast<xcb_atom_t>(cm->data.data32[0]) == mDeleteReply->atom)
                {
                    this->OnClose();
                    this->Close();
                }
                break;
            }

            case XCB_KEY_PRESS:
            {
                xcb_key_press_event_t* kp = reinterpret_cast<xcb_key_press_event_t*>(event);
                mKeys[kp->detail] = true;
                OnKeyPress(static_cast<KeyCode>(kp->detail));
                break;
            }

            case XCB_KEY_RELEASE:
            {
                xcb_key_release_event_t* kr = reinterpret_cast<xcb_key_release_event_t*>(event);
                mKeys[kr->detail] = false;
                //OnKeyUp(static_cast<KeyCode>(kr->detail));
                break;
            }

            case XCB_MOTION_NOTIFY:
            {
                xcb_motion_notify_event_t* m = reinterpret_cast<xcb_motion_notify_event_t*>(event);
                MouseMove(m->event_x, m->event_y);
                break;
            }

            case XCB_BUTTON_PRESS:
            {
                xcb_button_press_event_t* bp = reinterpret_cast<xcb_button_press_event_t*>(event);

                if (bp->detail < 4) // 1-3 MBtns, 4-5 MWheel
                    MouseDown((MouseButton)(bp->detail - 1), bp->event_x, bp->event_y);
                else if (bp->detail == 4)
                    OnScroll(1); // btn==4 is UP,
                else
                    OnScroll(-1); // btn==5 is DOWN
                break;
            }

            case XCB_BUTTON_RELEASE:
            {
                xcb_button_release_event_t* br = reinterpret_cast<xcb_button_release_event_t*>(event);
                MouseUp(MouseButton(br->detail - 1));
                break;
            }

            case XCB_FOCUS_OUT:
            {
                LostFocus();
                break;
            }

            case XCB_CONFIGURE_NOTIFY:
            {
                xcb_configure_notify_event_t* cn = reinterpret_cast<xcb_configure_notify_event_t*>(event);

                if (static_cast<Uint32>(cn->width) != mWidth ||
                    static_cast<Uint32>(cn->height) != mHeight)
                {
                    mWidth = cn->width;
                    mHeight = cn->height;
                    OnResize(mWidth, mHeight);
                }
                break;
            }
        }

        free(event);
    }
}

bool Window::DrawPixels(const rt::Bitmap& bitmap)
{
    xcb_image_t* img = xcb_image_create_native(mConnection,
                                               mWidth, mHeight,
                                               XCB_IMAGE_FORMAT_Z_PIXMAP,
                                               mScreen->root_depth, nullptr,
                                               4u * mWidth * mHeight,
                                               (uint8_t*)bitmap.GetData());
    if (!img)
    {
        RT_LOG_ERROR("Failed to create temporary image");
        return false;
    }

    xcb_void_cookie_t c = xcb_image_put(mConnection, mWindow, mGraphicsContext, img, 0, 0, 0);
    xcb_generic_error_t* err = xcb_request_check(mConnection, c);
    if (err)
    {
        RT_LOG_ERROR("Failed to put image on window: X11 protocol error: %s", TranslateErrorCodeToStr(err->error_code));
        free(err);
        return false;
    }

    xcb_image_destroy(img);

    return true;
}

void Window::LostFocus()
{
    MouseUp(MouseButton::Left);
    MouseUp(MouseButton::Middle);
    MouseUp(MouseButton::Right);

    for (int i = 0; i < 256; i++)
    {
        if (mKeys[i])
        {
            //OnKeyUp(static_cast<KeyCode>(i));
            mKeys[i] = false;
        }
    }
}

bool Window::IsClosed() const
{
    return mClosed;
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

void Window::GetMousePosition(int& x, int& y) const
{
    x = mMousePos[0];
    y = mMousePos[1];
}

bool Window::HasFocus() const
{
    xcb_get_input_focus_cookie_t inputCookie = xcb_get_input_focus(mConnection);
    xcb_get_input_focus_reply_t* input = xcb_get_input_focus_reply(mConnection, inputCookie, nullptr);
    return input->focus == mWindow;
}

void* Window::GetHandle() const
{
    return reinterpret_cast<void*>(mWindow);
}

void Window::GetSize(Uint32& Width, Uint32& Height) const
{
    Width = mWidth;
    Height = mHeight;
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
