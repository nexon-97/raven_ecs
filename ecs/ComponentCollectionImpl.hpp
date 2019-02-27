#pragma once
#include "IComponentCollection.hpp"

#include <set>
#include <cassert>
#include <array>

namespace ecs
{

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	using StorageType = std::array<ComponentType, 1024>;
	using CollectionType = ComponentCollectionImpl<ComponentType>;

public:
	~ComponentCollectionImpl()
	{
		for (auto& chunk : m_chunks)
		{
			for (std::size_t i = 0; i < chunk.usedSpace; ++i)
			{
				if (chunk.holesList.find(i) == chunk.holesList.end())
				{
					chunk.data[i].~ComponentType();
				}
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

	std::size_t Create() override
	{
		std::size_t insertedIndex = 0;

		// If there are holes - fill them first to keep data close to each other
		if (!m_holesList.empty())
		{
			// Pick first hole to fill
			auto startIt = m_holesList.begin();
			insertedIndex = *startIt;
			m_holesList.erase(startIt);
		}
		else if (m_usedSpace < CollectionSize)
		{
			// If not all space is allocated - put new element at the end
			insertedIndex = m_usedSpace;
			++m_usedSpace;
		}

		new (&m_storage[insertedIndex]) ComponentType();

		// Overwrite first alive index for iterator
		if (insertedIndex < m_firstAliveIndex)
		{
			m_firstAliveIndex = result.first;
		}

		return insertedIndex;
	}

	void Destroy(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto& chunk = m_chunks[chunkId.first];
		auto insertResult = chunk.holesList.insert(chunkId.second);
		if (insertResult.second)
		{
			chunk.data[chunkId.second].~ComponentType();

			// Overwrite first alive index
			if (chunkId.second == chunk.firstAliveIndex)
			{
				chunk.firstAliveIndex = GetNextAliveIndex(chunkId.second);
			}
		}
	}

	void* Get(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto& chunk = m_chunks[chunkId.first];
		return &chunk.data[chunkId.second];
	}

	std::size_t GetNextAliveIndex(const std::size_t from) const
	{
		auto chunkId = SplitObjectId(from);

		for (std::size_t i = chunkId.first; i < m_chunks.size(); ++i)
		{
			for (std::size_t j = from + 1; i < m_usedSpace; ++i)
			{
				if (m_holesList.find(i) == m_holesList.end())
				{
					return i;
				}
			}
		}

		return static_cast<std::size_t>(-1);
	}

	iterator begin()
	{
		auto arrayIt = (m_firstAliveIndex < CollectionSize) ? m_storage.begin() + m_firstAliveIndex : m_storage.end();
		return iterator(this, arrayIt, m_firstAliveIndex);
	}

	iterator end()
	{
		return iterator(this, m_storage.end(), CollectionSize);
	}

private:
	std::pair<std::size_t, std::size_t> SplitObjectId(const std::size_t id)
	{
		return std::make_pair(id / 1024, id % 1024);
	}

private:
	struct Chunk
	{
		StorageType data;
		std::size_t usedSpace = 0U;
		std::size_t firstAliveIndex = 0U;
		std::size_t lastAliveIndex = 0U;	// Unused (reserved for reverse iterators)
		std::set<std::size_t> holesList;
	};
	std::list<Chunk> m_chunks;
};

} // namespace ecs
