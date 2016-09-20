#ifndef CAPTURE_H
#define CAPTURE_H

#include "main.h"

class Capture{
public:
	Capture();
	bool getCaptureImage(const char* vSrcPath, char* vPath, int vSize);
	
private:
	HWND getMainWindow(char* vTitle);
	string getPathFileName(const char* vPath);
	bool captureAndSave(const HWND& hWnd, int nBitCount, const char* szFilePath);
};

#endif