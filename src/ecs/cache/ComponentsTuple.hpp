#pragma once
#include "ecs/detail/Types.hpp"
#include "ecs/component/ComponentPtr.hpp"

#include <tuple>

namespace ecs
{

template <typename ...ComponentTypes>
using ComponentTupleT = std::tuple<TComponentPtr<ComponentTypes>...>;

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

	// Conversion utils
	template <typename ...ComponentTypes>
	ComponentTupleT<ComponentTypes...> GetTyped()
	{
		using TupleT = ComponentTupleT<ComponentTypes...>;
		return PopulateTuple<TupleT>();
	}

	template <typename Tuple, std::size_t N = std::tuple_size<Tuple>::value, typename Indices = std::make_index_sequence<N>>
	Tuple PopulateTuple()
	{
		return PopulateTupleImpl<Tuple>(Indices{});
	}

	template <typename Tuple, std::size_t... I>
	Tuple PopulateTupleImpl(std::index_sequence<I...>)
	{
		return Tuple(m_data[I]...);
	}

private:
	ComponentPtr* m_data;
	std::size_t m_size;
};

}
