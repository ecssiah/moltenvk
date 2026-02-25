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

add_executable(
    vulkantest 
    src/main.cpp
    src/platform/renderer.cpp
    src/app/App.cpp
)

target_link_libraries(
    vulkantest
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)
