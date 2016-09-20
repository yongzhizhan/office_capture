#include "capture.h"

struct handle_data {
	string title;
	HWND best_handle;
};

BOOL is_main_window(HWND handle)
{   
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	char tTitle[256] = {0};
	if(0 == GetWindowText(handle, tTitle, sizeof(tTitle)))
		return TRUE;
		
	char buf[1024] = {0};
	sprintf(buf, "handle:%p, title:%s\n", handle, tTitle);
	OutputDebugStringA(buf);
	
	if (0 != strncmp(tTitle, data.title.c_str(), data.title.length()) || !is_main_window(handle)) {
		return TRUE;
	}
	
	data.best_handle = handle;
	return FALSE;   
}

HWND Capture::getMainWindow(char* vTitle)
{
	handle_data data;
	data.title = vTitle;
	data.best_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.best_handle;
}

Capture::Capture()
{
	srand(time(0));
}

string Capture::getPathFileName(const char* vPath)
{
	char* tSplitChar = strrchr((char*)vPath, '\\');
	if(0 == tSplitChar)
		return vPath;

	return (tSplitChar + 1);
}

bool Capture::getCaptureImage(const char* vSrcPath, char* vDestPath, int vSize)
{
	string tExePath = "\"D:\\LibreOffice 5\\program\\swriter.exe\"";
	string tCmdLine = " --nologo --quickstart --view ";
	tCmdLine = tCmdLine + "\"" + vSrcPath + "\"";
	
	//tExePath += tCmdLine;
	
	string tTitle = getPathFileName(vSrcPath);
	
	printf("command line:%s", tExePath.c_str());
	
	/*STARTUPINFO si = { sizeof(STARTUPINFO) };
	si.cb = sizeof(si);
	PROCESS_INFORMATION pi;
	if(false == CreateProcess(0, (char*)tExePath.c_str(), 0, 0, 0, 0, 0, 0, &si, &pi)){
		int tError = GetLastError();
		return false;
	}
	*/
	
	ShellExecute(0, "open", (char*)tExePath.c_str(), (char*)tCmdLine.c_str(), "", SW_SHOW);
		
	static long tIndex = 0;
	InterlockedIncrement(&tIndex);
		
	char buf[1024] = {0};
	sprintf(buf, "D:\\test_data\\%s_%d.png", getPathFileName(vSrcPath).c_str(), tIndex);
	
	HWND tHwnd = 0;
	long tStart = GetTickCount();
	for(;;){
		if(GetTickCount() - tStart >= 20 * 1000)
			return false;
			
		tHwnd = getMainWindow((char*)tTitle.c_str());
		if(0 != tHwnd)
			break;
			
		Sleep(1);
	}	
	
	//HWND tHwnd = (HWND)0x005F0762;
	string tSavePath = buf;
	strncpy(vDestPath, tSavePath.c_str(), vSize);
	
	LONG style = GetWindowLong(tHwnd, GWL_STYLE);
	style = style & ~WS_BORDER & ~ WS_THICKFRAME & ~WS_SYSMENU & ~WS_CAPTION | WS_POPUP;
	SetWindowLong(tHwnd, GWL_STYLE, style);
	
	//SetWindowPos(tHwnd, HWND_TOP, 0, 0, 1000, 1000, SWP_SHOWWINDOW);
	MoveWindow(tHwnd, -2000, 0, 1000, 4000, false);
	UpdateWindow(tHwnd);
	Sleep(500);
	
	return captureAndSave(tHwnd, 32, tSavePath.c_str());
}

bool Capture::captureAndSave(const HWND& hWnd, int nBitCount, const char* szFilePath)
{
	if(!szFilePath || !strlen(szFilePath))
	{
		printf("bad function arguments\n");
		return false;
	}

	//calculate the number of color indexes in the color table
	int nColorTableEntries = -1;
	switch(nBitCount)
	{
	case 1:
		nColorTableEntries = 2;
		break;
	case 4:
		nColorTableEntries = 16;
		break;
	case 8:
		nColorTableEntries = 256;
		break;
	case 16:
	case 24:
	case 32:
		nColorTableEntries = 0;
		break;
	default:
		nColorTableEntries = -1;
		break;
	}

	if(nColorTableEntries == -1)
	{
		printf("bad bits-per-pixel argument\n");
		return false;
	}

	HDC hDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hDC);

	int nWidth = 0;
	int nHeight = 0;

	if(hWnd != HWND_DESKTOP)
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		nWidth = rect.right - rect.left;
		nHeight = rect.bottom - rect.top;
	}
	else
	{
		nWidth = ::GetSystemMetrics(SM_CXSCREEN);
		nHeight = ::GetSystemMetrics(SM_CYSCREEN);
	}

	//fix image
	int x = 0;
	int y = 100;
	nWidth -= 80;
	nHeight -= 100 + y;
	
	HBITMAP hBMP = CreateCompatibleBitmap(hDC, nWidth, nHeight);
	SelectObject(hMemDC, hBMP);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hDC, x, y, SRCCOPY);

	int nStructLength = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries;
	LPBITMAPINFOHEADER lpBitmapInfoHeader = (LPBITMAPINFOHEADER)new char[nStructLength];
	::ZeroMemory(lpBitmapInfoHeader, nStructLength);

	lpBitmapInfoHeader->biSize = sizeof(BITMAPINFOHEADER);
	lpBitmapInfoHeader->biWidth = nWidth;
	lpBitmapInfoHeader->biHeight = nHeight;
	lpBitmapInfoHeader->biPlanes = 1;
	lpBitmapInfoHeader->biBitCount = nBitCount;
	lpBitmapInfoHeader->biCompression = BI_RGB;
	lpBitmapInfoHeader->biXPelsPerMeter = 0;
	lpBitmapInfoHeader->biYPelsPerMeter = 0;
	lpBitmapInfoHeader->biClrUsed = nColorTableEntries;
	lpBitmapInfoHeader->biClrImportant = nColorTableEntries;

	DWORD dwBytes = ((DWORD) nWidth * nBitCount) / 32;
	if(((DWORD) nWidth * nBitCount) % 32) {
		dwBytes++;
	}
	dwBytes *= 4;

	DWORD dwSizeImage = dwBytes * nHeight;
	lpBitmapInfoHeader->biSizeImage = dwSizeImage;

	LPBYTE lpDibBits = 0;
	HBITMAP hBitmap = ::CreateDIBSection(hMemDC, (LPBITMAPINFO)lpBitmapInfoHeader, DIB_RGB_COLORS,  (void**)&lpDibBits, NULL, 0);
	SelectObject(hMemDC, hBitmap);
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hDC, x, y, SRCCOPY);
	ReleaseDC(hWnd, hDC);

	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4d42;  // 'BM'
	int nHeaderSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries;
	bmfh.bfSize = 0;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColorTableEntries;

	FILE *pFile = 0;
	fopen_s(&pFile, szFilePath, "wb");
	if(!pFile)
	{
		::DeleteObject(hBMP);
		::DeleteObject(hBitmap);
		delete[]lpBitmapInfoHeader;
		printf("can not open file\n");
		return false;
	}

	DWORD nColorTableSize = 0;
	if (nBitCount != 24)
		nColorTableSize = (1 << nBitCount) * sizeof(RGBQUAD);
	else
		nColorTableSize = 0;


	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, pFile);
	fwrite(lpBitmapInfoHeader, nHeaderSize,1,pFile);

	if(nBitCount < 16)
	{
		int nBytesWritten = 0;
		RGBQUAD *rgbTable = new RGBQUAD[nColorTableEntries * sizeof(RGBQUAD)];
		//fill RGBQUAD table and write it in file
		for(int i = 0; i < nColorTableEntries; ++i)
		{
			rgbTable[i].rgbRed = rgbTable[i].rgbGreen = rgbTable[i].rgbBlue = i;
			rgbTable[i].rgbReserved = 0;

			fwrite(&rgbTable[i], sizeof(RGBQUAD), 1, pFile);
		}
		delete[]rgbTable;

		/*
		RGBQUAD rgb;
		for (DWORD i = 0; i < nColorTableEntries ; i++)
		{
		rgb.rgbBlue = rgb.rgbGreen = rgb.rgbRed = (BYTE)(i*(255/(nColorTableEntries-1)));
		nBytesWritten = fwrite(&rgb, 1, sizeof(rgb), pFile);
		if (nBytesWritten != sizeof(rgb))
		{
		printf("error while writing rgb header\n");
		fclose(pFile);

		::DeleteObject(hBMP);
		::DeleteObject(hBitmap);
		delete[]lpBitmapInfoHeader;

		return false;
		}
		}
		*/
	}

	fwrite(lpDibBits, dwSizeImage, 1, pFile);

	fclose(pFile);

	::DeleteObject(hBMP);
	::DeleteObject(hBitmap);
	delete[]lpBitmapInfoHeader;
	
	return true;
}