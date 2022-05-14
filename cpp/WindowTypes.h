#include "layerWindow.h"

namespace Windows::UI::DynamicWindow::Basic
{
    class ColorWindow : public Window
    {
    protected:
        DWORD m_color = 0x00000000;
    public:
        static ColorWindow* Create(Rect rect, DWORD color, Window* parent = nullptr)
        {
            ColorWindow* pWindow = new ColorWindow();
            pWindow->m_color = color;
            pWindow->initilize(
                parent,
                rect,
                L"ColorWindow");
            return pWindow;
        }

        static ColorWindow* CreateSizeable(Rect rect, DWORD color, Window* parent = nullptr)
        {
            ColorWindow* pWindow = new ColorWindow();
            pWindow->m_color = color;
            pWindow->m_sizeable = true;
            pWindow->initilize(
                parent,
                rect,
                L"ColorWindow_Sizeable",
                WS_SIZEBOX);
            return pWindow;
        }

        static ColorWindow* CreateBorderless(Rect rect, DWORD color, Window* parent = nullptr)
        {
            ColorWindow* pWindow = new ColorWindow();
            pWindow->m_color = color;
            pWindow->m_isBorderless = true;
            pWindow->initilize(
                parent,
                rect,
                L"ColorWindow_Borderless");
            return pWindow;
        }

        static ColorWindow* CreateSizeableBorderless(Rect rect, DWORD color, Window* parent = nullptr)
        {
            ColorWindow* pWindow = new ColorWindow();
            pWindow->m_color = color;
            pWindow->m_sizeable = true;
            pWindow->m_isBorderless = true;
            pWindow->initilize(
                parent,
                rect,
                L"ColorWindow_SizeableBorderless",
                WS_SIZEBOX);
            return pWindow;
        }
    protected:
        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            switch (msg)
            {
                case WM_PAINT:
                {
                    handled = true;
                    PAINTSTRUCT ps;
                    HDC hdc = BeginPaint(m_hwnd, &ps);

                    Rect rcPaint({});
                    rcPaint.GetClientRect(m_hwnd);
                    rcPaint.Point({0, 0});
                    
                    COLORREF colorref = m_color;
                    HBRUSH brush = CreateSolidBrush(colorref);
                    FillRect(hdc, rcPaint.RectPointer(), brush);

                    DeleteObject(brush);
                    EndPaint(m_hwnd, &ps);
                }
                break;

                case WM_NCPAINT:
                {
                    if (m_isBorderless)
                    {
                        handled = true;
                        HDC hdc = GetDCEx(m_hwnd, (HRGN)wParam, DCX_WINDOW | DCX_INTERSECTRGN);

                        COLORREF colorref = m_color;
                        HBRUSH brush = CreateSolidBrush(colorref);
                        FillRect(hdc, m_windowRect.RectPointer(), brush);
                        
                        ReleaseDC(m_hwnd, hdc);
                    }
                }
                break;
            }
            return 0;
        }
    public:
        void SetColor(DWORD color)
        {
            m_color = color;
            InvalidateRect(m_hwnd, m_windowRect.RectPointer(), true);
        }
        DWORD GetColor() const
        {
            return m_color;
        }

        DWORD GetTextBackgroundColor()
        {
            return m_textBackgroundColor;
        }
    };

    class RoundedWindow : public Window
    {
    private:
        HBITMAP m_hBitmap = nullptr;
        int m_template = s_resourcePack->ninegrid.standard;
    public: 
        static RoundedWindow* Create(Rect rect, int templateImage, Window* parent)
        {
            RoundedWindow* pWindow = new RoundedWindow();
            pWindow->initilize(
                parent,
                rect,
                L"RoundedWindow",
                0,
                WS_EX_LAYERED);
            pWindow->m_template = templateImage;
            pWindow->Update();

            return pWindow;
        }

    protected:
        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            if (msg == WM_WINDOWPOSCHANGED)
            {
                WINDOWPOS* pos = reinterpret_cast<WINDOWPOS*>(lParam);
                if ((pos->cx != 0) && (pos->cy != 0))
                {   
                    Update();
                }
            }
            return 0;
        }

        void Update()
        {
            if (m_hBitmap != nullptr)
            {
                DeleteObject(m_hBitmap);
            }

            HBITMAP hBmpTemplate = nullptr;
            CreateBitmapFromResource(s_wicFactory, m_template, &hBmpTemplate);
            
            m_hBitmap = Utility::BitmapUtility::GenerateBitmapFrom9Grid(hBmpTemplate, m_windowRect.Size());

            DeleteObject(hBmpTemplate);

            LayerWindow(m_hwnd, m_hBitmap, m_windowRect.Size(), m_windowRect.Point());
        }
    };

    class TextWindow : public Window
    {
    private:
        HFONT m_font = nullptr;
        wstring m_text = L"";
        Alignment m_textAlignmentHorizontal = align_default;
        Alignment m_textAlignmentVertical = align_default;
    public: 
        static TextWindow* Create(Rect rect, wstring text, Alignment horizontal, Alignment vertical, Window* parent)
        {
            TextWindow* pWindow = new TextWindow();
            pWindow->initilize(
                parent,
                rect,
                L"TextWindow");
            pWindow->m_text = text;
            pWindow->m_textAlignmentHorizontal = horizontal;
            pWindow->m_textAlignmentVertical = vertical;
            pWindow->Update();
            return pWindow;
        }

    protected:
        HBITMAP MakeTextBitmap(wstring text, DWORD backgroundColor)
        {
            HDC hdc = CreateCompatibleDC(nullptr);

            size_t cchLength;
            StringCchLength(text.c_str(), MAX_PATH, &cchLength);

            HFONT hFontSave = nullptr;
            if (m_font != nullptr)
            {
                hFontSave = SelectFont(hdc, m_font);
            }

            SIZE textSize;
            GetTextExtentPoint(
                hdc,
                text.c_str(),
                static_cast<UINT>(cchLength),
                &textSize);


            BITMAPINFO bi = {};
            bi.bmiHeader.biSize = sizeof(bi);
            bi.bmiHeader.biWidth = m_windowRect.Size().cx;
            bi.bmiHeader.biHeight = -m_windowRect.Size().cy; // negative so (0,0) is at top left
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;

            unsigned char* pvBits;
            HBITMAP hBitmap = CreateDIBSection(
                hdc,
                &bi,
                DIB_RGB_COLORS,
                (void**)&pvBits,
                nullptr,
                0);

            if (hBitmap == nullptr)
            {
                __debugbreak();
            }

            HGDIOBJ hSaveObj = SelectObject(hdc, hBitmap);

            SetBkColor(hdc, backgroundColor);
            
            HBRUSH brush = CreateSolidBrush(backgroundColor);
            Rect rect = m_windowRect; rect.Point({0, 0});
            FillRect(hdc, rect.RectPointer(), brush);

            POINT textPrintPos = {0, 0};

            if (m_textAlignmentHorizontal == align_right)
            {
                textPrintPos.x = rect.Width() - textSize.cx;
            }
            if (m_textAlignmentHorizontal == 3)
            {
                textPrintPos.x = ((rect.Width() / 2) - (textSize.cx / 2));
            }

            if (m_textAlignmentVertical == align_bottom)
            {
                textPrintPos.y = rect.Height() - textSize.cy;
            }
            if (m_textAlignmentVertical == 3)
            {
                textPrintPos.y = ((rect.Height() / 2) - (textSize.cy / 2));
            }

            ExtTextOut(
                hdc,
                textPrintPos.x,
                textPrintPos.y,
                ETO_OPAQUE,
                nullptr,
                text.c_str(),
                static_cast<UINT>(cchLength),
                nullptr);

            const UINT pitch = 4 * m_windowRect.Size().cx;
            // Fixup the alpha for the text to be opaque v2
            for (int y = 0; y < m_windowRect.Size().cy; y++)
            {
                for (int x = 0; x < m_windowRect.Size().cx; x++)
                {
                    UINT index = (pitch * y) + (x * 4) + 3;

                    pvBits[index] = 0xFF;
                }
            }

            // Cleanup
            SelectObject(hdc, hSaveObj);
            if (m_font != nullptr)
            {
                SelectFont(hdc, hFontSave);
            }

            DeleteObject(brush);

            DeleteDC(hdc);

            return hBitmap;
        }

        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            return 0;
        }

    public:
        wstring GetText() const
        {
            return m_text;
        }
        
        wstring SetText(wstring text)
        {
            wstring oldText = GetText();
            m_text = text;
            return oldText;
        }

        SIZE GetTextExtent()
        {
            HDC hdc = CreateCompatibleDC(nullptr);

            size_t cchLength;
            StringCchLength(m_text.c_str(), MAX_PATH, &cchLength);

            HFONT hFontSave = nullptr;
            if (m_font != nullptr)
            {
                hFontSave = SelectFont(hdc, m_font);
            }

            SIZE textSize;
            GetTextExtentPoint(
                hdc,
                m_text.c_str(),
                static_cast<UINT>(cchLength),
                &textSize);

            SelectFont(hdc, hFontSave);
            DeleteDC(hdc);

            return textSize;
        }

        void Update()
        {
            HBITMAP hBmp = MakeTextBitmap(m_text, m_pParent->m_textBackgroundColor);
            LayerWindow(m_hwnd, hBmp, m_windowRect.Size(), m_windowRect.Point());
            DeleteObject(hBmp);
        }

        void SetFont(wstring fontName, int fontSize, bool italic = false, bool underline = false)
        {
            if (m_font != nullptr)
            {
                DeleteObject(m_font);
            }

            m_font = CreateFontW(
                fontSize, // font size
                0, // Auto width
                0,
                0,
                FW_NORMAL,
                italic, // italic
                underline, // underline
                false, // strikethrough
                DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS,
                CLIP_DEFAULT_PRECIS,
                ANTIALIASED_QUALITY,
                VARIABLE_PITCH,
                fontName.c_str());
        }   
    };

    class ToggleSwitch : public Window
    {
    public:
        typedef void (*PFN_toggleStateChange)(ToggleSwitch*, bool);
    private:
        bool m_state = false;

        HBITMAP m_hBitmap = nullptr;
        HBITMAP m_hBitmapActive = nullptr;

        PFN_toggleStateChange m_onChange = nullptr;
        PFN_toggleStateChange m_onActivate = nullptr;
        PFN_toggleStateChange m_onDeactivate = nullptr;

    public: 
        static ToggleSwitch* Create(POINT pt, bool initialState, Window* parent)
        {
            Rect rc = {}; rc.Point(pt); rc.Size({40, 20});

            ToggleSwitch* pWindow = new ToggleSwitch();
            pWindow->initilize(
                parent,
                rc,
                L"ToggleSwitch");
            pWindow->Init(initialState);
            pWindow->Update(false);
            return pWindow;
        }

    protected:
        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            switch (msg)
            {
                case WM_LBUTTONUP:
                {
                    Toggle();
                }
            }
            return 0;
        }

        void Init(bool initialState)
        {
            m_state = initialState;
            CreateBitmapFromResource(s_wicFactory, s_resourcePack->switches.off, &m_hBitmap);
            CreateBitmapFromResource(s_wicFactory, s_resourcePack->switches.on, &m_hBitmapActive);
        }

        void Update(bool notify = true)
        {
            HBITMAP hBmp = nullptr;
            if (m_state)
            {
                hBmp = m_hBitmapActive;
            }
            else
            {
                hBmp = m_hBitmap;
            }
            LayerWindow(m_hwnd, hBmp, m_windowRect.Size(), m_windowRect.Point());

            if (notify)
            {
                if (m_state)
                {
                    if (m_onActivate != nullptr)
                    {
                        m_onActivate(this, m_state);
                    }
                }
                else
                {
                    if (m_onDeactivate != nullptr)
                    {
                        m_onDeactivate(this, m_state);
                    }
                }

                if (m_onChange != nullptr)
                {
                    m_onChange(this, m_state);
                }
            }
        }

    public:
        void SetState(bool state)
        {
            m_state = state;
            Update();
        }
        void Toggle()
        {
            m_state = !m_state;
            Update();
        }

        bool GetState()
        {
            return m_state;
        }

        void OnChange(PFN_toggleStateChange func)
        {
            m_onChange = func;
        }
        void OnActivate(PFN_toggleStateChange func)
        {
            m_onActivate = func;
        }
        void OnDeactivate(PFN_toggleStateChange func)
        {
            m_onDeactivate = func;
        }
    };

    class ImageWindow : public Window
    {
    private:
        HBITMAP m_hBitmap = nullptr;
    public: 
        static ImageWindow* Create(POINT point, int imageID, Window* parent)
        {
            ImageWindow* pWindow = new ImageWindow();
            
            CreateBitmapFromResource(s_wicFactory, imageID, &pWindow->m_hBitmap);
            Rect rect = {}; rect.Point(point); rect.Size(Utility::BitmapUtility::GetBitmapSize(pWindow->m_hBitmap));
            
            pWindow->initilize(
                parent,
                rect,
                L"ImageWindow",
                0,
                WS_EX_LAYERED);

            LayerWindow(pWindow->m_hwnd, pWindow->m_hBitmap, rect.Size(), rect.Point());

            return pWindow;
        }

    protected:
        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            return 0;
        }
    };

    class GraphWindow : public Window
    {
    private:
        deque<long> m_list;
        unsigned long m_listLength;
        HBITMAP m_hBitmap = nullptr;
        
        unsigned char* m_pvBits;

        RoundedWindow* m_pPopup = nullptr;
        TextWindow* m_pPopupText = nullptr;
        
        SIZE m_popupTextPadding = {8, 4};

        bool m_popupVisible = false;

    public: 

        struct GraphSettings
        {
            DWORD lineColor = 0xff00ff00;
            DWORD fillColor = 0xff00ff00;
            DWORD backgroundColor = 0x00000000;

            unsigned int lineThickness = 4;

            SIZE graphRange = {};

            long startingValue = 0;

            enum
            {
                graph_LineOnly = 1,
                graph_LineAndFill = 2,
                graph_FillOnly = 3
            } graphType;

            wstring popupSufix = L"";
        };

        static GraphWindow* Create(Rect rect, GraphSettings settings, Window* parent)
        {
            GraphWindow* pWindow = new GraphWindow();
            
            pWindow->initilize(
                parent,
                rect,
                L"GraphWindow",
                0,
                WS_EX_LAYERED);
            pWindow->Init(settings);
            return pWindow;
        }

    protected:
        GraphSettings m_settings = {};

        void EnsurePopupState(bool state)
        {
            if (m_popupVisible != state)
            {
                if (state)
                {
                    m_pPopup->ShowWindow();
                }
                else
                {
                    m_pPopup->HideWindow();
                }
                m_popupVisible = state;
            }
        }

        void UpdatePopupPosition(POINT mousePos)
        {
            long queueValue = m_list[(((float)mousePos.x / (float)m_windowRect.Width()) * m_settings.graphRange.cx)];

            wstring text = to_wstring(queueValue); text += m_settings.popupSufix;
            m_pPopupText->SetText(text);
            m_pPopupText->Update();

            SIZE textExtent = m_pPopupText->GetTextExtent();

            POINT bottomRight = {
                mousePos.x, 
                (m_windowRect.Height() - (((float)(queueValue + 2) / (float)m_settings.graphRange.cy) * m_windowRect.Height()))};

            Rect rect = {};
            rect.Point({bottomRight.x - (m_popupTextPadding.cx * 2) - textExtent.cx,
                        bottomRight.y - (m_popupTextPadding.cy * 2) - textExtent.cy});
            rect.BottomRight(bottomRight);

            m_pPopup->SetWindowPos(nullptr, rect, 0);
        }

        LRESULT OnWindowProc(UINT msg, WPARAM wParam, LPARAM lParam, bool& handled) override
        {
            if (msg == WM_MOUSEMOVE)
            {
                POINT mousePos = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
                EnsurePopupState(true);
                UpdatePopupPosition(mousePos);
            }
            else if (msg == WM_MOUSELEAVE)
            {
                EnsurePopupState(false);
            }
            return 0;
        }
        
        DWORD* GetPixelPtr(POINT ptPixel, unsigned char* pvBits, unsigned long cbStride)
        {
            BYTE* pPixel = reinterpret_cast<BYTE*>(pvBits) + (cbStride * ptPixel.y) + (ptPixel.x * 4);
            return reinterpret_cast<DWORD*>(pPixel);
        }

        void Init(GraphSettings settings)
        {
            m_settings = settings;
            m_listLength = m_settings.graphRange.cx;
            
            for (int i = 0; i < m_listLength; i++)
            {
                m_list.push_back(settings.startingValue); // Fill the list
            }            
            
            HDC hdc = CreateCompatibleDC(nullptr);

            BITMAPINFO bi = {};
            bi.bmiHeader.biSize = sizeof(bi);
            bi.bmiHeader.biWidth = m_windowRect.Size().cx;
            bi.bmiHeader.biHeight = -m_windowRect.Size().cy; // negative so (0,0) is at top left
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;

            m_hBitmap = CreateDIBSection(
                hdc,
                &bi,
                DIB_RGB_COLORS,
                (void**)&m_pvBits,
                nullptr,
                0);

            m_pPopup = RoundedWindow::Create({0, 0, 20, 20}, s_resourcePack->ninegrid.popup, this);
            m_pPopup->SetTextBackgroundColor(0x00ffffff);
            m_pPopup->HideWindow();

            m_pPopupText = TextWindow::Create({
                m_popupTextPadding.cx, 
                m_popupTextPadding.cy, 
                m_popupTextPadding.cx + 48, 
                m_popupTextPadding.cy + 18},

                L"text", 
                align_left, align_middle,
                m_pPopup);
            m_pPopupText->SetFont(L"Segoe", 16);

            DeleteDC(hdc);

            Update();
        }

        void Update()
        {
            unsigned int stride = m_windowRect.Width() * 4;
            for (int x = 0; x < m_windowRect.Width(); x++)
            {
                int xIndex = (((float)x / (float)m_windowRect.Width()) * m_settings.graphRange.cx);
                long queueValue = m_list[xIndex];
                for (int y = 0; y < m_windowRect.Height(); y++)
                {
                    int yIndex = (((float)y / (float)m_windowRect.Height()) * m_settings.graphRange.cy);
                    
                    DWORD* pixel = GetPixelPtr({x, (m_windowRect.Height() - y)}, m_pvBits, stride);
                    
                    if (m_settings.graphType < 3)
                    {
                        if ((yIndex <= queueValue + (m_settings.lineThickness / 2)) &&
                            (yIndex >= queueValue - (m_settings.lineThickness / 2)))
                        {
                            *pixel = m_settings.lineColor;
                        }
                        else if ((yIndex < queueValue - (m_settings.lineThickness / 2)) && (m_settings.graphType == 2))
                        {
                            *pixel = m_settings.fillColor;
                        }
                        else
                        {
                            *pixel = m_settings.backgroundColor;
                        }
                    }
                }
            }
            
            LayerWindow(m_hwnd, m_hBitmap, m_windowRect.Size(), m_windowRect.Point());
        }
    public: 
        void AddPoint(long value)
        {
            m_list.pop_front();
            m_list.push_back(value);
            Update();
        }
    };
}