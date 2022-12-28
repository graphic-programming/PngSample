//
//  This is a program to read PNG files.
//     

#include "stdafx.h"
#include "pngsample.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

unsigned int x, y, width, height, scaledwidth, scaledheight, bitcount;
int color_type;
png_byte* buffer;
BITMAPINFOHEADER bih;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

bool OpenPngFile(wchar_t* szFile);

void OnPaint(HWND hWnd);
void OnCreate(HWND hWnd);
void OnDestroy(HWND hWnd);
void OnSize(HWND hWnd, int cx, int cy);

void OnFileOpen(HWND hWnd);
void OnFileExit(HWND hWnd);

void OnHelpAbout(HWND hWnd, HINSTANCE hInst);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PNGSAMPLE, szWindowClass, MAX_LOADSTRING);

	WNDCLASSEXW wcex;

	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.style         = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc   = WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PNGSAMPLE));
	wcex.hCursor       = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName  = MAKEINTRESOURCEW(IDC_PNGSAMPLE);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm       = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassExW(&wcex);

    // save the instance handle in a global variable
	hInst = hInstance;

	// create the main program window
	int X, Y, nWidth, nHeight, Cx, Cy;

    // 480p: 854x480

    Cx = 854;
    Cy = 480;

    nWidth  = Cx + 16;
    nHeight = Cy + 58;

    X = (GetSystemMetrics(SM_CXSCREEN) - nWidth)/2;
    Y = (GetSystemMetrics(SM_CYSCREEN) - nHeight)/2;

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		                      X, Y,
		                      nWidth, nHeight,
		                      nullptr, nullptr, hInstance, nullptr);

	if (!hWnd) return FALSE;

	// display the main program window
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PNGSAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//  Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
			case IDM_OPEN:  OnFileOpen(hWnd);						break;
			case IDM_EXIT:  OnFileExit(hWnd);						break;
			case IDM_ABOUT: OnHelpAbout(hWnd, hInst);				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_PAINT:   OnPaint(hWnd);									break;
	case WM_CREATE:  OnCreate(hWnd);								break;
	case WM_DESTROY: OnDestroy(hWnd);								break;
	case WM_SIZE:    OnSize(hWnd, LOWORD (lParam), HIWORD (lParam));break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// A description on how to use and modify libpng
// http://www.libpng.org/pub/png/libpng-1.0.3-manual.html
bool OpenPngFile(wchar_t* szFile)
{
	errno_t err;
	FILE* fp;
	bool result = true;
	int is_not_png;
	const unsigned int number = 8;
	png_byte header[number];
	png_bytep* row_pointers;
	int rowbytes;
	png_infop end_info;
	png_structp png_ptr;
	png_infop info_ptr;

	result = true;

	// open file for reading
	if( (err = _wfopen_s(&fp, szFile, L"rb")) != 0 ) return false;

	// check if a file is a PNG file
	if(fread(header, 1, number, fp) != number) return false;

	is_not_png = png_sig_cmp(header, 0, number);

	if(is_not_png) {
		result = false;
		goto Close_File;
	}

	// allocate and initialize png_struct
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr)
    {
		result = false;
		goto Close_File;
	}

	// allocate and initialize png_info
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		result = false;
		goto Close_File;
    }

    end_info = png_create_info_struct(png_ptr);
    if (!end_info) {
        png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		result = false;
		goto Close_File;
    }

	// set up error handling
	if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		result = false;
		goto Close_File;
    }

	// set up the input code
	png_init_io(png_ptr, fp);

	// tell libpng that we already read a file
	png_set_sig_bytes(png_ptr, number);

	// read all the file information up to the actual image data
	png_read_info(png_ptr, info_ptr);
	
    // get the information from the info_ptr
	int bit_depth;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	// bit_depth - holds the bit depth of one of the image channels
	// read only 8 bitdepth image file
    if (bit_depth != 8) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        result = false;
		goto Close_File;
    }
	
    // color_type - describes which color/alpha channels
    switch(color_type)
    {
    case PNG_COLOR_TYPE_RGB:
        bitcount = 24;
        break;
    case PNG_COLOR_TYPE_RGB_ALPHA:
        bitcount = 32;
        break;
    default:
		bitcount = 0;
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
        result = false;
		goto Close_File;
    }

	// color_type - describes which color/alpha channels
	// color type can be either RGB or RGBA
	if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
		png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        result = false;
		goto Close_File;
	}
	
	// PNG files store 3-color pixels in red, green, blue order.
	// Windows bitmap store 3-color pixels in blue, green, red order.
	// This code changes the storage of the pixels to blue, green, red:
	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		png_set_bgr(png_ptr);

	// rowbytes - number of bytes needed to hold a row
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // make it 4-byte aligned
	int rem;
	rem = (rowbytes % 4);
	rowbytes += (rem > 0 ? (4 - rem) : 0);

    // allocate memory for the image
	if(buffer != NULL) delete[] buffer;
	buffer = new png_byte[rowbytes * height];
	if(buffer == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        result = false;
		goto Close_File;
    }

    // row_pointers is for pointing to image_data for reading the png with libpng
	row_pointers = new png_bytep [height];
    if (row_pointers == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
        result = false;
		goto Close_File;
    }

	// set the individual row_pointers to point at the correct offsets of image data
	unsigned int i;
	for (i = 0; i < height; i++)
		row_pointers[height - 1 - i] = buffer + i * rowbytes;

	// read the whole image
    png_read_image(png_ptr, row_pointers);

    // free all memory
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	delete[] row_pointers;

Close_File:

   // close file
	fclose(fp);

	return result;
}

//
void OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hDC;
    HDC hdcMem;
	HBITMAP hBitmap;

    hDC = BeginPaint(hWnd, &ps);

	// creates a bitmap compatible with the device hDC
	hBitmap = CreateCompatibleBitmap(hDC, width, height);

	// sets the pixels in a compatible bitmap hBitmap using the color data found in the bih
	SetDIBits(hDC, hBitmap, 0, (UINT)height, buffer, (BITMAPINFO *)&bih, DIB_RGB_COLORS);

	// creates a memory device context
	hdcMem = CreateCompatibleDC(NULL);

	// select bitmap into memory device context
	SelectObject(hdcMem,hBitmap);

	// sets the bitmap stretching mode in the specified device context.
	SetStretchBltMode(hDC,HALFTONE);

	// copy a memory device context into display device context
	StretchBlt(hDC, x, y, scaledwidth, scaledheight, hdcMem, 0, 0,  width,  height, SRCCOPY);

	//
	//BitBlt(hDC, x, y, width, height, hdcMem, 0, 0, SRCCOPY);

	// release resources
	DeleteObject(hdcMem);
	DeleteObject(hBitmap);

    EndPaint(hWnd, &ps);
}

//
void OnCreate(HWND hWnd)
{
	buffer   = NULL;
	width    = height = 256;
	color_type = 0;
	
    bih.biSize          = sizeof(BITMAPINFOHEADER);
    bih.biWidth         = width;
    bih.biHeight        = height;
    bih.biPlanes        = 1;
	bih.biBitCount      = 24;
    bih.biCompression   = BI_RGB;
    bih.biSizeImage     = 0;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed       = 0;
    bih.biClrImportant  = 0;
}

//
void OnDestroy(HWND hWnd)
{
	if(buffer != NULL) delete[] buffer;

	PostQuitMessage(0); // close the program.
}

// fit the image inside the window
void OnSize(HWND hWnd, int cx, int cy)
{
	scaledheight = (png_uint_32)cy;
	scaledwidth  = (int)((float)scaledheight * ((float)width / (float)height));
	x = (cx - scaledwidth) / 2;
	y = 0;

	if(scaledwidth > (png_uint_32)cx)
	{
		scaledwidth  = (png_uint_32)cx;
		scaledheight =  (int)((float)scaledwidth * ((float)height / (float)width));
		x = 0;
		y = (cy - scaledheight) / 2;
	}
}

//
void OnFileOpen(HWND hWnd)
{
	OPENFILENAME fn;
	TCHAR szFile[MAX_PATH] = _T("");

	ZeroMemory(&fn, sizeof(OPENFILENAME));

	fn.lStructSize = sizeof(OPENFILENAME);
	fn.hwndOwner = hWnd;
	fn.hInstance = hInst;
	fn.lpstrFilter = _T("PNG Files\0*.png\0All Files\0*.*\0");
	fn.nFilterIndex = 0;
	fn.lpstrFile = szFile;
	fn.nMaxFile = MAX_PATH;
	fn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&fn)) return;
	
	if(!OpenPngFile(szFile))
	{
		OutputDebugStringA("Error openning file.");
		return;
	}

	switch(color_type)
	{
	case PNG_COLOR_TYPE_RGB:       OutputDebugStringA("PNG_COLOR_TYPE_RGB\n"); break;
	case PNG_COLOR_TYPE_RGB_ALPHA: OutputDebugStringA("PNG_COLOR_TYPE_RGB_ALPHA\n"); break;
	}
	
	wchar_t szText[MAX_PATH];
	swprintf_s(szText, MAX_PATH, L"BIT_COUNT :%10d\n", bitcount);
	OutputDebugString(szText);
	
    bih.biWidth         = width;
    bih.biHeight        = height;
	bih.biBitCount      = bitcount;

	// baguhin ang window name
	wcscpy_s(szText, MAX_PATH, szTitle);
	wcscat_s(szText, MAX_PATH, L" - ");
	wcscat_s(szText, MAX_PATH, szFile);
	SetWindowText(hWnd, szText);
	
	// fit image inside window
	WPARAM wparam;
	LPARAM lparam;
	RECT rect;

	GetClientRect(hWnd, &rect);
	wparam = 0;
	lparam = rect.bottom * 0x00010000 + rect.right;
	SendMessage(hWnd,WM_SIZE, wparam, lparam);

	InvalidateRect(hWnd, NULL, TRUE);
}

//
void OnFileExit(HWND hWnd)
{
	DestroyWindow(hWnd);
}

//
void OnHelpAbout(HWND hWnd, HINSTANCE hInst)
{
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
}