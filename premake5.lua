workspace "ParticleEditor"
    language "C++"
    cppdialect "C++17"
    
    architecture "x86_64"
    configurations { "Debug", "Release" }

    warnings "Extra"

    filter { "configurations:Debug" }
        defines { "_DEBUG" }
        symbols "On"

    filter { "configurations:Release" }
        optimize "On"

    filter { "configurations:Release", "system:Windows" }
        linkoptions { "-static", "-static-libgcc", "-static-libstdc++" }
        links { "pthread" }

    filter { }

    targetdir ("bin/%{prj.name}/%{cfg.longname}")
    objdir ("obj/%{prj.name}/%{cfg.longname}")

project "ParticleEditor"
    kind "ConsoleApp"
    files "ParticleEditor/**"

    includedirs {
		"ParticleEditor/src",
        "Dependencies/Raylib/%{cfg.system}/include",
		"Dependencies/imgui/include",
        "Dependencies/fmt/%{cfg.system}/include",
        "Dependencies/nfd/include",
		"../Difu/bin/Difu/%{cfg.longname}/include"
    }

    libdirs { 
        "Dependencies/Raylib/%{cfg.system}/lib",
		"Dependencies/imgui/lib",
        "Dependencies/fmt/%{cfg.system}/lib",
		"Dependencies/nfd/lib",
		"../Difu/bin/Difu/%{cfg.longname}"
    }

    filter { "system:Windows" }
        links { "raylib", "fmt", "Winmm", "gdi32", "opengl32" }
		linkoptions { "-static-libstdc++" } -- filesystem dll not linking
    
    filter { "system:Linux" }
		links { "raylib", "fmt", "Difu", "rlImGui", "nfd" } -- , "m", "dl", "rt", "X11"
		linkoptions { "`pkg-config gtk+-3.0 --libs`" }
    filter {}

