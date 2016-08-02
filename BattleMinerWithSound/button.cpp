#include "button.h"

Button::Button(){	
}

void Button::mouseState(RawMouse &mouse){

	int x = mouse.GetX();
	int y = mouse.GetY();

	if (mouse.LeftButton() &&
		clickable &&
		size.left < x && x < size.right &&
		size.top < y && y < size.bottom){

		action();
	}
}

void Button::display(LPD3DXSPRITE spt){
	if (visible){
		D3DXVECTOR3 center(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 position((float)size.left, (float)size.top, 1.0f);
		spt->Draw(*sprite, NULL, &center, &position, D3DCOLOR_XRGB(255, 255, 255));
		(*font)->DrawTextA(spt,
					    text.c_str(),
					    text.length(),
					    &size,
					    DT_CENTER | DT_VCENTER | DT_SINGLELINE,
					    color);
	}
}

void Button::setSize(int left, int top, int right, int bottom){
	size.left   = left;
	size.top    = top;
	size.right  = right;
	size.bottom = bottom;
}

