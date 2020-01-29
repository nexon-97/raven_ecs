#pragma once

#include <tuple>
#include <memory>
#include <vector>

#include <iostream>

// Check that type is tuple
template <typename T>
struct IsTuple : std::false_type
{};

template <typename ...Args>
struct IsTuple<std::tuple<Args...>> : std::true_type
{};

template <typename RetType, typename T, typename ...Args>
using MemberFuncSignature = RetType(T::*)(Args...);

// Delegate contain multiple parts
// * Return type
// * Delegate signature

template <typename ...Args>
class BaseDelegateBind
{
public:
	virtual void Execute(Args&&... args) = 0;
};

template <typename SignatureT, typename T, typename PayloadT, typename ...Args>
class MemberFuncDelegateBind
	: public BaseDelegateBind<Args...>
{
public:
	MemberFuncDelegateBind() = delete;
	MemberFuncDelegateBind(SignatureT signature, T* objectPtr, PayloadT&& payload)
		: signature(signature)
		, boundObjectPtr(objectPtr)
		, payloadTuple(std::move(payload))
	{}

	void Execute(Args&&... args) override
	{
		auto paramsTuple = std::tuple_cat(std::forward_as_tuple(boundObjectPtr, std::forward<Args>(args)...), payloadTuple);
		std::apply(signature, paramsTuple);
	}

private:
	SignatureT signature;
	T* boundObjectPtr;
	PayloadT payloadTuple;
};

template <typename ...Args>
class Delegate
{
public:
	Delegate() = default;

	void Execute(Args&&... args)
	{
		if (binding)
		{
			binding->Execute(std::forward<Args>(args)...);
		}
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	void BindMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		binding = std::make_unique<MemberFuncDelegateBind<SignatureT, T, std::tuple<PayloadArgs...>, Args...>>(signature, object, std::forward_as_tuple(payloadArgs...));
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	static Delegate CreateMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		Delegate delegateInstance;
		delegateInstance.BindMemberFunction(signature, object, std::forward<PayloadArgs>(payloadArgs)...);
		return delegateInstance;
	}

private:
	std::unique_ptr<BaseDelegateBind<Args...>> binding;
};

template <typename ...Args>
class MulticastDelegate
{
public:
	MulticastDelegate() = default;

	void Broadcast(Args&&... args)
	{
		for (const auto& binding : bindings)
		{
			binding->Execute(std::forward<Args>(args)...);
		}
	}

	template <typename SignatureT, typename T, typename ...PayloadArgs>
	void BindMemberFunction(SignatureT signature, T* object, PayloadArgs&&... payloadArgs)
	{
		auto binding = std::make_unique<MemberFuncDelegateBind<SignatureT, T, std::tuple<PayloadArgs...>, Args...>>(signature, object, std::forward_as_tuple(payloadArgs...));
		bindings.push_back(std::move(binding));
	}

private:
	std::vector<std::unique_ptr<BaseDelegateBind<Args...>>> bindings;
};

class TestClass
{
public:
	void Foo(int a, float b)
	{
		std::cout << a << " + " << b << std::endl;
	}

	void Bar(int a, float b, int c)
	{
		std::cout << a << " + " << b << " + " << c << std::endl;
	}
};

// int main()
// {
	// TestClass testInstance;

	// Delegate<int> testDelegate = Delegate<int>::CreateMemberFunction(&TestClass::Foo, &testInstance, 725.5f);
	// testDelegate.Execute(81);

	// Delegate<int> testDelegateForBinding;
	// testDelegateForBinding.BindMemberFunction(&TestClass::Bar, &testInstance, 61.5f, 18);
	// testDelegateForBinding.Execute(56);
	// testDelegateForBinding.Execute(12);

	// MulticastDelegate<int> testMulticastDelegate;
	// testMulticastDelegate.BindMemberFunction(&TestClass::Bar, &testInstance, -5.f, 1);
	// testMulticastDelegate.BindMemberFunction(&TestClass::Bar, &testInstance, 25.f, 2);
	// testMulticastDelegate.BindMemberFunction(&TestClass::Bar, &testInstance, 15.5f, 5);
	// testMulticastDelegate.Broadcast(67);

	// return 0;
// }
