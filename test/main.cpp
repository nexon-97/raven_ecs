#include <iostream>

#include "ecs/Manager.hpp"
#include "test/UISystem.hpp"
#include "test/StaticMesh.hpp"

#include "function/FunctionFactory.hpp"

int testFreeFunction(const int testVar1, std::string& testVar2)
{
	testVar2 += "Lambda called!";
	return testVar1 + 5;
}

class TestClass
{
public:
	int TestMethod(const int testVar1, std::string& testVar2)
	{
		testVar2 += "Memfun called!";
		return testVar1 + 15;
	}
};

int main()
{
	ecs::Manager manager;

	manager.RegisterSystem<UISystem>();
	manager.RegisterComponentType<StaticMesh>("StaticMesh");

	manager.Init();

	// Fill all required space
	ecs::ComponentHandle handles[103];
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
	testMesh->id = 2500;

	// Go through test iterations
	for (int i = 0; i < 1; ++i)
	{
		std::cout << "ECS iteration start" << std::endl;

		auto& staticMeshes = *manager.GetComponentCollection<StaticMesh>();
		for (const auto& mesh : staticMeshes)
		{
			std::cout << "Iterating static mesh [" << "]" << std::endl;
		}

		manager.Update();
		manager.Render();

		std::cout << "ECS iteration finish" << std::endl;
	}

	std::cout << "===============================" << std::endl << "Finished." << std::endl;

	manager.Destroy();

	system("pause");
	return 0;
}
