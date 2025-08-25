cmake_minimum_required(VERSION 3.19)
project(gm82audio CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "/O2 /W2 /WX /EHsc")

add_library(gm82audio SHARED source/gm82audio.cpp)

option(INSTALL_GEX "Install the generated .gex" OFF)

find_package(Python COMPONENTS Interpreter)

set(GEX_URL https://raw.githubusercontent.com/GM82Project/gm82gex/main/gm82gex.py)
set(GEX_PY ${CMAKE_SOURCE_DIR}/gm82gex.py)
file(DOWNLOAD ${GEX_URL} STATUS DOWNLOAD_STATUS)
list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
if(${STATUS_CODE} EQUAL 0)
	file(DOWNLOAD ${GEX_URL} ${GEX_PY})
else()
	message(WARNING "Couldn't redownload gm82gex.py")
endif()

if (INSTALL_GEX)
	add_custom_command(
		TARGET gm82audio
		POST_BUILD
		COMMAND ${Python_EXECUTABLE} ${GEX_PY} gm82audio.gej
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Building .gex"
	)
else()
	add_custom_command(
		TARGET gm82audio
		POST_BUILD
		COMMAND ${Python_EXECUTABLE} ${GEX_PY} gm82audio.gej --noinstall
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Building .gex"
	)
endif()
