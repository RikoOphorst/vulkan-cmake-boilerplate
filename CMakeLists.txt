cmake_minimum_required(VERSION 3.27)
project(MyVulkanProject)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set(CMAKE_VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")

file(GLOB_RECURSE files 
    src/*.cpp
    src/*.h
)

add_executable(MyVulkanApp ${files})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyVulkanApp)

# you must have the Vulkan SDK installed for this to work, make sure to enable all the optional features when installing the Vulkan SDK!
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined SPIRV-Tools)

# link vulkan
target_link_libraries(MyVulkanApp PUBLIC 
    Vulkan::Vulkan
    Vulkan::Headers
    Vulkan::SPIRV-Tools 
    Vulkan::shaderc_combined
)

# Configure variables for GLFW
set(BUILD_SHARED_LIBS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)

# Add GLFW
add_subdirectory(submodules/glfw)

# Link GLFW
target_link_libraries(MyVulkanApp PUBLIC glfw)

# Add GLFW includes
target_include_directories(MyVulkanApp PUBLIC "${CMAKE_SOURCE_DIR}/external/glfw/include")