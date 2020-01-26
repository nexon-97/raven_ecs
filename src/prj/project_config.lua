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
	
	generateProjectAPIDefExport('ecs')
	pic "On"
	targetname "ecs"

	configuration "Debug"
		targetsuffix "_d"
		objdir(_ACTION.."/obj/Debug")
		
	configuration "Development"
		targetsuffix "_dev"
		objdir(_ACTION.."/obj/Development")

	configuration "Release"
		objdir(_ACTION.."/obj/Release")
