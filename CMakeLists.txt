cmake_minimum_required(VERSION 3.11)
project(CLOUDS)

set(CLOUDS_EXECUTABLES_FOLDER       ${PROJECT_SOURCE_DIR}/src/exe)
set(CLOUDS_LIBRARIES_FOLDER			${PROJECT_SOURCE_DIR}/libs)
set(CLOUDS_EXTERNAL_FOLDER			${PROJECT_SOURCE_DIR}/ext)
set(CLOUDS_BINARIES_FOLDER			${PROJECT_SOURCE_DIR}/bin)
set(CLOUDS_RESOURCES_PATH			"../res")
set(CLOUDS_SHADERS_PATH				${PROJECT_SOURCE_DIR}/src/shaders)

find_package(OpenGL3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(GLM REQUIRED)

#LIST(APPEND CUSTOM_DEFINITIONS "/W4")
#LIST(APPEND CUSTOM_DEFINITIONS "/wd4201")

##### internal libraries
list(APPEND CLOUDS_INCLUDE_DIRECTORIES "${CLOUDS_LIBRARIES_FOLDER}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CLOUDS_BINARIES_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CLOUDS_BINARIES_FOLDER}")

file(GLOB children RELATIVE ${CLOUDS_LIBRARIES_FOLDER} ${CLOUDS_LIBRARIES_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${CLOUDS_LIBRARIES_FOLDER}/${subdir})
        file(GLOB_RECURSE CLOUDS_SOURCE_FILE "${CLOUDS_LIBRARIES_FOLDER}/${subdir}/*.cpp")
		file(GLOB_RECURSE CLOUDS_HEADER_FILE "${CLOUDS_LIBRARIES_FOLDER}/${subdir}/*.h")
        if(NOT CLOUDS_SOURCE_FILE)
            set(success 0)
        else()
            add_library(${subdir} ${CLOUDS_SOURCE_FILE} ${CLOUDS_HEADER_FILE})
            list(APPEND CLOUDS_LIBRARIES ${subdir})
            set_target_properties(${subdir} PROPERTIES
											LINKER_LANGUAGE CXX
											CXX_STANDARD 17
											CXX_STANDARD_REQUIRED ON)
            target_include_directories(${subdir} PUBLIC ${CLOUDS_INCLUDE_DIRECTORIES})
        endif()
    endif()
endforeach()
include_directories(${CLOUDS_INCLUDE_DIRECTORIES})

##### external libraries residing inside the project
list(APPEND CLOUDS_INCLUDE_DIRECTORIES ${CLOUDS_EXTERNAL_FOLDER})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  "${CLOUDS_BINARIES_FOLDER}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  "${CLOUDS_BINARIES_FOLDER}")

file(GLOB children RELATIVE ${CLOUDS_EXTERNAL_FOLDER} ${CLOUDS_EXTERNAL_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${CLOUDS_EXTERNAL_FOLDER}/${subdir})
        file(GLOB_RECURSE CLOUDS_SOURCE_FILE "${CLOUDS_EXTERNAL_FOLDER}/${subdir}/*.cpp""${CLOUDS_EXTERNAL_FOLDER}/${subdir}/*.c")
		file(GLOB_RECURSE CLOUDS_HEADER_FILE "${CLOUDS_EXTERNAL_FOLDER}/${subdir}/*.h")
        if(NOT CLOUDS_SOURCE_FILE)
            set(success 0)
        else()
            ### handle structures like: external/<something>/include as proper include paths
            if(IS_DIRECTORY "${CLOUDS_EXTERNAL_FOLDER}/${subdir}/include")
                list(APPEND CLOUDS_INCLUDE_DIRECTORIES "${CLOUDS_EXTERNAL_FOLDER}/${subdir}/include")
            endif()

            add_library(${subdir} ${CLOUDS_SOURCE_FILE} ${CLOUDS_HEADER_FILE})
            list(APPEND CLOUDS_LIBRARIES ${subdir})
            set_target_properties(${subdir} PROPERTIES
											LINKER_LANGUAGE CXX
											CXX_STANDARD 17
											CXX_STANDARD_REQUIRED ON)
            target_include_directories(${subdir} PUBLIC ${CLOUDS_INCLUDE_DIRECTORIES})
        endif()
    endif()
endforeach()
include_directories(${CLOUDS_INCLUDE_DIRECTORIES})

##### executables
file(GLOB children RELATIVE ${CLOUDS_EXECUTABLES_FOLDER} ${CLOUDS_EXECUTABLES_FOLDER}/*)
foreach(subdir ${children})
    if(IS_DIRECTORY ${CLOUDS_EXECUTABLES_FOLDER}/${subdir})
        file(GLOB_RECURSE EXE_SOURCES "${CLOUDS_EXECUTABLES_FOLDER}/${subdir}/*.cpp")
        	include_directories(
				${OpenGL3_INCLUDE_PATH}
				${GLEW_INCLUDE_PATH}
				${GLFW3_INCLUDE_PATH}
				${ASSIMP_INCLUDE_PATH}
				${GLM_INCLUDE_PATH}
				${EXTERNAL_LIBRARY_PATHS}
				${LIBRARIES_PATH}
			)
		add_executable(${subdir} ${EXE_SOURCES})
		target_link_libraries(
				${subdir}
				${CLOUDS_LIBRARIES}
				${GLFW3_LIBRARIES}
				${ASSIMP_LIBRARIES}
				${GLEW_LIBRARIES}
				${OpenGL3_LIBRARIES}
			)
        file(COPY ${CLOUDS_DLLS} DESTINATION "${CLOUDS_BINARIES_FOLDER}")
        set_target_properties(${subdir} PROPERTIES
										LINKER_LANGUAGE CXX
										RUNTIME_OUTPUT_DIRECTORY "${CLOUDS_BINARIES_FOLDER}/bin"
										CXX_STANDARD 17
										CXX_STANDARD_REQUIRED ON)
        target_include_directories(${subdir} PUBLIC ${CLOUDS_INCLUDE_DIRECTORIES})
		add_definitions(-DCLOUDS_SHADERS_PATH="${CLOUDS_SHADERS_PATH}")
		add_definitions(-DCLOUDS_RESOURCES_PATH="${CLOUDS_RESOURCES_PATH}")
		add_definitions(-DGLFW_INCLUDE_GLCOREARB)
		add_definitions(-DGLEW_STATIC)
		# glew.h sets __gl_h_ which makes gl3.h think /gl.h (OpenGL 2) is included. Calm that warning:
		add_definitions(-DGL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED)
		# ... and really don't include GLU and GL(2)
		add_definitions(-DGLEW_NO_GLU)
		add_definitions(-DGLM_FORCE_RADIANS)
    endif()
endforeach()

if(EXISTS ${CLOUDS_SHADERS_PATH})
	add_subdirectory(${CLOUDS_SHADERS_PATH})
endif()
