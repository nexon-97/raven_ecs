#pragma once
#include "ecs/System.hpp"

class MovementSystem
	: public ecs::System
{
public:
	MovementSystem(ecs::Manager& manager);

	void Update() override;
};
