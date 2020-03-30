#pragma once
#include "IComponentCollection.hpp"
#include "ecs/storage/MemoryPool.hpp"
#include "ecs/storage/ObjectPool.hpp"
#include "ecs/entity/Entity.hpp"

#include <cassert>

namespace ecs
{

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	struct ComponentData
	{
		ComponentType component;
		ComponentPtrBlock controlBlock;

		ComponentData() = default;
	};
	
	using CollectionType = ComponentCollectionImpl<ComponentType>;

public:
	ComponentCollectionImpl(ComponentTypeId typeId)
		: m_typeId(typeId)
	{}
	~ComponentCollectionImpl() = default;

	void Clear() final
	{
		m_data.Clear();
	}

	// Disable collection copy
	ComponentCollectionImpl(const ComponentCollectionImpl&) = delete;
	ComponentCollectionImpl& operator=(const ComponentCollectionImpl&) = delete;

	// Collection iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = TComponentPtr<ComponentType>;
		using pointer = TComponentPtr<ComponentType>*;
		using reference = TComponentPtr<ComponentType>&;

		iterator() = default;
		iterator(CollectionType* collection, int32_t index)
			: collection(collection)
			, index(index)
		{}

		value_type operator*()
		{
			return collection->GetTypedItemPtr(index);
		}

		pointer operator->()
		{
			return &**this;
		}

		iterator& operator++()
		{
			index = collection->GetNextIndex(index);
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
		int32_t index;
	};

	ComponentPtr Create() override
	{
		// Create data using default constructor
		auto& insertResult = m_data.Emplace();
		insertResult.ref.controlBlock = ComponentPtrBlock(m_typeId, static_cast<int32_t>(insertResult.index), Entity::GetInvalidId(), 1);

		return ComponentPtr(&insertResult.ref.controlBlock);
	}

	void Destroy(const std::size_t index) override
	{
		m_data.RemoveAt(index);
	}

	void* GetData(const std::size_t index) override
	{
		ComponentData& data = m_data.At(index);
		return &data.component;
	}

	ComponentPtrBlock* GetControlBlock(const std::size_t index) override
	{
		ComponentData& data = m_data.At(index);
		return &data.controlBlock;
	}

	ComponentPtr GetItemPtr(const std::size_t index) override
	{
		ComponentPtrBlock* controlBlock = GetControlBlock(index);
		if (nullptr != controlBlock && controlBlock->refCount > 0)
		{
			++controlBlock->refCount;
		}

		return ComponentPtr(controlBlock);
	}

	TComponentPtr<ComponentType> GetTypedItemPtr(const std::size_t index)
	{
		ComponentPtrBlock* controlBlock = GetControlBlock(index);
		if (nullptr != controlBlock && controlBlock->refCount > 0)
		{
			++controlBlock->refCount;
		}

		return TComponentPtr<ComponentType>(controlBlock);
	}

	int32_t GetNextIndex(int32_t currentIndex)
	{
		std::size_t nextIndex = m_data.GetNextObjectIndex(static_cast<std::size_t>(currentIndex + 1));
		return (nextIndex == m_data.GetInvalidPoolId()) ? -1 : static_cast<int32_t>(nextIndex);
	}

	void CopyData(const std::size_t index, const void* dataSource) override
	{
		const ComponentType& dataRef = *reinterpret_cast<const ComponentType*>(dataSource);
		ComponentData& data = m_data.At(index);
		data.component = dataRef;
	}

	void MoveData(const std::size_t index, void* dataSource) override
	{
		ComponentType* sourcePtr = reinterpret_cast<ComponentType*>(dataSource);
		ComponentData& data = m_data.At(index);
		data.component = std::move(*sourcePtr);
	}

	ComponentPtr CloneComponent(const std::size_t index) override
	{
		// Create data using default constructor
		auto& insertResult = m_data.Emplace();
		insertResult.ref.controlBlock = ComponentPtrBlock(m_typeId, static_cast<int32_t>(insertResult.index), Entity::GetInvalidId(), 1);

		// Copy data using copy constructor
		ComponentData& dataToClone = m_data.At(index);
		insertResult.ref.component = ComponentType(dataToClone.component);

		return ComponentPtr(&insertResult.ref.controlBlock);
	}

	iterator begin()
	{
		return iterator(this, GetNextIndex(-1));
	}

	iterator end()
	{
		return iterator(this, -1);
	}

private:
	ObjectPool<ComponentData> m_data;
	ComponentTypeId m_typeId;
};

} // namespace ecs
