cmake_minimum_required(VERSION 3.16)
project(VulkanUtils LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CPM_SOURCE_CACHE
    "${CMAKE_CURRENT_SOURCE_DIR}/.fetched"
    CACHE PATH "CPM source cache"
)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")

CPMAddPackage("gh:Mozzarella32/VulkanBindings#0c02a1434dd56e66abf79d1c3b9cd05b5e919ba8")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/utils")

CPMAddPackage("gh:Mozzarella32/ResourceLoader#c823482a6c185731d4d4cd2d310811cf9d0c0f19")

find_package(glfw3 QUIET)
if(NOT glfw3_FOUND)
    CPMAddPackage(
        NAME GLFW
        GITHUB_REPOSITORY glfw/glfw
        GIT_TAG 3.5.1
        OPTIONS
            "BUILD_SHARED_LIBS OFF"
            "GLFW_BUILD_TESTS OFF"
            "GLFW_BUILD_EXAMPLES OFF"
            "GLFW_BUILD_DOCS OFF"
            "GLFW_INSTALL OFF"
            "GLFW_VULKAN_STATIC OFF"
    )
endif()

find_package(Vulkan REQUIRED)

find_package(glm QUIET)
if(NOT glm_FOUND)
    CPMAddPackage("gh:g-truc/glm#1.0.3")
endif()

add_library(VkUtilsHeaders INTERFACE)
add_library(VkUtils::Headers ALIAS VkUtilsHeaders)

target_link_libraries(VkUtilsHeaders
    INTERFACE
        glfw
        glm::glm
        Vulkan::Vulkan
        VulkanUtils::Headers
        ResourceLoader::Headers
)

add_library(VkUtils INTERFACE)
add_library(VkUtils::Library ALIAS VkUtils)

target_link_libraries(VkUtils
    INTERFACE
        glfw
        glm::glm
        VulkanUtils::Library
        VkBindings::Library
        ResourceLoader::Library
        ResourceLoader::Preprocessed
    )
