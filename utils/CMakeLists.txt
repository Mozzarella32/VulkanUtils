if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Werror -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
endif()

file(GLOB_RECURSE UTILS_SRCS
  CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_library(VulkanUtils STATIC ${UTILS_SRCS})

target_include_directories(VulkanUtils
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/VkUtils"
)

add_library(VulkanUtils::Library ALIAS VulkanUtils)

add_library(VulkanUtilsHeaders INTERFACE)
add_library(VulkanUtils::Headers ALIAS VulkanUtilsHeaders)

target_include_directories(VulkanUtilsHeaders
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(VulkanUtilsHeaders
  INTERFACE
    VkBindings::Headers
    VmaBindings::Headers
)

target_link_libraries(VulkanUtils
  PUBLIC
    VkBindings::Headers
    VmaBindings::Headers
)

