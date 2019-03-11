include "build_config.lua"

function latestSDK10Version()
	local arch = iif(os.is64bit(), "\\WOW6432Node\\", "\\")
	local version = os.getWindowsRegistry("HKLM:SOFTWARE" .. arch .."Microsoft\\Microsoft SDKs\\Windows\\v10.0\\ProductVersion")
	return iif(version ~= nil, version .. ".0", nil)
end

function configureWindowsSDK()
	local sdkVersion = latestSDK10Version()
	if sdkVersion ~= nil then
		systemversion(sdkVersion)
	end
end

EcsStandalone = true
EngineRootLocation = path.getabsolute("../")

workspace "raven_ecs"
	configurations { "Debug", "Release" }
	platforms { "x64" }
	defines { "OS_WINDOWS" }

solution "raven_ecs"
	location ("../.")

	include ("../benchmark/prj/benchmark_config.lua")
	include ("../src/prj/project_config.lua")
	
	if build_config["gtest_location"] then
		include ("../test/prj/test_config.lua")
	end
