cmake_minimum_required(VERSION 3.20)

project(
    vulkantest 
    LANGUAGES C
)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

find_program(GLSLC glslc REQUIRED)

set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/assets/shaders/src)
set(SHADER_BIN_DIR ${CMAKE_SOURCE_DIR}/assets/shaders/bin)

file(
    GLOB 
    SHADER_SOURCES
    ${SHADER_SRC_DIR}/*.vert
    ${SHADER_SRC_DIR}/*.frag
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    
    set(SPIRV ${SHADER_BIN_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC} -Werror ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${FILE_NAME}"
        VERBATIM
    )

    list(APPEND SPIRV_SHADERS ${SPIRV})
endforeach()

add_custom_target(Shaders ALL DEPENDS ${SPIRV_SHADERS})

add_executable(
    vulkantest 
    ${CMAKE_SOURCE_DIR}/src/main.c
    ${CMAKE_SOURCE_DIR}/src/app/app.c
    ${CMAKE_SOURCE_DIR}/src/core/file.c
    ${CMAKE_SOURCE_DIR}/src/core/log.c
    ${CMAKE_SOURCE_DIR}/src/backend/vulkan/vulkan_backend.c
    ${CMAKE_SOURCE_DIR}/src/backend/vulkan/vulkan_device.c
    ${CMAKE_SOURCE_DIR}/src/backend/vulkan/vulkan_pipeline.c
    ${CMAKE_SOURCE_DIR}/src/backend/vulkan/vulkan_frame.c
    ${CMAKE_SOURCE_DIR}/src/backend/vulkan/vulkan_swapchain.c
    ${CMAKE_SOURCE_DIR}/src/platform/platform_window.c
    ${CMAKE_SOURCE_DIR}/src/renderer/mesh.c
    ${CMAKE_SOURCE_DIR}/src/renderer/texture.c
    ${CMAKE_SOURCE_DIR}/src/renderer/renderer.c
)

add_dependencies(
    vulkantest 
    Shaders
)

target_include_directories(
    vulkantest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(
    vulkantest
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)

add_custom_command(
    TARGET vulkantest POST_BUILD
    COMMAND 
        ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_SOURCE_DIR}/assets 
        $<TARGET_FILE_DIR:vulkantest>/assets
)