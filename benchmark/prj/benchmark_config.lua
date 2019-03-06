project "benchmark"
	kind "WindowedApp"
	language "C++"
	location (_ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
		EngineRootLocation.."/src",
		EngineRootLocation.."/externals/sdl2/include",
	}
	libdirs
	{
		EngineRootLocation.."/externals/sdl2/lib",
	}
	
	links { "ecs", "SDL2" }
	targetdir(EngineRootLocation.."/bin")
	
	configureWindowsSDK()

	configuration "Debug"
		defines { "DEBUG" }
		symbols "on"
		optimize "Off"
		targetname "benchmark_d"
		objdir(_ACTION.."/obj/Debug")

	configuration "Release"
		optimize "Full"
		targetname "benchmark"
		objdir(_ACTION.."/obj/Release")
