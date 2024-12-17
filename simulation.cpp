#include "simulation.h"
#include "ui_mainwindow.h"
#include<cmath>
#include<vector>
#include<QMessageBox>
#include<QDebug>
#include<QChart>
#include<QChartView>
#include<QLineSeries>
#include<QLogValueAxis>
#include<QValueAxis>

namespace Ui {
class MainWindow;
}

Simulation::Simulation(Ui::MainWindow *ui)
{
    this->ui = ui;
    laser_wl = ui->laser_wl->value() * 1e-9;
    pump_wl = ui->pump_wl->value() * 1e-9;
    upper_level_decay_time = ui->decay_time->value() * 1e-6;
    eff_stim_cross_section = ui->eff_stim_cross_section->value() * 1e-24;
    ion_density = ui->ion_density->value() * 1e26;
    percent_excited = ui->percent_excited->value() / 100;
    n_crystal = ui->n_crystal->value();

    absorption = ui->absorption->value() / 100;
    cavity_diam = ui->cavity_diam->value() / 1000;
    cavity_length = ui->cavity_length->value() / 1000;
    init_photon_density = ui->init_photon_density->value() * 1e11;
    pump_area_diam = ui->pump_area_diam->value() / 1000;
    pump_power = ui->pump_power->value();
    r1 = ui->r1->value();
    r2 = ui->r2->value();

    N_0 = ion_density * percent_excited;
    t_round = 2 * n_crystal * cavity_length / c;
    cavity_decay_time = - t_round / log(r1 * r2);
    pump_rate = pump_power * absorption / (M_PI * pow(pump_area_diam/2, 2) * cavity_length * h * c / pump_wl);
}

void Simulation::simulate(int nIter)
{
    float N_old = N_0;
    float N = N_0;
    float phi_old = init_photon_density;
    float phi = init_photon_density;
    std::vector<float> phi_arr = {init_photon_density};
    std::vector<float> N_arr = {N_0};
    // qDebug()<<eff_stim_cross_section;
    float dt = t_round;
    // int eta = 1;
    int countIter = 0;

    while(countIter < nIter or N > ion_density)
    {
        N = (pump_rate * dt + N_old) / (1 + dt * (phi_old * eff_stim_cross_section * c - pow(upper_level_decay_time, -1)));
        float gamma = N * eff_stim_cross_section * c - pow(cavity_decay_time, -1);
        if(gamma < 0)
            phi = phi_old / (1 - (dt * gamma));
        else
            phi = phi_old * (1 + (dt * gamma));
        phi_arr.push_back(phi);
        N_arr.push_back(N);
        ++countIter;
        phi_old = phi;
        N_old = N;
    }
    ui->phi_final->setValue(phi / 1e20);
    qDebug()<<N<<" "<<phi;
    // QMessageBox msgBox;
    // msgBox.setText(phi);
    // msgBox.exec();
    ui->N_final->setValue(N / 1e20);
    // QMessageBox msgBox;
    // msgBox.setText(N);
    // msgBox.exec();
    float intensity = ((phi * cavity_length) / t_round) * (1 - r1 * r2) * (h * c / laser_wl);
    float p_out = intensity * M_PI * pow(pump_area_diam / 2, 2);
    ui->op_power->setValue(p_out);

    auto photon_density_series = new QLineSeries;
    auto pop_inv_series = new QLineSeries;
    for(int i = 0; i < phi_arr.size(); ++i)
    {
        photon_density_series->append(i, phi_arr[i]);
        pop_inv_series->append(i, N_arr[i]);
    }
    auto photon_chart = new QChart;
    photon_chart->legend()->hide();
    photon_chart->addSeries(photon_density_series);
    auto axisX1 = new QValueAxis;
    photon_chart->addAxis(axisX1, Qt::AlignBottom);
    axisX1->setLabelFormat("%11.2e");
    axisX1->setTitleText("Timesteps");

    auto axisY1 = new QLogValueAxis;
    photon_chart->addAxis(axisY1, Qt::AlignLeft);
    axisY1->setBase(10.0);
    axisY1->setLabelFormat("%11.2e");
    axisY1->setTitleText("Photon Density");

    photon_density_series->attachAxis(axisX1);
    photon_density_series->attachAxis(axisY1);
    photon_chart->setTitle("Photon density");
    ui->PhotonChartView->setChart(photon_chart);

    auto pop_inv_chart = new QChart;
    pop_inv_chart->legend()->hide();
    pop_inv_chart->addSeries(pop_inv_series);
    auto axisX2 = new QValueAxis;
    pop_inv_chart->addAxis(axisX2, Qt::AlignBottom);
    axisX2->setLabelFormat("%11.2e");
    axisX2->setTitleText("Timesteps");
    pop_inv_series->attachAxis(axisX2);

    auto axisY2 = new QLogValueAxis;
    pop_inv_chart->addAxis(axisY2, Qt::AlignLeft);
    axisY2->setBase(10.0);
    axisY2->setLabelFormat("%11.2e");
    axisY2->setTitleText("Timesteps");
    pop_inv_series->attachAxis(axisY2);

    pop_inv_chart->setTitle("Population Inversion");
    ui->PopDensityChartView->setChart(pop_inv_chart);
}
