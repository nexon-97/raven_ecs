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

	std::pair<std::size_t, bool> TryCreate() override
	{
		std::pair<std::size_t, bool> result(0U, false);

		// If not all space is allocated
		if (m_usedSpace < CollectionSize)
		{
			result.first = m_usedSpace;
			result.second = true;
			++m_usedSpace;
		}

		if (!m_holesList.empty())
		{
			// Pick first hole to fill
			auto startIt = m_holesList.begin();
			std::size_t insertPos = *startIt;
			m_holesList.erase(startIt);

			result.first = insertPos;
			result.second = true;
		}

		if (result.second)
		{
			new (&m_storage[result.first]) ComponentType();
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

private:
	StorageType m_storage;
	std::size_t m_usedSpace = 0U;
	std::set<std::size_t> m_holesList;
};

} // namespace ecs
