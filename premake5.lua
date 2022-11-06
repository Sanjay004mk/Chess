include "dependencies.lua"

workspace "Chess"
    architecture "x64"
    startproject "Chess"
    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

group "dependencies"
    include "ext/glfw"
    include "ext/imgui"

group ""
    include "EntropyCore"
    include "Chess"