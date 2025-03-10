#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "paramdisplay.h"
#include <Qt3DRender>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ParamDisplay *disp;

private slots:
    void readFile();
    void writeFile();
    void runSimulation();
    void saturableAbsorber();
};
#endif // MAINWINDOW_H
