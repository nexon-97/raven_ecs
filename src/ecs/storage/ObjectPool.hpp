#pragma once
#include <cassert>
#include <iterator>
#include <list>
#include <vector>
#include <unordered_set>

namespace ecs
{

const std::size_t k_objectPoolRoomSize = 32U;
	
template <class T>
class ObjectPool
{
	using PoolType = ObjectPool<T>;

public:
	class TRoom
	{
		friend class PoolType;

	public:
		TRoom(int32_t inRoomIndex)
			: roomIndex(inRoomIndex)
		{}

		~TRoom()
		{
			for (std::size_t i = 0U; i < k_objectPoolRoomSize; ++i)
			{
				if (filledPositions.test(i))
				{
					items[i].~T();
				}
			}

			filledPositions.reset();
		}

		template <typename ...Args>
		std::size_t Emplace(Args&&... args)
		{
			std::size_t insertPos = _GetEmptyPosition(0U);
			assert(insertPos < k_objectPoolRoomSize);

			items[insertPos] = T(std::forward<Args>(args)...);
			filledPositions.set(insertPos);
			++size;

			return insertPos;
		}

		std::size_t Push(const T& object)
		{
			std::size_t insertPos = _GetEmptyPosition(0U);
			assert(insertPos < k_objectPoolRoomSize);

			items[insertPos] = T(object);
			filledPositions.set(insertPos);
			++size;

			return insertPos;
		}

		std::size_t Push(T&& object)
		{
			std::size_t insertPos = _GetEmptyPosition(0U);
			assert(insertPos < k_objectPoolRoomSize);

			items[insertPos] = T(object);
			filledPositions.set(std::move(insertPos));
			++size;

			return insertPos;
		}

		T& GetMutable(const std::size_t index)
		{
			return items[index];
		}

		const T& GetConst(const std::size_t index) const
		{
			return items[index];
		}

		void Remove(const std::size_t index)
		{
			assert(filledPositions.test(index));

			items[index].~T();
			filledPositions.reset(index);
			--size;
		}

		std::size_t _GetEmptyPosition(const std::size_t startPos) const
		{
			for (std::size_t i = startPos; i < k_objectPoolRoomSize; ++i)
			{
				if (!filledPositions.test(i))
					return i;
			}

			return k_objectPoolRoomSize;
		}

		std::size_t _GetFilledPosition(const std::size_t startPos) const
		{
			for (std::size_t i = startPos; i < k_objectPoolRoomSize; ++i)
			{
				if (filledPositions.test(i))
					return i;
			}

			return k_objectPoolRoomSize;
		}

	private:
		T items[k_objectPoolRoomSize];
		std::bitset<k_objectPoolRoomSize> filledPositions;
		int32_t roomIndex;
		int8_t size = 0;

		// [TODO] add cache of first and last free items cache for performance
	};

	using StorageType = std::list<TRoom>;
	using StorageTypeIterator = typename std::list<TRoom>::iterator;
	using ItemLocation = std::pair<std::size_t, std::size_t>;

	struct InsertResult
	{
		T& ref;
		std::size_t index;

		InsertResult(T& inRef, const std::size_t inIndex)
			: ref(inRef)
			, index(inIndex)
		{}
	};

public:
	ObjectPool()
		: m_firstIndex(GetInvalidPoolId())
	{}

	~ObjectPool() = default;

	ObjectPool(ObjectPool&& other)
		: m_storage(std::move(other.m_storage))
	{
		FillPoolIteratorsInitial();
	}

	ObjectPool& operator=(ObjectPool&& other)
	{
		m_storage = std::move(other.m_storage);
		FillPoolIteratorsInitial();

		return *this;
	}

	ObjectPool(const ObjectPool& other)
		: m_storage(other.m_storage)
	{
		FillPoolIteratorsInitial();
	}

	ObjectPool& operator=(const ObjectPool& other)
	{
		m_storage = other.m_storage;
		FillPoolIteratorsInitial();

		return *this;
	}

	T& operator[](const std::size_t index)
	{
		return At(index);
	}

	const T& operator[](const std::size_t index) const
	{
		return At(index);
	}

	T& At(const std::size_t index)
	{
		ItemLocation itemLocation = SplitIndexIntoRoomLocation(index);
		return GetItemAtLocation(itemLocation);
	}

	const T& At(const std::size_t index) const
	{
		ItemLocation itemLocation = SplitIndexIntoRoomLocation(index);
		return GetItemAtLocation(itemLocation);
	}

	void RemoveAt(const std::size_t index)
	{
		ItemLocation itemLocation = SplitIndexIntoRoomLocation(index);

		auto roomIt = m_poolIteratorById[itemLocation.first];
		roomIt->Remove(itemLocation.second);

		OnItemRemoved(roomIt);
	}

	void Clear()
	{
		m_storage.clear();
		m_poolIteratorById.clear();
		m_availableRooms.clear();
	}

	InsertResult Push(const T& object)
	{
		StorageTypeIterator roomIt = GetRoomForInsertion();
		std::size_t roomDataIndex = roomIt->Push(object);
		OnItemInserted(roomIt);

		return GenInsertResult(roomIt, roomDataIndex);
	}

	InsertResult Push(T&& object)
	{
		StorageTypeIterator roomIt = GetRoomForInsertion();
		std::size_t roomDataIndex = roomIt->Push(std::move(object));
		OnItemInserted(roomIt);

		return GenInsertResult(roomIt, roomDataIndex);
	}

	template <typename ...Args>
	InsertResult Emplace(Args&&... args)
	{
		StorageTypeIterator roomIt = GetRoomForInsertion();
		std::size_t roomDataIndex = roomIt->Emplace(std::forward<Args>(args)...);
		OnItemInserted(roomIt);

		return GenInsertResult(roomIt, roomDataIndex);
	}
	
public:
	struct iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using pointer = T*;
		using reference = T&;

		iterator(PoolType* collection, std::size_t index)
			: collection(collection)
			, index(index)
		{}

		reference operator*()
		{
			return collection->At(index);
		}

		pointer operator->()
		{
			return &**this;
		}

		iterator& operator++()
		{
			index = collection->GetNextObjectIndex(index + 1U);
			return *this;
		}

		iterator operator++(int)
		{
			const auto temp(*this); ++*this; return temp;
		}

		bool operator==(const iterator& other) const
		{
			return collection == other.collection && index == other.index;
		}

		bool operator!=(const iterator& other) const
		{
			return !(*this == other);
		}

		PoolType* collection;
		std::size_t index;
	};

	iterator begin()
	{
		return iterator(this, m_firstIndex);
	}

	iterator end()
	{
		return iterator(this, GetInvalidPoolId());
	}

private:
	friend struct iterator;

	StorageTypeIterator GetRoomForInsertion()
	{
		if (m_availableRooms.empty())
		{
			return CreateNewRoom();
		}
		else
		{
			return *m_availableRooms.begin();
		}
	}

	StorageTypeIterator CreateNewRoom()
	{
		int32_t roomId = static_cast<int32_t>(m_storage.size());
		m_storage.emplace_back(roomId);
		StorageTypeIterator insertedIt = std::prev(m_storage.end());

		m_poolIteratorById.push_back(insertedIt);
		m_availableRooms.insert(insertedIt);

		return insertedIt;
	}

	void OnItemInserted(StorageTypeIterator roomIt)
	{
		auto it = m_availableRooms.find(roomIt);
		assert(it != m_availableRooms.end());

		if (it != m_availableRooms.end())
		{
			if (roomIt->size >= k_objectPoolRoomSize)
			{
				m_availableRooms.erase(it);
			}
		}
	}

	void OnItemRemoved(StorageTypeIterator roomIt)
	{
		// Room became not full, so add it to available rooms
		m_availableRooms.insert(roomIt);
	}

	void FillPoolIteratorsInitial()
	{
		m_poolIteratorById.reserve(m_storage.size());

		for (auto it = m_storage.begin(); it != m_storage.end(); ++it)
		{
			m_poolIteratorById.push_back(it);

			if (it->size() < k_objectPoolRoomSize)
			{
				m_availableRooms.push_back(it);
			}
		}
	}

	ItemLocation SplitIndexIntoRoomLocation(const std::size_t index) const
	{
		return ItemLocation(index / k_objectPoolRoomSize, index % k_objectPoolRoomSize);
	}

	const T& GetItemAtLocation(const ItemLocation& location) const
	{
		auto roomIt = m_poolIteratorById[location.first];
		return roomIt->GetConst(location.second);
	}

	T& GetItemAtLocation(const ItemLocation& location)
	{
		auto roomIt = m_poolIteratorById[location.first];
		return roomIt->GetMutable(location.second);
	}

	InsertResult GenInsertResult(StorageTypeIterator roomIt, const std::size_t dataIndex) const
	{
		return InsertResult(roomIt->items[dataIndex], roomIt->roomIndex + dataIndex);
	}

	std::size_t GetNextObjectIndex(const std::size_t index) const
	{
		std::size_t startRoomPos = index % k_objectPoolRoomSize;
		for (std::size_t poolId = index / k_objectPoolRoomSize; poolId < m_poolIteratorById.size(); ++poolId)
		{
			std::size_t filledPos = m_poolIteratorById[poolId]->_GetFilledPosition(startRoomPos);
			if (filledPos != k_objectPoolRoomSize)
			{
				return poolId * k_objectPoolRoomSize + filledPos;
			}
		}

		return GetInvalidPoolId();
	}

	const std::size_t GetInvalidPoolId()
	{
		return std::numeric_limits<std::size_t>::max();
	}

private:
	struct StorageTypeIteratorHasher
	{
		std::size_t operator()(StorageTypeIterator it) const
		{
			return std::hash<int>()(it->roomIndex);
		}
	};

	StorageType m_storage;
	std::vector<StorageTypeIterator> m_poolIteratorById;
	std::unordered_set<StorageTypeIterator, StorageTypeIteratorHasher> m_availableRooms;
	std::size_t m_firstIndex;
};

}
