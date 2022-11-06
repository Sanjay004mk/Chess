project "ImGui"
    kind "StaticLib"
    language "C++"

    targetdir("%{wks.location}/bin/" .. outputdir)
    objdir("%{wks.location}/bin-int/" .. outputdir)

    files
    {
        "imconfig.h",
        "imgui_draw.cpp",
        "imgui_impl_glfw.cpp",
        "imgui_impl_glfw.h",
        "imgui_impl_vulkan.cpp",
        "imgui_impl_vulkan.h",
        "imgui_internal.h",
        "imgui_tables.cpp",
        "imgui_widgets.cpp",
        "imgui.cpp",
        "imgui.h",
        "imstb_rectpack.h",
        "imstb_textedit.h",
        "imstb_truetype.h",
        "imgui_demo.cpp",

        "ImGuizmo.h",
        "ImGuizmo.cpp",
    }

    includedirs 
    {
        "%{IncludeDir.glfw}",
        "%{IncludeDir.VulkanSDK}"
    }

    filter "system:windows"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "Off"

    filter "system:linux"
        pic "On"
        systemversion "latest"
        cppdialect "C++17"
        staticruntime "Off"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"