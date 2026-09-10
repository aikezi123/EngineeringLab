#include <cuda_runtime.h>

#include <array>
#include <cstdio>

namespace engineeringlab::lessons {
namespace {

__global__ void transformValues(int* values, int count)
{
    const int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < count) {
        values[index] = values[index] * 3 + 1;
    }
}

bool checkCuda(cudaError_t status, const char* operation)
{
    if (status != cudaSuccess) {
        std::fprintf(stderr, "%s failed: %s\n", operation, cudaGetErrorString(status));
        return false;
    }
    return true;
}

} // namespace

int runCudaSmoke()
{
    // 非整块大小用于验证最后一个 block 的索引边界，同时覆盖负数、零和正数。
    constexpr int count = 257;
    constexpr int threadsPerBlock = 128;
    constexpr int blockCount = (count + threadsPerBlock - 1) / threadsPerBlock;
    std::array<int, count> values{};
    for (int index = 0; index < count; ++index) {
        values[index] = index - count / 2;
    }

    const auto byteCount = values.size() * sizeof(int);
    int* deviceValues = nullptr;
    if (!checkCuda(cudaMalloc(&deviceValues, byteCount), "cudaMalloc")) {
        return 1;
    }

    // 分配成功后，所有路径都先释放显存再退出。
    bool succeeded = checkCuda(
        cudaMemcpy(deviceValues, values.data(), byteCount, cudaMemcpyHostToDevice),
        "cudaMemcpy host to device");
    if (succeeded) {
        transformValues<<<blockCount, threadsPerBlock>>>(deviceValues, count);
        succeeded = checkCuda(cudaGetLastError(), "kernel launch");
    }
    if (succeeded) {
        succeeded = checkCuda(cudaDeviceSynchronize(), "kernel execution");
    }
    if (succeeded) {
        succeeded = checkCuda(
            cudaMemcpy(values.data(), deviceValues, byteCount, cudaMemcpyDeviceToHost),
            "cudaMemcpy device to host");
    }
    const bool freed = checkCuda(cudaFree(deviceValues), "cudaFree");
    if (!succeeded || !freed) {
        return 1;
    }

    for (int index = 0; index < count; ++index) {
        const int expected = (index - count / 2) * 3 + 1;
        if (values[index] != expected) {
            std::fprintf(stderr, "Mismatch at %d: expected %d, got %d\n",
                index, expected, values[index]);
            return 2;
        }
    }
    std::printf("CUDA GPU check passed: %d/%d values correct.\n", count, count);
    return 0;
}

} // namespace engineeringlab::lessons

int main()
{
    return engineeringlab::lessons::runCudaSmoke();
}
