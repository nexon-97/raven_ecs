workspace "raven_ecs"
	configurations { "Debug", "Release" }
	platforms { "x64" }

solution "raven_ecs"
	location (_ACTION)

	include ("project_config.lua")
