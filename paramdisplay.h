#ifndef PARAMDISPLAY_H
#define PARAMDISPLAY_H

#include "ui_mainwindow.h"

#include <Qt3DCore/qentity.h>
#include <QObject>

namespace Ui {
class MainWindow;
}

class ParamDisplay : public QObject
{
public:
    ParamDisplay(Ui::MainWindow *ui);

public slots:
    void changeCrystalLength();

private:
    Qt3DCore::QEntity *rootEntity;
    Qt3DCore::QEntity *m_cylinderEntity;
};

#endif // PARAMDISPLAY_H
