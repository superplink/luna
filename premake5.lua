

workspace "luna"
    configurations { "debug", "release", "dist" }

project "tests"
    kind "ConsoleApp"
    language "C++"
    targetdir "build/%{cfg.buildcfg}/bin"
    objdir "build/%{cfg.buildcfg}/obj"

    cppdialect "C++23"

    includedirs {
        "src"
    }

    files {
        "src/**.h",
        "src/**.cpp",
        "tests/**.h",
        "tests/**.cpp"
    }

    filter { "system:windows" }
        links { "stdc++", "winmm", "gdi32" }

    filter "configurations:debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:release"
        defines { "DEBUG" }
        optimize "On"

    filter "configurations:dist"
        defines { "NDEBUG" }
        optimize "On"


