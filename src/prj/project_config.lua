local ecs_path = EngineRootLocation.."/framework/core/raven_ecs"
local ecs_src = ecs_path.."/src"

local module_definition =
{
	["name"] = "ecs",
	["prj_location"] = ecs_src.."/prj/".._ACTION,
	["dependencies"] = {},
	["files"] = { ecs_src.."/**.cpp", ecs_src.."/**.hpp" },
	["include_dirs"] = { ecs_src },
	["defines"] = {},
	["link_type"] = "dynamic",
}

registerModuleDef(module_definition)
