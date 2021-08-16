cmake_minimum_required(VERSION 3.3)
project(Tangine)
find_package(vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 CONFIG REQUIRED)
find_package(fcl REQUIRED)
INCLUDE_DIRECTORIES(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(
	3rdParty
    3rdParty/imgui
    3rdParty/imgui/impl
    3rdParty/tinyObjLoader
    3rdParty/glm 
    3rdParty/vulkan/include
    3rdParty/termcolor/
    3rdParty/vma/src
    ${VULKAN_INCLUDE_DIRS} 
    3rdParty/glfw/include
    include
    include/tEngine
    include/tComponent
    include/tPhysics)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")







set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Tangine)
add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
add_definitions(-DGLM_FORCE_DEPTH_ZERO_TO_ONE)
add_definitions(-DGLFW_INCLUDE_VULKAN)
add_definitions(-DASSET_PATH=${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(src)
add_subdirectory(example/MassSpring)
add_subdirectory(example/TexturedObj)
add_subdirectory(example/PCSS)
add_subdirectory(example/ParticlePhysicsDemo)
add_subdirectory(example/FlightSims)
add_subdirectory(example/AABB)
