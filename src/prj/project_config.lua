local ecs_path = EngineRootLocation.."/framework/core/raven_ecs"
local ecs_src = ecs_path.."/src"

local module_definition =
{
	["name"] = "ecs",
	["kind"] = "shared_lib",
	["dependencies"] = {},
	["files"] = { ecs_src.."/**.cpp", ecs_src.."/**.hpp" },
	["include_dirs"] = { ecs_src },
	["defines"] = {},
}

registerModuleDef(module_definition)
