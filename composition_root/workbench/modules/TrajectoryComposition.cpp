#include "TrajectoryComposition.h"

#include <TrajectoryExportView.h>

namespace engineeringlab::composition {

QWidget* TrajectoryComposition::createPage(
    application::diagnostics::ILogger& logger,
    QWidget* parent
)
{
    return new ui::TrajectoryExportView(logger, parent);
}

} // namespace engineeringlab::composition
