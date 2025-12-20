project(gm2wav)

option(USE_SYSTEM_SNDFILE "使用通过系统源安装的libgme" ON)

if(USE_SYSTEM_SNDFILE)
	find_library(LIBSNDFILE_DIR NAMES sndfile)
	find_path(LIBSNDFILE_INCLUDE_DIR NAMES sndfile)
	include_directories(${LIBGME_INCLUDE_DIR})
	message(${LIBSNDFILE_INCLUDE_DIR})
endif()
if(NOT USE_SYSTEM_SNDFILE)
	file(READ "${CMAKE_CURRENT_LIST_DIR}/cannot_haqi.txt" HAQI_IMAGE)
	message(FATAL_ERROR "\n${HAQI_IMAGE}\n您选择了不使用系统源内的libsndfile。请自行修改本CMakeLists.txt文件以自行支持，sry~")
endif()


option(USE_SYSTEM_LIBGME "使用通过系统源安装的libgme" OFF)

set(GME "game-music-emu")

if(USE_SYSTEM_LIBGME)
	file(READ "${CMAKE_CURRENT_LIST_DIR}/cannot_haqi.txt" HAQI_IMAGE)
	message(STATUS "\n${HAQI_IMAGE}\n您选择了使用系统源内的libgme，请确保版本>=0.6.4，否则即使编译过了也是用不了的。")
	add_compile_options(-DLIBGME064)

	find_library(LIBGME_DIR NAMES gme)
	find_path(LIBGME_INCLUDE_DIR NAMES gme)
	include_directories(${LIBGME_INCLUDE_DIR})
endif()
if(NOT USE_SYSTEM_LIBGME)
	#add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/${GME}")
	add_subdirectory(${GME})
	add_compile_options(-DLIBGME064)
	include_directories(${GME})
	#find_library(LIBGME_DIR NAMES gme PATHS "${CMAKE_CURRENT_SOURCE_DIR}/${GME}/gme/")
	set(LIBGME_DIR "${CMAKE_CURRENT_BINARY_DIR}/${GME}/gme/libgme.so")
	message("${GME}\t${LIBGME_DIR}")
endif()

set(SOURCES
	main.c
	converter.c
	playlist.c
)

add_executable(gm2wav ${SOURCES})
target_link_libraries(gm2wav ${LIBGME_DIR} ${LIBSNDFILE_DIR})
