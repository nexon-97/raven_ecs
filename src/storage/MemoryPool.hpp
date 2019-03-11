#pragma once
#include <cassert>
#include <cstdlib>
#include <utility>
#include <iterator>

namespace ecs
{
namespace detail
{

/**
* @brief Memory pool is an unordered objects collection with random access
*
* When the object is destroyed, iterator to the end element and destroyed element are invalidated.
*/
template <class T>
class MemoryPool
{
public:
	MemoryPool() = delete;
	MemoryPool(const std::size_t chunkSize)
		: m_chunkSize(chunkSize)
	{
		assert(m_chunkSize > 16U);
	}
	~MemoryPool()
	{
		if (nullptr != m_chunks)
		{
			free(m_chunks);
		}
	}

	// Pool iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using pointer = T * ;
		//using reference = T&;

		iterator() = default;
		iterator(MemoryPool<T>* pool, std::size_t index)
			: pool(pool)
			, index(index)
		{}

		pointer operator*()
		{
			return pool->GetItem(index);
		}

		/*pointer operator->()
		{
			return &**this;
		}*/

		iterator& operator++()
		{
			++index;
			return *this;
		}

		iterator operator++(int)
		{
			const auto temp(*this); ++*this; return temp;
		}

		bool operator==(const iterator& other) const
		{
			return index == other.index;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		MemoryPool<T>* pool;
		std::size_t index;
	};

	using CreationResult = std::pair<std::size_t, T*>;
	CreationResult CreateItem()
	{
		auto poolLocation = GetPoolLocation(m_usedSpace);

		if (poolLocation.first >= m_chunksCount)
		{
			AllocateNewChunk();
		}

		assert(poolLocation.first < m_chunksCount);
		auto createdIndex = m_usedSpace;
		++m_usedSpace;
		CreationResult result = { createdIndex, GetItem(createdIndex) };

		new (result.second) T();
		
		return result;
	}

	// Destroys item at index, and returns iterator to element after destroyed, and new end iterator
	std::pair<iterator, iterator> DestroyItem(const std::size_t index)
	{
		assert(index < m_usedSpace && m_usedSpace > 0U);

		auto destroyPoolLocation = GetPoolLocation(index);
		auto lastPoolLocation = GetPoolLocation(m_usedSpace - 1);

		// Swap destroyed element and the last one to remove holes
		std::swap(m_chunks[destroyPoolLocation.first][destroyPoolLocation.second], m_chunks[lastPoolLocation.first][lastPoolLocation.second]);

		--m_usedSpace;

		m_chunks[lastPoolLocation.first][lastPoolLocation.second].~T();

		return { iterator(this, index), end() };
	}

	T* GetItem(const std::size_t index) const
	{
		assert(index < m_usedSpace);

		auto poolLocation = GetPoolLocation(index);
		return &m_chunks[poolLocation.first][poolLocation.second];
	}

	std::size_t GetItemsCount() const
	{
		return m_usedSpace;
	}

	void Clear()
	{
		m_usedSpace = 0U;
	}

	std::size_t GetAllocatedCount() const
	{
		return m_chunksCount * m_chunkSize;
	}

	iterator begin()
	{
		return iterator(this, 0U);
	}

	iterator end()
	{
		return iterator(this, m_usedSpace);
	}

	std::pair<iterator, iterator> erase(const iterator& it)
	{
		return DestroyItem(it.index);
	}

	T* operator[](const std::size_t index) const
	{
		return GetItem(index);
	}

private:
	void AllocateNewChunk()
	{
		Chunk newChunk = reinterpret_cast<Chunk>(malloc(m_chunkSize * sizeof(T)));
		assert(nullptr != newChunk);

		++m_chunksCount;

		if (nullptr != m_chunks)
		{
			m_chunks = reinterpret_cast<Chunk*>(realloc(m_chunks, m_chunksCount * sizeof(Chunk)));
		}
		else
		{
			m_chunks = reinterpret_cast<Chunk*>(malloc(m_chunksCount * sizeof(Chunk)));
		}

		assert(nullptr != m_chunks);

		// Assign new chunk pointer to the end of the list
		m_chunks[m_chunksCount - 1U] = newChunk;
	}
	
	using PoolLocation = std::pair<std::size_t, std::size_t>;
	PoolLocation GetPoolLocation(const std::size_t index) const
	{
		return { index / m_chunkSize, index % m_chunkSize };
	}

private:
	const std::size_t m_chunkSize = 1024U;
	std::size_t m_usedSpace = 0U;
	std::size_t m_chunksCount = 0U;
	using Chunk = T*;
	Chunk* m_chunks = nullptr;
};

} // namespace detail
} // namespace ecs
