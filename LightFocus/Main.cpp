#include "stdafx.h"
#include "LightFocus.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WPARAM returnCode;

	// Create application
	LightFocus *application = new LightFocus();

	// Init application
	application->init(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

	// Run
	returnCode = application->run();

	// Clean up
	delete application;

	return returnCode;
}