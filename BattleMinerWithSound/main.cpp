#include "main.h"


//-----------------------------------------------------------------------------
// Forward declaration
//-----------------------------------------------------------------------------
HRESULT PrepareXACT();
LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
VOID CleanupXACT();
//HRESULT FindMediaFileCch( WCHAR* strDestPath, int cchDest, LPCWSTR strFilename );
//bool DoesCommandLineContainAuditionSwitch();


//-----------------------------------------------------------------------------
// Struct to hold audio game state
//-----------------------------------------------------------------------------
struct AUDIO_STATE
{
	IXACT3Engine* pEngine;
	IXACT3WaveBank* pWaveBank;
	IXACT3SoundBank* pSoundBank;
	XACTINDEX iZap;

    VOID* pbWaveBank; // Handle to wave bank data.  Its memory mapped so call UnmapViewOfFile() upon cleanup to release file
    VOID* pbSoundBank; // Pointer to sound bank data.  Call delete on it when the sound bank is destroyed
};

AUDIO_STATE g_audioState;




int mode = TITLE_SCREEN_MODE;

BOOL activated = TRUE;
BOOL board_clickable = TRUE;

#define DEF_BOARD_WIDTH  30
#define DEF_BOARD_HEIGHT 16
#define DEF_MINES        99
#define MAX_BOARD_WIDTH  30
#define MAX_BOARD_HEIGHT 25
#define MAX_MINES        BOARD_WIDTH * BOARD_HEIGHT
int BOARD_WIDTH  = DEF_BOARD_WIDTH;
int BOARD_HEIGHT = DEF_BOARD_HEIGHT;
int MINES        = DEF_MINES;
#define MAX_EMPTY    BOARD_WIDTH * BOARD_HEIGHT - MINES
#define LEFT         ((SCREEN_WIDTH  - BOARD_WIDTH *16)/2)
#define UPPER        ((SCREEN_HEIGHT - BOARD_HEIGHT*16)/2)

#define DEF_GAME_TIME 100
#define MAX_GAME_TIME 999
int GAME_TIME = DEF_GAME_TIME;
int time_gone = 0;
BOOL paused = TRUE;
#define TIME_LEFT    (int)(GAME_TIME - time_gone/1000)
#define TIME_OUT     (TIME_LEFT <= 0)

#define KO_START     5
#define KO_ADD       0

#define STR_NUM_SIZE 10

#define BOMB (-2*(MAX_MOUSES+2))

int empty_left;
int bombs_left;
int board[MAX_BOARD_HEIGHT][MAX_BOARD_WIDTH];
int game_start;

#define BOARD_CLEARED (empty_left == 0 && bombs_left == 0)

RawMouse mouse[MAX_MOUSES];

// include the Direct3D Library file
/*
#pragma comment (lib, "d3d9.lib")
#pragma comment (lib, "d3dx9.lib")
*/

// global declarations
LPDIRECT3D9 d3d;    // the pointer to our Direct3D interface
LPDIRECT3DDEVICE9 d3ddev;    // the pointer to the device class
LPD3DXSPRITE d3dspt;    // the pointer to our Direct3D Sprite interface

// sprite declarations
LPDIRECT3DTEXTURE9 sprite;    // the pointer to the pointer sprite
LPDIRECT3DTEXTURE9 boardSprite; // the pointer to the board sprite
LPDIRECT3DTEXTURE9 menuSprite;
LPDIRECT3DTEXTURE9 titleScreen;
LPDIRECT3DTEXTURE9 instructions;

// font declaration
LPD3DXFONT font_1;    // create a font object
LPD3DXFONT font_2;    // create a big font

// cursor color array
#define SET_CURS_COLOR(x) if (x < MAX_MOUSES) player[x].curs_color

// player declaration
struct Player{
	int KO_time;
	int KO_now;
	int KO_start;
	int wrong_flags;
	int flags;
	D3DXCOLOR curs_color;
} player[MAX_MOUSES];

HWND hWnd2;
HINSTANCE _hInstance;

Menu escMenu;

// function prototypes
BOOL RegisterMouses(HWND hWndTarget); // makes the program listen for mouses
HWND createWindow(HINSTANCE hInstance); // creates the window
void initD3D(HWND hWnd/*, HINSTANCE hInstance*/); // sets up and initializes Direct3D
void render_frame(void); // renders a single frame
void cleanD3D(void); // closes Direct3D and releases memory
void draw_board(void); // draws the board
void clear_board(void); // clears the board
void init_board(int x, int y); // initializes the board
void update_board(void); // updates the board
BOOL open(int x, int y); // opens a tile on the board
BOOL open_all(int x, int y); // opens all tiles around another
BOOL flag(int i, int x, int y); // flags/unflags a tile
BOOL bomb(int x, int y); // check for a bomb on a tile
int  count_bombs(int x, int y); // counts all bombs around a tile
int  check_flags(int x, int y); // checks if there is a flag on a given coordinate
int  count_flags(int x, int y); // count the flags around a tile
BOOL inside(int x, int y); // check if a coordinate is inside the board
void draw_text(void); // displays the text
void draw_mouses(void); // draws the mouses
void init_players(void); // initalizes the players
void init_cursor_colors(void); // initializes the cursor colors
void getKeyInput(HWND hWnd); // gets input from the keyboard
void initMenues(void);
void title_screen(HWND hWnd);
void view_instructions(HWND hWnd);
void main_mode(HWND hWnd);
void viewScreen(LPDIRECT3DTEXTURE9 screen);
void add_time(void);

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){

	HWND hWnd = createWindow(hInstance);
	hWnd2 = hWnd;

    ShowWindow(hWnd, nCmdShow);

    // Prepare to use XACT
    HRESULT hr;
    if( FAILED( hr = PrepareXACT() ) )
    {
        if( hr == HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND ) )
            MessageBox( hWnd, L"Failed to init XACT because media not found.", L"XACT Tutorial", MB_OK );
        else
            MessageBox( hWnd, L"Failed to init XACT.", L"XACT Tutorial", MB_OK );
        CleanupXACT();
        return 0;
    }

    // set up and initialize Direct3D
    initD3D(hWnd/*, hInstance*/);

	clear_board();
	init_cursor_colors();
	initMenues();

	// register raw input from mouses
	if (!RegisterMouses(NULL)){
		return FALSE;
	}
    // enter the main loop:

    MSG msg;

	SetTimer(hWnd, 1, 25, NULL);

    while(TRUE)
    {
        DWORD starting_point = GetTickCount();

		while (TRUE/*(GetTickCount() - starting_point) < 25*/){
			GetMessage(&msg, NULL, 0, 0);

			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			if (DispatchMessage(&msg) == 1) break;
			
		}
		
		if (msg.message == WM_QUIT)
			break;
		
		switch (mode){
		case TITLE_SCREEN_MODE:
			title_screen(hWnd);
			break;
		case INSTRUCTIONS_MODE:	
			view_instructions(hWnd);
			break;
		case MAIN_MODE:	
			main_mode(hWnd);
			break;
		}

		if (g_audioState.pEngine)
			g_audioState.pEngine->DoWork();

		add_time();

		for (int i=0; i<MAX_MOUSES; i++){
			mouse[i].NewCycle();
		}
    }

    // clean up DirectX and COM
    cleanD3D();
	CleanupXACT();

    return msg.wParam;
}

//-----------------------------------------------------------------------------------------
// This function does the following:
//
//      1. Initialize XACT by calling pEngine->Initialize 
//      2. Create the XACT wave bank(s) you want to use
//      3. Create the XACT sound bank(s) you want to use
//      4. Store indices to the XACT cue(s) your game uses
//-----------------------------------------------------------------------------------------
HRESULT PrepareXACT()
{
	
    HRESULT hr;
    WCHAR str[MAX_PATH];
    HANDLE hFile;
    DWORD dwFileSize;
    DWORD dwBytesRead;
    HANDLE hMapFile;

    // Clear struct
    ZeroMemory( &g_audioState, sizeof( AUDIO_STATE ) );

    hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );  // COINIT_APARTMENTTHREADED will work too
    if( SUCCEEDED( hr ) )
    {
        // Switch to auditioning mode based on command line.  Change if desired
        bool bAuditionMode = false;
        bool bDebugMode = false;

        DWORD dwCreationFlags = 0;
        if( bAuditionMode ) dwCreationFlags |= XACT_FLAG_API_AUDITION_MODE;
        if( bDebugMode ) dwCreationFlags |= XACT_FLAG_API_DEBUG_MODE;

        hr = XACT3CreateEngine( dwCreationFlags, &g_audioState.pEngine );
    }
    if( FAILED( hr ) || g_audioState.pEngine == NULL )
        return E_FAIL;

    // Initialize & create the XACT runtime 
    XACT_RUNTIME_PARAMETERS xrParams = {0};
    xrParams.lookAheadTime = XACT_ENGINE_LOOKAHEAD_DEFAULT;
    hr = g_audioState.pEngine->Initialize( &xrParams );
    if( FAILED( hr ) )
        return hr;

	/*
    if( FAILED( hr = FindMediaFileCch( str, MAX_PATH, L"sounds.xwb" ) ) )
        return hr;
	*/

	StringCchCopy(str, MAX_PATH, L"Sound/sounds.xwb");

    // Create an "in memory" XACT wave bank file using memory mapped file IO
    // Memory mapped files tend to be the fastest for most situations assuming you 
    // have enough virtual address space for a full map of the file
    hr = E_FAIL; // assume failure
    hFile = CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
    if( hFile != INVALID_HANDLE_VALUE )
    {
        dwFileSize = GetFileSize( hFile, NULL );
        if( dwFileSize != -1 )
        {
            hMapFile = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, dwFileSize, NULL );
            if( hMapFile )
            {
                g_audioState.pbWaveBank = MapViewOfFile( hMapFile, FILE_MAP_READ, 0, 0, 0 );
                if( g_audioState.pbWaveBank )
                {
                    hr = g_audioState.pEngine->CreateInMemoryWaveBank( g_audioState.pbWaveBank, dwFileSize, 0,
                                                                       0, &g_audioState.pWaveBank );
                }
                CloseHandle( hMapFile ); // pbWaveBank maintains a handle on the file so close this unneeded handle
            }
        }
        CloseHandle( hFile ); // pbWaveBank maintains a handle on the file so close this unneeded handle
    }
    if( FAILED( hr ) )
        return E_FAIL; // CleanupXACT() will cleanup state before exiting

    // Read and register the sound bank file with XACT.  Do not use memory mapped file IO because the 
    // memory needs to be read/write and the working set of sound banks are small.
	/*
    if( FAILED( hr = FindMediaFileCch( str, MAX_PATH, L"sounds.xsb" ) ) )
        return hr;
	*/
	StringCchCopy(str, MAX_PATH, L"Sound/sounds.xsb");
    hr = E_FAIL; // assume failure
    hFile = CreateFile( str, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
    if( hFile != INVALID_HANDLE_VALUE )
    {
        dwFileSize = GetFileSize( hFile, NULL );
        if( dwFileSize != -1 )
        {
            // Allocate the data here and free the data when recieving the sound bank destroyed notification
            g_audioState.pbSoundBank = new BYTE[dwFileSize];
            if( g_audioState.pbSoundBank )
            {
                if( 0 != ReadFile( hFile, g_audioState.pbSoundBank, dwFileSize, &dwBytesRead, NULL ) )
                {
                    hr = g_audioState.pEngine->CreateSoundBank( g_audioState.pbSoundBank, dwFileSize, 0,
                                                                0, &g_audioState.pSoundBank );
                }
            }
        }
        CloseHandle( hFile );
    }
    if( FAILED( hr ) )
        return E_FAIL; // CleanupXACT() will cleanup state before exiting

    // Get the sound cue index from the sound bank
    //
    // Note that if the cue does not exist in the sound bank, the index will be XACTINDEX_INVALID
    // however this is ok especially during development.  The Play or Prepare call will just fail.
    g_audioState.iZap = g_audioState.pSoundBank->GetCueIndex( "CANNON" );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Releases all previously initialized XACT objects
//-----------------------------------------------------------------------------
VOID CleanupXACT()
{
    // Shutdown XACT
    //
    // Note that pEngine->ShutDown is synchronous and will take some time to complete 
    // if there are still playing cues.  Also pEngine->ShutDown() is generally only 
    // called when a game exits and is not the preferred method of changing audio 
    // resources. To know when it is safe to free wave/sound bank data without 
    // shutting down the XACT engine, use the XACTNOTIFICATIONTYPE_SOUNDBANKDESTROYED 
    // or XACTNOTIFICATIONTYPE_WAVEBANKDESTROYED notifications 
    if( g_audioState.pEngine )
    {
        g_audioState.pEngine->ShutDown();
        g_audioState.pEngine->Release();
    }

    if( g_audioState.pbSoundBank )
        delete[] g_audioState.pbSoundBank;
    g_audioState.pbSoundBank = NULL;

    // After pEngine->ShutDown() returns it is safe to release memory mapped files
    if( g_audioState.pbWaveBank )
        UnmapViewOfFile( g_audioState.pbWaveBank );
    g_audioState.pbWaveBank = NULL;

    CoUninitialize();
}

// this is the function that creates the window
HWND createWindow(HINSTANCE hInstance){
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)WindowProc;
    wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINE));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass1";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(NULL,
                          L"WindowClass1",
                          L"BattleMiner",
                          WS_POPUP,
                          0, 0,
                          SCREEN_WIDTH, SCREEN_HEIGHT,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

	_hInstance = hInstance;

	return hWnd;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RAWINPUT* raw;
	BOOL cont = TRUE;
	static RawInputHandler rawTranslator;
    switch(message)
    {
	case WM_INPUT:
		raw = rawTranslator.translate((HRAWINPUT)lParam);
		if (raw == NULL) return 0;

		if (raw->header.dwType == RIM_TYPEMOUSE){
			for (int i=0; i<MAX_MOUSES && cont; i++){
				if (mouse[i].Is(raw)){
					mouse[i].Update(raw);
					if (mouse[i].IsHidden() && empty_left == MAX_EMPTY){
						if (raw->data.mouse.usButtonFlags != 0){
							mouse[i].Hide(FALSE);
							mouse[i].SetIgnore(TRUE);
						}
					}
					cont = FALSE;
				}
				if (!mouse[i].Attached()){
					//MessageBox(NULL, TEXT("Another mouse!"), TEXT("IMPORTANT MESSAGE!"), MB_OK);
					mouse[i].SetHandle(raw);
					mouse[i].Limit(0, 0, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
					mouse[i].Hide(TRUE);
					cont = FALSE;
				}
			}
		}
		return 0;

		case WM_ACTIVATE:
			activated = !activated;
			paused    = !activated;
			if (activated){
				cleanD3D();
				initD3D(hWnd);
			}
			return 0;

        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;

		case WM_TIMER: return 1;
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

// the function making the program listening for mouse input
BOOL RegisterMouses(HWND hWndTarget) { 
	RAWINPUTDEVICE KB[1]; 
	KB[0].usUsagePage = 0x01; 
	KB[0].usUsage = 0x02; // For the mouse
	KB[0].dwFlags = RIDEV_NOLEGACY; 
	KB[0].hwndTarget = hWndTarget; 
	if(RegisterRawInputDevices(KB, 1, sizeof(KB[0])) == FALSE) { 
		//ShowMessage("Error: Mouse Registration Failed!"); 
		return FALSE; 
	}
	
	return TRUE;
}

// this function initializes and prepares Direct3D for use
void initD3D(HWND hWnd/*, HINSTANCE hInstance*/)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);

    D3DPRESENT_PARAMETERS d3dpp;

    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = FALSE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferWidth = SCREEN_WIDTH;
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;

    // create a device class using this information and the info from the d3dpp stuct
    d3d->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                      &d3dpp,
                      &d3ddev);

	ShowCursor(FALSE);

    D3DXCreateSprite(d3ddev, &d3dspt);    // create the Direct3D Sprite object

	/*
	D3DXCreateTextureFromFile(d3ddev, L"art/Pointer.png", &sprite);
	D3DXCreateTextureFromFile(d3ddev, L"art/minetiles.png", &boardSprite); 
	D3DXCreateTextureFromFile(d3ddev, L"art/menu_screen_basic.png", &menuSprite); 
	D3DXCreateTextureFromFile(d3ddev, L"art/titlescreen.png", &titleScreen); 
	D3DXCreateTextureFromFile(d3ddev, L"art/instructions.png", &instructions); 
	*/

	D3DXCreateTextureFromResource(d3ddev, _hInstance, MAKEINTRESOURCE(IDI_POINTER), &sprite);
	D3DXCreateTextureFromResource(d3ddev, _hInstance, MAKEINTRESOURCE(IDI_MINETILES), &boardSprite);
	D3DXCreateTextureFromResource(d3ddev, _hInstance, MAKEINTRESOURCE(IDI_MENU_BG), &menuSprite); 
	D3DXCreateTextureFromResource(d3ddev, _hInstance, MAKEINTRESOURCE(IDI_TITLE_SCREEN), &titleScreen); 
	D3DXCreateTextureFromResource(d3ddev, _hInstance, MAKEINTRESOURCE(IDI_INSTRUCTIONS), &instructions); 

	D3DXCreateFont(d3ddev,    // the Direct3D Device
		           20, 0,    // font size 20 with the default width
			       FW_NORMAL,    // normal font weight
				   1,    // no mipmap levels
				   FALSE,    // not italic
				   DEFAULT_CHARSET,    // default character set
				   OUT_DEFAULT_PRECIS,    // default precision
				   DEFAULT_QUALITY,    // default quality
				   DEFAULT_PITCH || FF_DONTCARE,    // more defaults...
				   L"Arial",    // typeface "Arial"
				   &font_1);    // address of the font object created above

	D3DXCreateFont(d3ddev,    // the Direct3D Device
		           36, 0,    // font size 36 with the default width
			       FW_BOLD,    // bold font weight
				   1,    // no mipmap levels
				   FALSE,    // not italic
				   DEFAULT_CHARSET,    // default character set
				   OUT_DEFAULT_PRECIS,    // default precision
				   DEFAULT_QUALITY,    // default quality
				   DEFAULT_PITCH || FF_DONTCARE,    // more defaults...
				   L"Arial",    // typeface "Arial"
				   &font_2);    // address of the font object created above

    return;
}


// this is the function used to render a single frame
void render_frame(void)
{
    // clear the window to black
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    d3ddev->BeginScene();    // begins the 3D scene
    d3dspt->Begin(D3DXSPRITE_ALPHABLEND);    // begin sprite drawing

	// draw the board
	draw_board();
	// draw the menues
	escMenu.display(d3dspt);
    // draw the mouses
	draw_mouses();

	draw_text();

    d3dspt->End();    // end sprite drawing
    d3ddev->EndScene();    // ends the 3D scene	
    d3ddev->Present(NULL, NULL, NULL, NULL);
}


// this is the function that cleans up Direct3D and COM
void cleanD3D(void)
{

	font_1->Release();
	font_2->Release();
	sprite->Release();

	boardSprite->Release(); 
	menuSprite->Release(); 
	titleScreen->Release();
	instructions->Release();
	d3dspt->Release();
    d3ddev->Release();
    d3d->Release();
}

// this is the function used to draw the board
void draw_board(){
	D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);
	int col_key;
	RECT rect;
	int num;
	for (int y=0; y<BOARD_HEIGHT; y++){
		for (int x=0; x<BOARD_WIDTH; x++){
			D3DXVECTOR3 position((float)(LEFT + x*16), (float)(UPPER + y*16), 0.0f);
			num = board[y][x];
			if (num < 0){
				SetRect(&rect, 0, 0, 15, 15); // light brown back
				d3dspt->Draw(boardSprite, &rect, &center, &position, D3DCOLOR_XRGB(255, 255, 255));

				if (num == BOMB || (TIME_LEFT <= 0 && num == -2)){
					SetRect(&rect, 48, 0, 63, 15); // bomb
					d3dspt->Draw(boardSprite, &rect, &center, &position, D3DCOLOR_XRGB(255, 255, 255));
				} 
				
				else if (num < -2){
					SetRect(&rect, 16, 0, 31, 15); // flag
					col_key = (-(num+3))>>1;
					d3dspt->Draw(boardSprite, &rect, &center, &position, player[col_key].curs_color);

					if (TIME_LEFT <= 0 && (num & 1) == 1){
						SetRect(&rect, 32, 0, 47, 15); // cross
						d3dspt->Draw(boardSprite, &rect, &center, &position, D3DCOLOR_XRGB(255, 255, 255));						
					}				
				}
			} else{
				num += 3;
				SetRect(&rect, (num%4)*16, (num/4)*16+16, (num%4)*16+15, (num/4)*16+31); // number
				d3dspt->Draw(boardSprite, &rect, &center, &position, D3DCOLOR_XRGB(255, 255, 255));
			}
		}
	}
}
// this is the function to clear the board
void clear_board(){
	init_players();
	empty_left = MAX_EMPTY;
	bombs_left = MINES;
	time_gone = 0;
	paused = TRUE;
	for (int y=0; y<MAX_BOARD_HEIGHT; y++){
		for (int x=0; x<MAX_BOARD_WIDTH; x++){
			board[y][x] = -1;
		}
	}
}
// this is the function to initialize the board
void init_board(int x, int y){
	srand(GetTickCount());
	int posx, posy;
	int now_mines = 0;
	paused = FALSE;

	while (now_mines < MINES){
		posx = rand() % BOARD_WIDTH;
		posy = rand() % BOARD_HEIGHT;
		if (abs(x-posx)>1 || abs(y-posy)>1){
			if (!bomb(posx, posy)){
				board[posy][posx] -= 1;
				now_mines++;
			}
		}
	}
}
// this is the function that updates the board
void update_board(){
	static BOOL ignoremouse[MAX_MOUSES];
	if (TIME_LEFT > 0 && board_clickable){
		for (int i=0; i<MAX_MOUSES; i++){
			if (!mouse[i].Ignore() && !(player[i].KO_now > 0)){
				int x = mouse[i].GetX();
				int y = mouse[i].GetY();
				x = (x-LEFT)/16;
				y = (y-UPPER)/16;
				if (0<=x && x<BOARD_WIDTH && 0<=y && y<BOARD_HEIGHT){
					if (mouse[i].LeftButton()){
						if(!open(x, y)) {
							g_audioState.pSoundBank->Play( g_audioState.iZap, 0, 0, NULL );
							player[i].KO_now = player[i].KO_time*1000;
							player[i].KO_time += KO_ADD;
							player[i].KO_start = GetTickCount();
						}
						mouse[i].SetIgnore(TRUE);
					}
					if (mouse[i].RightButton()){
						if(!flag(i, x, y)) {
							g_audioState.pSoundBank->Play( g_audioState.iZap, 0, 0, NULL );
							player[i].KO_now = player[i].KO_time*1000;
							player[i].KO_time += KO_ADD;
							player[i].KO_start = GetTickCount();
						}
						mouse[i].SetIgnore(TRUE);
					}
				}
			} 
		}
	}
}

// this is the function to open a tile
BOOL open(int x, int y){
	BOOL survive = TRUE;
	if (empty_left == BOARD_WIDTH * BOARD_HEIGHT - MINES) init_board(x, y);
	if (board[y][x] == -1){
		empty_left--;
		int mines = count_bombs(x, y);
		board[y][x] = mines;
		if (mines == 0){
			open_all(x, y);
		}	
	} else if (board[y][x] == -2){
		survive = FALSE;
		board[y][x] = BOMB;
		bombs_left--;
	}
	return survive;
}
// this is the function that open all tiles around an other
BOOL open_all(int x, int y){
	BOOL survive = TRUE;
	for (int x2=x-1; x2<=x+1; x2++){
		for (int y2=y-1; y2<=y+1; y2++){
			if (inside(x2, y2)){
				if (board[y2][x2] == -1 || board[y2][x2] == -2){
					int s = open(x2, y2);
					survive = survive ? s : FALSE;
				}
			}
		}
	}
	return survive;
}
// this is the function to flag a tile
BOOL flag(int i, int x, int y){
	BOOL survive = TRUE; 
	int num = board[y][x];
	if (num<0){
		if (num == -1 || num == -2){
			if (num == -1) player[i].wrong_flags++;
			player[i].flags++;
			board[y][x] = num - (i+1)*2;
			bombs_left--;
		} else {
			num = num + (i+1)*2;
			if (num == -1 || num == -2){
				if (num == -1) player[i].wrong_flags--;
				player[i].flags--;
				board[y][x] = num;
				bombs_left++;
			}
		}
	}
	else if (count_flags(x, y) == board[y][x]){
		survive = open_all(x, y);
	}
	return survive;
}
// this is the function to check for a bomb on a tile
BOOL bomb(int x, int y){
	int num = board[y][x];
	return ((num<0) && (num & 1) == 0);
}
// this is the function to count the bombs around a tile
int  count_bombs(int x, int y){
	int mines = 0;
	for (int x2=x-1; x2<=x+1; x2++){
		for (int y2=y-1; y2<=y+1; y2++){
			if (inside(x2, y2)){
				if (bomb(x2,y2)){
					mines++;
				}
			}
		}
	}
	return mines;
}
// this is the function that counts the flags
int  check_flags(int x, int y){
	return board[y][x] < -2;
}
// this is the function that counts the flags
int  count_flags(int x, int y){
	int flags = 0;
	for (int x2=x-1; x2<=x+1; x2++){
		for (int y2=y-1; y2<=y+1; y2++){
			if (inside(x2, y2)){
				if (check_flags(x2,y2)){
					flags++;
				}
			}
		}
	}
	return flags;
}
// this is the function to check if coordinates are inside the board
BOOL inside(int x, int y){
	return (0<=x && x<BOARD_WIDTH && 0<=y && y<BOARD_HEIGHT);
}

// this is the function that shows the text
void draw_text(){
	RECT textbox;

	static int time_left = 0;

	// display the time
    SetRect(&textbox, 0, UPPER+BOARD_HEIGHT*16, SCREEN_WIDTH/2-1, SCREEN_HEIGHT-1);
	if (empty_left == MAX_EMPTY) game_start = GetTickCount();
	if (!BOARD_CLEARED) time_left = TIME_LEFT;
	time_left = max(time_left, 0);


    static char strNumber[STR_NUM_SIZE];
	
	
	WCHAR textTimeLeft[STR_NUM_SIZE];
	wsprintf(textTimeLeft, L"Time: %d", time_left);
	font_1->DrawText(d3dspt,
                     textTimeLeft,
                     -1,
                     &textbox,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                     D3DCOLOR_XRGB(255, 255, 255));


	/*
	_itoa_s(time_left, strNumber, STR_NUM_SIZE);
	font_1->DrawTextA(NULL,
                     (LPCSTR)&strNumber,
                     strlen((LPCSTR) &strNumber),
                     &textbox,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                     D3DCOLOR_XRGB(255, 255, 255));
	*/

	// display mines left
	WCHAR textMinesLeft[STR_NUM_SIZE+2];
	wsprintf(textMinesLeft, L"Mines: %d", bombs_left);
	SetRect(&textbox, SCREEN_WIDTH/2, UPPER+BOARD_HEIGHT*16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
	font_1->DrawText(d3dspt,
                     textMinesLeft,
                     -1,
                     &textbox,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                     D3DCOLOR_XRGB(255, 255, 255));

	/*
	// display bombs left
    SetRect(&textbox, SCREEN_WIDTH/2, UPPER+BOARD_HEIGHT*16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
	_itoa_s(bombs_left, strNumber, STR_NUM_SIZE);
	font_1->DrawTextA(NULL,
                     (LPCSTR)&strNumber,
                     strlen((LPCSTR) &strNumber),
                     &textbox,
                     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
                     D3DCOLOR_XRGB(255, 255, 255));
	 */

	if (time_left == 0){
		SetRect(&textbox, 0, 0, SCREEN_WIDTH-1, UPPER-1);
		font_2->DrawText(d3dspt,
						 L"Time's up!",
						 11,
						 &textbox,
					     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
						 D3DCOLOR_XRGB(255, 0, 0));
	}
	if (BOARD_CLEARED){
		SetRect(&textbox, 0, 0, SCREEN_WIDTH-1, UPPER-1);
		font_2->DrawText(d3dspt,
						 L"Board cleared!",
						 15,
						 &textbox,
					     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
						 D3DCOLOR_XRGB(255, 0, 0));
	}
	if (time_left == 0 || BOARD_CLEARED){
		SetRect(&textbox, 0, UPPER+BOARD_HEIGHT*16, SCREEN_WIDTH-1, SCREEN_HEIGHT-1);
		font_1->DrawText(d3dspt,
						 L"Press Space",
						 12,
						 &textbox,
					     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
						 D3DCOLOR_XRGB(255, 255, 255));
	}



	for (int i=0; i<MAX_MOUSES; i++){
		if (!mouse[i].IsHidden()){
			SetRect(&textbox, 0, i*SCREEN_HEIGHT/MAX_MOUSES, LEFT-1, (i+1)*SCREEN_HEIGHT/MAX_MOUSES-1);
			if (time_left == 0 || BOARD_CLEARED){
				_itoa_s(player[i].flags-3*player[i].wrong_flags, strNumber, STR_NUM_SIZE);
			}else{
				_itoa_s((player[i].KO_now+999)/1000, strNumber, STR_NUM_SIZE);
			}
			font_1->DrawTextA(d3dspt,
				             (LPCSTR)&strNumber,
							 strlen((LPCSTR) &strNumber),
							 &textbox,
						     DT_CENTER | DT_VCENTER | DT_SINGLELINE,
							 player[i].curs_color);
		}
	}
}
// this is the function that draws the mouses
void draw_mouses(){
	for (int i=0; i<MAX_MOUSES; i++){
		if (!mouse[i].IsHidden()){
			D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);    // center at the upper-left corner
			D3DXVECTOR3 position((float)mouse[i].GetX(), (float)mouse[i].GetY(), 0.0f);
			D3DCOLOR color = (player[i].KO_now==0) ? player[i].curs_color : D3DCOLOR_XRGB(0, 0, 0);
			d3dspt->Draw(sprite, NULL, &center, &position, color);
		}
	}
}

// this is the function that initializes the players
void init_players(){
	for (int i=0; i<MAX_MOUSES; i++){
		player[i].KO_time     = KO_START;
		player[i].KO_start    = 0;
		player[i].KO_now      = 0;
		player[i].wrong_flags = 0;
		player[i].flags       = 0;

		mouse[i].Hide(TRUE);
	}
}
// this is the function that initializes the cursor colors
void init_cursor_colors(){
	SET_CURS_COLOR(0) = D3DCOLOR_XRGB(255, 0, 0);
	SET_CURS_COLOR(1) = D3DCOLOR_XRGB(0, 255, 0);
	SET_CURS_COLOR(2) = D3DCOLOR_XRGB(0, 0, 255);
	SET_CURS_COLOR(3) = D3DCOLOR_XRGB(0, 255, 255);
	SET_CURS_COLOR(4) = D3DCOLOR_XRGB(255, 255, 0);
	SET_CURS_COLOR(5) = D3DCOLOR_XRGB(255, 100, 0);
	SET_CURS_COLOR(6) = D3DCOLOR_XRGB(150, 0, 150);
	SET_CURS_COLOR(7) = D3DCOLOR_XRGB(200, 150, 50);
}

// the function that gets input from the keyboard
void getKeyInput(HWND hWnd){
	// check the 'escape' key
	if (KEY_DOWN(VK_ESCAPE)){
		// PostMessage(hWnd, WM_DESTROY, 0, 0);
		paused = true;
		board_clickable = false;
		escMenu.setVisible(TRUE);
	}

	if (KEY_DOWN(VK_RETURN)){
		clear_board();
	}

	// check the 'space' key
	if (KEY_DOWN(VK_SPACE)){
		if (TIME_LEFT <= 0 || BOARD_CLEARED)
			clear_board();
	}

	if (empty_left == MAX_EMPTY){
		if (KEY_DOWN(VK_SHIFT)){
			if (KEY_DOWN(VK_UP)){
				MINES++;
				if (MAX_EMPTY < 9) MINES--;
				empty_left = MAX_EMPTY;	
			}
			if (KEY_DOWN(VK_DOWN)){
				MINES--;
				MINES = max(MINES, 0);
				empty_left = MAX_EMPTY;
			}
			bombs_left = MINES;
		}
		else if (KEY_DOWN(VK_CONTROL)){
			if (KEY_DOWN(VK_UP)){
				GAME_TIME++;
				GAME_TIME = min(GAME_TIME, MAX_GAME_TIME);
			}
			if (KEY_DOWN(VK_DOWN)){
				GAME_TIME--;
				GAME_TIME = max(GAME_TIME, 0);
			}
		}
		else{
			if (KEY_DOWN(VK_RIGHT)){
				BOARD_WIDTH++;
				BOARD_WIDTH = min(BOARD_WIDTH, MAX_BOARD_WIDTH);
				empty_left = MAX_EMPTY;
			}
			if (KEY_DOWN(VK_DOWN)){
				BOARD_HEIGHT++;
				BOARD_HEIGHT = min(BOARD_HEIGHT, MAX_BOARD_HEIGHT);
				empty_left = MAX_EMPTY;
			}
			if (KEY_DOWN(VK_LEFT)){
				BOARD_WIDTH--;
				if (MAX_EMPTY < 9) BOARD_WIDTH++;
				empty_left = MAX_EMPTY;
			}
			if (KEY_DOWN(VK_UP)){
				BOARD_HEIGHT--;
				if (MAX_EMPTY < 9) BOARD_HEIGHT++;
				empty_left = MAX_EMPTY;
			}
		}
		if (KEY_DOWN(VK_BACK)){
			GAME_TIME    = DEF_GAME_TIME;
			BOARD_HEIGHT = DEF_BOARD_HEIGHT;
			BOARD_WIDTH  = DEF_BOARD_WIDTH;
			MINES        = DEF_MINES;
			bombs_left   = MINES;
			empty_left   = MAX_EMPTY;
		}
	}
}

// the function that initializes the menues
void initMenues(void){
	createEscMenu();
}


void title_screen(HWND hWnd){
	viewScreen(titleScreen);
	if (KEY_DOWN(VK_SPACE)) mode = INSTRUCTIONS_MODE;
	for (int i=0; i<MAX_MOUSES; i++){
		if (mouse[i].Attached() && mouse[i].LeftButton()){
			mode = INSTRUCTIONS_MODE;
		}
	}
}
void view_instructions(HWND hWnd){
	viewScreen(instructions);
	if (KEY_DOWN(VK_SPACE)) mode = MAIN_MODE;
	for (int i=0; i<MAX_MOUSES; i++){
		if (mouse[i].Attached() && mouse[i].LeftButton()){
			mode = MAIN_MODE;
		}
	}
/*
	if (mode == MAIN_MODE){
		board_clickable = TRUE;
		if (empty_left != MAX_EMPTY){
			paused = FALSE;
		}
	}
*/
}
void main_mode(HWND hWnd){
	getKeyInput(hWnd);
	escMenu.performButtonActons();

	update_board();
    render_frame();

	for (int i=0; i<MAX_MOUSES; i++){
		if (player[i].KO_now > 0){
			int now_time = GetTickCount();
			if (paused) player[i].KO_start = now_time;
			player[i].KO_now -= (now_time - player[i].KO_start);
			player[i].KO_start = now_time;
		}
		player[i].KO_now = max(player[i].KO_now, 0);
	}
}

void viewScreen(LPDIRECT3DTEXTURE9 screen){
	// clear the window to black
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();    // begins the 3D scene
	
	d3dspt->Begin(D3DXSPRITE_ALPHABLEND);    // begin sprite drawing

	RECT source;
	SetRect(&source, 0, 0, 640, 480);

	D3DXVECTOR3 center(0, 0, 0);
	D3DXVECTOR3 position(0, 0, 0);

	d3dspt->Draw(screen, NULL, &center, &position, D3DCOLOR_XRGB(255, 255, 255)); 

	d3dspt->End();
	d3ddev->EndScene();
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

void add_time(void){
	static int last_time;
	int now_time = GetTickCount();

	if (paused) last_time = now_time;
	
	time_gone += now_time - last_time;
	
	last_time = now_time;
}