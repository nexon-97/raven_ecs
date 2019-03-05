#pragma once
#include "IComponentCollection.hpp"
#include "IComponentCollectionCallback.hpp"
#include "ComponentHandle.hpp"

#include <set>
#include <cassert>
#include <list>
#include <sstream>

#include <Windows.h>

namespace ecs
{

template <typename ComponentType>
class ComponentCollectionImpl
	: public IComponentCollection
{
	static constexpr const std::size_t k_chunkSize = 1024U;
	struct ComponentData
	{
		ComponentType component;
		uint32_t entityId = Entity::k_invalidId;
		bool isEnabled = false;
	};
	using StorageType = ComponentData[k_chunkSize];
	using HandlesStorage = uint32_t[k_chunkSize];
	using HandleIndexesStorage = HandleIndex[k_chunkSize];
	using CollectionType = ComponentCollectionImpl<ComponentType>;
	using CallbackType = IComponentCollectionCallback<ComponentType>;
	using PositionId = std::pair<std::size_t, std::size_t>;

public:
	ComponentCollectionImpl() = default;

	~ComponentCollectionImpl()
	{
		// Release all alive objects (call destructors)
		std::size_t chunkId = 0U;
		for (auto& chunk : m_chunks)
		{
			for (std::size_t i = 0; i < chunk.usedSpace; ++i)
			{
				auto objectId = chunk.handlesData[i];
				chunk.data[objectId].component.~ComponentType();
			}

			++chunkId;
		}
	}

	// Disable collection copy
	ComponentCollectionImpl(const ComponentCollectionImpl&) = delete;
	ComponentCollectionImpl& operator=(const ComponentCollectionImpl&) = delete;

	// Collection iterator implementation
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = ComponentData;
		using pointer = ComponentData * ;
		using reference = ComponentData & ;
		using difference_type = std::ptrdiff_t;

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
		// Pick a chunk for insertion
		static const std::size_t k_notInsertedIndex = static_cast<std::size_t>(-1);
		std::size_t insertedIndex = k_notInsertedIndex;
		Chunk* selectedChunk = nullptr;

		// Check last chunk for available place
		const bool hasPlaceInLastChunk = (!m_chunks.empty() && m_chunks.back().usedSpace < k_chunkSize);

		// No place in existing chunks -> create new chunk
		if (hasPlaceInLastChunk)
		{
			selectedChunk = &m_chunks.back();
		}
		else
		{
			selectedChunk = &CreateNewChunk();
		}

		insertedIndex = (m_chunks.size() - 1U) * k_chunkSize + selectedChunk->usedSpace;

		// Get offset of created handle in selected chunk
		std::size_t chunkOffset = insertedIndex % k_chunkSize;

		// Get freshly created component handle
		auto objectId = selectedChunk->handlesData[chunkOffset];
		// Given the handle, get component pointer
		auto componentPtr = &selectedChunk->data[objectId].component;
		// Call constructor for created component instance
		new (componentPtr) ComponentType();

		// Increase used space counter
		++selectedChunk->usedSpace;

		return &selectedChunk->handleIndexes[chunkOffset];
	}

	void Destroy(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto chunk = GetChunkByIndex(chunkId.first);
		auto objectId = chunk->handlesData[chunkId.second];
		auto& object = chunk->data[objectId];

		assert(object.entityId != Entity::k_invalidId);

		object.component.~ComponentType();

		object.entityId = Entity::k_invalidId;
		object.isEnabled = false;

		--chunk->usedSpace;

		// Swap handles to fill holes and keep handle pointers valid
		if (chunkId.second < chunk->usedSpace)
		{
			std::size_t dataIndexL = chunkId.second;
			std::size_t dataIndexR = chunk->usedSpace;

			auto handleL = chunk->handlesData[dataIndexL];
			auto handleR = chunk->handlesData[dataIndexR];

			// Swap handles to make fill the hole, made by destroyed item
			std::swap(chunk->handlesData[dataIndexL], chunk->handlesData[dataIndexR]);

			// Swap handle pointers
			std::swap(chunk->handleIndexes[handleL], chunk->handleIndexes[handleR]);
		}
	}

	// Returns component value given data offset (usually retrieved from handle)
	void* Get(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		auto componentData = GetComponentDataByPosition(chunkId);

		return &componentData->component;		
	}

	// Returns component data given handle index (translates handle to data offset)
	ComponentData& GetComponentData(const std::size_t index)
	{
		auto chunkId = SplitObjectId(index);
		auto componentData = GetComponentDataByPosition(chunkId);
		return *componentData;
	}

	void SetItemEntityId(const std::size_t index, const uint32_t entityId) override
	{
		auto chunkId = SplitObjectId(index);
		auto componentData = GetComponentDataByPosition(chunkId);
		componentData->entityId = entityId;
		componentData->isEnabled = true;
	}

	uint32_t GetItemEntityId(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		auto componentData = GetComponentDataByPosition(chunkId);
		return componentData->entityId;
	}

	// Registers callback in collection. Callback cannot be removed currently.
	void RegisterCallback(CallbackType* callback)
	{
		m_callbacks.RegisterCallback(callback);
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
		if (m_chunks.empty())
		{
			return iterator(this, 0U);
		}
		else
		{
			auto& lastChunk = m_chunks.back();
			return iterator(this, k_chunkSize * (m_chunks.size() - 1U) + lastChunk.usedSpace);
		}
	}

	// [TODO] Implement erasure collection methods
	iterator erase(const iterator& pos)
	{
		return end();
	}

	iterator erase(const iterator& from, const iterator& to)
	{
		return end();
	}

	void DumpState() override
	{
		std::stringstream ss;

		ss << "==========================================" << std::endl;
		ss << "Collection dump:" << std::endl;

		std::size_t i = 0U;
		for (auto& chunk : m_chunks)
		{
			ss << "Chunk [" << i << "]:" << std::endl;

			for (int j = 0; j < /*chunk.usedSpace*/ 55; ++j)
			{
				auto& data = chunk.data[chunk.handlesData[j]];
				ss << "[" << j << "]: Handle: " << chunk.handlesData[j] << "; Data(entityId: " << data.entityId << "; enabled: " << data.isEnabled << " )" << "; ";
				ss << "HandleIndex: [" << chunk.handleIndexes[j] << "] (Handle " << chunk.handlesData[chunk.handleIndexes[j]] << ")" << std::endl;
			}

			++i;
		}

		ss << "==========================================" << std::endl;

		auto dumpStr = ss.str();

		OutputDebugStringA(dumpStr.c_str());
	}

private:
	PositionId SplitObjectId(const std::size_t id) const
	{
		return std::make_pair(id / k_chunkSize, id % k_chunkSize);
	}

	ComponentData* GetComponentDataByPosition(const PositionId& position)
	{
		std::size_t i = 0U;
		for (auto& chunk : m_chunks)
		{
			if (i == position.first)
			{
				auto objectId = chunk.handlesData[position.second];
				return &chunk.data[objectId];
			}

			++i;
		}

		return nullptr;
	}

private:
	struct Chunk
	{
		StorageType data;
		HandlesStorage handlesData;
		HandleIndexesStorage handleIndexes;
		std::size_t usedSpace = 0U;

		Chunk() = default;
	};

	Chunk& CreateNewChunk(std::size_t* newChunkId = nullptr)
	{
		std::size_t chunkId = m_chunks.size();

		m_chunks.emplace_back();
		auto& chunk = m_chunks.back();

		if (nullptr != newChunkId)
		{
			*newChunkId = chunkId;
		}

		// Create initial handles data set
		for (std::size_t i = 0U; i < k_chunkSize; ++i)
		{
			chunk.handlesData[i] = static_cast<uint32_t>(i);
			chunk.handleIndexes[i] = static_cast<HandleIndex>(i);
		}

		return chunk;
	}

	Chunk* GetChunkByIndex(const std::size_t index)
	{
		std::size_t i = 0U;
		for (auto& chunk : m_chunks)
		{
			if (i == index)
			{
				return &chunk;
			}

			++i;
		}

		return nullptr;
	}

	struct CallbacksPack
	{
		struct PrioritySorter
		{
			bool operator()(CallbackType* lhs, CallbackType* rhs) const
			{
				return lhs->GetPriority() > rhs->GetPriority();
			}
		};
		using CallbackCollection = std::set<CallbackType*, PrioritySorter>;

		CallbackCollection createCallbacks;
		CallbackCollection destroyCallbacks;
		CallbackCollection enableCallbacks;
		CallbackCollection disableCallbacks;

		void RegisterCallback(CallbackType* callback)
		{
			if (callback->WantsCreateNotifications())
			{
				createCallbacks.insert(callback);
			}

			if (callback->WantsDestroyNotifications())
			{
				destroyCallbacks.insert(callback);
			}

			if (callback->WantsEnableNotifications())
			{
				enableCallbacks.insert(callback);
			}

			if (callback->WantsDisableNotifications())
			{
				disableCallbacks.insert(callback);
			}
		}
	};

	std::list<Chunk> m_chunks;
	CallbacksPack m_callbacks;
	uint8_t m_typeId;
};

} // namespace ecs
