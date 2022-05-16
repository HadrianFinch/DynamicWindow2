#include "decode.h"
#include "rect.h"
#include "ResourcePack.h"
#include <vector>
#include <string>

#pragma once

namespace Windows::UI::DynamicWindow
{
class Window
{
public:
    typedef LRESULT (*PFN_wndProc)(Window* pThis, UINT, WPARAM, LPARAM);
    typedef void (*PFN_wndFunc)(Window* pThis, WPARAM, LPARAM);
protected:
    Rect m_windowRect;
    HWND m_hwnd = NULL;

    Window* m_dragWindow = nullptr;
    POINT m_dragPtStart = {};
    bool m_mouseDown = false;

    bool m_sizeable = false;

    vector<Window*> m_children;
    Window* m_pParent = nullptr;

    LRESULT WndProc(UINT msg, WPARAM wParam, LPARAM lParam);
    PFN_wndProc m_pWndProc = nullptr;

    PFN_wndFunc m_WMCREATE = nullptr;
    PFN_wndFunc m_WMLBUTTONDOWN = nullptr;
    PFN_wndFunc m_WMLBUTTONUP = nullptr;
    PFN_wndFunc m_WMRBUTTONUP = nullptr;
    PFN_wndFunc m_WMCOMMAND = nullptr;
    PFN_wndFunc m_WMMOUSEMOVE = nullptr;
    PFN_wndFunc m_WMNCHITTEST = nullptr;
    PFN_wndFunc m_WMKEYUP = nullptr;
    PFN_wndFunc m_WMDESTROY = nullptr;
    PFN_wndFunc m_WMTIMER = nullptr;

    bool m_isBorderless = false;

    Rect m_minimumSize = {};

    bool m_swpAnimationRunning = false;
    unsigned long m_swpAnimateTick = 0;
    unsigned long m_swpAnimateTotalTime = 0;
    unsigned int m_swpAnimateFlags = 0;
    Rect m_swpAnimateStartRect = {};
    Rect m_swpAnimateTarget = {};
protected:
    bool Call(PFN_wndFunc pfnFunc, WPARAM wParam, LPARAM lParam)
    {
        if (pfnFunc != nullptr)
        {
            pfnFunc(this, wParam, lParam);
            return true;
        }
        return false;
    }

    static const unsigned int s_wmMakeForground = WM_USER + 1;
    static const unsigned int s_wmMakeTopmost = WM_USER + 2;
    static LRESULT CALLBACK s_wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    static bool s_classRegistered;
    static HINSTANCE s_hInstance;
    static IWICImagingFactory* s_wicFactory;
    static ResourcePack* s_resourcePack;
    static const bool s_enableDarkMode;

    static bool RegisterClass()
    {
        if (!s_classRegistered)
        {
            WNDCLASSW wc = {};

            wc.lpfnWndProc = Window::s_wndProc;
            wc.hInstance = s_hInstance;
            wc.lpszClassName = s_className;
            wc.hCursor = LoadCursor(s_hInstance, IDC_ARROW);
            wc.hIcon = LoadIcon(s_hInstance, MAKEINTRESOURCE(s_resourcePack->mainIcon));

            if (::RegisterClassW(&wc))
            {
                s_classRegistered = true;
            }
        }
        
        return s_classRegistered; 
    }

    Window()
    {
    }

public:
    static constexpr PCWSTR s_className = L"DynamicWindow_v3";
    static const int s_ncborderSize = 4;
    ~Window()
    {
        DestroyWindow(m_hwnd);
    }

    static bool Begin(HINSTANCE hinstance, ResourcePack resourcePack)
    {
        s_resourcePack = new ResourcePack(resourcePack);
        s_hInstance = hinstance;

        if (FAILED(CreateWICFactory(&s_wicFactory)))
        {
            return false;
        }
    }

    static int Run()
    {
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);        
            DispatchMessage(&msg);
        }
        return msg.wParam;
    }
protected:
    void initilize(
        Window* hwndParent,
        Rect rc, 
        PCWSTR windowName, 
        DWORD dwStyle = NULL, 
        DWORD dwExStyle = NULL);

    virtual LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) = 0;

public:
    DWORD m_textBackgroundColor = 0x00000000;

    HWND GetHWND() const
    {
        return m_hwnd;
    }

    void SetTitle(wstring title)
    {
        SetWindowText(m_hwnd, title.c_str());
    }

    void SetWindowPos(HWND hwndInsertAfter, Rect rc, UINT flags)
    {
        ::SetWindowPos(
            m_hwnd, 
            hwndInsertAfter, 
            rc.Point().x,
            rc.Point().y,
            rc.Width(),
            rc.Height(),
            flags);
    }

    bool SetWindowPosAnimate(Rect rc, UINT flags, unsigned long duration)
    {
        m_swpAnimateStartRect = m_windowRect;
        m_swpAnimateFlags = flags;
        m_swpAnimateTarget = rc;
        m_swpAnimateTick = 0;
        m_swpAnimateTotalTime = duration;
        
        if (!m_swpAnimationRunning)
        {
            m_swpAnimationRunning = true;
            SetTimer(322, 1);
            return true;
        }
        else
        {
            return false;
        }
    }

    void SetWindowPosClient(HWND hwndInsertAfter, Rect rc, UINT flags)
    {
        rc.ClientToScreen(m_hwnd);

        ::SetWindowPos(
            m_hwnd, 
            hwndInsertAfter, 
            rc.Point().x, 
            rc.Point().y,
            rc.Width(),
            rc.Height(),
            flags);
    }

    void SetDrag(Window* parent)
    {
        m_dragWindow = parent;
    }

    Window* GetParent()
    {
        return m_pParent;
    }

    void MakeForground()
    {
        PostMessageW(m_hwnd, s_wmMakeForground, 0, 0);
    }
    void MakeTopmost()
    {
        PostMessageW(m_hwnd, s_wmMakeTopmost, 0, 0);
    }

    void OnClick(PFN_wndFunc func)
    {
        m_WMLBUTTONUP = func;
    }
    void OnMouseDown(PFN_wndFunc func)
    {
        m_WMLBUTTONDOWN = func;
    }
    void OnMouseMove(PFN_wndFunc func)
    {
        m_WMMOUSEMOVE = func;
    }
    void OnCreate(PFN_wndFunc func)
    {
        m_WMCREATE = func;
    }
    void OnRightClick(PFN_wndFunc func)
    {
        m_WMRBUTTONUP = func;
    }
    void OnMenuItemClick(PFN_wndFunc func)
    {
        m_WMCOMMAND = func;
    }
    void OnKeyPress(PFN_wndFunc func)
    {
        m_WMKEYUP = func;
    }
    void OnDestroy(PFN_wndFunc func)
    {
        m_WMDESTROY = func;
    }
    void OnTimer(PFN_wndFunc func)
    {
        m_WMTIMER = func;
    }
    void SetTimer(int id, int time)
    {
        ::SetTimer(m_hwnd, id, time, nullptr);
    }

    void SimulateClick()
    {
        Call(m_WMLBUTTONDOWN, 0, 0);
        Call(m_WMLBUTTONUP, 0, 0);
    }

    void ShowWindow()
    {
        ::ShowWindow(m_hwnd, SW_SHOW);
    }

    void HideWindow()
    {
        ::ShowWindow(m_hwnd, SW_HIDE);
    }

    void Disable()
    {
        EnableWindow(m_hwnd, FALSE);
    }
    
    void Enable()
    {
        EnableWindow(m_hwnd, TRUE);
    }

    void SetTextBackgroundColor(DWORD color)
    {
        m_textBackgroundColor = color;
    }
    
    // In client space
    void SetMinimumSize(Rect size)
    {
        m_minimumSize.RECT(size.RECT());
        m_minimumSize.ClientToScreen(m_hwnd);
    }

    const Rect& GetRect()
    {
        return m_windowRect;
    }
};
}
