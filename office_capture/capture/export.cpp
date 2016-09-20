#include "main.h"
#include "export.h"
#include "capture.h"

typedef std::tr1::shared_ptr<Capture> CapturePtr;

CapturePtr getCaptureInstance()
{
	static CapturePtr gCapture;
	if(0 == gCapture)
		gCapture.reset(new Capture());
		
	return gCapture;
}

bool getCaptureImage(const char* vSrcPath, char* vDestPath, int vSize)
{
	CapturePtr tCapture = getCaptureInstance();	
	return tCapture->getCaptureImage(vSrcPath, vDestPath, vSize);
}