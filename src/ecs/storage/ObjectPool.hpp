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
public:
	using TRoom = std::vector<T>;
	using StorageType = std::list<TRoom>;
	using ItemLocation = std::pair<std::size_t, std::size_t>;

public:
	ObjectPool() = default;
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
		roomIt->erase(roomIt->begin() + itemLocation.second);

		OnItemRemoved(roomIt);
	}

	void Clear()
	{
		m_storage.clear();
		m_poolIteratorById.clear();
		m_availableRooms.clear();
	}

	void Push(const T& object)
	{
		StorageType::iterator roomIt = GetRoomForInsertion();
		roomIt->push_back(object);
		OnItemInserted(roomIt);
	}

	void Push(T&& object)
	{
		StorageType::iterator roomIt = GetRoomForInsertion();
		roomIt->push_back(std::move(object));
		OnItemInserted(roomIt);
	}

	template <typename ...Args>
	void Emplace(Args&&... args)
	{
		StorageType::iterator roomIt = GetRoomForInsertion();
		roomIt->emplace_back(std::forward(...args));
		OnItemInserted(roomIt);
	}
	
private:
	StorageType::iterator GetRoomForInsertion()
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

	StorageType::iterator CreateNewRoom()
	{
		StorageType::iterator insertedIt = m_storage.emplace_back();
		insertedIt->reserve(k_objectPoolRoomSize);

		m_poolIteratorById.push_back(insertedIt);
		m_availableRooms.push_back(insertedIt);

		return insertedIt;
	}

	void OnItemInserted(StorageType::iterator roomIt)
	{
		auto it = m_availableRooms.find(roomIt);
		assert(it != m_availableRooms.end());
		if (it != m_availableRooms.end())
		{
			if (it->size() >= k_objectPoolRoomSize)
			{
				m_availableRooms.erase(it);
			}
		}
	}

	void OnItemRemoved(StorageType::iterator roomIt)
	{
		// Room became not full, so add it to available rooms
		m_availableRooms.try_insert(roomIt);
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
		return (*roomIt)[location.second];
	}

	T& GetItemAtLocation(const ItemLocation& location)
	{
		auto roomIt = m_poolIteratorById[location.first];
		return (*roomIt)[location.second];
	}

private:
	StorageType m_storage;
	std::vector<StorageType::iterator> m_poolIteratorById;
	std::unordered_set<StorageType::iterator> m_availableRooms;
};

}
