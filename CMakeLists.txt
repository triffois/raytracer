cmake_minimum_required(VERSION 3.20.0 FATAL_ERROR)

project(OpenGLTemplate)

#! Export compile_commands.json for lsps
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.hpp
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

find_package(OpenGL REQUIRED)

if (WIN32)
	set(LIBS glfw opengl32 glad)
elseif (UNIX)
	set(LIBS glfw GL glad)
endif ()

set(GLFW_DIR glfw)
set(GLAD_DIR glad)
set(TINYGLTF_DIR tinygltf)

add_subdirectory(${GLFW_DIR})
add_subdirectory(${GLAD_DIR})

target_include_directories(${PROJECT_NAME} 
    PRIVATE include
	PUBLIC ${GLFW_DIR}/include
    PUBLIC ${GLAD_DIR}/include
    PUBLIC ${TINYGLTF_DIR}
)

target_link_directories(${PROJECT_NAME} 
	PRIVATE ${GLFW_DIR}/src
    PRIVATE ${GLAD_DIR}/src
)

target_link_libraries(${PROJECT_NAME} ${LIBS})
