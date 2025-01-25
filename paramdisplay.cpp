#include "paramdisplay.h"

#include<QMainWindow>
#include<Qt3DCore>
#include<Qt3DExtras>
#include<Qt3DRender>
#include<QDebug>
#include<QObjectPicker>



ParamDisplay::ParamDisplay(Ui::MainWindow *ui, QObject *parent) : QObject(parent)
{

    this->ui = ui;

    // Creating a 3d window object and placing it in the UI
    Qt3DExtras::Qt3DWindow *view = new Qt3DExtras::Qt3DWindow();
    view->defaultFrameGraph()->setClearColor(QColor(QRgb(0x4d4d4f)));
    QWidget *container = QWidget::createWindowContainer(view);
    ui->horizontalLayout_4->addWidget(container);

    // Root entity
    Qt3DCore::QEntity *rootEntity = new Qt3DCore::QEntity();

    // Camera
    Qt3DRender::QCamera *cameraEntity = view->camera();

    cameraEntity->lens()->setPerspectiveProjection(45.0f, 16.0f/9.0f, 0.1f, 1000.0f);
    cameraEntity->setPosition(QVector3D(0, 0, 30.0f));
    cameraEntity->setUpVector(QVector3D(0, 1, 0));
    cameraEntity->setViewCenter(QVector3D(0, 0, 0));

    // Light
    Qt3DCore::QEntity *lightEntity = new Qt3DCore::QEntity(rootEntity);
    Qt3DRender::QPointLight *light = new Qt3DRender::QPointLight(lightEntity);
    light->setColor("white");
    light->setIntensity(1);
    lightEntity->addComponent(light);
    Qt3DCore::QTransform *lightTransform = new Qt3DCore::QTransform(lightEntity);
    lightTransform->setTranslation(cameraEntity->position());
    lightEntity->addComponent(lightTransform);

    // For camera controls. QOrbitCameraController enables rotation
    Qt3DExtras::QOrbitCameraController *camController = new Qt3DExtras::QOrbitCameraController(rootEntity);
    camController->setCamera(cameraEntity);

    // Set root object of the scene
    view->setRootEntity(rootEntity);

    // Cylinder shape data for crystal
    crystal = new Qt3DExtras::QCylinderMesh();
    crystal->setRadius(5);
    crystal->setLength(10);
    crystal->setRings(100);
    crystal->setSlices(20);

    // CylinderMesh Transform (translation and rotation)
    crystalTransform = new Qt3DCore::QTransform();
    /*crystalTransform->setScale(1.5f);
    cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));*/
    crystalTransform->setRotationX(90.0f);
    crystalTransform->setRotationY(-60.0f);
    // Translate crystal so that mid point comes between crystal and saturable absorber
    crystalTransform->setTranslation(QVector3D(-0.5 * crystal->length() * sqrt(3) / 2, 0.0f, crystal->length() *0.25));

    // Phong lighting effect
    Qt3DExtras::QPhongMaterial *crystalMaterial = new Qt3DExtras::QPhongMaterial();
    crystalMaterial->setDiffuse(QColor(QRgb(0x1e90ff)));

    // Crystal entity
    {
        m_crystalEntity = new Qt3DCore::QEntity(rootEntity);
        m_crystalEntity->addComponent(crystal);
        m_crystalEntity->addComponent(crystalMaterial);
        m_crystalEntity->addComponent(crystalTransform);

    }

    // Setting up a picker for crystal
    cr_picker = new Qt3DRender::QObjectPicker(m_crystalEntity);
    m_crystalEntity->addComponent(cr_picker);



    // Cylinder shape data for saturable absorber
    satAbsorber = new Qt3DExtras::QCylinderMesh();
    satAbsorber->setRadius(5);
    satAbsorber->setLength(10);
    satAbsorber->setRings(100);
    satAbsorber->setSlices(20);

    // CylinderMesh Transform (translation and rotation)
    satAbsTransform = new Qt3DCore::QTransform();
    // satAbsTransform->setScale(1.5f);
    // cylinderTransform->setRotation(QQuaternion::fromAxisAndAngle(QVector3D(1.0f, 0.0f, 0.0f), 45.0f));
    satAbsTransform->setRotationX(90.0f);
    satAbsTransform->setRotationY(-60.0f);
    // Translate crystal so that mid point comes between crystal and saturable absorber
    satAbsTransform->setTranslation(QVector3D(0.5 * satAbsorber->length() * sqrt(3) / 2, 0.0f, -0.5 * satAbsorber->length() * 0.5));

    // Phong lighting effect
    Qt3DExtras::QPhongMaterial *satAbsMaterial = new Qt3DExtras::QPhongMaterial();
    satAbsMaterial->setDiffuse(QColor(QRgb(0xfc8eac)));

    // Saturable absorber entity
    {
        m_satAbsorberEntity = new Qt3DCore::QEntity(rootEntity);
        m_satAbsorberEntity->addComponent(satAbsorber);
        m_satAbsorberEntity->addComponent(satAbsMaterial);
        m_satAbsorberEntity->addComponent(satAbsTransform);

    }

    // Setting up a picker for saturable absorber
    satAbsorber_picker = new Qt3DRender::QObjectPicker(m_satAbsorberEntity);
    m_satAbsorberEntity->addComponent(satAbsorber_picker);

    // Connecting the pickers to respective functions to be called when entity is picked
    connect(cr_picker, &Qt3DRender::QObjectPicker::clicked, this, ParamDisplay::selectCrystalLength);
    connect(satAbsorber_picker, &Qt3DRender::QObjectPicker::clicked, this, ParamDisplay::selectSatAbsorber);
}

// Change crystal length when cavity_length value is changed
void ParamDisplay::changeCrystalLength()
{
    crystal->setLength(ui->cavity_length->value());
    crystalTransform->setTranslation(QVector3D(-0.5 * crystal->length() * sqrt(3) / 2, 0.0f, crystal->length() *0.25));
}

// Change diameter of crystal and saturable absorber when cavity_diam value is changed
void ParamDisplay::changeCrystalDiam()
{
    crystal->setRadius(ui->cavity_diam->value()/2);
    satAbsorber->setRadius(ui->cavity_diam->value()/2);
}

// Change saturable absorber length when cavity_length value is changed
void ParamDisplay::changeSatAbsorberLength()
{
    satAbsorber->setLength(ui->sa_length->value());
    satAbsTransform->setTranslation(QVector3D(0.5 * satAbsorber->length() * sqrt(3) / 2, 0.0f, -0.5 * satAbsorber->length() * 0.5));
}

// Move cursor to cavity_length doublespinbox when crystal is selected
void ParamDisplay::selectCrystalLength()
{
    this->ui->tabWidget->setCurrentWidget(this->ui->tab_7);
    this->ui->tabWidget_2->setCurrentWidget(this->ui->tab_3);
    this->ui->cavity_length->setFocus();
}

// Move cursor to sa_length doublespinbox when saturable absorber is selected
void ParamDisplay::selectSatAbsorber()
{
    this->ui->tabWidget->setCurrentWidget(this->ui->tab_7);
    this->ui->tabWidget_2->setCurrentWidget(this->ui->tab);
    this->ui->sa_length->setFocus();
}
