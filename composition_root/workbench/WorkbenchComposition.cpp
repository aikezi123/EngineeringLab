#include "WorkbenchComposition.h"

#include "modules/CameraComposition.h"
#include "modules/TrajectoryComposition.h"

#include <MainWindow.h>

#include <QMainWindow>
#include <QString>

#include <memory>

namespace engineeringlab::composition {

WorkbenchComposition::WorkbenchComposition(application::diagnostics::ILogger& logger) noexcept
    : m_logger(logger)
{
}

std::unique_ptr<QMainWindow> WorkbenchComposition::createMainWindow() const
{
    auto mainWindow = std::make_unique<ui::MainWindow>(m_logger);

    mainWindow->addBusinessPage(
        QStringLiteral("相机模块"),
        QStringLiteral("大恒相机预览"),
        CameraComposition::createPage(m_logger, mainWindow.get())
    );

    mainWindow->addBusinessPage(
        QStringLiteral("轨迹算法"),
        QStringLiteral("螺旋线导出"),
        TrajectoryComposition::createPage(m_logger, mainWindow.get())
    );

    return mainWindow;
}

} // namespace engineeringlab::composition
