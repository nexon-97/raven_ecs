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

		SwapHandles(dataIndexL, dataIndexR);
		m_handles.DestroyItem(dataIndexR);

		if (wasActivated)
		{
			--m_activatedCount;
		}
	}

	void* Get(const std::size_t index) override
	{
		return &GetItemByHandleIndex(index).component;
	}

	ComponentData& GetComponentData(const std::size_t index)
	{
		return GetItemByHandleIndex(index);
	}

	void SetItemEntityId(const std::size_t index, const uint32_t entityId) override
	{
		auto& componentData = GetItemByHandleIndex(index);
		componentData.entityId = entityId;
		RefreshComponentActivation(index);
	}

	uint32_t GetItemEntityId(const std::size_t index) const override
	{
		return GetItemByHandleIndex(index).entityId;
	}

	void SetItemEnabled(const std::size_t index, const bool enabled) override
	{
		auto& componentData = GetItemByHandleIndex(index);
		if (enabled != componentData.isEnabled)
		{
			componentData.isEnabled = enabled;
			RefreshComponentActivation(index);
		}
	}

	bool IsItemEnabled(const std::size_t index) const override
	{
		return GetItemByHandleIndex(index).isEnabled;
	}

	void RefreshComponentActivation(const std::size_t index) override
	{
		auto& componentData = GetItemByHandleIndex(index);
		auto entityData = m_managerConnection.GetEntityData(componentData.entityId);
		RefreshComponentActivation(index, entityData.isEnabled, entityData.isActivated);
	}

	void RefreshComponentActivation(const std::size_t index, const bool ownerEnabled, const bool ownerActivated) override
	{
		auto& componentData = GetItemByHandleIndex(index);
		bool shouldBeActivated = ownerActivated && ownerEnabled && componentData.isEnabled;
		if (shouldBeActivated != componentData.isActivated)
		{
			componentData.isActivated = shouldBeActivated;
			
			if (shouldBeActivated)
			{
				assert(index >= m_activatedCount);
				SwapHandles(index, m_activatedCount);
				++m_activatedCount;
			}
			else
			{
				assert(index < m_activatedCount);
				--m_activatedCount;
				SwapHandles(index, m_activatedCount);
			}

			assert(Validate());
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

	HandleIndex* CloneComponent(const std::size_t index) override
	{
		auto handleIndex = Create();

		auto& originalData = GetItemByHandleIndex(index);
		auto& cloneData = GetItemByHandleIndex(*handleIndex);

		// Actually copy data
		cloneData.component = originalData.component;

		return handleIndex;
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

	bool Validate() override
	{
		auto count = m_componentsData.GetItemsCount();

		for (std::size_t i = 0U; i < count; ++i)
		{
			auto handleValue = *m_handles[i];

			bool check1 = m_componentsData[handleValue]->isActivated == (i < m_activatedCount);
			bool check2 = *m_handles[*m_handleIndexes[i]] == i;

			if (!check1 || !check2)
				return false;
		}

		return true;
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

	// Mutable and immutable versions of item accessor by handle index
	ComponentData& GetItemByHandleIndex(const std::size_t handleIndex)
	{
		auto handleValue = *m_handles[handleIndex];
		return *m_componentsData[handleValue];
	}

	const ComponentData& GetItemByHandleIndex(const std::size_t handleIndex) const
	{
		auto handleValue = *m_handles[handleIndex];
		return *m_componentsData[handleValue];
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
