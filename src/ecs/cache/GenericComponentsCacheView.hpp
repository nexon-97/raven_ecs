#pragma once
#include "ecs/cache/ComponentsTupleCache.hpp"

namespace ecs
{

class GenericComponentsCacheView
{
public:
	GenericComponentsCacheView() = delete;
	GenericComponentsCacheView(ComponentsTupleCache* inCache)
		: m_cache(inCache)
	{}

	using CollectionT = std::unordered_map<EntityId, ComponentsTuple>;

	// Collection iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = ComponentsTuple;
		using pointer = ComponentsTuple*;
		using reference = ComponentsTuple&;

		iterator() = default;
		iterator(CollectionT::iterator internalIterator)
			: internalIterator(internalIterator)
		{}

		reference operator*()
		{
			return internalIterator->second;
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

		CollectionT::iterator internalIterator;
	};

	iterator begin()
	{
		if (nullptr != m_cache)
		{
			return iterator(m_cache->GetData().begin());
		}

		return iterator();
	}

	iterator end()
	{
		if (nullptr != m_cache)
		{
			return iterator(m_cache->GetData().end());
		}

		return iterator();
	}

private:
	ComponentsTupleCache* m_cache;
};

}
