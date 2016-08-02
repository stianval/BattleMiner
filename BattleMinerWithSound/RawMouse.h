#pragma once

#include <windows.h>

class RawMouse{
protected:
	HANDLE m_handle;
	double m_x;
	double m_y;
	double speedFactor;
	int m_buttons;
	BOOL ignore;
	BOOL hidden;
	BOOL restricted;
	RECT limits; // where the mouse can be
	void Adjust(void);

public:
	RawMouse(HANDLE handle=0, int x=0, int y=0);

	BOOL Is(RAWINPUT* raw);
	void SetHandle(RAWINPUT* raw);
	void Update(RAWINPUT* raw);
	void Position(int x, int y);
	void SetIgnore(BOOL state);
	void NewCycle(void);
	void Hide(BOOL state);
	void Limit(LPRECT rect);
	void Limit(int left, int upper, int right, int lower);
	void SetSpeedFactor(double factor);
	void AddSpeedFactor(double factor);

	BOOL Attached(void){return m_handle != 0;}
	int GetX(void){return (int) m_x;}
	int GetY(void){return (int) m_y;}
	int ButtonState(void){return m_buttons;}
	BOOL LeftButton(void){return ((RI_MOUSE_LEFT_BUTTON_DOWN & m_buttons) == RI_MOUSE_LEFT_BUTTON_DOWN);}
	BOOL RightButton(void){return ((RI_MOUSE_RIGHT_BUTTON_DOWN & m_buttons) == RI_MOUSE_RIGHT_BUTTON_DOWN);}
	BOOL Ignore(void){return ignore;}
	BOOL IsHidden(void){return hidden;};
};
