#pragma once
#include "IComponentCollection.hpp"
#include "ComponentHandle.hpp"
#include "ecs/storage/MemoryPool.hpp"
#include "ecs/entity/Entity.hpp"

#include <set>
#include <cassert>
#include <list>
#include <sstream>

namespace ecs
{

struct Entity;

namespace detail
{

// Helper class, that helps components collection to deal with entities stuff
class ComponentCollectionManagerConnection
{
public:
	struct EntityActivationData
	{
		bool isEnabled;
		bool isActivated;

		explicit ECS_API EntityActivationData(const bool isEnabled, const bool isActivated)
			: isEnabled(isEnabled)
			, isActivated(isActivated)
		{}
	};

	EntityActivationData ECS_API GetEntityActivationData(const EntityId id) const;
	void ECS_API InvokeComponentActivationEvent(const ComponentHandle& handle, bool activated);
	static void ECS_API SetManagerInstance(ecs::Manager* manager);
};

} // namespace detail

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	struct ComponentData
	{
		ComponentType component;
		EntityId entityId;
		bool isEnabled : 1;
		bool isActivated : 1;

		ComponentData()
			: entityId(Entity::GetInvalidId())
			, isEnabled(true)
			, isActivated(false)
		{}
	};
	
	using CollectionType = ComponentCollectionImpl<ComponentType>;

public:
	static constexpr const std::size_t k_chunkSize = 32U;
	ComponentCollectionImpl()
		: m_componentsData(k_chunkSize)
		, m_handles(k_chunkSize)
		, m_handleIndexes(k_chunkSize)
		, m_typeId(ComponentHandleInternal::GetInvalidTypeId())
	{}
	~ComponentCollectionImpl() = default;

	void Clear() final
	{
		m_componentsData.Clear();
		m_handles.Clear();
		m_handleIndexes.Clear();
	}

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

	ComponentHandle::HandleIndex* Create() override
	{
		auto newComponentData = m_componentsData.CreateItem();
		auto newHandleData = m_handles.CreateItem();
		auto newHandleIndexData = m_handleIndexes.CreateItem();

		// Make handle point to the component location
		std::size_t componentLocation = newComponentData.first;
		*newHandleData.second = static_cast<uint32_t>(componentLocation);

		// Make handle index point to the handle location
		std::size_t handleLocation = newHandleData.first;
		*newHandleIndexData.second = static_cast<ComponentHandle::HandleIndex>(handleLocation);

		// Return pointer to the created handle index
		return newHandleIndexData.second;
	}

	void Destroy(const std::size_t index) override
	{
		return; // Disable components destroy...

		uint32_t storageLocation = *m_handles[index];
		uint32_t lastIndex = static_cast<uint32_t>(m_componentsData.GetItemsCount() - 1U);

		// Destroy data (move last element to m_componentsData[storageLocation])
		m_componentsData.DestroyItem(storageLocation);

		// Find handle position, which was used to point to m_componentsData[lastIndex]
		//uint32_t movedDataHandleIndex = *m_handleIndexes[lastIndex];
		//assert(*m_handles[movedDataHandleIndex] == lastIndex); // Check that handle points to last index

		// Swap correct handle and current removed handle by indexes
		SwapHandles(index, lastIndex);
		uint32_t replacedLocation = *m_handles[index];
		m_componentsData.Swap(storageLocation, replacedLocation);
		//m_componentsData.DestroyItem(replacedLocation);
		//m_componentsData.pop_back();
		//m_handles.DestroyItem(index);
		//m_handles.Swap(movedDataHandleIndex, index);
		m_handles.pop_back();
		
		//*m_handleIndexes[storageLocation] = static_cast<uint16_t>(index);
		m_handleIndexes.pop_back();
		//*m_handleIndexes[storageLocation] = static_cast<uint16_t>(index);

		assert(Validate());
	}

	void* Get(const std::size_t index) override
	{
		return &GetItemByHandleIndex(index).component;
	}

	void CopyData(const std::size_t index, const void* dataSource) override
	{
		const ComponentType& dataRef = *reinterpret_cast<const ComponentType*>(dataSource);
		GetItemByHandleIndex(index).component = dataRef;
	}

	void MoveData(const std::size_t index, void* dataSource) override
	{
		ComponentType& targetRef = GetItemByHandleIndex(index).component;
		auto sourcePtr = reinterpret_cast<ComponentType*>(dataSource);
		targetRef = std::move(*sourcePtr);
	}

	ComponentData& GetComponentData(const std::size_t index)
	{
		return GetItemByHandleIndex(index);
	}

	void SetItemEntityId(const std::size_t index, const EntityId entityId) override
	{
		auto& componentData = GetItemByHandleIndex(index);
		componentData.entityId = entityId;
		RefreshComponentActivation(index);
	}

	EntityId GetItemEntityId(const std::size_t index) const override
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
		if (componentData.entityId != Entity::GetInvalidId())
		{
			auto entityData = m_managerConnection.GetEntityActivationData(componentData.entityId);
			RefreshComponentActivation(index, entityData.isEnabled, entityData.isActivated);
		}
		else
		{
			RefreshComponentActivation(index, false, false);
		}
	}

	void RefreshComponentActivation(const std::size_t index, const bool ownerEnabled, const bool ownerActivated) override
	{
		ComponentData& componentData = GetItemByHandleIndex(index);
		bool shouldBeActivated = ownerActivated && ownerEnabled && componentData.isEnabled;

		if (shouldBeActivated != componentData.isActivated)
		{
			componentData.isActivated = shouldBeActivated;

			ComponentHandle tempHandle(m_typeId, m_handleIndexes[index]);
			m_managerConnection.InvokeComponentActivationEvent(tempHandle, componentData.isActivated);
			
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

	ComponentHandle::HandleIndex* CloneComponent(const std::size_t index) override
	{
		ComponentHandle::HandleIndex* handleIndex = Create();

		ComponentData& originalData = GetItemByHandleIndex(index);
		ComponentData& cloneData = GetItemByHandleIndex(*handleIndex);

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

	bool Validate() override
	{
		std::size_t dataCount = m_componentsData.GetItemsCount();
		std::size_t handlesCount = m_handles.GetItemsCount();
		std::size_t indexesCount = m_handleIndexes.GetItemsCount();

		bool storageSizesMatch = ((dataCount == handlesCount) && (handlesCount == indexesCount));
		if (storageSizesMatch)
		{
			for (std::size_t i = 0U; i < dataCount; ++i)
			{
				uint32_t storageLocation = *m_handles[i];

				// Check that component at location [storageLocation] is at correct position relative to activation border
				bool activationBorderCheck = m_componentsData[storageLocation]->isActivated == (i < m_activatedCount);

				// Check, that handle index at [i] points to handle, that points to storage location at [i]
				ComponentHandle::HandleIndex indexLocation = *m_handleIndexes[i];
				bool indexedHandleStorageLocationMatch = *m_handles[indexLocation] == i;

				if (!activationBorderCheck || !indexedHandleStorageLocationMatch)
					return false;
			}

			return true;
		}

		return false;
	}

private:
	void SwapHandles(const std::size_t handleIndexL, const std::size_t handleIndexR)
	{
		uint32_t storageLocationL = *m_handles[handleIndexL];
		uint32_t storageLocationR = *m_handles[handleIndexR];

		// Swap handle pointers
		m_handleIndexes.Swap(storageLocationL, storageLocationR);
		m_handles.Swap(handleIndexL, handleIndexR);
	}

	// Mutable and immutable versions of item accessor by handle index
	ComponentData& GetItemByHandleIndex(const std::size_t handleIndex)
	{
		uint32_t storageOffset = *m_handles[handleIndex];
		return *m_componentsData[storageOffset];
	}

	const ComponentData& GetItemByHandleIndex(const std::size_t handleIndex) const
	{
		uint32_t storageOffset = *m_handles[handleIndex];
		return *m_componentsData[storageOffset];
	}

private:
	detail::MemoryPool<ComponentData> m_componentsData; // Contains data of the components
	detail::MemoryPool<uint32_t> m_handles;				// Contains integer locations of the components data in m_componentsData
	detail::MemoryPool<ComponentHandle::HandleIndex> m_handleIndexes; // Constant cells, that are not moved until the component handle is alive,
	// because ComponentHandle stores pointer to the index location, which is used to point to a handle index
	// Collection is always sorted in a way, that there are activated items first (that means they are enabled too,
	// then go deactivated items, and enabled and disabled items are mixed)
	std::size_t m_activatedCount = 0U; 
	detail::ComponentCollectionManagerConnection m_managerConnection;
	uint8_t m_typeId;
};

} // namespace ecs
