#include "RawMouse.h"

RawMouse::RawMouse(HANDLE handle, int x, int y)
: m_handle(handle), m_x(x), m_y(y){
	SetIgnore(FALSE);
	Hide(FALSE);
	SetSpeedFactor(1.0);
	restricted = FALSE;
}

BOOL RawMouse::Is(RAWINPUT *raw){
	if (m_handle == raw->header.hDevice) return TRUE;
	return FALSE;
}

void RawMouse::SetHandle(RAWINPUT *raw){
	m_handle = raw->header.hDevice;
}

void RawMouse::Update(RAWINPUT *raw){
	if (!IsHidden()){
		m_x += raw->data.mouse.lLastX * speedFactor;
		m_y += raw->data.mouse.lLastY * speedFactor;
		Adjust();
		m_buttons |= raw->data.mouse.usButtonFlags;
	}
}

void RawMouse::Position(int x, int y){
	m_x = x;
	m_y = y;
}

void RawMouse::SetIgnore(BOOL state){
	ignore = state;
}

void RawMouse::NewCycle(){
	SetIgnore(FALSE);
	m_buttons = 0;
}

void RawMouse::Hide(BOOL state){
	hidden = state;
}

void RawMouse::Limit(LPRECT rect){
	limits = *rect;
	restricted = TRUE;
}

void RawMouse::Limit(int left, int upper, int right, int lower){
	SetRect(&limits, left, upper, right, lower);
	restricted = TRUE;
}

void RawMouse::Adjust(){
	if (restricted == TRUE){
		m_x = max(m_x, limits.left);
		m_x = min(m_x, limits.right);
		m_y = max(m_y, limits.top);
		m_y = min(m_y, limits.bottom);
	}
}

void RawMouse::SetSpeedFactor(double factor){
	speedFactor = factor;
}

void RawMouse::AddSpeedFactor(double factor){
	speedFactor += factor;
	speedFactor = max(speedFactor, 0.0);
}