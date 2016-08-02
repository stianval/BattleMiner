// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <xact3.h>
#include <strsafe.h>
#include "RawMouse.h"
#include "resource.h"
#include "RawInputHandler.h"
#include "menu.h"
#include "menudefs.h"


// define the screen resolution and keyboard macros
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)
#define MAX_MOUSES 8

#define TITLE_SCREEN_MODE 0
#define INSTRUCTIONS_MODE 1
#define MAIN_MODE         2

