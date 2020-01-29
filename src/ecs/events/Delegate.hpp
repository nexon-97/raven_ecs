#pragma once
#include "ecs/events/DelegateDetails.hpp"

template <typename ...Args>
class Delegate
{
public:
	Delegate() = default;

	void Execute(Args&&... args)
	{
		if (binding)
		{
			m_isExecuting = true;
			binding->Execute(std::forward<Args>(args)...);
			m_isExecuting = false;
		}

		if (m_pendingUnbind)
		{
			Unbind();
		}
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	void BindMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		assert(!m_isExecuting);
		binding = std::make_unique<MemberFuncDelegateBind<SignatureT, T, std::tuple<PayloadArgs...>, Args...>>(signature, object, std::forward_as_tuple(payloadArgs...));
	}

	template <typename SignatureT, typename ...PayloadArgs>
	void BindFreeFunction(SignatureT signature, PayloadArgs&&... payloadArgs)
	{
		assert(!m_isExecuting);
		binding = std::make_unique<FreeFuncDelegateBind<SignatureT, std::tuple<PayloadArgs...>, Args...>>(signature, std::forward_as_tuple(payloadArgs...));
	}

	void Unbind()
	{
		if (m_isExecuting)
		{
			m_pendingUnbind = true;
		}
		else
		{
			binding.reset();
			m_pendingUnbind = false;
		}
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	static Delegate CreateMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		Delegate delegateInstance;
		delegateInstance.BindMemberFunction(signature, object, std::forward<PayloadArgs>(payloadArgs)...);
		return delegateInstance;
	}

	template <typename SignatureT, typename ...PayloadArgs>
	static Delegate CreateFreeFunction(SignatureT signature, PayloadArgs&&... payloadArgs)
	{
		Delegate delegateInstance;
		delegateInstance.BindFreeFunction(signature, std::forward<PayloadArgs>(payloadArgs)...);
		return delegateInstance;
	}

private:
	std::unique_ptr<BaseDelegateBind<Args...>> binding;
	bool m_isExecuting = false;
	bool m_pendingUnbind = false;
};
