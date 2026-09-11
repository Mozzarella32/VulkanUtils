find_package(VulkanMemoryAllocator CONFIG QUIET)
if(NOT VulkanMemoryAllocator_FOUND)
    CPMAddPackage(
        NAME VMA
        GITHUB_REPOSITORY GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.4.0
        OPTIONS
            "VMA_ENABLE_INSTALL OFF"
    )
endif()

file(GLOB_RECURSE GLFW_UTILS_SRC
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(VmaBindings STATIC
    ${GLFW_UTILS_SRC}
)

target_include_directories(VmaBindings
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include/"
)

target_link_libraries(VmaBindings
    PRIVATE
        GPUOpen::VulkanMemoryAllocator
        VkBindings::Vulkan
        VkBindings::Headers
        VulkanUtils::Headers
)

add_library(VmaBindings::Library ALIAS VmaBindings)

add_library(VmaBindingsHeaders INTERFACE)

target_include_directories(VmaBindingsHeaders
    INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(VmaBindingsHeaders
    INTERFACE
        glfw
        VkBindings::Headers
)

add_library(VmaBindings::Headers ALIAS VmaBindingsHeaders)
