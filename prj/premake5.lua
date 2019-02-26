-- If no EngineRootLocation provided - setup as standalone project
local isStandaloneProject = (nil == EngineRootLocation)
if isStandaloneProject then
	EngineRootLocation = "../"
end

project "ecs"
	kind "StaticLib"
	language "C++"
	location ("../prj/".._ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs { "../." }
	
	if isStandaloneProject then
		excludes { "../test/**" }
	end

	targetdir(EngineRootLocation.."/bin")

	configuration "Debug"
		defines { "DEBUG" }
		symbols "on"
		optimize "Off"
		targetname "ecs_d"

	configuration "Release"
		defines { "NDEBUG" }
		optimize "Full"
		targetname "ecs"
