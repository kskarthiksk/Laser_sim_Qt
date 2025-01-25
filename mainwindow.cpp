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
#include "paramdisplay.h"
#include<math.h>
#include<Qt3DRender>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tabWidget->setTabEnabled(2, false); // Disabling graph display tabs
    ui->tabWidget->setTabEnabled(3, false); // Disabling graph display tabs
    ui->tabWidget->setCurrentWidget(ui->tab_7);
    ui->tabWidget_2->setCurrentWidget(ui->tab_2);   // Setting material data tab as current widget

    // Connections to read file, write file and run simulation when corresponding button is clicked
    connect(ui->pushButtonReadXML, &QPushButton::clicked, this, &MainWindow::readFile);
    connect(ui->pushButtonWriteXML, &QPushButton::clicked, this, &MainWindow::writeFile);
    connect(ui->pushButtonRunSimulation, &QPushButton::clicked, this, &MainWindow::runSimulation);

    // Connection to enable/disable saturable absorber as per checkbox
    connect(ui->saturableAbsorber, &QCheckBox::clicked, this, &MainWindow::saturableAbsorber);

    // Creating object for 3d display
    disp = new ParamDisplay(ui);

    // Connections for changing length and diameter of cryatal and saturable absorber in display when corresponding doublespinbox is changed
    connect(ui->cavity_length, QOverload<double>::of(&QDoubleSpinBox::valueChanged), disp, disp->changeCrystalLength);
    connect(ui->cavity_diam, QOverload<double>::of(&QDoubleSpinBox::valueChanged), disp, disp->changeCrystalDiam);
    connect(ui->sa_length, QOverload<double>::of(&QDoubleSpinBox::valueChanged), disp, disp->changeSatAbsorberLength);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// --------------------------------------------------
// Read XML file
// --------------------------------------------------
void MainWindow::readFile()
{
    QDomDocument doc;   // Object representing xml document
    QString filePath = QFileDialog::getOpenFileName(this, "Open File", "", "*.xml");    // Open file dialog box to get filepath
    QFile file(filePath);

    // set content of file to 'doc', close file if not able to parse
    if(!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();    // Document element
    file.close();

    QDomNode material_data = docElem.firstChild();
    QDomNode params = material_data.firstChild();   // holds the material parameters

    while(!params.isNull()) // to not read the data in case the element is empty
    {
        QDomElement e = params.toElement();

        // make a list of all spinboxes under material data tab
        QList<QDoubleSpinBox *> material_data_spinboxes = ui->tab_2->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);

        // Reading all material data parameters from the xml file and writing it to the double spinboxes
        for(int i = 0; i < material_data_spinboxes.size(); ++i)
        {
            // each attribute's value is written to the double spinbox with the same object name as attribute name
            material_data_spinboxes[i]->setValue(e.attribute(material_data_spinboxes[i]->objectName()).toDouble());
        }
        params = params.nextSibling();
    }

    QDomNode device_data = material_data.nextSibling();
    params = device_data.firstChild();  // holds the device parameters

    while(!params.isNull()) // to not read the data in case the element is empty
    {
        QDomElement e = params.toElement();

        // make a list of all spinboxes under device data tab
        QList<QDoubleSpinBox *> device_data_spinboxes = ui->tab_3->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);

        // Reading all device data parameters from the xml file and writing it to the double spinboxes
        for(int i = 0; i < device_data_spinboxes.size(); ++i)
        {
            // each attribute's value is written to the double spinbox with the same object name as attribute name
            device_data_spinboxes[i]->setValue(e.attribute(device_data_spinboxes[i]->objectName()).toDouble());
        }
        params = params.nextSibling();
    }

    QDomNode saturable_absorber_data = device_data.nextSibling();
    params = saturable_absorber_data.firstChild();  // holds the saturable absorber parameters

    while(!params.isNull()) // to not read the data in case the element is empty
    {
        QDomElement e = params.toElement();

        if(e.attribute(ui->saturableAbsorber->objectName()).toInt() == 2)   // checks if saturable absorber checkbox is checked in saved data
        {
            ui->saturableAbsorber->setChecked(true);    // check the saturable absorber checkbox
            ui->frame->setEnabled(true);    // enable the saturable absorber parameter spinboxes

            // make a list of all spinboxes under saturable absorber tab
            QList<QDoubleSpinBox *> saturable_absorber_spinboxes = ui->tab->findChildren<QDoubleSpinBox *>();

            // Reading all saturable absorber parameters from the xml file and writing it to the double spinboxes
            for(int i = 0; i < saturable_absorber_spinboxes.size(); ++i)
            {
                // each attribute's value is written to the double spinbox with the same object name as attribute name
                saturable_absorber_spinboxes[i]->setValue(e.attribute(saturable_absorber_spinboxes[i]->objectName()).toDouble());
            }
        }
        else    // uncheck the saturable absorber checkbox and disable the parameter spinboxes if saturable absorber data is not available
        {
            ui->saturableAbsorber->setChecked(false);
            ui->frame->setEnabled(false);
        }
        params = params.nextSibling();
    }

}

// --------------------------------------------------
// Write XML file
// --------------------------------------------------
void MainWindow::writeFile()
{
    QDomDocument doc;   // Object representing xml document

    // Document element
    QDomElement laser_params = doc.createElement("laser_params");
    doc.appendChild(laser_params);

    // Material data element
    QDomElement material_data = doc.createElement("material_data");
    laser_params.appendChild(material_data);
    QDomElement material_data_default = doc.createElement("default");
    material_data.appendChild(material_data_default);

    // Material data element
    QDomElement device_data = doc.createElement("device_data");
    laser_params.appendChild(device_data);
    QDomElement device_data_default = doc.createElement("default");
    device_data.appendChild(device_data_default);

    // make a list of all spinboxes under material data tab
    QList<QDoubleSpinBox *> material_data_spinboxes = ui->tab_2->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);
    // Reading all material data parameters from the double spinboxes and writing it to the xml object
    for(int i = 0; i < material_data_spinboxes.size(); ++i)
    {
        // write the parameters as attributes where attribute name is same as object name of double spinbox
        material_data_default.setAttribute(material_data_spinboxes[i]->objectName(), material_data_spinboxes[i]->value());
    }

    // make a list of all spinboxes under device data tab
    QList<QDoubleSpinBox *> device_data_spinboxes = ui->tab_3->findChildren<QDoubleSpinBox *>(Qt::FindDirectChildrenOnly);
    // Reading all device data parameters from the double spinboxes and writing it to the xml object
    for(int i = 0; i < device_data_spinboxes.size(); ++i)
    {
        // write the parameters as attributes where attribute name is same as object name of double spinbox
        device_data_default.setAttribute(device_data_spinboxes[i]->objectName(), device_data_spinboxes[i]->value());
    }

    // Saturable absorber data element
    QDomElement saturable_absorber_data = doc.createElement("saturable_absorber_data");
    laser_params.appendChild(saturable_absorber_data);
    QDomElement saturable_absorber_default = doc.createElement("default");
    saturable_absorber_data.appendChild(saturable_absorber_default);
    // Storing the check state of the checkbox
    saturable_absorber_default.setAttribute(ui->saturableAbsorber->objectName(), ui->saturableAbsorber->checkState());
    if(ui->saturableAbsorber->isChecked())  // store the saturable absorber data if checkbox is checked
    {
        // make a list of all spinboxes under saturable absorber data tab
        QList<QDoubleSpinBox *> saturable_absorber_spinboxes = ui->tab->findChildren<QDoubleSpinBox *>();
        // Reading all saturable absorber parameters from the double spinboxes and writing it to the xml object
        for(int i = 0; i < saturable_absorber_spinboxes.size(); ++i)
        {
            // write the parameters as attributes where attribute name is same as object name of double spinbox
            saturable_absorber_default.setAttribute(saturable_absorber_spinboxes[i]->objectName(), saturable_absorber_spinboxes[i]->value());
        }
    }

    // save file dialog box to get filepath from user
    QString filePath = QFileDialog::getSaveFileName(this, "Save File", "", "XML files (*.xml)");
    QFile file(filePath);   // file object
    // convert xml object to text and write to the file
    if(file.open(QFile::WriteOnly | QFile::Text)){
        QTextStream in(&file);
        in<<doc.toString();
        file.flush();
        file.close();
    }
}

// --------------------------------------------------
// Run simulation
// --------------------------------------------------
void MainWindow::runSimulation()
{
    Simulation sim(ui); // initialize simulation object, reads all data from GUI
    sim.simulate(ui->timeStep_mantissa->value() * pow(10, ui->timeStep_exponent->value())); // runs the simulation
    // set the photon density graph as current widget, enables both graph tabs
    ui->tabWidget->setCurrentWidget(ui->tab_5);
    ui->tabWidget->setTabEnabled(2, true);
    ui->tabWidget->setTabEnabled(3, true);
}

// --------------------------------------------------
// Enable/disable editing for saturable absorber parameters
// --------------------------------------------------
void MainWindow::saturableAbsorber()
{
    if(ui->saturableAbsorber->isChecked())  // enable editing if checkbox is checked
    {
        ui->frame->setEnabled(true);
    }
    else
    {
        ui->frame->setEnabled(false);   // disable editing if checkbox is not checked
    }
}
