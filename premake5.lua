
-- NOTE: Requires at least Premake5 alpha 15 for ccpdialect and vs2019 support.

 -- NOTE: I built GLFW from source. If you're using a binary distribution you may need to change some paths down below.
GLFW_PATH = "C:/Repos/Libraries/glfw/3.3-dll/"
GLM_PATH = "C:/Repos/Libraries/glm/0.9.9.6/"
GLI_PATH = "C:/Repos/Libraries/gli/0.8.2.0/"


workspace "KHBBSMap" 
    configurations {"Debug", "Release"} 
    platforms {"Win32"} 
    location "build"

    project "KHBBSMap"
        kind "ConsoleApp"
        language "C++"
		cppdialect "C++17"  -- Needed for proper <filesystem> support.
        system "Windows" 
        architecture "x32"
		vpaths {
			["Source"] = { "**.h", "**.c", "**.hpp", "**.cpp"},
			["Shaders"] = { "**.glsl", "**.vert", "**.frag" },
		}
		files { "src/**", "resources/shaders/**" }
        flags {"NoPCH"}  -- TODO: Check to see if more flags are needed.
        targetdir "bin"
        --debugdir "./" -- TODO: This doesn't give the right results and the .user file overrides it once generated.
                        --       Also, now rendered irrelevent by the new FileManager search functionality.
		
		-- GLM
        includedirs(GLM_PATH)

        -- GLI (Not used?)
        includedirs(GLI_PATH)
        
        -- GLFW
        includedirs(GLFW_PATH .. "include")
        defines("GLFW_DLL")
        links("glfw3dll")
        filter("configurations:Debug")
            postbuildcommands("{COPY} " .. GLFW_PATH .. "src/Debug/glfw3.dll %{cfg.targetdir}")
            postbuildcommands("{COPY} " .. GLFW_PATH .. "src/Debug/glfw3.pdb %{cfg.targetdir}")
            libdirs(GLFW_PATH .. "src/Debug")
        filter("configurations:Release")
            postbuildcommands("{COPY} ".. GLFW_PATH .. "src/Release/glfw3.dll %{cfg.targetdir}")
            libdirs(GLFW_PATH .. "src/Release")
            --libdirs(GLFW_PATH .. "src/RelWithDebInfo") -- NOTE: you'll need to change the postbuildcommands as well.
        filter({})

        -- GLAD
        includedirs("./glad/include")
        files("./glad/src/glad.c")
        filter "files:**.c" -- TODO: make this only effect glad.c
            flags { "NoPCH" } 
        filter ({})

        filter "configurations:Debug"
            defines {"DEBUG"} 
            symbols "on" 
            optimize "off" 

        filter "configurations:Release"
            defines {"NDEBUG"} 
            optimize "on" 