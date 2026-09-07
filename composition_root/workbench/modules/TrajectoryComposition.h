#pragma once

class QWidget;

namespace engineeringlab::application::diagnostics {
class ILogger;
}

namespace engineeringlab::composition {

// 工作台轨迹装配：创建轨迹功能对应的 UI 页面并注入依赖。
class TrajectoryComposition final {
public:
    static QWidget* createPage(
        application::diagnostics::ILogger& logger,
        QWidget* parent = nullptr
    );
};

} // namespace engineeringlab::composition
