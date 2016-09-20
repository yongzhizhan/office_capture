#include "main.h"
#include "export.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD     fdwReason, LPVOID    lpvReserved)
{
	return true;
}

int main()
{
	char buf[256];
	getCaptureImage("C:\\Users\\AtomView\\Desktop\\1.doc", buf, 256);
	
	return 0;
}