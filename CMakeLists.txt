cmake_minimum_required(VERSION 3.16)
project(VulkanUtils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory("${PROJECT_SOURCE_DIR}/submodules/VulkanBindings")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/utils")

add_subdirectory("${PROJECT_SOURCE_DIR}/submodules/ResourceLoader")

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
    option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
    option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
    option(GLFW_BUILD_DOCS "Build the GLFW documentation" OFF)
    option(GLFW_INSTALL "Generate installation target" OFF)
    option(GLFW_VULKAN_STATIC "Assume the Vulkan loader is linked with the application" OFF)    
    add_subdirectory("${PROJECT_SOURCE_DIR}/submodules/glfw")
endif()

find_package(Vulkan REQUIRED)

find_package(glm QUIET)
if(NOT glm_FOUND)
    add_subdirectory("${PROJECT_SOURCE_DIR}/submodules/glm")
endif()

add_library(VkUtilsHeaders INTERFACE)
add_library(VkUtils::Headers ALIAS VkUtilsHeaders)

target_link_libraries(VkUtilsHeaders
    INTERFACE
        glfw
        glm
        Vulkan::Vulkan
        VulkanUtils::Headers
        ResourceLoader::Headers
)

add_library(VkUtils INTERFACE)
add_library(VkUtils::Library ALIAS VkUtils)

target_link_libraries(VkUtils
    INTERFACE
        VulkanUtils::Library
        VkBindings::Library
        ResourceLoader::Library
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ResourceLoaderPreprocessedLibrary>"
    )
