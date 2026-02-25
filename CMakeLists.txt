cmake_minimum_required(VERSION 3.20)

project(vulkantest)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(
    vulkantest 
    main.cpp
)

target_link_libraries(
    vulkantest
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)