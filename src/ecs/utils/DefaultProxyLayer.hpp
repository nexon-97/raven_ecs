#pragma once
#include "ecs/utils/ProxyLayer.hpp"

namespace ecs
{

/*
* @brief Default proxy layer allow only one proxy per entity
*/
template <class ProxyT, typename IdType = int32_t>
class DefaultProxyLayer
	: public ProxyLayer<ProxyT, IdType>
{
public:
	IdType GetProxyIdByEntity(const EntityId entityId) const
	{
		auto it = m_entityToProxyMapping.find(entityId);
		if (it != m_entityToProxyMapping.end())
		{
			return it->second;
		}

		return ProxyLayer<ProxyT, IdType>::GetInvalidProxyId();
	}

protected:
	ProxyT* AddProxyInternal(const typename ObjectPool<ProxyT>::InsertResult& insertResult, const EntityId entityId, const IdType proxyId) override
	{
		ProxyT* proxyValue = ProxyLayer<ProxyT, IdType>::AddProxyInternal(insertResult, entityId, proxyId);
		m_entityToProxyMapping.emplace(entityId, proxyId);

		return proxyValue;
	}

	void RemoveProxyInternal(const IdType proxyId) override
	{
		EntityId entityId = ProxyLayer<ProxyT, IdType>::m_proxyToEntityMapping[proxyId];
		m_entityToProxyMapping.erase(entityId);

		ProxyLayer<ProxyT, IdType>::RemoveProxyInternal(proxyId);
	}

protected:
	std::unordered_map<EntityId, IdType> m_entityToProxyMapping;
};

}
