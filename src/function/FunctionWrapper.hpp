#pragma once

// Signatures construction
template <class Result, class ...Args>
struct ConstructSignature
{
	using type = Result(Args...);
};

template <class T, class Result, class ...Args>
struct ConstructClassSignature
{
	using type = Result(T::*)(Args...);
};

// Signature expansion
template <typename S>
struct ResultType;

template <typename R, class ...Args>
struct ResultType<R(Args...)>
{
	using return_type = R;
	using argument_type = std::tuple<Args...>;
};

template <typename T, typename R, class ...Args>
struct ResultType<R(T::*)(Args...)>
{
	using return_type = R;
	using argument_type = std::tuple<Args...>;
};

// C++17 std::apply implementation (extended to take additional object for member functions call context)
namespace detail {
	template <class F, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
	}

	template <class F, class T, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_memfn_impl(F&& f, T&& obj, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), std::forward<T>(obj), std::get<I>(std::forward<Tuple>(t))...);
	}
}  // namespace detail

template <class F, class Tuple>
constexpr decltype(auto) apply(F&& f, Tuple&& t)
{
	return detail::apply_impl(
		std::forward<F>(f), std::forward<Tuple>(t),
		std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

template <class F, class T, class Tuple>
constexpr decltype(auto) apply_memfn(F&& f, T&& obj, Tuple&& t)
{
	return detail::apply_memfn_impl(
		std::forward<F>(f), std::forward<T>(obj), std::forward<Tuple>(t),
		std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

// Function wrapper interface
template <class Signature>
class FunctionWrapper
{
public:
	using ReturnType = typename ResultType<Signature>::return_type;
	using ArgsType = typename ResultType<Signature>::argument_type;

	template <class ...Args>
	ReturnType Invoke(Args&&...args)
	{
		return InvokeInternal(std::forward_as_tuple(args...));
	}

protected:
	virtual ReturnType InvokeInternal(ArgsType&& argsTuple) = 0;
};

// Concrete wrapper implementation
template <class FunctionType, class Signature>
class DefaultFunctionWrapper
	: public FunctionWrapper<Signature>
{
public:
	explicit DefaultFunctionWrapper(const FunctionType& function)
		: m_function(function)
	{}

protected:
	ReturnType InvokeInternal(ArgsType&& argsTuple) override
	{
		return apply(m_function, argsTuple);
	}

protected:
	FunctionType m_function;
};

// Member function wrapper implementation
template <class T, class FunctionType, class Signature>
class MemberFunctionWrapper
	: public FunctionWrapper<Signature>
{
public:
	explicit MemberFunctionWrapper(T* objectPtr, const FunctionType& function)
		: m_function(function)
		, m_objectPtr(objectPtr)
	{}

protected:
	ReturnType InvokeInternal(ArgsType&& argsTuple) override
	{
		return apply_memfn(m_function, m_objectPtr, argsTuple);
	}

protected:
	FunctionType m_function;
	T* m_objectPtr;
};
