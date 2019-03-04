#pragma once
#include <SDL.h>
#include "ecs/Manager.hpp"

#include <chrono>

class App
{
public:
	App();
	~App();

	bool Init();
	int Run();

	void UpdateLoop();

	ecs::Manager& GetECSManager();
	SDL_Renderer* GetRenderer() const;
	float GetTimeDelta() const;

	static App* GetInstance();

private:
	static App* s_instance;

private:
	ecs::Manager m_ecsManager;
	SDL_Window* m_sdlWindow = nullptr;
	SDL_Renderer* m_sdlRenderer = nullptr;
	std::chrono::high_resolution_clock::time_point m_frameTimerPoint;
	long long m_frameNanosecondsElapsed = 0;
	int m_fpsValue = 0;
	std::string m_debugFPSString;
	float m_timeDelta = 0.f;
	bool m_sdlInitialized = false;
	bool m_wantQuit = false;
};
