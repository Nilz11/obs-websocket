cmake_minimum_required(VERSION 3.2)

project(obs-websocket)

set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_BUILD_TYPE Release)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

include(external/FindLibObs.cmake)

find_package(LibObs REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5WebSockets REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_subdirectory(deps/mbedtls EXCLUDE_FROM_ALL)
add_subdirectory(deps/curlcpp)
set(ENABLE_PROGRAMS false)

set(obs-websocket_SOURCES 
	obs-websocket.cpp
	WSServer.cpp
	WSRequestHandler.cpp
	WSEvents.cpp
	Config.cpp
	Utils.cpp
	forms/settings-dialog.cpp)

set(obs-websocket_HEADERS
	obs-websocket.h
	WSServer.h
	WSRequestHandler.h
	WSEvents.h
	Config.h
	Utils.h
	forms/settings-dialog.h)

# --- Platform-independent build settings ---
add_library(obs-websocket MODULE 
	${obs-websocket_SOURCES}
	${obs-websocket_HEADERS})

add_dependencies(obs-websocket mbedcrypto)

include_directories( 
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5WebSockets_INCLUDES}
	${Qt5Widgets_INCLUDES}
	${mbedcrypto_INCLUDES}
	"${CMAKE_SOURCE_DIR}/deps/mbedtls/include"
	"${CMAKE_SOURCE_DIR}/deps/curlcpp/include"
	"${CMAKE_SOURCE_DIR}/socket-io/include"
)

target_link_libraries(obs-websocket 
	libobs
	Qt5::Core
	Qt5::WebSockets
	Qt5::Widgets
	mbedcrypto
	socket-io/lib/Release/libboost_random.a
	socket-io/lib/Release/libboost_date_time.a
	socket-io/lib/Release/libboost_system.a
	socket-io/lib/Release/libsioclient.a
)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(WIN32)
	if(NOT DEFINED OBS_FRONTEND_LIB)
		set(OBS_FRONTEND_LIB "OBS_FRONTEND_LIB-NOTFOUND" CACHE FILEPATH "OBS frontend library")
		message(FATAL_ERROR "Could not find OBS Frontend API\'s library !")
	endif()

	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
	endif()

	include_directories(
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/UI"
	)

	target_link_libraries(obs-websocket
		"${OBS_FRONTEND_LIB}")

	add_custom_command(TARGET obs-websocket POST_BUILD
		COMMAND if $<CONFIG:Release>==1 ("${CMAKE_COMMAND}" -E copy
			"${QTDIR}/bin/Qt5WebSockets.dll"
			"${QTDIR}/bin/Qt5Network.dll"
			"${CMAKE_BINARY_DIR}/$<CONFIG>")

		COMMAND if $<CONFIG:Debug>==1 ("${CMAKE_COMMAND}" -E copy
			"${QTDIR}/bin/Qt5WebSocketsd.dll"
			"${QTDIR}/bin/Qt5Networkd.dll"
			"${CMAKE_BINARY_DIR}/$<CONFIG>")
	)

	# --- Release package helper ---
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

	add_custom_command(TARGET obs-websocket POST_BUILD
		COMMAND if $<CONFIG:Release>==1 (
			"${CMAKE_COMMAND}" -E make_directory
			"${RELEASE_DIR}/data/obs-plugins/obs-websocket"
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")
			
		COMMAND if $<CONFIG:Release>==1 ("${CMAKE_COMMAND}" -E copy_directory
			"${PROJECT_SOURCE_DIR}/data"
			"${RELEASE_DIR}/data/obs-plugins/obs-websocket")

		COMMAND if $<CONFIG:Release>==1 ("${CMAKE_COMMAND}" -E copy 
			"$<TARGET_FILE:obs-websocket>"
			"${QTDIR}/bin/Qt5WebSockets.dll" 
			"${QTDIR}/bin/Qt5Network.dll" 
			"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		# Copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy
				"$<TARGET_FILE:obs-websocket>"
				"${QTDIR}/bin/Qt5WebSocketsd.dll"
				"${QTDIR}/bin/Qt5Networkd.dll"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E make_directory
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket")

		COMMAND if $<CONFIG:Debug>==1 (
			"${CMAKE_COMMAND}" -E copy_directory
				"${PROJECT_SOURCE_DIR}/data"
				"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-websocket")
	)
	# --- End of sub-section ---

endif()
# --- End of section ---

# --- Linux-specific build settings and tasks ---
if(UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

	target_compile_options(mbedcrypto PRIVATE -fPIC)
	set_target_properties(obs-websocket PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket
		obs-frontend-api)

	file(GLOB locale_files data/locale/*.ini)

	install(TARGETS obs-websocket
		LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/obs-plugins")
	install(FILES ${locale_files}
		DESTINATION "${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-websocket/locale")
endif()
# --- End of section ---

# -- OS X specific build settings and tasks --
if(APPLE)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fvisibility=default")

	set_target_properties(obs-websocket PROPERTIES PREFIX "")
	target_link_libraries(obs-websocket "${OBS_FRONTEND_LIB}")
endif()
# -- End of section --
