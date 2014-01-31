// SKoreaMap.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SKoreaMap.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
void SetRegionFromBitmap();
HBITMAP hSkinBmp;
HWND hWnd;
#define IMAGEHEIGHT 592
#define IMAGEWIDTH 438

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SKOREAMAP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_SKOREAMAP);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

void SetRegionFromBitmap()
{
	BITMAP bm;                  // BITMAP structure
	HBITMAP hDIBSection = NULL; // Handle to DIB
	COLORREF* pBits = NULL;    // Pointer to the DIB's bit values
	HRGN hRgn = NULL;          // Handle to Region
	COLORREF mask=RGB(255,255,255);
	HRGN hRegion, hFragment;

	HDC hMemDC = NULL;          // Handle to a device context : DIB
	HDC hCopyDC = NULL;        // Handle to a device context : DDB -> DIB
	HBITMAP hOldDIBsBmp = NULL; // Handle of the object being replaced : DIB
	HBITMAP hOldDDBsBmp = NULL; // Handle of the object being replaced : DDB
	BITMAPINFOHEADER bmih;      // DIB's BITMAPINFOHEADER structure
	BOOL res = TRUE;  // Result of BitBlt function

	hMemDC = CreateCompatibleDC(NULL);
	if (!hMemDC)
		return;

	bmih.biSize            = sizeof(BITMAPINFOHEADER), // size of the structure
	bmih.biWidth            = IMAGEWIDTH;  // width of the bitmap
	bmih.biHeight          = IMAGEHEIGHT;  // height of the bitmap
	bmih.biPlanes          = 1;        // must be set to 1
	bmih.biBitCount        = 32;      // maximum of 2^32 colors
	bmih.biCompression      = BI_RGB;  // an uncompressed format
	bmih.biSizeImage        = 0;        // must be set to zero for BI_RGB bitmaps
	bmih.biXPelsPerMeter    = 0;        // set to zero
	bmih.biYPelsPerMeter    = 0;        // set to zero
	bmih.biClrUsed          = 0;        // must be set to zero
	bmih.biClrImportant    = 0;        // set to zero, all colors are required

	hDIBSection = CreateDIBSection(
		hMemDC,            // HDC hdc : handle to device context
		(CONST BITMAPINFO*)&bmih,  // CONST BITMAPINFO *pbmi : pointer to BITMAPINFO structure
		DIB_RGB_COLORS, // UINT iUsage : color data type(RGB values)
		(void**)&pBits, // VOID *ppvBits : pointer to receive the bitmap's bit values
		NULL,          // HANDLE hSection : handle to a file mapping object.
		0);            // DWORD dwOffset : ignored by hSection

	if (!hDIBSection)
		return;

	hOldDIBsBmp = (HBITMAP)SelectObject(hMemDC, hDIBSection);
	hCopyDC = CreateCompatibleDC(hMemDC);

	if (!hCopyDC)
		return;

	hOldDDBsBmp = (HBITMAP)SelectObject(hCopyDC, hSkinBmp);
	res = BitBlt(hMemDC, 0, 0, IMAGEWIDTH, IMAGEHEIGHT, hCopyDC, 0, 0, SRCCOPY);

	if (hOldDDBsBmp)    SelectObject(hCopyDC, hOldDDBsBmp);
	if (hCopyDC)        DeleteDC(hCopyDC);
	if (hOldDIBsBmp)    SelectObject(hMemDC, hOldDIBsBmp);
	if (hMemDC)        DeleteDC(hMemDC);

	if (!res)
	{
		DeleteObject(hDIBSection);
		hDIBSection = NULL;
		pBits = NULL;
	}

	if (!hDIBSection)
		return;

	int xx;

	DWORD maxRects = 200;

	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects));
	if (!hData)
		return;

	RGNDATA *pRgnData = (RGNDATA *)GlobalLock(hData);
	if (!pRgnData)
	{
		GlobalFree(hData);
		DeleteObject(hDIBSection);
		return;
	}

	pRgnData->rdh.dwSize = sizeof(RGNDATAHEADER);
	pRgnData->rdh.iType = RDH_RECTANGLES;
	pRgnData->rdh.nCount = pRgnData->rdh.nRgnSize = 0;
	SetRect(&pRgnData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);

	//if (cTransparentColor == 0xffffffff)
	//	cTransparentColor = *pBits;

	pBits += (IMAGEHEIGHT - 1) * IMAGEWIDTH;

	for (int y = 0; y < IMAGEHEIGHT; y++) 
	{
		for (int x = 0; x < IMAGEWIDTH; x++)  
		{
			if (*pBits != mask) 
			{  
				xx = x;  

				for (x = x + 1; x < IMAGEWIDTH; x++)
				{
					pBits++;

					if (*pBits == mask)
						break;
				}

				RECT *pr = (RECT *)pRgnData->Buffer;
				SetRect(&pr[pRgnData->rdh.nCount], xx, y, x, y+1);

				if (xx < pRgnData->rdh.rcBound.left)
					pRgnData->rdh.rcBound.left = xx;
				if (y < pRgnData->rdh.rcBound.top)
					pRgnData->rdh.rcBound.top = y;
				if (x > pRgnData->rdh.rcBound.right)
					pRgnData->rdh.rcBound.right = x;
				if (y+1 > pRgnData->rdh.rcBound.bottom)
					pRgnData->rdh.rcBound.bottom = y+1;

				pRgnData->rdh.nCount++;

				if (maxRects <= pRgnData->rdh.nCount)
				{
					HRGN hRgnTmp = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pRgnData);
					if (!hRgnTmp)
					{
						if (hRgn)
							DeleteObject(hRgn);

						GlobalUnlock(hData);
						GlobalFree(hData);
						DeleteObject(hDIBSection);

						return ;
					}

					if (hRgn)
					{
						CombineRgn(hRgn, hRgn, hRgnTmp, RGN_OR);
						DeleteObject(hRgnTmp);
					}
					else
						hRgn = hRgnTmp;

					pRgnData->rdh.nCount = 0;
					SetRect(&pRgnData->rdh.rcBound, MAXLONG, MAXLONG, 0, 0);
				}
			}
			pBits ++;
		}
		pBits -= 2 * IMAGEWIDTH;
	}

	HRGN hRgnTmp = ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT) * maxRects), pRgnData);
	if (!hRgnTmp)
	{
		if (hRgn)
			DeleteObject(hRgn);
	}
	else
	{
		if (hRgn)
		{
			CombineRgn(hRgn, hRgn, hRgnTmp, RGN_OR);
			DeleteObject(hRgnTmp);
		}
		else
			hRgn = hRgnTmp;
	}

	GlobalUnlock(hData);
	GlobalFree(hData);


	DeleteObject(hDIBSection);

	SetWindowRgn(hWnd,hRgn,TRUE);
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_SKOREAMAP);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	//wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= (LPCTSTR)IDC_SKOREAMAP1;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

UINT bitmapID=IDB_BITMAP;

void SetBitmap()
{
	hSkinBmp = LoadBitmap(hInst,MAKEINTRESOURCE(bitmapID));
	if (!hSkinBmp) return;
	SetRegionFromBitmap();	
	RedrawWindow(hWnd,NULL,NULL,RDW_ERASENOW|RDW_INVALIDATE);
	if(bitmapID==IDB_BITMAP)
		bitmapID=IDB_BITMAPK;
	else if(bitmapID==IDB_BITMAPK)
		bitmapID=IDB_BITMAPUSA;
	else if(bitmapID==IDB_BITMAPUSA)
		bitmapID=IDB_BITMAPE;
	else 
		bitmapID=IDB_BITMAP;
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, IMAGEWIDTH, IMAGEHEIGHT, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
	DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);
	dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);
	SetWindowLong(hWnd, GWL_STYLE, dwStyle);

	SetBitmap();

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	HDC dcSkin=NULL;
	HBITMAP hBmp;

	switch (message) 
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam); 
		wmEvent = HIWORD(wParam); 
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case ID_FILE_KOREAN:
			SetBitmap();
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		dcSkin = CreateCompatibleDC(hdc);
			hBmp = (HBITMAP)SelectObject(dcSkin, hSkinBmp);		
			BitBlt(hdc, 0, 0, IMAGEWIDTH, IMAGEHEIGHT, dcSkin, 0, 0, SRCCOPY);
			DeleteDC(dcSkin);
		EndPaint(hWnd, &ps);
		break;
			case WM_NCRBUTTONDOWN:
		{
			int xPos, yPos;
			HMENU hMenu, hSubMenu;

			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDC_SKOREAMAP));
			hSubMenu = GetSubMenu(hMenu, 0);

			TrackPopupMenu(hSubMenu,
				TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON,
				xPos, yPos, 0, hWnd, NULL);
			break;
		}

	case WM_NCHITTEST:
		{
			wParam = DefWindowProc(hWnd, message, wParam, lParam);
			if (wParam == HTCLIENT)
				return HTCAPTION;
			return wParam;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}
