project "EntropyCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir ("%{wks.location}/bin/" .. outputdir)
    objdir ("%{wks.location}/bin-int/" .. outputdir)

    files 
    {
         "Entropy/**",
         
         "vendor/stb_image/stb_image.h",
         "vendor/stb_image/stb_image.cpp" 
    }

    pchheader "etpch.h"
    pchsource "Entropy/etpch.cpp"

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
        "GLFW_INCLUDE_NONE",
        "GLFW_INCLUDE_VULKAN",
        "GLM_FORCE_RADIANS",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE"
    }

    includedirs 
    {
        "%{IncludeDir.VulkanSDK}",
        "%{IncludeDir.glfw}",
        "%{wks.location}/EntropyCore/vendor",
        "%{wks.location}/EntropyCore"
    }

    links
    {
       "glfw",
       "ImGui",
       "%{Library.Vulkan}"
    }

    filter "system:windows"
        defines "ET_PLATFORM_WINDOWS"
        systemversion "latest"

    filter "configurations:Debug"
        defines "ET_DEBUG"
        runtime "Debug"
        symbols "on"
        links
        {
            "%{Library.ShaderC_Debug}",
            "%{Library.SPIRV_Cross_Debug}",
            "%{Library.SPIRV_Tools_Debug}",
            
        }

    filter "configurations:Release"
        defines "ET_RELEASE"
        runtime "Release"
        optimize "on"
        links
        {
            "%{Library.ShaderC_Release}",
            "%{Library.SPIRV_Cross_Release}"
        }