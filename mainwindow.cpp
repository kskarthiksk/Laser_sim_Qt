#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QWidget>
#include<QPushButton>
#include<QFile>
#include<QDomDocument>
#include<QDomElement>
#include<QTextStream>
#include<QFileDialog>
#include<QList>
#include<QDoubleSpinBox>
#include<QMessageBox>
#include<QString>
#include<QCheckBox>
#include "simulation.h"
#include<math.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setTabEnabled(4, false);
    ui->tabWidget->setTabEnabled(5, false);
    ui->tabWidget->setCurrentWidget(ui->tab_2);

    connect(ui->pushButtonReadXML, &QPushButton::clicked, this, &MainWindow::readFile);
    connect(ui->pushButtonWriteXML, &QPushButton::clicked, this, &MainWindow::writeFile);
    connect(ui->pushButtonRunSimulation, &QPushButton::clicked, this, &MainWindow::runSimulation);
    connect(ui->saturableAbsorber, &QCheckBox::clicked, this, &MainWindow::saturableAbsorber);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readFile()
{
    QDomDocument doc;
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "", "*.xml");

    QFile file(filePath);
    if(!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    file.close();
    QDomNode material_data = docElem.firstChild();
    QDomNode params = material_data.firstChild();

    // QMessageBox msgBox;
    // msgBox.setText(params.toElement().tagName());
    // msgBox.exec();

    while(!params.isNull())
    {
        QDomElement e = params.toElement();
        QList<QDoubleSpinBox *> material_data_spinboxes = ui->tab_2->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);

        for(int i = 0; i < material_data_spinboxes.size(); ++i)
        {
            material_data_spinboxes[i]->setValue(e.attribute(material_data_spinboxes[i]->objectName()).toDouble());
        }
        params = params.nextSibling();
    }

    QDomNode device_data = material_data.nextSibling();
    params = device_data.firstChild();
    while(!params.isNull())
    {
        QDomElement e = params.toElement();
        QList<QDoubleSpinBox *> device_data_spinboxes = ui->tab_3->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);
        for(int i = 0; i < device_data_spinboxes.size(); ++i)
        {
            device_data_spinboxes[i]->setValue(e.attribute(device_data_spinboxes[i]->objectName()).toDouble());
        }
        params = params.nextSibling();
    }

    QDomNode saturable_absorber_data = device_data.nextSibling();
    params = saturable_absorber_data.firstChild();
    while(!params.isNull())
    {
        QDomElement e = params.toElement();
        if(e.attribute(ui->saturableAbsorber->objectName()).toInt() == 2)
        {
            ui->saturableAbsorber->setChecked(true);
            ui->frame->setEnabled(true);
            QList<QDoubleSpinBox *> saturable_absorber_spinboxes = ui->tab->findChildren<QDoubleSpinBox *>();
            for(int i = 0; i < saturable_absorber_spinboxes.size(); ++i)
            {
                saturable_absorber_spinboxes[i]->setValue(e.attribute(saturable_absorber_spinboxes[i]->objectName()).toDouble());
            }
        }
        else
        {
            ui->saturableAbsorber->setChecked(false);
            ui->frame->setEnabled(false);
        }
        params = params.nextSibling();
    }

}

void MainWindow::writeFile()
{
    QDomDocument doc;
    QDomElement laser_params = doc.createElement("laser_params");
    doc.appendChild(laser_params);
    QDomElement material_data = doc.createElement("material_data");
    laser_params.appendChild(material_data);
    QDomElement material_data_default = doc.createElement("default");
    material_data.appendChild(material_data_default);

    QDomElement device_data = doc.createElement("device_data");
    laser_params.appendChild(device_data);
    QDomElement device_data_default = doc.createElement("default");
    device_data.appendChild(device_data_default);

    QList<QDoubleSpinBox *> material_data_spinboxes = ui->tab_2->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);
    for(int i = 0; i < material_data_spinboxes.size(); ++i)
    {
        material_data_default.setAttribute(material_data_spinboxes[i]->objectName(), material_data_spinboxes[i]->value());
    }

    QList<QDoubleSpinBox *> device_data_spinboxes = ui->tab_3->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);
    for(int i = 0; i < device_data_spinboxes.size(); ++i)
    {
        device_data_default.setAttribute(device_data_spinboxes[i]->objectName(), device_data_spinboxes[i]->value());
    }

    QDomElement saturable_absorber_data = doc.createElement("saturable_absorber_data");
    laser_params.appendChild(saturable_absorber_data);
    QDomElement saturable_absorber_default = doc.createElement("default");
    saturable_absorber_data.appendChild(saturable_absorber_default);
    saturable_absorber_default.setAttribute(ui->saturableAbsorber->objectName(), ui->saturableAbsorber->checkState());
    if(ui->saturableAbsorber->isChecked())
    {
        QList<QDoubleSpinBox *> saturable_absorber_spinboxes = ui->tab->findChildren<QDoubleSpinBox *>();
        for(int i = 0; i < saturable_absorber_spinboxes.size(); ++i)
            saturable_absorber_default.setAttribute(saturable_absorber_spinboxes[i]->objectName(), saturable_absorber_spinboxes[i]->value());
    }

    QString filePath = QFileDialog::getSaveFileName(this, "Save File", "", "XML files (*.xml)");

    QFile file(filePath);
    if(file.open(QFile::WriteOnly | QFile::Text)){
        QTextStream in(&file);
        in<<doc.toString();
        file.flush();
        file.close();
    }
}

void MainWindow::runSimulation()
{
    // QList<QDoubleSpinBox *> data_spinboxes = ui->tabWidget->findChildren<QDoubleSpinBox *>();
    Simulation sim(ui);
    sim.simulate(ui->timeStep_mantissa->value() * pow(10, ui->timeStep_exponent->value()));
    ui->tabWidget->setCurrentWidget(ui->tab_5);
    ui->tabWidget->setTabEnabled(4, true);
    ui->tabWidget->setTabEnabled(5, true);
}

void MainWindow::saturableAbsorber()
{
    if(ui->saturableAbsorber->isChecked())
    {
        ui->frame->setEnabled(true);
    }
    else
    {
        ui->frame->setEnabled(false);
    }
}
