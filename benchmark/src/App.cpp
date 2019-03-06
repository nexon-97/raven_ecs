#include "App.hpp"

#include "ecs/Manager.hpp"
#include "RenderSystem.hpp"
#include "MovementSystem.hpp"
#include "Transform.hpp"
#include "SpriteRender.hpp"
#include "MovementBehavior.hpp"

#include <ctime>
#include <cstdlib>

#ifdef OS_WINDOWS
#include <Windows.h>
#endif

App* App::s_instance = nullptr;

App::App()
{
	s_instance = this;
}

App::~App()
{
	m_ecsManager.Destroy();

	if (m_sdlInitialized)
	{
		SDL_DestroyRenderer(m_sdlRenderer);
		SDL_DestroyWindow(m_sdlWindow);
		SDL_Quit();
	}
}

bool App::Init()
{
	// Init SDL2
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
		return false;

	m_sdlInitialized = true;
	if (SDL_CreateWindowAndRenderer(1800, 900, SDL_WINDOW_SHOWN | SDL_RENDERER_ACCELERATED, &m_sdlWindow, &m_sdlRenderer) < 0)
		return false;

	srand(static_cast<unsigned int>(time(nullptr)));

	// Init ECS manager
	m_ecsManager.RegisterSystem<MovementSystem>();
	m_ecsManager.RegisterSystem<RenderSystem>();
	m_ecsManager.RegisterComponentType<MovementBehavior>("MovementBehavior");
	m_ecsManager.RegisterComponentType<Transform>("Transform");
	m_ecsManager.RegisterComponentType<SpriteRender>("SpriteRender");

	m_ecsManager.Init();

	return true;
}

int App::Run()
{
	while (!m_wantQuit)
	{
		UpdateLoop();
	}

	return 0;
}

void App::UpdateLoop()
{
	m_frameTimerPoint = std::chrono::high_resolution_clock::now();

	SDL_Event e;
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			m_wantQuit = true;
			break;
		}
	}

	SDL_SetRenderTarget(m_sdlRenderer, NULL);
	SDL_SetRenderDrawColor(m_sdlRenderer, 50, 50, 50, 255);
	SDL_RenderClear(m_sdlRenderer);

	m_ecsManager.Update();

	SDL_RenderPresent(m_sdlRenderer);

	// Measure FPS
	auto timePointEnd = std::chrono::high_resolution_clock::now();
	long long timeDeltaNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timePointEnd - m_frameTimerPoint).count();
	m_frameNanosecondsElapsed += timeDeltaNanoseconds;
	std::swap(m_frameTimerPoint, timePointEnd);
	m_fpsValue++;

	m_timeDelta = timeDeltaNanoseconds * 1e-9f;

	if (m_frameNanosecondsElapsed > 1000000000)
	{
		m_frameNanosecondsElapsed %= 1000000000;

#ifdef OS_WINDOWS
		m_debugFPSString = "FPS: " + std::to_string(m_fpsValue) + "\n";
		OutputDebugStringA(m_debugFPSString.c_str());
#endif

		m_fpsValue = 0;
	}
}

ecs::Manager& App::GetECSManager()
{
	return m_ecsManager;
}

SDL_Renderer* App::GetRenderer() const
{
	return m_sdlRenderer;
}

float App::GetTimeDelta() const
{
	return m_timeDelta;
}

App* App::GetInstance()
{
	return s_instance;
}
