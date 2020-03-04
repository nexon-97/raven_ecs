#pragma once
#include "ecs/events/DelegateDetails.hpp"
#include <vector>
#include <memory>

template <typename ...Args>
class MulticastDelegate
{
public:
	MulticastDelegate() = default;

	void Broadcast(Args... args)
	{
		m_isBroadcasting = true;
		for (const auto& binding : bindings)
		{
			binding->Execute(std::forward<Args&&>(args)...);
		}
		m_isBroadcasting = false;

		if (m_pendingUnbindAll)
		{
			UnbindAll();
		}
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	void BindMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		assert(!m_isBroadcasting);
		auto binding = std::make_unique<MemberFuncDelegateBind<SignatureT, T, std::tuple<PayloadArgs...>, Args...>>(signature, object, std::forward_as_tuple(payloadArgs...));
		bindings.push_back(std::move(binding));
	}

	template <typename SignatureT, typename ...PayloadArgs>
	void BindFreeFunction(SignatureT signature, PayloadArgs&&... payloadArgs)
	{
		assert(!m_isBroadcasting);
		auto binding = std::make_unique<FreeFuncDelegateBind<SignatureT, std::tuple<PayloadArgs...>, Args...>>(signature, std::forward_as_tuple(payloadArgs...));
		bindings.push_back(std::move(binding));
	}

	void UnbindAll()
	{
		if (m_isBroadcasting)
		{
			m_pendingUnbindAll = true;
		}
		else
		{
			bindings.clear();
		}
	}

private:
	std::vector<std::unique_ptr<BaseDelegateBind<Args...>>> bindings;
	bool m_isBroadcasting = false;
	bool m_pendingUnbindAll = false;
};
