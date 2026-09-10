# 指定 CMake 最低版本
# 低于 3.21 的 CMake 不允许配置这个项目
cmake_minimum_required(VERSION 3.21)

# 定义项目名称
# LANGUAGES C CXX 表示项目同时启用 C 和 C++ 两种语言
# C   用来编译 glad.c
# CXX 用来编译项目中的 C++ 源文件和入口文件
project(EngineeringLab LANGUAGES C CXX)

# 提供标准 BUILD_TESTING 开关并在启用时注册 CTest。
include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
    ENGINEERINGLAB_ENABLE_ASAN
    "Enable AddressSanitizer for memory error diagnostics"
    OFF
)

# 定义一个保存在 CMakeCache.txt 中的布尔开关：变量名、说明文字、默认值。
# OFF 只在尚未设置该变量时作为默认值；preset 或 -D 传入的 ON 不会被覆盖。
# 当前 Windows Debug/Release preset 将它设为 ON，ASan preset 将它设为 OFF。
option(ENGINEERINGLAB_ENABLE_CUDA "Build CUDA examples" OFF)

# 开关为 ON 时才配置 CUDA；为 OFF 时跳过这一整段，不检测 CUDA 编译器或 Toolkit。
if(ENGINEERINGLAB_ENABLE_CUDA)
    # 本项目目前的 MSVC ASan 参数是全局设置，尚未适配 CUDA 编译。
    # 这是当前工程的配置限制，不表示 CUDA 与所有内存检查工具都不能共存。
    if(ENGINEERINGLAB_ENABLE_ASAN)
        # FATAL_ERROR 会立即终止 CMake 配置，避免之后出现难以理解的编译错误。
        message(FATAL_ERROR "CUDA examples do not support the current MSVC AddressSanitizer configuration. Set ENGINEERINGLAB_ENABLE_CUDA=OFF.")
    endif()

    # project() 原先只启用了 C/C++；这里增加 CUDA 语言并检测 nvcc 及宿主编译器。
    # 启用后，target 源文件列表里的 .cu 可以按 CUDA 源码编译。
    enable_language(CUDA)

    # 查找已安装的 Toolkit，不负责下载；12.6 是最低版本，REQUIRED 表示找不到就报错。
    # 查找成功后提供 CUDA::cudart、CUDA::cudart_static 等导入 target，携带头文件和库信息。
    find_package(CUDAToolkit 12.6 REQUIRED)

    # 为之后创建的 target 设置 CUDA 源码的默认 C++ 语言标准：C++17。
    # 这里的 17 不是 CUDA Toolkit 的版本号；CMAKE_CXX_STANDARD 不替代此设置。
    set(CMAKE_CUDA_STANDARD 17)
    # 必须支持所选标准，不能因编译器能力不足而自动退回较低标准。
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

if(ENGINEERINGLAB_ENABLE_ASAN)
    if(NOT MSVC)
        message(FATAL_ERROR "ENGINEERINGLAB_ENABLE_ASAN currently supports the MSVC toolchain only.")
    endif()

    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR "The current AddressSanitizer preset supports x64 builds only.")
    endif()

    add_compile_options(/fsanitize=address /Zi)
    add_link_options(/fsanitize=address /DEBUG /INCREMENTAL:NO)
endif()

# MSVC 使用 UTF-8，避免中文注释或字符串导致 C4819
if (MSVC)
    add_compile_options(
        "$<$<COMPILE_LANGUAGE:C,CXX>:/utf-8>"
        "$<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/utf-8>"
    )
endif()

# 把 third_party/glad 子目录加入构建系统
# 这个子目录负责定义 glad 这个库
add_subdirectory(third_party/glad)

# 把 third_party/glfw 子目录加入构建系统
# 这个子目录负责导入已经下载好的 glfw3.lib
add_subdirectory(third_party/glfw)

# 把 third_party/stb 子目录加入构建系统
add_subdirectory(third_party/stb)

# 把 third_party/glm 子目录加入构建系统
add_subdirectory(third_party/glm)

# 把 third_party/Galaxy 子目录加入构建系统
# 这个子目录负责把大恒 Galaxy SDK 的头文件和 .lib 包装成 CMake target
add_subdirectory(third_party/Galaxy)

# 开源依赖由 vcpkg manifest 提供，项目只消费标准 CMake target。
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
endif()

add_subdirectory(domain)
add_subdirectory(application)

add_subdirectory(infrastructure)

# OpenGL 课程代码依赖独立的 OpenGL 基础设施 target。
add_subdirectory(lessons)

add_subdirectory(ui)

# 程序入口层
add_subdirectory(composition_root)

if(BUILD_TESTING)
    # 测试作为最外层消费者，必须在所有生产 target 定义完成后加载。
    add_subdirectory(tests)
endif()
