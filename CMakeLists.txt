cmake_minimum_required(VERSION 3.10)


project(BV VERSION 1.0)

macro(ADD_SRC_HEADER_FILES )
	file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE
		${CMAKE_CURRENT_SOURCE_DIR} *.c??)
	file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE
		${CMAKE_CURRENT_SOURCE_DIR} *.h)    
endmacro()

# ADD_SRC_HEADER_FILES()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# if(${WINDOWING} STREQUAL "VKAL_SDL")
# 	message("Using SDL2...")
# 	add_subdirectory(src/dependencies/SDL)
# 	# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/dependencies/SDL)
# endif()

add_subdirectory(dependencies/SDL2.28.4)

# add_executable(DFT
# 	${SRC_FILES}
# 	${HEADER_FILES}
# )
	
add_executable(DFT
	dft.cpp
	camera.h
    camera.cpp
    stb_image.h
)

target_include_directories(DFT
    PUBLIC dependencies
)

target_link_libraries(DFT PUBLIC
    PUBLIC SDL2
    PUBLIC SDL2main
)

add_executable(Morphing
	morphing.cpp
	camera.h
    camera.cpp
    stb_image.h
	dependencies/glad/src/glad.c
)

target_include_directories(Morphing
    PUBLIC dependencies
	PUBLIC dependencies/glad/include
)

target_link_libraries(Morphing PUBLIC
    PUBLIC SDL2
    PUBLIC SDL2main
)
