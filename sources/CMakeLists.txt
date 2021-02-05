configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/config.hpp" @ONLY)

set(HEADERS
    "config.hpp"
    "cropform.hpp"
    "editor.hpp"
    "mainwindow.hpp"
    "preset.hpp"
    "reportissuedialog.hpp"
    "watermark.hpp"
    "watermarkform.hpp")

set(SOURCES
    "cropform.cpp"
    "editor.cpp"
    "main.cpp"
    "mainwindow.cpp"
    "preset.cpp"
    "reportissuedialog.cpp"
    "watermark.cpp"
    "watermarkform.cpp")

add_executable(${PROJECT_NAME} ${RESSOURCES} ${HEADERS} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)
target_compile_definitions(${PROJECT_NAME} PRIVATE "-DQT_DISABLE_DEPRECATED_BEFORE=0x060000")
