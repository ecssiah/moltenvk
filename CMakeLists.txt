cmake_minimum_required(VERSION 3.20)

project(
    vulkantest 
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

find_program(GLSLC glslc REQUIRED)

set(SHADER_SRC_DIR ${CMAKE_SOURCE_DIR}/shaders/src)
set(SHADER_BIN_DIR ${CMAKE_SOURCE_DIR}/shaders/bin)

file(
    GLOB SHADER_SOURCES
    ${SHADER_SRC_DIR}/*.vert
    ${SHADER_SRC_DIR}/*.frag
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPIRV ${SHADER_BIN_DIR}/${FILE_NAME}.spv)

    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${GLSLC} ${SHADER} -o ${SPIRV}
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
    src/main.cpp
    src/platform/renderer.cpp
    src/app/App.cpp
)

add_dependencies(
    vulkantest 
    Shaders
)

target_link_libraries(
    vulkantest
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)
