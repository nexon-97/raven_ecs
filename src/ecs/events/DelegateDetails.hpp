#pragma once

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
		std::apply(signature, std::tuple_cat(std::forward_as_tuple(boundObjectPtr, std::forward<Args>(args)...), payloadTuple));
	}

private:
	SignatureT signature;
	T* boundObjectPtr;
	PayloadT payloadTuple;
};

template <typename SignatureT, typename PayloadT, typename ...Args>
class FreeFuncDelegateBind
	: public BaseDelegateBind<Args...>
{
public:
	FreeFuncDelegateBind() = delete;
	FreeFuncDelegateBind(SignatureT signature, PayloadT&& payload)
		: signature(signature)
		, payloadTuple(std::move(payload))
	{}

	void Execute(Args&&... args) override
	{
		std::apply(signature, std::tuple_cat(std::forward_as_tuple(std::forward<Args>(args)...), payloadTuple));
	}

private:
	SignatureT signature;
	PayloadT payloadTuple;
};
