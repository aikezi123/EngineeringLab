# 课程导航界面：只依赖 Qt Widgets 和课程目录，不链接工作台 UI 或相机模块。
add_library(engineeringlab_lesson_launcher STATIC
    include/LessonLauncherWindow.h
    src/LessonLauncherWindow.cpp
)

add_library(englab::lesson_launcher ALIAS engineeringlab_lesson_launcher)

target_include_directories(engineeringlab_lesson_launcher
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(engineeringlab_lesson_launcher
    PUBLIC
        Qt6::Widgets
    PRIVATE
        englab::opengl_lessons
)

target_compile_features(engineeringlab_lesson_launcher PUBLIC cxx_std_17)
