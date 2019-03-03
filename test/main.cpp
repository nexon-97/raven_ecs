#include "App.hpp"

#ifdef OS_WINDOWS
#include <Windows.h>
#endif

#ifdef OS_WINDOWS
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
	App app;

	if (!app.Init())
	{
		return 1;
	}

	int result = app.Run();
	return result;
}
