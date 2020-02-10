#pragma once
#include "ecs/cache/ComponentsTupleCache.hpp"

namespace ecs
{
	
template <class ...ComponentTypes>
class TypedComponentsCacheView
{
public:
	TypedComponentsCacheView() = delete;
	TypedComponentsCacheView(ComponentsTupleCache* inCache)
		: m_cache(inCache)
	{}

	using CollectionT = std::unordered_map<EntityId, ComponentsTuple>;
	using StdComponentsTupleT = std::tuple<TComponentPtr<ComponentTypes>...>;

	// Collection iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = StdComponentsTupleT;
		using pointer = StdComponentsTupleT*;
		using reference = StdComponentsTupleT&;

		iterator() = default;
		iterator(CollectionT::iterator internalIterator)
			: internalIterator(internalIterator)
		{}

		value_type operator*()
		{
			ComponentsTuple& cachedTuple = internalIterator->second;
			StdComponentsTupleT outTuple = cachedTuple.GetTyped<ComponentTypes...>();
			return outTuple;
		}

		pointer operator->()
		{
			return &**this;
		}

		iterator& operator++()
		{
			internalIterator++;
			return *this;
		}

		iterator operator++(int)
		{
			const auto temp(*this); ++*this; return temp;
		}

		bool operator==(const iterator& other) const
		{
			return internalIterator == other.internalIterator;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		template <typename T>
		TComponentPtr<T> ConvertToTypedPtr(const std::size_t i)
		{
			const ComponentsTuple& cachedTuple = internalIterator->second;
			return TComponentPtr<T>(cachedTuple[i]);
		}

		CollectionT::iterator internalIterator;
	};

	iterator begin()
	{
		if (m_cache)
		{
			return iterator(m_cache->GetData().begin());
		}
		else
		{
			return iterator();
		}
	}

	iterator end()
	{
		if (m_cache)
		{
			return iterator(m_cache->GetData().end());
		}
		else
		{
			return iterator();
		}
	}

private:
	ComponentsTupleCache* m_cache;
};

}
