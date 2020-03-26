set(HEADERS
	"aboutdialog.hpp"
	"cropform.hpp"
	"editor.hpp"
	"licensedialog.hpp"
	"mainwindow.hpp"
	"preset.hpp"
	"preview.hpp"
	"reportissuedialog.hpp"
	"watermark.hpp"
	"watermarkform.hpp")

set(SOURCES
	"aboutdialog.cpp"
	"cropform.cpp"
	"editor.cpp"
	"licensedialog.cpp"
	"main.cpp"
	"mainwindow.cpp"
	"preset.cpp"
	"preview.cpp"
	"reportissuedialog.cpp"
	"watermark.cpp"
	"watermarkform.cpp")

add_executable(${PROJECT_NAME} ${RESSOURCES} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
