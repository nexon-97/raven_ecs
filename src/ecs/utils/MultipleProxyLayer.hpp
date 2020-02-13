#pragma once
#include "ecs/utils/ProxyLayer.hpp"

namespace ecs
{

/*
* @brief Multiple proxy layer allows multiple proxies per entity
*/
template <class ProxyT, typename IdType = int32_t>
class MultipleProxyLayer
	: public ProxyLayer<ProxyT, IdType>
{
public:
	const std::vector<IdType>& GetProxiesByEntity(const EntityId entityId) const
	{
		auto it = m_entityToProxyMapping.find(entityId);
		if (it != m_entityToProxyMapping.end())
		{
			return it->second;
		}

		static std::vector<IdType> k_emptyProxiesList;
		return k_emptyProxiesList;
	}

	void RemoveProxiesByEntity(const EntityId entityId)
	{
		auto it = m_entityToProxyMapping.find(entityId);
		if (it != m_entityToProxyMapping.end())
		{
			std::vector<IdType>& proxies = it->second;

			m_isRemovingAllEntityProxies = true;
			for (IdType proxyId : proxies)
			{
				RemoveProxyById(proxyId);
			}
			m_isRemovingAllEntityProxies = false;
		}
	}

protected:
	ProxyT* AddProxyInternal(const typename ObjectPool<ProxyT>::InsertResult& insertResult, const EntityId entityId, const IdType proxyId) override
	{
		ProxyT* proxyValue = ProxyLayer<ProxyT, IdType>::AddProxyInternal(insertResult, entityId, proxyId);
		m_entityToProxyMapping[entityId].push_back(proxyId);

		return proxyValue;
	}

	void RemoveProxyInternal(const IdType proxyId) override
	{
		if (!m_isRemovingAllEntityProxies)
		{
			EntityId entityId = m_proxyToEntityMapping[proxyId];
			std::vector<IdType>& entityProxies = m_entityToProxyMapping[entityId];

			auto it = std::find(entityProxies.begin(), entityProxies.end(), proxyId);
			if (it != entityProxies.end())
			{
				entityProxies.erase(it);
			}
		}

		ProxyLayer<ProxyT, IdType>::RemoveProxyInternal(proxyId);
	}

private:
	std::unordered_map<EntityId, std::vector<IdType>> m_entityToProxyMapping;
	bool m_isRemovingAllEntityProxies = false;
};

}
