if(DEFINED ENV{QT_ROOT})
    list(PREPEND CMAKE_PREFIX_PATH "$ENV{QT_ROOT}")
endif()

find_package(Qt6 6.9 CONFIG REQUIRED COMPONENTS
    Widgets
)

# 工作台启动与装配；功能实现由下层 target 提供。
add_executable(EngineeringWorkbench
    workbench/main.cpp
    workbench/WorkbenchComposition.h
    workbench/WorkbenchComposition.cpp
    workbench/modules/CameraComposition.h
    workbench/modules/CameraComposition.cpp
    workbench/modules/TrajectoryComposition.h
    workbench/modules/TrajectoryComposition.cpp
)

target_include_directories(EngineeringWorkbench
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/workbench
)

target_link_libraries(EngineeringWorkbench
    PRIVATE
        englab::application
        englab::camera_galaxy
        englab::logging
        englab::ui
        Qt6::Widgets
)

target_compile_features(EngineeringWorkbench PRIVATE
    cxx_std_17
)

# 课程程序入口；导航界面是独立 UI target，不依赖工作台 UI。
add_executable(OpenGLLessons
    opengl_lessons/main.cpp
)

target_link_libraries(OpenGLLessons PRIVATE
    englab::opengl_lessons
    englab::lesson_launcher
    Qt6::Widgets
)

target_compile_features(OpenGLLessons PRIVATE
    cxx_std_17
)

# 统一 exe 输出目录
set_target_properties(
    EngineeringWorkbench
    OpenGLLessons
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

foreach(galaxy_runtime_file IN LISTS GALAXY_RUNTIME_FILES)
    add_custom_command(
        TARGET EngineeringWorkbench
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            "${galaxy_runtime_file}"
            "$<TARGET_FILE_DIR:EngineeringWorkbench>"
        COMMENT "Deploying ${galaxy_runtime_file}"
        VERBATIM
    )
endforeach()

if(ENGINEERINGLAB_ENABLE_ASAN)
    get_filename_component(MSVC_COMPILER_DIRECTORY "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(
        MSVC_ASAN_RUNTIME_DLL
        "${MSVC_COMPILER_DIRECTORY}/clang_rt.asan_dbg_dynamic-x86_64.dll"
    )

    if(NOT EXISTS "${MSVC_ASAN_RUNTIME_DLL}")
        message(FATAL_ERROR "MSVC AddressSanitizer runtime was not found: ${MSVC_ASAN_RUNTIME_DLL}")
    endif()

    foreach(target_name IN ITEMS EngineeringWorkbench OpenGLLessons)
        add_custom_command(
            TARGET ${target_name}
            POST_BUILD
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different
                "${MSVC_ASAN_RUNTIME_DLL}"
                "$<TARGET_FILE_DIR:${target_name}>"
            COMMENT "Deploying the MSVC AddressSanitizer runtime for ${target_name}"
            VERBATIM
        )
    endforeach()
endif()
