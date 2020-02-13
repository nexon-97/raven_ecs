#pragma once
#include <unordered_map>
#include "ecs/storage/ObjectPool.hpp"
#include "ecs/entity/Entity.hpp"

namespace ecs
{

template <class ProxyT, typename IdType = int32_t>
class ProxyLayer
{
public:
	EntityId GetEntityByProxyId(const IdType proxyId) const
	{
		auto it = m_proxyToEntityMapping.find(proxyId);
		if (it != m_proxyToEntityMapping.end())
		{
			return it->second;
		}

		return Entity::GetInvalidId();
	}

	IdType GetProxyIdByEntity(const EntityId entityId) const
	{
		auto it = m_entityToProxyMapping.find(entityId);
		if (it != m_entityToProxyMapping.end())
		{
			return it->second;
		}

		return GetInvalidProxyId();
	}

	ProxyT* GetProxyById(const IdType proxyId)
	{
		auto it = m_proxiesById.find(proxyId);
		if (it != m_proxiesById.end())
		{
			const std::size_t poolIndex = it->second;
			return &m_proxiesStorage[poolIndex];
		}

		return nullptr;
	}

	ProxyT* GetProxyByEntityId(const EntityId entityId)
	{
		auto proxyIdIt = m_entityToProxyMapping.find(entityId);
		if (proxyIdIt != m_entityToProxyMapping.end())
		{
			return GetProxyById(proxyIdIt->second);
		}

		return nullptr;
	}

	template <typename ...Args>
	std::pair<IdType, ProxyT*> CreateProxy(const EntityId entityId, Args... args)
	{
		auto it = m_entityToProxyMapping.find(entityId);
		if (it != m_entityToProxyMapping.end())
		{
			std::size_t poolLocation = m_proxiesById[it->second];
			ProxyT* proxy = &m_proxiesStorage[poolLocation];

			return std::make_pair(it->second, proxy);
		}

		IdType proxyId = m_nextProxyId;
		++m_nextProxyId;

		auto insertResult = m_proxiesStorage.Emplace(std::forward<Args>(args)...);
		m_proxiesById.emplace(proxyId, insertResult.index);
		m_entityToProxyMapping.emplace(entityId, proxyId);
		m_proxyToEntityMapping.emplace(proxyId, entityId);

		ProxyT* proxyValue = &insertResult.ref;
		return std::make_pair(proxyId, proxyValue);
	}

	void RemoveProxyById(const IdType proxyId)
	{
		auto it = m_proxiesById.find(proxyId);
		if (it != m_proxiesById.end())
		{
			EntityId entityId = m_proxyToEntityMapping[proxyId];
			m_entityToProxyMapping.erase(entityId);
			m_proxyToEntityMapping.erase(proxyId);
			m_proxiesStorage.RemoveAt(it->second);
			m_proxiesById.erase(it);
		}
	}

	static const IdType GetInvalidProxyId()
	{
		return std::numeric_limits<IdType>::max();
	}

private:
	std::unordered_map<EntityId, IdType> m_entityToProxyMapping;
	std::unordered_map<IdType, EntityId> m_proxyToEntityMapping;
	std::unordered_map<IdType, std::size_t> m_proxiesById;
	ObjectPool<ProxyT> m_proxiesStorage;
	IdType m_nextProxyId = 0;
};

}
