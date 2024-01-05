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

if (WIN32)
	# res/app.rc
	SET(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc)
endif()

add_subdirectory(dependencies/SDL2.28.4)

# add_executable(DFT
# 	${SRC_FILES}
# 	${HEADER_FILES}
# )
	
add_executable(DFT
	dft.cpp
	camera.h
    camera.cpp
    dependencies/stb_image.h
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
	events.cpp
	shader.h
	shader.cpp
	texture.h
	texture.cpp
	fbo.h
	fbo.cpp
	editor.h
	editor.cpp
	processor.h
	processor.cpp
	render_common.h
	batch.h
	batch.cpp
	common.h
	common.cpp
	utils.h
	utils.cpp
	parser.h
	parser.cpp
	static_geometry.h
	static_geometry.cpp
	static_image_data.h
	static_image_data.cpp

    dependencies/stb_image.h
	dependencies/stb_image_write.h
	dependencies/gif.h

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

	${RESOURCE_FILES}
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

if (WIN32)
	set(ASSETS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/assets")
	set(SHADERS_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/shaders")

	set(DEST_ASSETS_DIR_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/assets")
	set(DEST_ASSETS_DIR_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/assets")
	set(DEST_ASSETS_DIR_RELDEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/assets")
	set(DEST_SHADERS_DIR_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug/shaders")
	set(DEST_SHADERS_DIR_RELEASE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/shaders")
	set(DEST_SHADERS_DIR_RELDEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/RelWithDebInfo/shaders")

	add_custom_command(TARGET Morphing POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
					${ASSETS_DIR}
					${DEST_ASSETS_DIR_DEBUG}
	)
	add_custom_command(TARGET Morphing POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
					${ASSETS_DIR}
					${DEST_ASSETS_DIR_RELEASE}
	)

	add_custom_command(TARGET Morphing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${SHADERS_DIR}
			${DEST_SHADERS_DIR_DEBUG}
	)
	add_custom_command(TARGET Morphing POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
				${SHADERS_DIR}
				${DEST_SHADERS_DIR_RELEASE}
	)

	add_custom_command(TARGET Morphing POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_directory
					${ASSETS_DIR}
					${DEST_ASSETS_DIR_RELDEBUG}
	)
	add_custom_command(TARGET Morphing POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
			${SHADERS_DIR}
			${DEST_SHADERS_DIR_RELDEBUG}
	)

endif()

# Is this needed???
# add_custom_target(copy_directory_target ALL DEPENDS ${DEST_ASSETS_DIR})


# file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
# file(COPY shaders DESTINATION ${CMAKE_BINARY_DIR})


