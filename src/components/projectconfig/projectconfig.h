#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QtWidgets/QWidget>

namespace QtNodes {
class Node;
}

class ProjectConfigPrivate;

class ProjectConfig : public QWidget {
    Q_OBJECT
    Q_DECLARE_PRIVATE(ProjectConfig)

public:
    explicit ProjectConfig();
    ~ProjectConfig();
    void closeEvent(QCloseEvent* e);
    QByteArray save();
    void load(const QByteArray& data);
    void clearGraphView();

signals:
    void handleDock(QWidget* component);
    void handleWidgetDeletion(QWidget* widget);
    void handleWidgetShowing(QWidget* widget, bool docked);
    void componentWidgetCreated(QWidget* component);
    void stopSimulation();
    void startSimulation();
    void closeProject();

private slots:
    void nodeCreatedCallback(QtNodes::Node& node);
    void nodeDeletedCallback(QtNodes::Node& node);
    void nodeDoubleClickedCallback(QtNodes::Node& node);

private:
    QScopedPointer<ProjectConfigPrivate> d_ptr;
};

#endif // PROJECTCONFIG_H
