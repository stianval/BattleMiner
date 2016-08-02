#include "menudefs.h"

extern LPD3DXSPRITE d3dspt;    // the pointer to our Direct3D Sprite interface
extern LPD3DXFONT font_1; // small font
extern HWND hWnd2;
extern RawMouse mouse[MAX_MOUSES];
extern Menu escMenu;
extern BOOL paused;
extern BOOL board_clickable;
extern LPDIRECT3DTEXTURE9 menuSprite;
extern int mode;

void quit_action(void);
void resume_action(void);
void instruct_action(void);

void createEscMenu(){
	Button button_resume;
	button_resume.setSize(204, 49, 435, 148);
	button_resume.setAction(resume_action);
	button_resume.setVisible(FALSE);
	button_resume.setClickable(TRUE);

	Button button_instruct;
	button_instruct.setSize(207, 171, 438, 270);
	button_instruct.setAction(instruct_action);
	button_instruct.setVisible(FALSE);
	button_instruct.setClickable(TRUE);

	Button button_quit;
	button_quit.setSize(208, 294, 439, 393);
	button_quit.setAction(quit_action);
	button_quit.setVisible(FALSE);
	button_quit.setClickable(TRUE);

	escMenu.setBackground(&menuSprite);
	escMenu.addButton(button_resume);
	escMenu.addButton(button_instruct);
	escMenu.addButton(button_quit);
	for (int i = 0; i<MAX_MOUSES; i++){
		escMenu.addMouse(mouse[i]);
	}
	escMenu.setVisible(FALSE);
}

void quit_action(){
	PostMessage(hWnd2, WM_DESTROY, 0, 0);
}

void instruct_action(void){
	mode = INSTRUCTIONS_MODE;
}

void resume_action(){
	paused = FALSE;
	board_clickable = TRUE;
	escMenu.setVisible(FALSE);
}