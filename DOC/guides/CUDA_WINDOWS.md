# Windows CUDA 配置与验证

## 1. 当前范围

项目通过 CMake 原生 CUDA 语言编译 `.cu`，通过 `find_package(CUDAToolkit 12.6 REQUIRED)` 查找系统 Toolkit。vcpkg 继续管理现有通用库；本次不向 manifest 添加 CUDA，也不复制 Toolkit 到 `third_party/`。

`lessons/cuda/src/CudaSmoke.cu` 定义独立命令行示例 `CudaSmoke`，不进入 OpenGL 课程导航器或工作台。它上传 257 个包含负数、零和正数的整数，在 GPU 上执行 `value * 3 + 1`，回读并逐项校验。257 不是线程块大小 128 的整数倍，可验证最后一个 block 的边界。所有 CUDA API 错误都会报告，显存分配成功后的每条退出路径都会尝试释放显存。

正式复用的 CUDA 实现应随真实用例进入 infrastructure，不向 domain/application 暴露 CUDA 类型。Linux 移植尚未实施。

## 2. 环境及配置

需要 Windows x64、兼容的 MSVC 工具集、CUDA Toolkit 12.6 或更新的兼容版本，以及项目原有 Qt/vcpkg 环境。运行示例还需要兼容驱动和支持目标架构的 NVIDIA GPU。

安装后重新打开终端和 IDE，确认 `CUDA_PATH` 指向 Toolkit 根目录：

```powershell
$env:CUDA_PATH
& "$env:CUDA_PATH\bin\nvcc.exe" --version
nvidia-smi
```

若当前进程没有读取到安装后的系统环境变量，可仅刷新当前 PowerShell：

```powershell
$env:CUDA_PATH = [Environment]::GetEnvironmentVariable('CUDA_PATH', 'Machine')
$env:Path = "$env:CUDA_PATH\bin;$env:Path"
```

| 设置 | 当前配置 |
| --- | --- |
| `ENGINEERINGLAB_ENABLE_CUDA` | 普通 CMake 默认 OFF；Windows Debug/Release preset 为 ON |
| `CMAKE_CUDA_COMPILER` | `$env{CUDA_PATH}/bin/nvcc.exe`，无机器绝对路径 |
| `CMAKE_CUDA_ARCHITECTURES` | Windows Debug/Release 为 `75`，对应 RTX 2060 |
| CUDA 语言标准 | C++17，要求编译器支持 |
| 示例运行库 | `CUDA::cudart_static` 和 `CUDA_RUNTIME_LIBRARY Static` |

静态 CUDA Runtime 不改变 MSVC 动态 CRT 或 `x64-windows-static-md`。示例不需要部署 cudart DLL，但需要系统驱动。全局 `/utf-8` 按编译语言分发：C/C++ 直接传给 MSVC，CUDA 使用 `-Xcompiler=/utf-8` 转发。

ASan preset 关闭 CUDA，并取消继承的 CUDA 编译器和架构设置。当前全局 MSVC ASan 参数尚未适配 CUDA，同时启用两者会在配置阶段报错。

## 3. 构建和运行

普通 PowerShell 使用现有脚本自动初始化 MSVC 环境：

```powershell
powershell -ExecutionPolicy Bypass -File .\msvc-cmake.ps1 -Config Debug -NoPause
.\out\build\ninja-msvc-debug\bin\CudaSmoke.exe
```

Release 使用同一脚本的 `-Config Release`，程序位于 `out/build/ninja-msvc-release/bin/`。

在已初始化 MSVC x64 开发环境的终端中，可以只构建示例：

```powershell
cmake --preset ninja-msvc-debug
cmake --build --preset ninja-msvc-debug --target CudaSmoke
.\out\build\ninja-msvc-debug\bin\CudaSmoke.exe
```

成功时输出 `CUDA GPU check passed: 257/257 values correct.` 并返回 0；CUDA API 失败返回 1，计算结果不匹配返回 2。该硬件示例不注册到 CTest，GPU 检查和普通单元测试分别运行和报告。

## 4. 禁用和本机覆盖

没有 Toolkit 时，在 MSVC 开发终端中关闭 CUDA 后构建：

```powershell
cmake --preset ninja-msvc-debug -DENGINEERINGLAB_ENABLE_CUDA=OFF
cmake --build --preset ninja-msvc-debug
```

再次运行标准脚本会重新应用 preset 并启用 CUDA。持久禁用可在被 Git 忽略的 `CMakeUserPresets.json` 中定义继承 preset，覆盖开关为 OFF，并将继承的 `CMAKE_CUDA_COMPILER`、`CMAKE_CUDA_ARCHITECTURES` 设为 null。

更换 GPU 时在本机 preset 或 configure 命令中覆盖 `CMAKE_CUDA_ARCHITECTURES`。更换 CUDA 编译器或 MSVC 工具集时使用新的构建目录，避免混用旧编译器缓存。CMake 中的 `12.6` 是最低要求，不是完整环境锁定。

## 5. 已验证结果

2026-09-10：Windows Debug configure/build 成功；RTX 2060 上实际上传、执行和回读验证通过 257/257，现有 CTest 实际发现并通过 27/27。环境为 Toolkit 12.6.2（nvcc 12.6.77）、MSVC 19.44、NVIDIA 驱动 560.94。构建使用进程级 `VCPKG_BINARY_SOURCES=clear` 复用已安装依赖，未改变项目 preset 的缓存策略或系统环境。未执行 Release、ASan 构建或 GUI 验证。

## 6. 参考

- [CMake FindCUDAToolkit](https://cmake.org/cmake/help/v3.21/module/FindCUDAToolkit.html)
- [CMake Presets](https://cmake.org/cmake/help/v3.21/manual/cmake-presets.7.html)
