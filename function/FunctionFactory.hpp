#pragma once
#include <type_traits>
#include <tuple>
#include "function/FunctionWrapper.hpp"

template <class Result, class ...Args>
using FreeFunctionPointer = Result(*)(Args...);
template <class T, class Result, class ...Args>
using MemberFunctionPointer = Result(T::*)(Args...);

////////////////////////////////////////////////////////////////////////////////////////////////
// Wrapper functions (support free functions, member functions (binds this context) and lambdas

/**
* @brief Wraps lambda function, passed as argument
* Should provide signature as first template parameter, because it can't be deduced
*/
template <class Signature, class LambdaType>
std::unique_ptr<FunctionWrapper<Signature>> WrapLambda(const LambdaType& function)
{
	return std::make_unique<DefaultFunctionWrapper<LambdaType, Signature>>(function);
}

/**
* @brief Wraps free function, passed as argument
*/
template <class Result, class ...Args>
std::unique_ptr<FunctionWrapper<typename ConstructSignature<Result, Args...>::type>>
WrapFreeFunction(FreeFunctionPointer<Result, Args...> function)
{
	using SignatureType = typename ConstructSignature < Result, Args...>::type;
	using FunctionType = FreeFunctionPointer<Result, Args...>;
	
	return std::make_unique<DefaultFunctionWrapper<FunctionType, SignatureType>>(function);
}

/**
* @brief Wraps class method function, passed as argument, and attaches class object pointer as context for function call
*/
template <class ClassType, class Result, class ...Args>
std::unique_ptr<FunctionWrapper<typename ConstructClassSignature<ClassType, Result, Args...>::type>>
WrapMemberFunction(MemberFunctionPointer<ClassType, Result, Args...> function, ClassType* objectPtr)
{
	using SignatureType = typename ConstructClassSignature<ClassType, Result, Args...>::type;
	using FunctionType = MemberFunctionPointer<ClassType, Result, Args...>;

	return std::make_unique<MemberFunctionWrapper<ClassType, FunctionType, SignatureType>>(objectPtr, function);
}
