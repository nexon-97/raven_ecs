#pragma once
#include "ecs/System.hpp"

class RenderSystem
	: public ecs::System
{
public:
	RenderSystem(ecs::Manager& manager);

	void Init() override;
	void Update() override;
};
