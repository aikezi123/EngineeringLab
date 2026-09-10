# EngineeringLab 工程文档

EngineeringLab 是个人 C++ 工程技术持续学习与实验平台。工程以可运行、可验证、可逐步演进为目标，当前包含 LearnOpenGL 课程、Qt/OpenGL 综合工作台、工业相机、轨迹算法和并发组件；后续可以继续学习 CAD、机器人、图形学、CUDA、IPC 等内容。

这些名称表示学习方向，不直接等同于顶层代码模块。源码继续按职责分为 domain、application、infrastructure、ui 和 composition_root；只有真实依赖或复用边界出现时，才拆分新的目录、端口和 CMake target。

## 文档索引

### 架构

- [代码架构与依赖关系](./DOC/architecture/ARCHITECTURE.md)：项目定位、当前结构、依赖规则和目标边界。

### 模块专题

- [相机采集与 OpenGL 显示链路](./DOC/modules/CAMERA_ARCHITECTURE.md)
- [线程池并发能力](./DOC/modules/THREAD_POOL.md)
- [日志模块](./DOC/modules/LOGGING.md)
- [二维阿基米德螺旋轨迹](./DOC/modules/TRAJECTORY_2D.md)

### 开发指南

- [编码规范](./DOC/guides/CODING_STYLE.md)
- [扩展指南](./DOC/guides/EXTENDING.md)
- [自动化测试](./DOC/guides/TESTING.md)
- [Windows CUDA 配置与验证](./DOC/guides/CUDA_WINDOWS.md)

### Agent 协作入口

- [项目协作规则](./.agents/AGENTS.md)
- [当前上下文](./.agents/CODEX_CONTEXT.md)

## 架构定位

稳定依赖方向为：

```text
composition_root ──> ui
composition_root ──> infrastructure
composition_root ──> application

ui ────────────────> application ──> domain
infrastructure ────> application ──> domain
```

各层职责：

- `domain/`：纯数据、数学模型和算法，例如图像帧、轨迹和以后真实需要的几何模型。
- `application/`：用例、流程和外部能力端口，例如相机采集流程。
- `infrastructure/`：OpenGL、CUDA、IPC、设备 SDK、并发和文件系统等具体技术实现。
- `ui/`：Qt 界面、交互和有效图形上下文中的显示行为。
- `composition_root/`：选择具体实现、创建对象并注入依赖。
- `lessons/`：保持可运行、可对照教程的课程代码，不要求为教学形式强套业务分层。
- `tools/`：独立命令行工具。

CAD、机器人、视觉和图形学可以同时使用多个层与能力目录，不预先建立对应的顶层模块。

## 当前 CMake target

```text
CudaSmoke (executable，ENGINEERINGLAB_ENABLE_CUDA=ON)
    └── CUDA::cudart_static (PRIVATE)

EngineeringWorkbench (executable)
    ├── englab::ui
    ├── englab::application
    ├── englab::camera_galaxy
    └── englab::logging

OpenGLLessons (executable)
    ├── englab::opengl_lessons
    │   ├── englab::graphics_opengl
    │   ├── glad
    │   ├── glfw3
    │   ├── OpenGL::GL
    │   ├── glm::glm
    │   └── stb_image
    └── englab::lesson_launcher
        ├── Qt6::Widgets
        └── englab::opengl_lessons (PRIVATE)

englab::ui
    ├── englab::application
    ├── englab::domain
    ├── englab::diagnostics
    └── Qt/OpenGL UI dependencies

englab::camera_galaxy
    ├── englab::application
    ├── englab::diagnostics
    └── Galaxy::SDK

englab::application
    ├── englab::domain
    └── englab::diagnostics

englab::diagnostics
    └── fmt::fmt（vcpkg，INTERFACE）

englab::concurrency
    └── Threads::Threads

englab::logging
    ├── englab::diagnostics
    └── spdlog::spdlog（vcpkg，PRIVATE）
```

OpenGL 课程不再链接包含相机 SDK 和并发实现的聚合 infrastructure target。OpenGL、Galaxy 相机、线程池和日志分别由 `englab::graphics_opengl`、`englab::camera_galaxy`、`englab::concurrency`、`englab::logging` 表达技术依赖。综合工作台的主界面、相机、显示和轨迹对象已共用组合根的进程级日志后端，当前尚未新增业务日志调用。

C++ 项目代码统一使用 `engineeringlab` 根命名空间；CMake alias 使用较短的 `englab::` 前缀。

## 当前功能

- `domain/image` 提供 `ImageFrame` / `PixelFormat`。
- `domain/trajectory` 提供二维阿基米德螺旋纯算法。
- `application/camera` 提供 `ICameraDevice` 和 `CameraCaptureService`。
- `infrastructure/camera/galaxy` 提供大恒相机适配器。
- `infrastructure/concurrency` 提供固定线程池。
- `application/diagnostics` 提供日志端口和 `ModuleLogger`；`infrastructure/logging` 提供 spdlog 实现；综合工作台的主窗口、相机服务及适配器、相机页面、图像显示、轨迹导出已注入同一个后端，当前尚未新增业务日志调用。
- `infrastructure/shader` 提供当前课程使用的 OpenGL Shader 封装。
- `ui` 提供相机预览、图像观察变换、轨迹生成与导出页面。
- `lessons` 保留 LearnOpenGL 课程注册和 GLFW 课程实现。

## 构建入口

源码入口按程序分组：

- [工作台入口](./composition_root/workbench/main.cpp)：初始化 Qt/OpenGL 和日志，再调用 [WorkbenchComposition](./composition_root/workbench/WorkbenchComposition.cpp) 汇总 `workbench/modules/` 中的功能装配。
- [课程程序入口](./composition_root/opengl_lessons/main.cpp)：解析参数、选择课程或启动导航窗口。
- [课程导航界面](./ui/lesson_launcher/src/LessonLauncherWindow.cpp)：独立 `englab::lesson_launcher` target，不依赖工作台 UI 或相机模块。

组合根只负责启动与装配；实际界面归入 `ui/`。日志保持显式依赖注入，不引入全局入口或单例。目录职责详见[架构文档](./DOC/architecture/ARCHITECTURE.md)。

fmt 12.1.0、spdlog 1.17.0 和 GoogleTest/GoogleMock 1.17.0 由根目录 `vcpkg.json` 统一管理，版本通过 `builtin-baseline` 固定。构建前需安装 vcpkg 并设置 `VCPKG_ROOT` 环境变量；CMake preset 会使用其 toolchain 自动恢复依赖。Windows preset 使用 `x64-windows-static-md`，即静态第三方库和动态 MSVC CRT。`vcpkg_installed/` 与依赖构建缓存不提交仓库。

Windows Debug/Release preset 同时启用独立的 `CudaSmoke` 示例，要求系统安装 CUDA Toolkit 12.6 或更新的兼容版本，并设置 `CUDA_PATH`。默认 GPU 架构为 `75`（RTX 2060）；使用其他 GPU 时应覆盖该参数。CUDA 使用 C++17 和静态 CUDA Runtime，不改变 MSVC CRT 设置；工作台和 OpenGL 课程不链接 CUDA。安装、禁用及运行方法见 [Windows CUDA 指南](./DOC/guides/CUDA_WINDOWS.md)。

Debug：

```powershell
.\msvc-cmake.ps1 -Config Debug -NoPause
```

Release：

```powershell
.\msvc-cmake.ps1 -Config Release -NoPause
```

测试：

```powershell
ctest --preset ninja-msvc-debug --output-on-failure
```

运行综合工作台：

```powershell
.\out\build\ninja-msvc-debug\bin\EngineeringWorkbench.exe
```

运行 OpenGL 课程：

```powershell
.\out\build\ninja-msvc-debug\bin\OpenGLLessons.exe
.\out\build\ninja-msvc-debug\bin\OpenGLLessons.exe transform
.\out\build\ninja-msvc-debug\bin\OpenGLLessons.exe --list
```

运行 CUDA 验证示例：

```powershell
.\out\build\ninja-msvc-debug\bin\CudaSmoke.exe
```

ASan preset 使用 `ENGINEERINGLAB_ENABLE_ASAN` 并关闭 CUDA，构建产物仍位于 `out/build/<preset>/bin`。
