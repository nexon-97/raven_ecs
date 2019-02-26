#include <iostream>
#include "Manager.hpp"

#include "test_systems/UISystem.hpp"
#include "test_components/StaticMesh.hpp"

int main()
{
	ecs::Manager manager;

	manager.RegisterSystem<UISystem>();
	manager.RegisterComponentType<StaticMesh>();

	manager.Init();

	// Fill all required space
	ecs::ComponentHandle handles[1024];
	StaticMesh* meshes[1024];
	for (int i = 0; i < 1024; ++i)
	{
		handles[i] = manager.CreateComponent<StaticMesh>(meshes[i]);
	}

	meshes[5]->colorA = 25.f;
	meshes[5]->colorX = 35.f;
	meshes[5]->colorY = 45.f;
	meshes[5]->colorZ = 56.f;

	manager.DestroyComponent(handles[5]);
	handles[5] = manager.CreateComponent<StaticMesh>(meshes[5]);

	// Go through test 1000 iterations
	for (int i = 0; i < 1000; ++i)
	{
		std::cout << "ECS iteration start" << std::endl;

		manager.Update();
		manager.Render();

		std::cout << "ECS iteration finish" << std::endl;
	}

	std::cout << "===============================" << std::endl << "Finished." << std::endl;

	manager.Destroy();

	system("pause");
	return 0;
}
