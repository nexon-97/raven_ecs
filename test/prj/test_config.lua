project "tests"
	kind "ConsoleApp"
	language "C++"
	location (_ACTION)

	files { "../**.cpp", "../**.hpp" }
	includedirs
	{
		"../.",
		build_config["gtest_location"].."/include",
		EngineRootLocation.."/src",
	}
	links { "ecs" }

	targetdir(EngineRootLocation.."/bin")
	debugdir(EngineRootLocation.."/bin")
	
	configureWindowsSDK()

	configuration "Debug"
		defines { "DEBUG" }
		libdirs { build_config["gtest_location"].."/lib/Debug" }
		links { "gtestd" }
		symbols "on"
		optimize "Off"
		targetname "ecs_tests_d"
		objdir(_ACTION.."/obj/Debug")

	configuration "Release"
		libdirs { build_config["gtest_location"].."/lib/Release" }
		links { "gtest" }
		optimize "Full"
		targetname "ecs_tests"
		objdir(_ACTION.."/obj/Release")
