#ifndef UNICODE
#define UNICODE
#endif

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib") 
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"Shell32.lib")
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Dwmapi.lib")
#pragma comment(lib,"Comdlg32.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dcomp.lib")

#include <exception>
#include <wincodec.h>
#include <windowsx.h>
#include <strsafe.h>
#include <pathcch.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <dwmapi.h>
#include <shobjidl.h>
#include <Objbase.h>
#include <commdlg.h>
#include <tchar.h>
#include <dcomp.h>
#include <string>
#include <vector>
#include <deque>

using namespace std;

#pragma warning(disable:4100)

#pragma once

#include "rect.h"
#include "ResourcePack.h"
#include "WindowTypes.h"
#include "decode.h"
#include "layerWindow.h"
#include "BitmapUtility.h"

#include "Window.h"
