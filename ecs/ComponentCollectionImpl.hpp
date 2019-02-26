#pragma once
#include "IComponentCollection.hpp"

#include <set>
#include <cassert>
#include <array>

namespace ecs
{

template <typename ComponentType, std::size_t CollectionSize>
class ComponentCollectionImpl
	: public IComponentCollection
{
	using StorageType = std::array<ComponentType, CollectionSize>;
	using CollectionType = ComponentCollectionImpl<ComponentType, CollectionSize>;

public:
	~ComponentCollectionImpl()
	{
		for (std::size_t i = 0; i < m_usedSpace; ++i)
		{
			if (m_holesList.find(i) == m_holesList.end())
			{
				m_storage[i].~ComponentType();
			}
		}
	}

	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = ComponentType;
		using pointer = ComponentType*;
		using reference = ComponentType&;
		using difference_type = std::ptrdiff_t;

		iterator() = default;
		iterator(const CollectionType* collection, typename StorageType::iterator curr, std::size_t index)
			: collection(collection)
			, curr(curr)
			, index(index)
		{}

		reference operator*()
		{
			return *curr;
		}

		pointer operator->()
		{
			return &**this;
		}

		iterator& operator++()
		{
			std::size_t nextIndex = collection->GetNextAliveIndex(index);
			curr += (nextIndex - index);
			return *this;
		}

		iterator operator++(int)
		{
			const auto temp(*this); ++*this; return temp;
		}

		bool operator==(const iterator& other) const
		{
			return curr == other.curr;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		const CollectionType* collection;
		typename StorageType::iterator curr;
		std::size_t index;
	};

	std::pair<std::size_t, bool> TryCreate() override
	{
		std::pair<std::size_t, bool> result(0U, false);

		// If there are holes - try to fill them first to keep data close to each other
		if (!m_holesList.empty())
		{
			// Pick first hole to fill
			auto startIt = m_holesList.begin();
			std::size_t insertPos = *startIt;
			m_holesList.erase(startIt);

			result.first = insertPos;
			result.second = true;
		}
		else if (m_usedSpace < CollectionSize)
		{
			// If not all space is allocated - put new element at the end
			result.first = m_usedSpace;
			result.second = true;
			++m_usedSpace;
		}

		if (result.second)
		{
			new (&m_storage[result.first]) ComponentType();

			// Overwrite first alive index for iterator
			if (result.first < m_firstAliveIndex)
			{
				m_firstAliveIndex = result.first;
			}
		}

		return result;
	}

	void Destroy(const std::size_t index) override
	{
		assert(index < CollectionSize);

		auto insertResult = m_holesList.insert(index);
		if (insertResult.second)
		{
			m_storage[index].~ComponentType();

			if (index == m_firstAliveIndex)
			{
				m_firstAliveIndex = GetNextAliveIndex(index);
			}
		}
	}

	void* Get(const std::size_t index) override
	{
		assert(index < CollectionSize);
		return &m_storage[index];
	}

	bool IsFull() const override
	{
		return m_usedSpace == CollectionSize && m_holesList.empty();
	}

	std::size_t GetFirstAliveIndex() const
	{
		return m_firstAliveIndex;
	}

	std::size_t GetNextAliveIndex(const std::size_t from) const
	{
		for (std::size_t i = from + 1; i < m_usedSpace; ++i)
		{
			if (m_holesList.find(i) == m_holesList.end())
			{
				return i;
			}
		}

		return CollectionSize;
	}

	iterator begin()
	{
		auto firstIndex = GetFirstAliveIndex();
		auto arrayIt = (firstIndex < CollectionSize) ? m_storage.begin() + firstIndex : m_storage.end();
		return iterator(this, arrayIt, firstIndex);
	}

	iterator end()
	{
		return iterator(this, m_storage.end(), CollectionSize);
	}

private:
	StorageType m_storage;
	std::size_t m_usedSpace = 0U;
	std::size_t m_firstAliveIndex = 0U;
	std::set<std::size_t> m_holesList;
};

} // namespace ecs
