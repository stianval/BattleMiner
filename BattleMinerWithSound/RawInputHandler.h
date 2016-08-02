#pragma once

#include <windows.h>

class RawInputHandler {
private:
	LPBYTE lpb;
	int lastDwSize;
	BOOL init;

public:
	RAWINPUT * translate(HRAWINPUT lParam);
	RawInputHandler();
	~RawInputHandler();
};
