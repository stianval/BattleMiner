#pragma once

#include <vector>
#include <windows.h>
#include <windowsx.h>
#include <d3d9.h>
#include <d3dx9.h>
#include "RawMouse.h"
#include "button.h"

class Menu{
private:
	std::vector<Button> buttons;
	std::vector<RawMouse*> mouses;
	BOOL visible;
	LPDIRECT3DTEXTURE9 *background;
	BOOL bgset;
public:
	Menu(void);
	void setBackground(LPDIRECT3DTEXTURE9 *s) { background = s; bgset = TRUE; }
	void display(LPD3DXSPRITE spt);
	void performButtonActons(void);
	void addButton(Button &b) { buttons.push_back(b); }
	void addMouse(RawMouse &m) { mouses.push_back(&m); }
	void setVisible(BOOL v) { visible = v; }
};