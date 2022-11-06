project "Chess"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir ("%{wks.location}/bin/" .. outputdir)
    objdir ("%{wks.location}/bin-int/" .. outputdir)

    files
    {
        "chs/**"
    }

    includedirs 
    {
        "%{wks.location}/Chess",
        "%{wks.location}/EntropyCore/vendor",
        "%{wks.location}/EntropyCore",
    }

    links
    {
        "EntropyCore"
    }

    filter "system:windows"
        defines "CHS_PLATFORM_WINDOWS"
        systemversion "latest"

    filter "configurations:Debug"
        defines "CHS_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "CHS_RELEASE"
        runtime "Release"
        optimize "on"