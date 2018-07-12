--  ugly hack to use clang
premake.gcc.cc  = 'clang'
premake.gcc.cxx = 'clang++'

solution "GOTH"
    configurations { "Debug", "Release" }

    project "flv"
        kind "StaticLib"
        language "C"

        files {
            "include/flv/**.h",
            "libs/flv/**.c"
        }

        includedirs { "include" }

        configuration "Debug"
            defines { "DEBUG" }
            flags { "Symbols" }
            targetdir "bin/debug"
            objdir ("bin/obj/debug")
            debugdir "bin/debug"

        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }
            targetdir "bin/release"
            objdir ("bin/obj/release")
            debugdir "bin/release"

    project "streamer"
        kind "ConsoleApp"
        language "C++"

        links { "rtmp", "flv", "stdc++fs", "stdc++" }

        buildoptions_cpp {
            "-std=c++17",
        }

        files {
            "include/**.h",
            "src/**.hpp",
            "src/**.cpp",
        }

        includedirs {
            "src",
            "include"
        }

        configuration "linux"
            links { "pthread", "atomic" }

        configuration "Debug"
            defines { "DEBUG" }
            flags { "Symbols" }
            targetdir "bin/debug"
            objdir ("bin/obj/debug")
            debugdir "bin/debug"

        configuration "Release"
            defines { "NDEBUG" }
            flags { "Optimize" }
            targetdir "bin/release"
            objdir ("bin/obj/release")
            debugdir "bin/release"
