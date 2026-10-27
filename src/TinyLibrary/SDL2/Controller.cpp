#if defined(SDL2)

#include <stdio.h>
#include "Controller.hpp"
#include "joykey.h"

Controller* Controller::controller = NULL;

Controller::Controller(void)
:button(0)
,touch(false)
{
	joykey_init();
}

Controller::~Controller(void)
{
}

void Controller::Initialize(void)
{
	Controller::controller = new Controller;
}

void Controller::Finalize(void)
{
	if(Controller::controller != NULL)
	{
		delete Controller::controller;
		Controller::controller = NULL;
	}
}

Controller& Controller::GetInstance()
{
	return *Controller::controller;
}

void Controller::Reset(void)
{
	this->button = 0;
}

void Controller::Update(void)
{
	unsigned int raw = joykey_read();
	unsigned int newButton = 0;
	if(raw & (1u << JK_UP))    newButton |= BUTTON_UP;
	if(raw & (1u << JK_DOWN))  newButton |= BUTTON_DOWN;
	if(raw & (1u << JK_LEFT))  newButton |= BUTTON_LEFT;
	if(raw & (1u << JK_RIGHT)) newButton |= BUTTON_RIGHT;
	if(raw & (1u << JK_A))     newButton |= BUTTON_1;
	if(raw & (1u << JK_B))     newButton |= BUTTON_2;
	if(raw & (1u << JK_L1))    newButton |= BUTTON_L;
	if(raw & (1u << JK_R1))    newButton |= BUTTON_R;
	if(raw & (1u << JK_START)) newButton |= BUTTON_START;
	if(raw & (1u << JK_SELECT)) newButton |= BUTTON_SELECT;
	if((newButton & (BUTTON_START | BUTTON_SELECT)) == (BUTTON_START | BUTTON_SELECT))
	{
		newButton |= BUTTON_QUIT;
	}
	this->button = newButton;
}

unsigned int Controller::GetButton(void)
{
	return this->button;
}

#endif
