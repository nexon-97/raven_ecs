project "tests"
	kind "ConsoleApp"
	language "C++"
	location (_ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
		"D:/GoogleTest/include",
		EngineRootLocation.."/src",
	}
	links { "ecs" }

	targetdir(EngineRootLocation.."/bin")
	debugdir(EngineRootLocation.."/bin")
	
	configureWindowsSDK()

	configuration "Debug"
		defines { "DEBUG" }
		libdirs { "D:/GoogleTest/lib/Debug" }
		links { "gtestd" }
		symbols "on"
		optimize "Off"
		targetname "ecs_tests_d"
		objdir(_ACTION.."/obj/Debug")

	configuration "Release"
		libdirs { "D:/GoogleTest/lib/Release" }
		links { "gtest" }
		optimize "Full"
		targetname "ecs_tests"
		objdir(_ACTION.."/obj/Release")
