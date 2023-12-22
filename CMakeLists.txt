cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
	beierneely.h
	beierneely.cpp
	image.h
	image.cpp
	camera.h
    camera.cpp
	input.h
	input.cpp
	event_handler.h
	event_handler.cpp
	ievent_subscriber.h
	event_subscriber.h
	event_subscriber.cpp
	ievent.h
	events.h
	shader.h
	shader.cpp
	texture.h
	texture.cpp
	fbo.h
	fbo.cpp
	editor.h
	editor.cpp
	render_common.h
	batch.h
	batch.cpp
	common.h
	common.cpp
	parser.h
	parser.cpp
	static_geometry.h
	static_geometry.cpp
	static_image_data.h
	static_image_data.cpp
    stb_image.h
	stb_image_write.h
	gif.h

	shaders/basic.vert
	shaders/basic.frag

	dependencies/imgui/imgui.h
	dependencies/imgui/imconfig.h
	dependencies/imgui/imgui_internal.h
	dependencies/imgui/imstb_rectpack.h
	dependencies/imgui/imstb_textedit.h
	dependencies/imgui/imstb_truetype.h

	dependencies/imgui/misc/cpp/imgui_stdlib.h
	dependencies/imgui/misc/cpp/imgui_stdlib.cpp

	dependencies/imgui/imgui.cpp
	dependencies/imgui/imgui_draw.cpp
	dependencies/imgui/imgui_tables.cpp
	dependencies/imgui/imgui_widgets.cpp
	dependencies/imgui/imgui_demo.cpp

	dependencies/imgui/backends/imgui_impl_opengl3.cpp
	dependencies/imgui/backends/imgui_impl_opengl3.h
	dependencies/imgui/backends/imgui_impl_sdl2.cpp
	dependencies/imgui/backends/imgui_impl_sdl2.h

	dependencies/tinyfiledialogs/tinyfiledialogs.h
	dependencies/tinyfiledialogs/tinyfiledialogs.c

	dependencies/glad/src/glad.c
)

target_include_directories(Morphing
    PUBLIC dependencies
	PUBLIC dependencies/glad/include
	PUBLIC dependencies/imgui
	PUBLIC dependencies/tinyfiledialogs
)

target_link_libraries(Morphing PUBLIC
    PUBLIC SDL2
    PUBLIC SDL2main
)
