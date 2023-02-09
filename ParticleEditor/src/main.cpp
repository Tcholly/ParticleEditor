#include <Difu/ScreenManagement/ScreenManager.h>
#include "Screens/MainScreen.h"
#include <Difu/WindowManagement/WindowManager.h>
#include <raylib.h>

int main()
{
	if (WindowManager::InitWindow("Chess", 800, 480, true))
	{
		SetTargetFPS(60);
		ScreenManager::ChangeScreen(MainScreen::GetScreen());
		WindowManager::RunWindow();
	}
}
