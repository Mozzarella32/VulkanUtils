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

CPMAddPackage("gh:Mozzarella32/VulkanBindings#964c13e7adfcd462965f086a7debe6e41b9c4f3c")
CPMAddPackage("gh:Mozzarella32/ResourceLoader#bad3cc2370362a5dccc4e0b31c38a4b5cf3a578f")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/utils")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/glfw")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vma")

find_package(glm QUIET)
if(NOT glm_FOUND)
    CPMAddPackage("gh:g-truc/glm#1.0.3")
endif()

add_library(VkUtilsHeaders INTERFACE)
add_library(VkUtils::Headers ALIAS VkUtilsHeaders)

target_link_libraries(VkUtilsHeaders
    INTERFACE
        glm::glm
        GlfwUtils::Headers
        VmaBindings::Headers
        VulkanUtils::Headers
        ResourceLoader::Headers
)

add_library(VkUtils INTERFACE)
add_library(VkUtils::Library ALIAS VkUtils)

target_link_libraries(VkUtils
    INTERFACE
        glm::glm
        GlfwUtils::Library
        VmaBindings::Library
        VulkanUtils::Library
        VkBindings::Library
        ResourceLoader::Library
        ResourceLoader::Preprocessed
)
