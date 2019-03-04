#pragma once
#include "IComponentCollection.hpp"
#include "IComponentCollectionCallback.hpp"
#include "ComponentHandle.hpp"

#include <set>
#include <cassert>
#include <list>

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
		std::size_t entityId = 0U;
		bool isEnabled = true;
	};
	using StorageType = ComponentData[k_chunkSize];
	using HandlesStorage = ComponentHandleInternal[k_chunkSize];
	using CollectionType = ComponentCollectionImpl<ComponentType>;
	using CallbackType = IComponentCollectionCallback<ComponentType>;

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
				chunk.data[i].component.~ComponentType();
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
		using pointer = ComponentData *;
		using reference = ComponentData &;
		using difference_type = std::ptrdiff_t;

		iterator() = default;
		iterator(const CollectionType* collection, typename StorageType::iterator curr, std::size_t index)
			: collection(collection)
			, curr(curr)
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

		const CollectionType* collection;
		std::size_t index;
	};

	ComponentHandleInternal* Create() override
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
		auto& handle = selectedChunk->handlesData[chunkOffset];
		// Given the handle, get component pointer
		auto componentPtr = &selectedChunk->data[handle.objectId].component;
		// Call constructor for created component instance
		new (componentPtr) ComponentType();

		// Increase used space counter
		++selectedChunk->usedSpace;
	
		return &handle;
	}

	void Destroy(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto chunk = GetChunkByIndex(chunkId.first);
		if (chunkId.second < chunk->usedSpace)
		{
			// Swap handles to make fill the hole, made by destroyed item
			std::swap(chunk->handlesData[chunkId.second], chunk->handlesData[chunk->usedSpace - 1]);
		}

		--chunk->usedSpace;
	}

	void* Get(const std::size_t index) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto chunk = GetChunkByIndex(chunkId.first);
		return &chunk->data[chunkId.second].component;
	}

	ComponentData& GetComponentData(const std::size_t index)
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto chunk = GetChunkByIndex(chunkId.first);
		return chunk->data[chunkId.second];
	}

	void SetItemEntityId(const std::size_t index, const std::size_t entityId) override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		auto chunk = GetChunkByIndex(chunkId.first);
		auto& componentData = chunk->data[chunkId.second];
		std::size_t lastEntityId = componentData.entityId;
		componentData.entityId = entityId;
	}

	std::size_t GetItemEntityId(const std::size_t index) const override
	{
		auto chunkId = SplitObjectId(index);
		assert(chunkId.first < m_chunks.size());

		std::size_t i = 0U;
		for (auto& chunk : m_chunks)
		{
			if (i == chunkId.first)
			{
				return chunk.data[chunkId.second].entityId;
				break;
			}

			++i;
		}

		return static_cast<std::size_t>(-1);
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
		HandlesStorage handlesData;
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
			chunk.handlesData[i].objectId = static_cast<uint32_t>(i);
			chunk.handlesData[i].typeId = m_typeId;
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
