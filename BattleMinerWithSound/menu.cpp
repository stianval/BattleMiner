#include "menu.h"

Menu::Menu(){
	bgset = FALSE;
}

void Menu::display(LPD3DXSPRITE spt){
	if (visible){
		if (bgset){
			RECT source;
			SetRect(&source, 0, 0, 640, 480);

			D3DXVECTOR3 center(0, 0, 0);
			D3DXVECTOR3 position(0, 0, 0);
	
			spt->Draw(*background, NULL, &center, &position, D3DCOLOR_XRGB(255, 255, 255));
		}
		for (UINT i=0; i<buttons.size(); i++){
			buttons[i].display(spt);
		}
	}
}

void Menu::performButtonActons(){
	if (visible){
		for (UINT i=0; i<mouses.size(); i++){
			for (UINT j=0; j<buttons.size(); j++){
				buttons[j].mouseState(*mouses[i]);
			}
		}
	}
}