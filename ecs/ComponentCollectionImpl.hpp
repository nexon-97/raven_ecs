#pragma once
#include "IComponentCollection.hpp"

#include <set>
#include <cassert>
#include <array>
#include <map>

namespace ecs
{

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	static constexpr const std::size_t k_chunkSize = 1024U;
	using StorageType = std::array<ComponentType, k_chunkSize>;
	using CollectionType = ComponentCollectionImpl<ComponentType>;

public:
	ComponentCollectionImpl()
	{
		// Create first storage
		m_chunks.emplace(std::piecewise_construct, std::forward_as_tuple(0U), std::forward_as_tuple());
	}

	~ComponentCollectionImpl()
	{
		// Release all alive objects (call destructors)
		for (auto& chunkData : m_chunks)
		{
			auto& chunk = chunkData.second;
			for (std::size_t i = 0; i < chunk.usedSpace; ++i)
			{
				if (chunk.holesList.find(i) == chunk.holesList.end())
				{
					chunk.data[i].~ComponentType();
				}
			}
		}
	}

	ComponentCollectionImpl(const ComponentCollectionImpl&) = delete;
	ComponentCollectionImpl& operator=(const ComponentCollectionImpl&) = delete;

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
			std::size_t nextIndex = collection->GetNextAliveIndex(0U, index + 1);
			curr += (nextIndex - index);
			index = nextIndex;
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
		static const std::size_t k_notInsertedIndex = static_cast<std::size_t>(-1);
		std::size_t insertedIndex = k_notInsertedIndex;
		Chunk& selectedChunk = m_chunks[0U];

		for (auto& chunkData : m_chunks)
		{
			selectedChunk = chunkData.second;

			// If there is first alive index -> chunk has space for an item
			if (selectedChunk.firstFreeIndex < k_chunkSize)
			{
				// Place for insertion found, insert here
				insertedIndex = chunkData.first * k_chunkSize + selectedChunk.firstFreeIndex;

				// Remove hole if it exists
				auto& holesList = selectedChunk.holesList;
				auto holeIt = holesList.find(selectedChunk.firstFreeIndex);
				if (holeIt != holesList.end())
				{
					holesList.erase(holeIt);
				}

				break;
			}
		}

		// Element not created -> create new storage then
		if (insertedIndex == k_notInsertedIndex)
		{
			std::size_t newChunkId = m_chunks.size();
			insertedIndex = newChunkId * k_chunkSize;

			m_chunks.emplace(std::piecewise_construct, std::forward_as_tuple(newChunkId), std::forward_as_tuple());
			selectedChunk = m_chunks[newChunkId];
			selectedChunk.firstFreeIndex = 1U;
		}

		std::size_t chunkId = insertedIndex / k_chunkSize;
		std::size_t chunkOffset = insertedIndex % k_chunkSize;
		new (&selectedChunk.data[chunkOffset]) ComponentType();

		// Increase used space counter
		if (chunkOffset >= selectedChunk.usedSpace)
		{
			selectedChunk.usedSpace = chunkOffset + 1;
		}

		// Overwrite first alive index for iterator
		if (chunkOffset == selectedChunk.firstFreeIndex)
		{
			selectedChunk.firstFreeIndex = GetNextFreeIndex(chunkId, chunkOffset);
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
			if (chunkId.second < chunk.firstFreeIndex)
			{
				chunk.firstFreeIndex = chunkId.second;
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

	std::size_t GetNextAliveIndex(const std::size_t chunkId, const std::size_t offset) const
	{
		auto it = m_chunks.find(chunkId);
		const auto& chunk = it->second;

		for (std::size_t i = offset; i < chunk.usedSpace; ++i)
		{
			if (chunk.holesList.find(i) == chunk.holesList.end())
			{
				return i;
			}
		}

		return k_chunkSize;
	}

	std::size_t GetNextFreeIndex(const std::size_t chunkId, const std::size_t offset) const
	{
		auto it = m_chunks.find(chunkId);
		const auto& chunk = it->second;

		for (std::size_t i = offset + 1; i < chunk.usedSpace; ++i)
		{
			if (chunk.holesList.find(i) != chunk.holesList.end())
			{
				return i;
			}
		}

		return chunk.usedSpace;
	}

	iterator begin()
	{
		for (std::size_t i = 0; i < m_chunks.size(); ++i)
		{
			auto& chunk = m_chunks[i];
			std::size_t firstAliveIndex = GetNextAliveIndex(i, 0U);

			if (firstAliveIndex < k_chunkSize)
			{
				auto arrayIt = chunk.data.begin() + firstAliveIndex;
				return iterator(this, arrayIt, k_chunkSize * i + firstAliveIndex);
			}
		}

		return end();
	}

	iterator end()
	{
		return iterator(this, m_chunks[m_chunks.size() - 1].data.end(), k_chunkSize);
	}

private:
	std::pair<std::size_t, std::size_t> SplitObjectId(const std::size_t id) const
	{
		return std::make_pair(id / k_chunkSize, id % k_chunkSize);
	}

private:
	struct Chunk
	{
		StorageType data;
		std::size_t usedSpace = 0U;
		std::size_t firstFreeIndex = 0U;
		std::size_t lastAliveIndex = 0U;	// Unused (reserved for reverse iterators)
		std::set<std::size_t> holesList;
	};
	std::map<std::size_t, Chunk> m_chunks;
};

} // namespace ecs
