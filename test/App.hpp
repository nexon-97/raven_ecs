#pragma once
#include <SDL.h>
#include "ecs/Manager.hpp"

class App
{
public:
	App();
	~App();

	bool Init();
	int Run();

	void UpdateLoop();

	ecs::Manager& GetECSManager();

	static App* GetInstance();

private:
	static App* s_instance;

private:
	ecs::Manager m_ecsManager;
	SDL_Window* m_sdlWindow = nullptr;
	SDL_Renderer* m_sdlRenderer = nullptr;
	bool m_sdlInitialized = false;
	bool m_wantQuit = false;
};
