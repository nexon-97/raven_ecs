project "ecs"
	kind "SharedLib"	
	language "C++"
	location (_ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
	}

	targetdir(EngineRootLocation.."/bin")
	
	configureWindowsSDK()
	
	defines { "ECS_EXPORTS" }

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
