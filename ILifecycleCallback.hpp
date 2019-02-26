#pragma once

namespace ecs
{

class ILifecycleCallback
{
public:
	virtual void Init() {}
	virtual void Destroy() {}
	virtual void Update() {}
	virtual void Render() {}
};

} // namespace ecs
