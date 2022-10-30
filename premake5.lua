workspace "FoF_AS"
	startproject "TestApp"

	configurations {"Debug", "Release"}
	
	platforms { "x86", "x86_64" }

	filter "platforms:x86"
		architecture "x86"
		defines "SYS_ARCH=x86"

	filter "platforms:x86_64"
		architecture "x86_64"
		defines "SYS_ARCH=x86_64"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

	filter {}
	
	defines 'COMPILE_CFG="%{cfg.buildcfg}"'
	defines 'CURR_ARCHTECTURE="%{cfg.architecture}"'
	defines 'CURR_SYSTEM="%{cfg.system}"'	
	
	IncludeDir = {}
	IncludeDir["GLM"]     = "%{wks.location}/Dependencies/glm/include"
	IncludeDir["SPDLOG"]  = "%{wks.location}/Dependencies/spdlog/include"
	IncludeDir["AUXAPIS"] = "%{wks.location}/Aux0/APIs"
	IncludeDir["IMGUI"]   = "%{wks.location}/Dependencies/imGui"
	IncludeDir["IMGUI_BK"]= "%{wks.location}/Dependencies/imGui/backends"
	IncludeDir["STB"]     = "%{wks.location}/Dependencies/stb/"
	IncludeDir["COMM_API"] = "%{wks.location}/CommLayer/API"
	IncludeDir["AS_API"] = "%{wks.location}/AgentSystem/API"
	

	LibDir = {}
	LibDir["AUX0"]   = ("%{wks.location}/Aux0/lib/" .. outputdir)
	LibDir["AGENT_SYSTEM"] = ("%{wks.location}/AgentSystem/lib/" .. outputdir)
	LibDir["COMM_LAYER"] = ("%{wks.location}/CommLayer/lib/" .. outputdir)

	binDir = "bin/" .. outputdir .. "/%{prj.name}"
	binIntDir = "bin-int/" .. outputdir .. "/%{prj.name}"
	

project "Aux0"
	location "Aux0"
	kind "Staticlib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	pchheader "miscStdHeaders.h"
	pchsource "%{prj.name}/src/miscStdHeaders.cpp"

	defines "AUX0"

	flags
	{
		"MultiProcessorCompile"
	}

	targetdir (binDir)
	objdir (binIntDir)

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/include/**.h",
		"%{prj.name}/APIs/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/include/**.hpp",
		"%{prj.name}/APIs/**.hpp"
	}

	includedirs {
		"%{IncludeDir.GLM}",
		"%{IncludeDir.SPDLOG}",
		"%{prj.name}/include",
		"%{prj.name}/APIs",
		"%{IncludeDir.AUXAPIS}"
	}

	filter "architecture:x86"
		includedirs
		{
		}

		defines	"X86"


	filter "architecture:x86_64"
		includedirs
		{
		}

		defines "X64"


	filter "system:windows"
		systemversion "latest"
		buildoptions "/MT"

		defines{
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_LIB"
		}

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines	"RELEASE"
		optimize "on" 		

	filter {}
	
	postbuildcommands{
		("{COPY} %{cfg.buildtarget.relpath} %{LibDir.AUX0}")
	}
	
	
project "AgentSystem"
	location "AgentSystem"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	dependson {"Aux0", "CommLayer"}

	defines "AS_AGENTSYSTEM"

	flags
	{
		"MultiProcessorCompile"
	}

	links ("%{LibDir.AUX0}/Aux0.lib")
	links ("%{LibDir.COMM_LAYER}/CommLayer.lib")

	targetdir (binDir)
	objdir (binIntDir)
	
	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/include/**.hpp",
		"%{prj.name}/API/**.hpp",
		"%{prj.name}/API/**.h"
	}

	includedirs
	{
		"%{IncludeDir.SPDLOG}",
		"%{IncludeDir.COMM_API}",
		"%{IncludeDir.AUXAPIS}",
		"%{IncludeDir.AS_API}"		
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions "/MD"

		defines {
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_DLL"
		}

	filter "architecture:x86_64"
		defines "X64"

	filter "architecture:x86"
		defines "X86"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines	"RELEASE"
		optimize "on" 
	
	filter {}
	
	postbuildcommands{
		("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/TestApp"),
		("{COPYFILE} ../" .. binDir .."/AgentSystem.lib %{LibDir.AGENT_SYSTEM}")	
	}
	
	
project "CommLayer"
	location "CommLayer"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	dependson {"Aux0"}

	defines "AS_COMMLAYER"

	flags
	{
		"MultiProcessorCompile"
	}

	links ("%{LibDir.AUX0}/Aux0.lib")

	targetdir (binDir)
	objdir (binIntDir)

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/include/**.hpp",
		"%{prj.name}/API/**.hpp",
		"%{prj.name}/API/**.h"
	}

	includedirs
	{
		"%{IncludeDir.SPDLOG}",
		"%{IncludeDir.AUXAPIS}",
		"%{IncludeDir.AS_API}",
		"%{IncludeDir.COMM_API}"
	}

	filter "system:windows"
		systemversion "latest"
		buildoptions "/MD"

		defines {
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_DLL"
		}

	filter "architecture:x86_64"
		defines "X64"

	filter "architecture:x86"
		defines "X86"

	filter "configurations:Debug"
		defines "DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines	"RELEASE"
		optimize "on" 

	filter {}
	
	postbuildcommands{
		("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/TestApp"),	
		("{COPYFILE} ../"  .. binDir .. "/CommLayer.lib %{LibDir.COMM_LAYER}")		
	}
	
	
project "TestApp"
	location "TestApp"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	dependson {"CommLayer", "Aux0", "AgentSystem"}

	defines {"AS_TESTAPP"}

	flags
	{
		"MultiProcessorCompile"
	}

	links ("%{LibDir.AUX0}/Aux0.lib")
	links ("%{LibDir.COMM_LAYER}/CommLayer.lib")
	links ("%{LibDir.AGENT_SYSTEM}/AgentSystem.lib")

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/include/**.h",
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/include/**.hpp"
	}

	includedirs
	{
		"%{wks.location}/Dependencies/spdlog/include",
		"%{prj.name}/include",
		"%{IncludeDir.SPDLOG}",
		"%{IncludeDir.COMM_API}",
		"%{IncludeDir.AUXAPIS}",
		"%{IncludeDir.AS_API}"
	}

	filter "architecture:x86"
		defines	"X86"

	filter "architecture:x86_64"
		defines "X64"


	filter "system:windows"
		systemversion "latest"
		buildoptions "/MT"

		defines{
			"AS_PLATFORM_WINDOWS",
			"AS_BUILD_APP"
		}

	filter "configurations:Debug"
		defines "AS_DEBUG"
		symbols "on"

	filter "configurations:Release"
		defines	"AS_RELEASE"
		optimize "on" 		