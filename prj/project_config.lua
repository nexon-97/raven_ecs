-- If no EngineRootLocation provided - setup as standalone project
local isStandaloneProject = (nil == EngineRootLocation)
if isStandaloneProject then
	EngineRootLocation = "../"
	
	defines { "OS_WINDOWS" } 
end

project "ecs"
	if isStandaloneProject then
		kind "WindowedApp"
	else
		kind "StaticLib"
	end
	
	language "C++"
	location ("../prj/".._ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
		"../externals/sdl2/include",
	}
	libdirs
	{
		"../externals/sdl2/lib",
	}
	
	links { "SDL2" }
	
	if not isStandaloneProject then
		excludes { "../test/**" }
	end

	targetdir(EngineRootLocation.."/bin")

	configuration "Debug"
		defines { "DEBUG" }
		symbols "on"
		optimize "Off"
		targetname "ecs_d"
		objdir(EngineRootLocation.."/bin/obj/Debug")

	configuration "Release"
		defines { "NDEBUG" }
		optimize "Full"
		targetname "ecs"
		objdir(EngineRootLocation.."/bin/obj/Release")
