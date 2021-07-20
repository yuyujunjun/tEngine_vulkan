
project(Engine)

#Engine source
file(GLOB USER_SRC_PATH ./TGine/*)
set(SRC_FILES ${USER_SRC_PATH} )
#Engine source
file(GLOB USER_SRC_PATH ./TComponent/*)
set(SRC_FILES ${USER_SRC_PATH} ${SRC_FILES})
#Engine physics source
file(GLOB USER_SRC_PATH ./TPhysics/*)
set(SRC_FILES ${USER_SRC_PATH} ${SRC_FILES})


# Imgui
file(GLOB USER_SRC_PATH ../3rdParty/imgui/*.cpp)
set(SRC_FILES ${SRC_FILES} ${USER_SRC_PATH})
# Imgui for vulkan
file(GLOB USER_SRC_PATH ../3rdParty/imgui/impl/*.cpp)
set(SRC_FILES ${SRC_FILES} ${USER_SRC_PATH})
message(${SRC_FILES})
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

assign_source_group(${INCLUDE_DIRECTORIES})

add_library(Engine ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PUBLIC glfw ${Vulkan_LIBRARIES})

install(TARGETS Engine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)