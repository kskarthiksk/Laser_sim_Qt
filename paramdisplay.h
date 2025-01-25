#ifndef PARAMDISPLAY_H
#define PARAMDISPLAY_H

#include "ui_mainwindow.h"

#include <Qt3DCore/qentity.h>
#include <QObject>
#include <Qt3DExtras/qcylindermesh.h>
#include <Qt3DRender>

namespace Ui {
class MainWindow;
}

class ParamDisplay : public QObject
{
    Q_OBJECT
public:
    ParamDisplay(Ui::MainWindow *ui, QObject *parent = nullptr);

private:
    Ui::MainWindow *ui;
    Qt3DCore::QEntity *rootEntity;
    Qt3DExtras::QCylinderMesh *crystal;
    Qt3DCore::QTransform *crystalTransform;
    Qt3DCore::QEntity *m_crystalEntity;
    Qt3DRender::QObjectPicker *cr_picker;

    Qt3DExtras::QCylinderMesh *satAbsorber;
    Qt3DCore::QEntity *m_satAbsorberEntity;
    Qt3DCore::QTransform *satAbsTransform;
    Qt3DRender::QObjectPicker *satAbsorber_picker;

public slots:
    void changeCrystalLength();
    void changeCrystalDiam();
    void selectCrystalLength();

    void changeSatAbsorberLength();
    void selectSatAbsorber();
};

#endif // PARAMDISPLAY_H
