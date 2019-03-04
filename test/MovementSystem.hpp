#pragma once
#include "ecs/System.hpp"

class MovementSystem
	: public ecs::System
{
public:
	void Update() override;
};
