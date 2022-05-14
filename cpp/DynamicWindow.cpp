#include "DynamicWindow.h"
#include "rect.h"

namespace Windows::UI::DynamicWindow
{
bool Window::s_classRegistered = false;
const bool Window::s_enableDarkMode = true;

void Window::initilize(
    Window* parentWindow,
    Rect rc,
    PCWSTR windowName, 
    DWORD dwStyle, 
    DWORD dwExStyle)
{
    m_pParent = parentWindow;

    // Change styles
    if (parentWindow != nullptr)
    {
        dwStyle |= (WS_CHILD | WS_CLIPSIBLINGS);
        parentWindow->m_children.push_back(this);
    }
    else
    {
        dwStyle |= (WS_CLIPCHILDREN | WS_SIZEBOX);

        if (m_isBorderless)
        {
            dwStyle |= WS_POPUPWINDOW;

            rc.Width(rc.Width() + (s_ncborderSize * 2));
            rc.Height(rc.Height() + s_ncborderSize);
        }
        else
        {
            if (m_sizeable)
            {
                dwStyle |= WS_OVERLAPPEDWINDOW;
            }
            else
            {
                dwStyle |= WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX;
            }
        }  
    }
    
    HWND hwndP = NULL;
    if (parentWindow != nullptr)
    {
        hwndP = parentWindow->m_hwnd;
    }

    // Make the window
    RegisterClass();
    m_hwnd = CreateWindowExW(
        dwExStyle,
        s_className,
        windowName,
        dwStyle,
        rc.Point().x,
        rc.Point().y,
        labs(rc.Width()),
        labs(rc.Height()),
        hwndP,
        NULL,
        s_hInstance,
        this);


    DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &s_enableDarkMode, sizeof(s_enableDarkMode));

    SetWindowPos(HWND_TOP, rc, 0);
    ShowWindow();

    m_windowRect = rc;
}

LRESULT CALLBACK Window::s_wndProc(
    HWND hwnd, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam)
{
    Window* pThis = nullptr;

    if (msg == WM_CREATE)
    {
        CREATESTRUCT* pCS = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<Window*>(pCS->lpCreateParams);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        ::SetTimer(hwnd, 321, 5, NULL);
    }
    else
    {
        pThis = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis != nullptr)
    {
        return pThis->WndProc(msg, wParam, lParam);
    } 

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT Window::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
    // if (m_pWndProc != nullptr)
    // {
    //     return m_pWndProc(this, msg, wParam, lParam);
    // }

    if ((msg == WM_TIMER) && (wParam == 321))
    {
        KillTimer(m_hwnd, wParam);
        Call(m_WMCREATE, wParam, lParam);
    }

    if ((msg == WM_TIMER) && (wParam == 322))
    {
        float animationPrecent = (float)m_swpAnimateTick / (float)m_swpAnimateTotalTime;

        Rect rect = {};
        rect.Point({
            (int)((m_swpAnimateTarget.Point().x - m_swpAnimateStartRect.Point().x) * animationPrecent) + m_swpAnimateStartRect.Point().x,
            (int)((m_swpAnimateTarget.Point().y - m_swpAnimateStartRect.Point().y) * animationPrecent) + m_swpAnimateStartRect.Point().y
        });
        rect.Size({
            (int)((m_swpAnimateTarget.Size().cx - m_swpAnimateStartRect.Size().cx) * animationPrecent) + m_swpAnimateStartRect.Size().cx,
            (int)((m_swpAnimateTarget.Size().cy - m_swpAnimateStartRect.Size().cy) * animationPrecent) + m_swpAnimateStartRect.Size().cy
        });

        SetWindowPos(NULL, rect, m_swpAnimateFlags);
        m_swpAnimateTick++;

        if (animationPrecent >= 1.0f)
        {
            m_swpAnimationRunning = false;
            SetWindowPos(NULL, m_swpAnimateTarget, m_swpAnimateFlags);
            KillTimer(m_hwnd, wParam);
        }
    }
    
    if ((::GetParent(m_hwnd) == NULL) && m_isBorderless)
    {
        if (msg == WM_NCCALCSIZE)
        {
            if (wParam)
            {
                NCCALCSIZE_PARAMS* pncc = reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam);

                auto rcWindow = pncc->rgrc[0];

                DefWindowProc(m_hwnd, msg, wParam, lParam);
                
                pncc->rgrc[0] = {rcWindow.left + s_ncborderSize,
                                rcWindow.top,
                                rcWindow.right - s_ncborderSize,
                                rcWindow.bottom - s_ncborderSize};

                InvalidateRect(m_hwnd, NULL, true);

                return 0;
            }
            return DefWindowProc(m_hwnd, msg, wParam, lParam);
        }
        else if (msg == WM_NCACTIVATE)
        {
            return 0;
        }
    }

    bool handled = false;
    LRESULT ret = OnWindowProc(msg, wParam, lParam, handled);
    if (handled == true)
    {
        return ret;
    }
    

    if (msg == WM_SETCURSOR)
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            LPCWSTR cursorImage = IDC_ARROW;
            
            HCURSOR cursor = LoadCursorW(NULL, cursorImage);
            if (cursor != NULL)
            {
                SetCursor(cursor);
            }
            return FALSE;
        }
        return DefWindowProc(m_hwnd, msg, wParam, lParam);
    }
    else if (msg == WM_WINDOWPOSCHANGED)
    {
        WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
        if ((pos->cx != 0) && (pos->cy != 0))
        {   
            if ((m_windowRect.Width() != Rect::SIZE_FULL) && (m_windowRect.Height() != Rect::SIZE_FULL))
            {
                m_windowRect.Size({pos->cx, pos->cy});
            }
            if ((m_windowRect.GetVerticalAlignment() == align_default) &&
                (m_windowRect.GetHorizontalAlignment() == align_default))
            {
                m_windowRect.Point({pos->x, pos->y}); 
            }

            for (Window* pChild : m_children)
            {
                Rect& rcChild = pChild->m_windowRect;

                if (rcChild.Width() == Rect::SIZE_FULL)
                {
                    Rect rc = rcChild;
                    rc.Width(m_windowRect.Width());
                    pChild->SetWindowPos(NULL, rc, SWP_NOMOVE);
                }
                
                if (rcChild.Height() == Rect::SIZE_FULL)
                {
                    Rect rc = rcChild; 
                    rc.Height(m_windowRect.Height());
                    pChild->SetWindowPos(NULL, rc, SWP_NOMOVE);
                }

                if (rcChild.GetVerticalAlignment() == align_bottom)
                {
                    Rect rc = rcChild;
                    long parentHeight = m_windowRect.Height();

                    // Account for Rect::SIZE_FULL
                    if (parentHeight == Rect::SIZE_FULL)
                    {
                        Rect rc2;
                        rc2.GetClientRect(::GetParent(m_hwnd));
                        parentHeight = rc2.Height();
                    }
                    
                    rc.Point({rc.Point().x, parentHeight - (rc.Point().y + rc.Height())});
                    pChild->SetWindowPos(NULL, rc, SWP_NOSIZE);
                }

                if (rcChild.GetHorizontalAlignment() == align_right)
                {
                    Rect rc = rcChild;
                    long parentWidth = m_windowRect.Width();

                    // Account for Rect::SIZE_FULL
                    if (parentWidth == Rect::SIZE_FULL)
                    {
                        Rect rc2;
                        rc2.GetClientRect(::GetParent(m_hwnd));
                        parentWidth = rc2.Width();
                    }
                    
                    rc.Point({parentWidth - (rc.Point().x + rc.Width()), rc.Point().y});
                    pChild->SetWindowPos(NULL, rc, SWP_NOSIZE);
                }
            }
        }
        return 0;
    }
    else if (msg == WM_LBUTTONDOWN)
    {
        Call(m_WMLBUTTONDOWN, wParam, lParam);
        m_mouseDown = true;
        if (m_dragWindow != nullptr)
        {
            m_dragPtStart.x = GET_X_LPARAM(lParam);
            m_dragPtStart.y = GET_Y_LPARAM(lParam);
            ClientToScreen(m_hwnd, &m_dragPtStart);
            
            SetCapture(m_hwnd);  
        }        
    }
    else if (msg == WM_LBUTTONUP)
    {
        Call(m_WMLBUTTONUP, wParam, lParam);
        ReleaseCapture();
        m_mouseDown = false;
        if (m_dragWindow != nullptr)
        {
            m_dragPtStart = {};
        }
    }
    else if (msg == WM_MOUSEMOVE)
    {
        Call(m_WMMOUSEMOVE, wParam, lParam);
        TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT)};
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hwnd;
        TrackMouseEvent(&tme); 

        if ((m_dragWindow != nullptr) && (m_mouseDown))
        {
            POINT ptMouse = {};
            ptMouse.x = GET_X_LPARAM(lParam);
            ptMouse.y = GET_Y_LPARAM(lParam);
            ClientToScreen(m_hwnd, &ptMouse);
            if (WindowFromPoint(ptMouse) == m_hwnd)
            {
                SIZE dist = {};
                dist.cx = (ptMouse.x - m_dragPtStart.x);
                dist.cy = (ptMouse.y - m_dragPtStart.y);

                RECT rcParent = {};
                GetWindowRect(m_dragWindow->m_hwnd, &rcParent);
                ::SetWindowPos(
                    m_dragWindow->m_hwnd,
                    NULL,
                    (rcParent.left + dist.cx),
                    (rcParent.top + dist.cy),
                    (rcParent.right - rcParent.left),
                    (rcParent.bottom - rcParent.top),
                    0);

                m_dragPtStart = ptMouse;
            }
        }
        return 0;
    }
    else if (msg == WM_MOUSELEAVE)
    {
        m_mouseDown = false;
    }
    else if (msg == WM_NCHITTEST)
    {
        if (m_sizeable)
        {
            return DefWindowProc(m_hwnd, msg, wParam, lParam);
        }
        return HTCLIENT;
    }
    else if (msg == WM_KEYUP)
    {
        Call(m_WMKEYUP, wParam, lParam);
        return 0;
    }
    else if (msg == WM_CONTEXTMENU)
    {
        Call(m_WMRBUTTONUP, wParam, lParam);
        return 0;
    }    
    else if (msg == WM_COMMAND)
    {
        Call(m_WMCOMMAND, wParam, lParam);
        return 0;
    }    
    else if (msg == s_wmMakeForground)
    {
        SetForegroundWindow(m_hwnd);
        return 0;
    }    
    else if (msg == s_wmMakeTopmost)
    {
        SetForegroundWindow(m_hwnd);
        SetWindowPos(HWND_TOPMOST, {}, SWP_NOMOVE | SWP_NOSIZE);
        return 0;
    }
    else if(msg == WM_DESTROY)
    {
        Call(m_WMDESTROY, wParam, lParam);
        return 0;
    }
    else if(msg == WM_GETMINMAXINFO)
    {
        if (m_sizeable && ((m_minimumSize.Size().cx != 0) && (m_minimumSize.Size().cy != 0)))
        {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize = {m_minimumSize.Size().cx, m_minimumSize.Size().cy};
            return 0;
        }
    }
    else if(msg == WM_CTLCOLORSTATIC)
    {
        SetTextColor((HDC)wParam,RGB(0, 0, 0));
        SetBkMode((HDC)wParam,TRANSPARENT);

        SetTextCharacterExtra((HDC)wParam, 1);

        COLORREF colorref = m_textBackgroundColor;
        HBRUSH brush = CreateSolidBrush(colorref);
        
        return (LRESULT)brush;
    }
    else if (msg == WM_TIMER)
    {
        Call(m_WMTIMER, wParam, lParam);
        return 0;
    }

    return DefWindowProc(m_hwnd, msg, wParam, lParam);
}
}
