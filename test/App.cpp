#include "App.hpp"

#include "ecs/Manager.hpp"
#include "test/RenderSystem.hpp"
#include "test/StaticMesh.hpp"
#include "test/Transform.hpp"
#include "test/SpriteRender.hpp"

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
	if (SDL_CreateWindowAndRenderer(1300, 600, SDL_WINDOW_SHOWN | SDL_RENDERER_ACCELERATED, &m_sdlWindow, &m_sdlRenderer) < 0)
		return false;

	// Init ECS manager
	m_ecsManager.RegisterSystem<RenderSystem>();
	m_ecsManager.RegisterComponentType<StaticMesh>("StaticMesh");
	m_ecsManager.RegisterComponentType<Transform>("Transform");
	m_ecsManager.RegisterComponentType<SpriteRender>("SpriteRender");

	m_ecsManager.Init();

	//Create a thousand entities
	for (int i = 0; i < 1000; ++i)
	{
		m_ecsManager.GetEntitiesCollection().CreateEntity();
	}

	// Fill all required space
	/*ecs::ComponentHandle handles[103];
	StaticMesh* meshes[103];
	for (int i = 0; i < 100; ++i)
	{
		handles[i] = manager.CreateComponent<StaticMesh>(meshes[i]);
		meshes[i]->id = i;
	}

	manager.DestroyComponent(handles[50]);
	manager.DestroyComponent(handles[51]);
	manager.CreateComponent<StaticMesh>(meshes[100]);
	manager.CreateComponent<StaticMesh>(meshes[101]);
	manager.CreateComponent<StaticMesh>(meshes[102]);
	meshes[100]->id = 100;
	meshes[101]->id = 101;
	meshes[102]->id = 102;
	manager.DestroyComponent(handles[0]);
	manager.DestroyComponent(handles[1]);
	manager.DestroyComponent(handles[2]);

	auto testMesh = manager.GetComponent<StaticMesh>(handles[25]);
	testMesh->id = 2500;*/

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

	//m_renderQueue->Render();

	SDL_RenderPresent(m_sdlRenderer);
}

ecs::Manager& App::GetECSManager()
{
	return m_ecsManager;
}

App* App::GetInstance()
{
	return s_instance;
}
