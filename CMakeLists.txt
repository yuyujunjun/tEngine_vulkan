cmake_minimum_required(VERSION 3.3)
project(tEngineLib)
find_package(vulkan REQUIRED FATAL_ERROR)

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
    Source/include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT tEngineLib)
#Engine source
file(GLOB USER_SRC_PATH ./Source/*.cpp)
set(SRC_FILES ${USER_SRC_PATH} )
# Engine Include
file(GLOB USER_HEADER_PATH ./Source/include/*.h)
set(SRC_FILES ${SRC_FILES} ${USER_HEADER_PATH})


# Imgui
file(GLOB USER_SRC_PATH 3rdParty/imgui/*.cpp)
set(SRC_FILES ${SRC_FILES} ${USER_SRC_PATH})
# Imgui for vulkan
set(SRC_FILES ${SRC_FILES} "3rdParty/imgui/impl/imgui_impl_vulkan.cpp")

# file(GLOB_RECURSE USER_SRC_PATH 3rdParty/assimp/code/*.*)
# set(SRC_FILES ${SRC_FILES} ${USER_SRC_PATH})
# file(GLOB_RECURSE USER_SRC_PATH 3rdParty/assimp/code/*.h)
# set(SRC_FILES ${SRC_FILES} ${USER_SRC_PATH})


function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)
assign_source_group(${SRC_FILES})

add_definitions(-DVULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1)
add_definitions(-DGLFW_INCLUDE_VULKAN)

add_executable(${PROJECT_NAME} ${SRC_FILES})

option (VK_USE_PLATFORM_ANDROID_KHR "VK_USE_PLATFORM_ANDROID_KHR" ON)

set(tlibrary ${Vulkan_LIBRARIES})

target_link_libraries(${PROJECT_NAME}
		${tlibrary}
		)