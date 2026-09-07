#pragma once

#include <memory>

class QMainWindow;

namespace engineeringlab::application::diagnostics {
class ILogger;
}

namespace engineeringlab::composition {

// 工作台总装配：创建顶层窗口，并把各功能模块装配到窗口中。
class WorkbenchComposition final {
public:
    explicit WorkbenchComposition(application::diagnostics::ILogger& logger) noexcept;

    std::unique_ptr<QMainWindow> createMainWindow() const;

private:
    // 非拥有引用；workbench/main.cpp 中的日志后端必须晚于本对象及窗口析构。
    application::diagnostics::ILogger& m_logger;
};

} // namespace engineeringlab::composition
