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
//#include<QMessageBox>
#include<QString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButtonReadXML, &QPushButton::clicked, this, &MainWindow::readFile);
    connect(ui->pushButtonWriteXML, &QPushButton::clicked, this, &MainWindow::writeFile);

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

    QString filePath = QFileDialog::getSaveFileName(this, "Save File", "", "XML files (*.xml)");

    QFile file(filePath);
    if(file.open(QFile::WriteOnly | QFile::Text)){
        QTextStream in(&file);
        in<<doc.toString();
        file.flush();
        file.close();
    }
}
