#pragma once
#include "ecs/System.hpp"

class RenderSystem
	: public ecs::System
{
public:
	RenderSystem() = default;

	void Init() override;
	void Update() override;
};
