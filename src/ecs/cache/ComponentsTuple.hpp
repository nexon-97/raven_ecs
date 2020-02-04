#pragma once
#include "ecs/detail/Types.hpp"
#include "ecs/component/ComponentPtr.hpp"

namespace ecs
{

struct ECS_API ComponentsTuple
{
	ComponentsTuple() = delete;
	ComponentsTuple(const std::size_t tupleSize);
	~ComponentsTuple();

	// Copy is disabled
	ComponentsTuple(const ComponentsTuple&) = delete;
	ComponentsTuple& operator=(const ComponentsTuple&) = delete;

	// Move is enabled
	ComponentsTuple(ComponentsTuple&& other);
	ComponentsTuple& operator=(ComponentsTuple&& other);

	const ComponentPtr* GetData() const;
	const std::size_t GetSize() const;

	ComponentPtr& operator[](const std::size_t index);
	const ComponentPtr& operator[](const std::size_t index) const;

private:
	ComponentPtr* m_data;
	std::size_t m_size;
};

}
