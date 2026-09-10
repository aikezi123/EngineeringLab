# .cu 文件由顶层 enable_language(CUDA) 启用的 CUDA 编译器处理。
add_executable(CudaSmoke
    src/CudaSmoke.cu
)

# 导入 target 同时提供 CUDA 头文件搜索路径和静态 Runtime 库的链接信息。
# PRIVATE 表示这是本 target 的实现依赖，不作为公共使用要求传给其他消费者。
target_link_libraries(CudaSmoke
    PRIVATE CUDA::cudart_static
)

# CUDA_RUNTIME_LIBRARY 控制 CUDA 语言的隐式 Runtime 链接选择，与上面的静态库保持一致。
# 若改用动态 Runtime，应同时改为 CUDA::cudart 和 CUDA_RUNTIME_LIBRARY Shared。
set_target_properties(CudaSmoke PROPERTIES
    CUDA_RUNTIME_LIBRARY Static
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
