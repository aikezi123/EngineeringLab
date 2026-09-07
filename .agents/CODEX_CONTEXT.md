# EngineeringLab 当前上下文

本文件是跨对话继续工作的精简快照。正式说明从 `README.md` 进入；若本文件与代码、CMake 或正式文档不一致，以当前实现为准并同步修正。

## 1. 项目定位

EngineeringLab 是个人 C++ 工程技术持续学习与实验平台。图形学、工业相机、轨迹算法、CAD、机器人、CUDA 和 IPC 等属于可交叉组合的学习方向，不直接等同于顶层代码模块。

工程继续使用 domain、application、infrastructure、ui 和 composition_root 分层。目录表达职责，CMake target 表达真实技术依赖；不为未来方向提前创建空目录、端口或包装层。

## 2. 当前命名与构建边界

- C++ 根命名空间：`engineeringlab`
- CMake alias 前缀：`englab::`
- 综合工作台：`EngineeringWorkbench`
- OpenGL 课程入口：`OpenGLLessons`
- 核心 target：`englab::domain`、`englab::application`、`englab::ui`
- 技术 target：`englab::graphics_opengl`、`englab::camera_galaxy`、`englab::concurrency`、`englab::logging`
- 日志端口 target：`englab::diagnostics`
- 课程 target：`englab::opengl_lessons`
- 课程导航 UI target：`englab::lesson_launcher`，独立于工作台 `englab::ui` 和相机模块。

组合根按程序分组：`composition_root/workbench/main.cpp` 创建日志后端并调用 `WorkbenchComposition`，功能装配在 `workbench/modules/`；`composition_root/opengl_lessons/main.cpp` 负责课程程序启动。课程导航界面位于 `ui/lesson_launcher/`。日志继续使用显式 `ILogger&` 注入，不采用全局日志入口。

OpenGL 课程只链接实际使用的 OpenGL、GLFW、GLAD、GLM 和 stb_image 能力，不再通过完整 infrastructure target 间接链接 Galaxy 相机或线程池。

## 3. 当前实现快照

- Domain 提供 `ImageFrame` / `PixelFormat` 和二维阿基米德螺旋算法。
- Application 提供 `ICameraDevice` 与带独立串行控制线程的 `CameraCaptureService`。
- `GalaxyCameraController` 实现相机端口，并通过 Pimpl 隐藏大恒 SDK。
- Qt 相机页面通过最新帧单槽邮箱切回 UI 线程；`DisplayOpenGLImage` 仍负责当前 OpenGL 纹理和观察变换。
- `englab::concurrency` 提供固定线程池，当前生产代码尚未使用，直接消费者只有测试。
- `englab::diagnostics` 提供日志端口和支持 fmt `{}` 格式化的 `ModuleLogger`，`englab::logging` 用私有 spdlog 1.17.0 后端实现；`EngineeringWorkbench` 的主窗口、相机服务、Galaxy 适配器、相机页面、图像显示和轨迹导出均已注入同一个后端，当前尚未新增业务日志调用。Designer 显示控件在 `setupUi()` 后通过 `setLogger()` 延迟注入，其余使用构造函数注入；模块名映射见 `DOC/modules/LOGGING.md`。
- `OpenGLLessons` 无参数时打开 Qt 课程导航器，带课程 ID 时运行对应 GLFW 课程。

## 4. 构建与验证

2026-09-07 组合根目录整理完成：Debug configure/build 成功，CTest 通过 27/27，`OpenGLLessons --list` 正常列出 6 门课程并返回 0。构建使用进程级 `VCPKG_BINARY_SOURCES=clear` 复用已安装依赖，未修改 preset 或全局环境。本轮未运行 GUI、真实相机、Release 或 ASan 验证。

2026-09-07 工作台各模块日志注入完成，Debug configure/build 成功，现有 CTest 通过 27/27（日志相关 7/7）。标准构建首次被本机 vcpkg 的 7-Zip 版本检测失败阻止；依赖已安装，使用进程级 `VCPKG_BINARY_SOURCES=clear` 临时关闭二进制缓存后验证成功，未修改 preset 或全局环境。本轮未运行 GUI、真实设备、Release 或 ASan 验证。

```powershell
powershell -ExecutionPolicy Bypass -File .\msvc-cmake.ps1 -Config Debug -NoPause
ctest --preset ninja-msvc-debug --output-on-failure
```

运行入口：

```powershell
.\out\build\ninja-msvc-debug\bin\EngineeringWorkbench.exe
.\out\build\ninja-msvc-debug\bin\OpenGLLessons.exe
.\out\build\ninja-msvc-debug\bin\OpenGLLessons.exe --list
```

2026-09-04 fmt 12.1.0、spdlog 1.17.0 和 GoogleTest/GoogleMock 1.17.0 已改由根目录 vcpkg manifest 管理，固定 baseline 为 `26283ac5e8a068561a718ce18b169bfad84c7dab`；仓库不再保存 spdlog 和 GoogleTest 的源码、头文件或二进制库。Windows preset 使用 `x64-windows-static-md`，保持静态依赖与动态 CRT。`ninja-msvc-debug` 和 `ninja-msvc-release` 均完成重新配置与构建，两个配置的 CTest 均发现并通过 27/27 个用例，其中日志相关测试 7/7；AddressSanitizer 尚未验证。本次已将日志接入 `EngineeringWorkbench` 组合根并注入 `CameraCaptureService`，Debug 重新配置与构建成功，CTest 通过 27/27；启动冒烟已创建 0 字节的 `logs/engineeringlab.log`，符合当前没有业务日志调用的状态。隐藏窗口无法通过 `CloseMainWindow()` 正常退出并被终止，正常交互关闭流程仍未验证；Release 和 AddressSanitizer 尚未针对本次接入重新验证。
