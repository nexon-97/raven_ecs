#include <iostream>

#include "ecs/Manager.hpp"
#include "test/UISystem.hpp"
#include "test/StaticMesh.hpp"

int main()
{
	ecs::Manager manager;

	manager.RegisterSystem<UISystem>();
	manager.RegisterComponentType<StaticMesh>();

	manager.Init();

	// Fill all required space
	ecs::ComponentHandle handles[100];
	StaticMesh* meshes[100];
	for (int i = 0; i < 100; ++i)
	{
		handles[i] = manager.CreateComponent<StaticMesh>(meshes[i]);
		meshes[i]->id = i;
	}

	meshes[5]->colorA = 25.f;
	meshes[5]->colorX = 35.f;
	meshes[5]->colorY = 45.f;
	meshes[5]->colorZ = 56.f;

	manager.DestroyComponent(handles[50]);
	manager.DestroyComponent(handles[51]);
	manager.CreateComponent<StaticMesh>();
	manager.CreateComponent<StaticMesh>();
	manager.CreateComponent<StaticMesh>();
	manager.DestroyComponent(handles[0]);
	manager.DestroyComponent(handles[1]);
	manager.DestroyComponent(handles[2]);

	auto testMesh = manager.GetComponent<StaticMesh>(handles[25]);

	// Go through test iterations
	for (int i = 0; i < 1; ++i)
	{
		std::cout << "ECS iteration start" << std::endl;

		auto it = manager.GetComponentsIterator<StaticMesh>();

		/*auto end = manager.GetComponentEndIterator<StaticMesh>();
		int j = 0;
		for (auto it = manager.GetComponentsIterator<StaticMesh>(); it != end; ++it)
		{
			std::cout << "Static mesh iteration " << j << std::endl;
			++j;
		}*/

		manager.Update();
		manager.Render();

		std::cout << "ECS iteration finish" << std::endl;
	}

	std::cout << "===============================" << std::endl << "Finished." << std::endl;

	manager.Destroy();

	system("pause");
	return 0;
}
