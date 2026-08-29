include(./.env.cmake OPTIONAL RESULT_VARIABLE LOCAL_ENV)
message(STATUS "Local .env.cmake: ${LOCAL_ENV}")

cmake_minimum_required(VERSION 3.11.0)
set(NAME ZEngine)

message(STATUS "using ${CMAKE_GENERATOR}")
if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
    if (NOT MINGW_PATH)
        message(FATAL_ERROR "MINGW_PATH not set in .env.cmake")
    endif()
    set(USE_MINGW "True")
    set(CMAKE_C_COMPILER ${MINGW_PATH}/bin/gcc.exe)
    set(CMAKE_CXX_COMPILER  ${MINGW_PATH}/bin/g++.exe)
endif()

project(${NAME} VERSION 0.23.0)

if (DEFINED VULKAN_SDK_PATH)
    set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include")
    set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib")
    set(Vulkan_FOUND "True")
else()
    find_package(Vulkan REQUIRED)
    message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan library!")
else()
    message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()


if (DEFINED GLFW_PATH)
    message(STATUS "Using GLFW path specified in .env")
    set(GLFW_INCLUDE_DIRS "${GLFW_PATH}/include")
    if (MSVC)
        set(GLFW_LIB "${GLFW_PATH}/lib-vc2019")
    elseif (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        message(STATUS "USING MINGW")
        set(GLFW_LIB "${GLFW_PATH}/lib-mingw-w64")
    endif()
else()
    find_package(glfw3 3.3 REQUIRED)
    set(GLFW_LIB glfw)
    message(STATUS "Found GLFW")
endif()
if (NOT GLFW_LIB)
    message(FATAL_ERROR "Could not find glfw library!")
else()
    message(STATUS "Using glfw lib at: ${GLFW_LIB}")
endif()

include_directories(external)
    
if (NOT TINYOBJ_PATH)
    message(STATUS "TINYOBJ_PATH not specified in .env.cmake, using external/tinyobjloader")
    set(TINYOBJ_PATH external/tinyobjloader)
endif()

if (NOT STBIMAGE_PATH)
    message(STATUS "STBIMAGE_PATH not specified in .env.cmake, using external/stb_image")
    set(STBIMAGE_PATH external/stb_image)
endif()

file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/ZEngine/src/*.cpp ${PROJECT_SOURCE_DIR}/ZEngine/src/*.h)
file(GLOB_RECURSE SHADERS ${PROJECT_SOURCE_DIR}/ZEngine/shaders/*.frag ${PROJECT_SOURCE_DIR}/ZEngine/shaders/*.vert)
file(GLOB_RECURSE LIBRARIES ${IMGUI_PATH}/*.cpp)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCE_DIR}/ZEngine/main.cpp
        ${SOURCES}
        ${SHADERS}
        ${LIBRARIES}
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build")

message(STATUS "CREATING BUILD FOR WINDOWS")

if (USE_MINGW)
    target_include_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/include
    )
    target_link_directories(${PROJECT_NAME} PUBLIC
            ${MINGW_PATH}/lib
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/ZEngine/src/
    ${Vulkan_INCLUDE_DIRS}
    ${TINYOBJ_PATH}
    ${GLFW_INCLUDE_DIRS}
    ${GLM_PATH} 
        ${STBIMAGE_PATH}
        ${IMGUI_PATH}
)

target_link_directories(${PROJECT_NAME} PUBLIC 
        ${Vulkan_LIBRARIES} 
        ${GLFW_LIB}
)

target_link_libraries(${PROJECT_NAME} glfw3 vulkan-1)

find_program(GLSL_COMPILER glslc HINTS
        ${VULKAN_SDK_PATH}/Bin
        ${VULKAN_SDK_PATH}/Bin32
        $ENV{VULKAN_SDK}/Bin
        $ENV{VULKAN_SDK}/Bin32
        /usr/bin
        /usr/local/bin
)

if(NOT GLSL_COMPILER)
    message(FATAL_ERROR "glslc compiler not found! Make sure Vulkan SDK is installed and VULKAN_SDK environment variable is set.")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${PROJECT_SOURCE_DIR}/ZEngine/shaders/*.frag"
        "${PROJECT_SOURCE_DIR}/ZEngine/shaders/*.vert"
)

set(SPIRV_BINARY_FILES "")

file(MAKE_DIRECTORY "${PROJECT_SOURCE_DIR}/ZEngine/shaders")

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${PROJECT_SOURCE_DIR}/ZEngine/shaders/${FILE_NAME}.spv")

    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${GLSL_COMPILER} "${GLSL}" -o "${SPIRV}"
            DEPENDS "${GLSL}"
            COMMENT "Compiling shader: ${FILE_NAME} -> ${FILE_NAME}.spv"
            VERBATIM
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

add_custom_target(
        Shaders ALL
        DEPENDS ${SPIRV_BINARY_FILES}
)

add_dependencies(${PROJECT_NAME} Shaders)