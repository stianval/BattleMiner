#include "RawInputHandler.h"

RawInputHandler::RawInputHandler() {
	lastDwSize=0;
	init=FALSE;
}

RawInputHandler::~RawInputHandler() {
	if(init) delete[] lpb;
}

RAWINPUT * RawInputHandler::translate(HRAWINPUT lParam)
{
	UINT dwSize;

	GetRawInputData(lParam, RID_INPUT, NULL, &dwSize, 
                sizeof(RAWINPUTHEADER));
	if(!init || (dwSize != lastDwSize)) {
		if(init) delete[] lpb;
		lpb = new BYTE[dwSize];
		lastDwSize = dwSize;
		init = TRUE;
	}

	if (lpb == NULL){
		return NULL;
	}

	if (GetRawInputData(lParam, RID_INPUT, lpb, &dwSize, 
						sizeof(RAWINPUTHEADER)) != dwSize )
			MessageBox(NULL, TEXT("ERROR!"), TEXT("Didn't read right number of bytes from raw input!"), MB_OK);

	return (RAWINPUT*)lpb;
}