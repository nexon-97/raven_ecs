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
		iterator(CollectionType* collection, std::size_t index)
			: collection(collection)
			, index(index)
		{}

		reference operator*()
		{
			return static_cast<TComponentPtr<ComponentType>>(collection->GetItemPtr(index));
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

	ComponentPtr Create() override
	{
		// Create data using default constructor
		ObjectPool<ComponentData>::InsertResult& insertResult = m_data.Emplace();
		insertResult.ref.controlBlock = ComponentPtrBlock(m_typeId, insertResult.index, -1, 1);

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
		//ComponentData& data = m_data.At(index);

		//ObjectPool<ComponentData>::InsertResult& insertResult = m_data.Emplace(data.component);
		//insertResult.ref.controlBlock = ComponentPtrBlock(m_typeId, insertResult.index, -1, 1);

		//return ComponentPtr(&insertResult.ref.controlBlock);
		return ComponentPtr();
	}

	iterator begin()
	{
		return iterator(this, 0U);
	}

	iterator end()
	{
		return iterator(this, 0U);
		//return iterator(this, m_activatedCount);
	}

private:
	ObjectPool<ComponentData> m_data;
	ComponentTypeId m_typeId;
};

} // namespace ecs
