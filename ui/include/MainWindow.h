#pragma once

#include <diagnostics/ModuleLogger.h>

#include <QMainWindow>

class QTreeWidgetItem;
class QWidget;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace engineeringlab::ui {

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(application::diagnostics::ILogger& logger, QWidget* parent = nullptr);
    ~MainWindow() override;

    // 将装配层创建好的功能页面注册到指定导航分类。
    void addBusinessPage(
        const QString& categoryName,
        const QString& pageName,
        QWidget* page
    );

private:
    void initUIStyle();
    void initPages();
    void connectSignals();

    QTreeWidgetItem* addCategoryNode(const QString& name);
    QTreeWidgetItem* findCategoryNode(const QString& name) const;
    void addPageToCategory(
        QTreeWidgetItem* parent,
        const QString& name,
        QWidget* page
    );
    void addRootBusinessPage(const QString& name, QWidget* page);
    QWidget* createHomePage();
    void decorateChildNodeUI(QTreeWidgetItem* item);

    // 非拥有后端引用；组合根保证 logger 晚于整个窗口对象树析构。
    application::diagnostics::ModuleLogger m_log;
    Ui::MainWindow* m_ui;
};

} // namespace engineeringlab::ui
