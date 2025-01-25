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
#include<QWidget>

namespace Ui {
class MainWindow;
}

Simulation::Simulation(Ui::MainWindow *ui)
{
    this->ui = ui;

    // Reading variable values from the doublespinboxes
    laser_wl = ui->laser_wl->value() * 1e-9;
    pump_wl = ui->pump_wl->value() * 1e-9;
    upper_level_decay_time = ui->decay_time->value() * 1e-6;
    eff_stim_cross_section = ui->eff_stim_cross_section->value() * 1e-24;
    ion_density = ui->ion_density->value() * 1e26;
    percent_excited = ui->percent_excited->value() / 100;
    n_crystal = ui->n_crystal->value();

    absorption = ui->absorption->value() / 100;
    cavity_diam = ui->cavity_diam->value() / 1000;
    crystal_length = ui->cavity_length->value() / 1000;
    init_photon_density = ui->init_photon_density->value() * 1e11;
    pump_area_diam = ui->pump_area_diam->value() / 1000;
    pump_power = ui->pump_power->value();
    r1 = ui->r1->value();
    r2 = ui->r2->value();
    cavity_loss = ui->cavityLoss->value()/100;

    // Read saturable absorber parameters if checkbox is checked. Otherwise assign 0 to all variables
    if(ui->saturableAbsorber->isChecked())
    {
        N_tot_sa = ui->sa_pop_density->value() * 1e23;
        cross_ground = ui->sa_gr_cross->value() * 1e-22;
        cross_excited = ui->sa_ex_cross->value() * 1e-23;
        sa_decay_time = ui->sa_decay->value() * 1e-6;
        sa_length = ui->sa_length->value() * 1e-3;
        n_sa = ui->sa_refractive->value();
    }
    else
    {
        N_tot_sa = 0;
        cross_ground = 0;
        cross_excited = 0;
        sa_decay_time = 0;
        sa_length = 0;
        n_sa = 0;
    }

    N_0 = ion_density * percent_excited;    // Initial excited state population
    t_round = 2 * (n_crystal * crystal_length + n_sa * sa_length) / c;  // Round trip time
    cavity_decay_time = - t_round / log(r1 * r2 * (1-cavity_loss));
    pump_rate = pump_power * absorption / (M_PI * pow(pump_area_diam/2, 2) * crystal_length * h * c / pump_wl);
}

void Simulation::simulate(int nIter)
{
    // Initializing variables
    float N_old = N_0;
    float N = N_0;
    float phi_old = init_photon_density;
    float phi = init_photon_density;
    float N_sa = N_tot_sa;
    std::vector<float> phi_arr = {init_photon_density};
    std::vector<float> N_arr = {N_0};
    std::vector<float> N_sa_arr = {N_tot_sa};
    float dt = t_round;
    // int eta = 1;
    int countIter = 0;

    // --------------------------------------------------
    // Iteration
    // --------------------------------------------------

    while(countIter < nIter && N < ion_density)
    {
        N = (pump_rate * dt + N_old) / (1 + dt * (phi_old * eff_stim_cross_section * c - pow(upper_level_decay_time, -1))); // Excited state population density
        float gamma = ((N * eff_stim_cross_section * 2 * crystal_length)
                       - (2 * sa_length * cross_ground * N_sa)
                       - (2 * sa_length * cross_excited * (N_tot_sa - N_sa))) / t_round - pow(cavity_decay_time, -1);
        // Do implicit or explicit calculation for photon density according to gamma
        if(gamma < 0)
            phi = phi_old / (1 - (dt * gamma));
        else
            phi = phi_old * (1 + (dt * gamma));
        phi_arr.push_back(phi);
        N_arr.push_back(N);
        ++countIter;
        phi_old = phi;
        N_old = N;
        // Calculate excited state population density for saturable absorber if applicable
        if(ui->saturableAbsorber->isChecked())
        {
            N_sa = (N_tot_sa * dt / sa_decay_time + N_sa) / (1 + dt * (phi * cross_ground * c + pow(sa_decay_time, -1)));
            N_sa_arr.push_back((N_sa));
        }
    }

    // Display final photon density and excited state population density
    ui->phi_final->setValue(phi / 1e20);
    ui->N_final->setValue(N / 1e20);

    // Calculating and displaying power
    float intensity = ((phi * crystal_length) / t_round) * (1 - r1 * r2) * (h * c / laser_wl);
    float p_out = intensity * M_PI * pow(pump_area_diam / 2, 2);
    ui->op_power->setValue(p_out);

    // --------------------------------------------------
    // Graphs
    // --------------------------------------------------

    // Creating two line series for drawing graphs
    auto photon_density_series = new QLineSeries;
    auto pop_inv_series = new QLineSeries;
    for(int i = 0; i < phi_arr.size(); ++i)
    {
        photon_density_series->append(i, phi_arr[i]);
        pop_inv_series->append(i, N_arr[i]);
    }

    // Photon density graph
    auto photon_chart = new QChart;
    photon_chart->legend()->hide();
    photon_chart->addSeries(photon_density_series); // Adds series to the chart

    // x-axis (linear)
    auto axisX1 = new QValueAxis;
    photon_chart->addAxis(axisX1, Qt::AlignBottom);
    axisX1->setLabelFormat("%1.2e");
    axisX1->setTitleText("Timesteps");

    // y-axis (logarithmic)
    auto axisY1 = new QLogValueAxis;
    photon_chart->addAxis(axisY1, Qt::AlignLeft);
    axisY1->setBase(10.0);
    axisY1->setLabelFormat("%1.2e");
    axisY1->setTitleText("Photon Density");
    axisY1->setTruncateLabels(false);

    // Attach axes to series
    photon_density_series->attachAxis(axisX1);
    photon_density_series->attachAxis(axisY1);

    photon_chart->setTitle("Photon density");
    ui->PhotonChartView->setChart(photon_chart); // Display chart

    // Population density chart
    auto pop_inv_chart = new QChart;
    pop_inv_chart->legend()->hide();
    pop_inv_chart->addSeries(pop_inv_series); // Add series to chart

    // x-axis (linear)
    auto axisX2 = new QValueAxis;
    pop_inv_chart->addAxis(axisX2, Qt::AlignBottom);
    axisX2->setLabelFormat("%1.2e");
    axisX2->setTitleText("Timesteps");
    pop_inv_series->attachAxis(axisX2); // Attach axis to series

    // y-axis (logarithmic)
    auto axisY2 = new QLogValueAxis;
    pop_inv_chart->addAxis(axisY2, Qt::AlignLeft);
    axisY2->setBase(10.0);
    axisY2->setLabelFormat("%1.2e");
    axisY2->setTitleText("Population Inversion Density");
    axisY2->setTruncateLabels(false);
    axisY2->setMinorTickCount(-1);
    pop_inv_series->attachAxis(axisY2); // Attach axis to series
    axisY2->setMax(pow(10, ceil(log10(axisY2->max()))));
    axisY2->setMin(pow(10, floor(log10(axisY2->min()))));

    pop_inv_chart->setTitle("Population Inversion");
    ui->PopDensityChartView->setChart(pop_inv_chart);   // Display chart
}
