#pragma once
#include <string>
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "RawMouse.h"

class Button{
private:
	void (*action)(void);
	BOOL visible;
	BOOL clickable;
	RECT size;
	LPDIRECT3DTEXTURE9 *sprite;
	LPD3DXFONT *font;
	D3DCOLOR color;
	std::string text;

public:
	Button(void);
	void mouseState(RawMouse &mouse);
	void display(LPD3DXSPRITE spt);
	void setAction(void (*newAction)(void)) { action = newAction; }
	void setVisible(BOOL v) { visible = v; }
	void setClickable(BOOL c) { clickable = c; }
	void setSize(LPRECT rect) { size = *rect; }
	void setSize(int left, int top, int right, int bottom);
	void setSprite(LPDIRECT3DTEXTURE9 *s) { sprite = s; }
	void setFont(LPD3DXFONT *f){ font = f; }
	void setText(std::string t){ text = t; }
	void setColor(D3DCOLOR c) { color = c; }
};
