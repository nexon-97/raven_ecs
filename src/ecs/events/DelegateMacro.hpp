#pragma once
#include "ecs/events/Delegate.hpp"
#include "ecs/events/MulticastDelegate.hpp"

#define DECLARE_DELEGATE(DelegateType, ...) typedef Delegate<__VA_ARGS__> DelegateType;
#define DECLARE_MULTICAST_DELEGATE(DelegateType, ...) typedef MulticastDelegate<__VA_ARGS__> DelegateType;
