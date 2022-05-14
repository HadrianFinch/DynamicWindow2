
namespace Windows::UI::DynamicWindow::Utility
{
    class BitmapUtility
    {
    private:
        BitmapUtility()
        {

        }
        ~BitmapUtility()
        {

        }
        
    public:
        static SIZE GetBitmapSize(HBITMAP hbmp)
        {
            BITMAP bm; 
            GetObject(hbmp, sizeof(BITMAP), &bm);
            SIZE size = {bm.bmWidth, bm.bmHeight};
            return size;
        }

        static HBITMAP GenerateBitmapFrom9Grid(
            HBITMAP templateBitmap,
            SIZE targetSize)
        {
            HDC hdcTemplate = CreateCompatibleDC(NULL);
            HDC hdcDest = CreateCompatibleDC(NULL);

            BITMAPINFO bi = {};
            bi.bmiHeader.biSize = sizeof(bi);
            bi.bmiHeader.biWidth = targetSize.cx;
            bi.bmiHeader.biHeight = -targetSize.cy; // negative so (0,0) is at top left
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;

            unsigned char* pvBits;
            HBITMAP hBitmapTarget = CreateDIBSection(
                hdcDest,
                &bi,
                DIB_RGB_COLORS,
                (void**)&pvBits,
                NULL,
                0);

            HGDIOBJ hTemplateSave = SelectObject(hdcTemplate, templateBitmap);
            HGDIOBJ hDestSave = SelectObject(hdcDest, hBitmapTarget);

            SIZE templateSize = GetBitmapSize(templateBitmap);
            POINT templateCenterPoint = {templateSize.cx / 2, templateSize.cx / 2};
            
            SIZE cornerSize = {templateCenterPoint.x, templateCenterPoint.y};

            const Rect templateRects[9] = 
            {
                {0, 0, templateCenterPoint.x, templateCenterPoint.y},
                {templateCenterPoint.x, 0, templateCenterPoint.x +1, templateCenterPoint.y},
                {templateCenterPoint.x + 1, 0, templateSize.cx, templateCenterPoint.y},

                {0, templateCenterPoint.y, templateCenterPoint.x, templateCenterPoint.y + 1},
                {templateCenterPoint.x, templateCenterPoint.y, templateCenterPoint.x + 1, templateCenterPoint.y + 1},
                {templateCenterPoint.x + 1, templateCenterPoint.y, templateSize.cx, templateCenterPoint.y + 1},

                {0, templateCenterPoint.y + 1, templateCenterPoint.x, templateSize.cy},
                {templateCenterPoint.x, templateCenterPoint.y + 1, templateCenterPoint.x + 1, templateSize.cy},
                {templateCenterPoint.x + 1, templateCenterPoint.y + 1, templateSize.cx, templateSize.cy}
            };

            const Rect destRects[9]
            {
                {0, 0, cornerSize.cx, cornerSize.cy},
                {cornerSize.cx, 0, targetSize.cx - cornerSize.cx, cornerSize.cy},
                {targetSize.cx - cornerSize.cx, 0, targetSize.cx, cornerSize.cy},

                {0, cornerSize.cy, cornerSize.cx, targetSize.cy - cornerSize.cy},
                {cornerSize.cx, cornerSize.cy, targetSize.cx - cornerSize.cx, targetSize.cy - cornerSize.cy},
                {targetSize.cx - cornerSize.cx, cornerSize.cy, targetSize.cx, targetSize.cy - cornerSize.cy},

                {0, targetSize.cy - cornerSize.cy, cornerSize.cx, targetSize.cy},
                {cornerSize.cx, targetSize.cy - cornerSize.cy, targetSize.cx - cornerSize.cx, targetSize.cy},
                {targetSize.cx - cornerSize.cx, targetSize.cy - cornerSize.cy, targetSize.cx, targetSize.cy},
            };

            /***************/
            /* Do the blts */
            /***************/

            for (int i = 0; i < 9; i++)
            {
                if ((i == 0) || (i == 2) || (i == 6) || (i == 8))
                {
                    BitBlt(
                        hdcDest,
                        destRects[i].Point().x,
                        destRects[i].Point().y,
                        templateRects[i].Width(),
                        templateRects[i].Height(),
                        hdcTemplate,
                        templateRects[i].Point().x,
                        templateRects[i].Point().y,
                        SRCCOPY);
                }
                else
                {
                    StretchBlt(
                        hdcDest,

                        destRects[i].Point().x,
                        destRects[i].Point().y,
                        destRects[i].Width(),
                        destRects[i].Height(),

                        hdcTemplate,

                        templateRects[i].Point().x,
                        templateRects[i].Point().y,
                        templateRects[i].Width(),
                        templateRects[i].Height(),

                        SRCCOPY);
                }
            }        

            
            // Cleanup
            SelectObject(hdcTemplate, hTemplateSave);
            SelectObject(hdcDest, hDestSave);

            DeleteDC(hdcTemplate);
            DeleteDC(hdcDest);

            return hBitmapTarget;
        }
    };    
}
