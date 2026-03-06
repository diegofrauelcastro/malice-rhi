
# Project name
project(MaliceRHI)

# Look for header and source files.
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "*.h")

add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/code/include)

# Ensure the computer has Vulkan SDK installed.
find_package(Vulkan)
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR
        "Vulkan SDK not found. Install it from https://vulkan.lunarg.com/")
endif()

# Link libraries.
target_link_libraries(${PROJECT_NAME}
	PUBLIC
    #Vulkan::Vulkan
	volk
	glfw
)

# Macro definitions.
target_compile_definitions(${PROJECT_NAME}
	PRIVATE VK_NO_PROTOTYPES
	GLFW_INCLUDE_NONE
)