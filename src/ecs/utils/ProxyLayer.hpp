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
	ProxyLayer() = default;
	~ProxyLayer() = default;

	ProxyLayer(const ProxyLayer&) = delete;
	ProxyLayer& operator=(const ProxyLayer&) = delete;

	ProxyLayer(ProxyLayer&&) = default;
	ProxyLayer& operator=(ProxyLayer&&) = default;

	EntityId GetEntityByProxyId(const IdType proxyId) const
	{
		auto it = m_proxyToEntityMapping.find(proxyId);
		if (it != m_proxyToEntityMapping.end())
		{
			return it->second;
		}

		return Entity::GetInvalidId();
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
		IdType proxyId = m_nextProxyId;
		++m_nextProxyId;

		auto insertResult = CreateProxyInternal(std::forward<Args>(args)...);

		ProxyT* proxyValue = AddProxyInternal(insertResult, entityId, proxyId);
		return std::make_pair(proxyId, proxyValue);
	}

	void RemoveProxyById(const IdType proxyId)
	{
		RemoveProxyInternal(proxyId);
	}

	ObjectPool<ProxyT>& GetProxies()
	{
		return m_proxiesStorage;
	}

	static const IdType GetInvalidProxyId()
	{
		return std::numeric_limits<IdType>::max();
	}

protected:
	virtual ProxyT* AddProxyInternal(const typename ObjectPool<ProxyT>::InsertResult& insertResult, const EntityId entityId, const IdType proxyId)
	{
		m_proxiesById.emplace(proxyId, insertResult.index);
		m_proxyToEntityMapping.emplace(proxyId, entityId);

		ProxyT* proxyValue = &insertResult.ref;
		return proxyValue;
	}

	virtual void RemoveProxyInternal(const IdType proxyId)
	{
		m_proxyToEntityMapping.erase(proxyId);
		std::size_t storageLocation = m_proxiesById[proxyId];
		m_proxiesStorage.RemoveAt(storageLocation);
		m_proxiesById.erase(proxyId);
	}

	template <typename ...Args>
	typename ObjectPool<ProxyT>::InsertResult CreateProxyInternal()
	{
		return m_proxiesStorage.Emplace(std::forward<Args>(args)...);
	}

protected:
	std::unordered_map<IdType, EntityId> m_proxyToEntityMapping;
	std::unordered_map<IdType, std::size_t> m_proxiesById;
	ObjectPool<ProxyT> m_proxiesStorage;
	IdType m_nextProxyId = 0;
};

}
