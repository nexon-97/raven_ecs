project "ecs"
	kind "SharedLib"	
	language "C++"
	location (_ACTION)
	cppdialect "C++17"

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
	}

	targetdir(EngineRootLocation.."/bin")
	
	if prj_config["os"] == "windows" then
		configureWindowsSDK()
	end
	
	defines { "ECS_EXPORTS" }
	pic "On"

	configuration "Debug"
		defines { "DEBUG" }
		symbols "on"
		optimize "Off"
		targetname "ecs_d"
		objdir(_ACTION.."/obj/Debug")

	configuration "Release"
		optimize "Full"
		targetname "ecs"
		objdir(_ACTION.."/obj/Release")
