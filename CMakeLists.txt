cmake_minimum_required(VERSION 3.8)

project(Watermark)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/ui)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

set(RESSOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/ressources/icons.qrc)

set(TRANSLATIONS
	${CMAKE_CURRENT_SOURCE_DIR}/translations/wm_fr.ts)

add_subdirectory(sources)
