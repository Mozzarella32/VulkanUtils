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

file(GLOB_RECURSE GLFW_UTILS_SRC
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(GlfwUtils STATIC
    ${GLFW_UTILS_SRC}
)

target_include_directories(GlfwUtils
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

target_link_libraries(GlfwUtils
    PRIVATE
        glfw
        VkBindings::Vulkan
        VkBindings::Headers
        VulkanUtils::Headers
)

add_library(GlfwUtils::Library ALIAS GlfwUtils)

add_library(GlfwUtilsHeaders INTERFACE)

target_include_directories(GlfwUtilsHeaders
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(GlfwUtilsHeaders
    INTERFACE
        glfw
        VkBindings::Headers
)

add_library(GlfwUtils::Headers ALIAS GlfwUtilsHeaders )
