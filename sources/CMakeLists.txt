set(HEADERS
	"cropform.hpp"
	"editor.hpp"
	"mainwindow.hpp"
	"preset.hpp"
	"preview.hpp"
	"reportissuedialog.hpp"
	"watermark.hpp"
	"watermarkform.hpp")

set(SOURCES
	"cropform.cpp"
	"editor.cpp"
	"main.cpp"
	"mainwindow.cpp"
	"preset.cpp"
	"preview.cpp"
	"reportissuedialog.cpp"
	"watermark.cpp"
	"watermarkform.cpp")

add_executable(${PROJECT_NAME} ${RESSOURCES} ${TRANSLATIONS} ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
