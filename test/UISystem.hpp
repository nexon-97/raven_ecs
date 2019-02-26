#pragma once
#include "ecs/System.hpp"

class UISystem
	: public ecs::System
{
public:
	UISystem() = default;

	void Init() override
	{
		std::cout << "UISystem Init" << std::endl;
	}

	void Destroy() override
	{

	}
};
