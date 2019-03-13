#pragma once
#include "IComponentCollection.hpp"
#include "ComponentHandle.hpp"
#include "storage/MemoryPool.hpp"

#include <set>
#include <cassert>
#include <list>
#include <sstream>

#include <Windows.h>

namespace ecs
{

struct Entity;

namespace detail
{

// Helper class, that helps components collection to deal with entities stuff
class ComponentCollectionManagerConnection
{
public:
	struct EntityData
	{
		bool isEnabled;
		bool isActivated;

		explicit ECS_API EntityData(const bool isEnabled, const bool isActivated)
			: isEnabled(isEnabled)
			, isActivated(isActivated)
		{}
	};

	EntityData ECS_API GetEntityData(const std::size_t id) const;
	static void ECS_API SetManagerInstance(ecs::Manager* manager);
};

} // namespace detail

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	static constexpr const std::size_t k_chunkSize = 1024U;
	struct ComponentData
	{
		ComponentType component;
		uint32_t entityId = Entity::GetInvalidId();
		bool isEnabled : 1;
		bool isActivated : 1;

		ComponentData()
			: entityId(Entity::GetInvalidId())
			, isEnabled(true)
			, isActivated(false)
		{}
	};
	using StorageType = ComponentData[k_chunkSize];
	using HandlesStorage = uint32_t[k_chunkSize];
	using HandleIndexesStorage = HandleIndex[k_chunkSize];
	using CollectionType = ComponentCollectionImpl<ComponentType>;
	using PositionId = std::pair<std::size_t, std::size_t>;

public:
	ComponentCollectionImpl()
		: m_componentsData(1024U)
		, m_handles(1024U)
		, m_handleIndexes(1024U)
		, m_typeId(ComponentHandleInternal::GetInvalidTypeId())
	{}
	~ComponentCollectionImpl() = default;

	// Disable collection copy
	ComponentCollectionImpl(const ComponentCollectionImpl&) = delete;
	ComponentCollectionImpl& operator=(const ComponentCollectionImpl&) = delete;

	// Collection iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = ComponentData;
		using pointer = ComponentData*;
		using reference = ComponentData&;

		iterator() = default;
		iterator(CollectionType* collection, std::size_t index)
			: collection(collection)
			, index(index)
		{}

		reference operator*()
		{
			return collection->GetComponentData(index);
		}

		pointer operator->()
		{
			return &**this;
		}

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

		CollectionType* collection;
		std::size_t index;
	};

	HandleIndex* Create() override
	{
		auto newComponentData = m_componentsData.CreateItem();
		auto newHandleData = m_handles.CreateItem();
		auto newHandleIndexData = m_handleIndexes.CreateItem();

		*newHandleData.second = static_cast<uint32_t>(newComponentData.first);
		*newHandleIndexData.second = static_cast<HandleIndex>(newHandleData.first);
		newComponentData.second->isEnabled = true;
		newComponentData.second->isActivated = false;

		return newHandleIndexData.second;
	}

	void Destroy(const std::size_t index) override
	{
		bool wasActivated = m_componentsData[index]->isActivated;
		m_componentsData.DestroyItem(index);

		// Swap handles to fill holes and keep handle pointers valid
		std::size_t dataIndexL = index;
		std::size_t dataIndexR = m_handles.GetItemsCount() - 1U;
		
		auto handleL = m_handles[dataIndexL];
		auto handleR = m_handles[dataIndexR];

		// Swap handle pointers
		m_handleIndexes.Swap(*handleL, *handleR);

		m_handles.DestroyItem(index);

		if (wasActivated)
		{
			--m_activatedCount;
		}
	}

	void* Get(const std::size_t index) override
	{
		return &m_componentsData[index]->component;		
	}

	ComponentData& GetComponentData(const std::size_t index)
	{
		return *m_componentsData[index];
	}

	void SetItemEntityId(const std::size_t index, const uint32_t entityId) override
	{
		m_componentsData[index]->entityId = entityId;
		RefreshComponentActivation(index);
	}

	uint32_t GetItemEntityId(const std::size_t index) const override
	{
		return m_componentsData[index]->entityId;
	}

	void SetItemEnabled(const std::size_t index, const bool enabled) override
	{
		auto componentData = m_componentsData[index];
		if (enabled != componentData->isEnabled)
		{
			componentData->isEnabled = enabled;
			RefreshComponentActivation(index);
		}
	}

	bool IsItemEnabled(const std::size_t index) const override
	{
		return m_componentsData[index]->isEnabled;
	}

	void RefreshComponentActivation(const std::size_t index) override
	{
		auto entityData = m_managerConnection.GetEntityData(m_componentsData[index]->entityId);
		RefreshComponentActivation(index, entityData.isEnabled, entityData.isActivated);
	}

	void RefreshComponentActivation(const std::size_t index, const bool ownerEnabled, const bool ownerActivated) override
	{
		bool shouldBeActivated = ownerActivated && ownerEnabled && m_componentsData[index]->isEnabled;
		if (shouldBeActivated != m_componentsData[index]->isActivated)
		{
			m_componentsData[index]->isActivated = shouldBeActivated;
			
			if (shouldBeActivated)
			{
				assert(index >= m_activatedCount);
				++m_activatedCount;
			}
			else
			{
				assert(index < m_activatedCount);
				--m_activatedCount;
			}

			SwapHandles(index, m_activatedCount);
		}
	}

	uint8_t GetTypeId() const override
	{
		return m_typeId;
	}

	void SetTypeId(const uint8_t typeId) override
	{
		m_typeId = typeId;
	}

	iterator begin()
	{
		return iterator(this, 0U);
	}

	iterator end()
	{
		return iterator(this, m_activatedCount);
	}

	//// [TODO] Implement erasure collection methods
	//iterator erase(const iterator& pos)
	//{
	//	return end();
	//}

	//iterator erase(const iterator& from, const iterator& to)
	//{
	//	return end();
	//}

	void DumpState() override
	{
		//std::stringstream ss;

		//ss << "==========================================" << std::endl;
		//ss << "Collection dump:" << std::endl;

		//std::size_t i = 0U;
		//for (auto& chunk : m_chunks)
		//{
		//	ss << "Chunk [" << i << "]:" << std::endl;

		//	for (int j = 0; j < /*chunk.usedSpace*/ 55; ++j)
		//	{
		//		auto& data = chunk.data[chunk.handlesData[j]];
		//		ss << "[" << j << "]: Handle: " << chunk.handlesData[j] << "; Data(entityId: " << data.entityId << "; enabled: " << data.isEnabled << " )" << "; ";
		//		ss << "HandleIndex: [" << chunk.handleIndexes[j] << "] (Handle " << chunk.handlesData[chunk.handleIndexes[j]] << ")" << std::endl;
		//	}

		//	++i;
		//}

		//ss << "==========================================" << std::endl;

		//auto dumpStr = ss.str();

		//OutputDebugStringA(dumpStr.c_str());
	}

private:
	void SwapHandles(const std::size_t lhs, const std::size_t rhs)
	{
		auto handleL = m_handles[lhs];
		auto handleR = m_handles[rhs];

		// Swap handle pointers
		m_handleIndexes.Swap(*handleL, *handleR);
		m_handles.Swap(lhs, rhs);
	}

private:
	detail::MemoryPool<ComponentData> m_componentsData;
	detail::MemoryPool<uint32_t> m_handles;
	detail::MemoryPool<HandleIndex> m_handleIndexes;
	// Collection is always sorted in a way, that there are activated items first (that means they are enabled too,
	// then go deactivated items, and enabled and disabled items are mixed)
	std::size_t m_activatedCount = 0U; 
	detail::ComponentCollectionManagerConnection m_managerConnection;
	uint8_t m_typeId;
};

} // namespace ecs
