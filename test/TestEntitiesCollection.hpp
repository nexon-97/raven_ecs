#pragma once
#include <ecs/Manager.hpp>

namespace test
{

class TestEntitiesCollection
	: public ecs::EntitiesCollection
{
public:
	TestEntitiesCollection(ecs::Manager& manager)
		: ecs::EntitiesCollection(manager)
	{}

public:
	using EntitiesCollection::GetEntitiesData;
};

} // namespace test
