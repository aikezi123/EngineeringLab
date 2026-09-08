#include "WorkbenchComposition.h"

#include <logging/SpdlogLogger.h>

#include <QApplication>
#include <QMainWindow>
#include <QSurfaceFormat>

#include <memory>

int main(int argc, char* argv[])
{
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setDepthBufferSize(24);
    QSurfaceFormat::setDefaultFormat(format);

    QApplication app(argc, argv);

    engineeringlab::infrastructure::logging::SpdlogLoggerOptions logOptions;
    logOptions.logFile = "logs/engineeringlab.log";

    // 进程只创建一个日志后端。它的生命周期覆盖窗口及其持有的全部业务对象，
    // 各模块通过 ILogger 引用共享同一个滚动日志文件。
    engineeringlab::infrastructure::logging::SpdlogLogger logger(logOptions);
    // 把唯一的 logger对象交给工作台装配对象。
    engineeringlab::composition::WorkbenchComposition composition(logger);
    std::unique_ptr<QMainWindow> window = composition.createMainWindow();
    window->show();

    return app.exec();
}
