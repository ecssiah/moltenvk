cmake_minimum_required(VERSION 3.20)

project(
    vulkantest 
    LANGUAGES C
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

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

add_executable(vulkantest)

target_sources(
    vulkantest
    PRIVATE
    external/nuklear/nuklear_loader.c
    external/stb/stb_image_loader.c
    src/main.c
    src/app/app.c
    src/app/camera.c
    src/app/world/world.c
    src/core/file.c
    src/core/log/log.c
    src/core/math/view.c
    src/core/math/projection.c
    src/platform/platform.c
    src/platform/platform_input.c
    src/render/image.c
    src/render/mesh.c
    src/render/texture.c
    src/render/vulkan_commands.c
    src/render/vulkan_memory.c
    src/render/vulkan_device.c
    src/render/vulkan_pipeline.c
    src/render/vulkan_frame.c
    src/render/vulkan_swapchain.c
    src/render/nuklear_context.c
    src/render/render.c
)

add_dependencies(
    vulkantest 
    Shaders
)

target_include_directories(
    vulkantest
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external
)

target_link_libraries(
    vulkantest
    PRIVATE
    Vulkan::Vulkan
    glfw
)

add_custom_command(
    TARGET vulkantest POST_BUILD
    COMMAND 
        ${CMAKE_COMMAND} -E copy_directory 
        ${CMAKE_SOURCE_DIR}/assets 
        $<TARGET_FILE_DIR:vulkantest>/assets
)