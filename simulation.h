#ifndef SIMULATION_H
#define SIMULATION_H

#include "ui_mainwindow.h"

namespace Ui {
class MainWindow;
}

class Simulation
{
public:
    Simulation(Ui::MainWindow *ui);
    void simulate(int nIter = 1e4);

private:
    Ui::MainWindow *ui;
    // Material parameters
    float laser_wl;
    float pump_wl;
    float upper_level_decay_time;
    float eff_stim_cross_section;
    float ion_density;
    float percent_excited;
    float n_crystal;
    float N_0;

    // Device parameters
    float absorption;
    float cavity_diam;
    float crystal_length;
    float init_photon_density;
    float pump_area_diam;
    float pump_power;
    float r1, r2;
    const int c = 3e8;
    const double h = 6.626e-34;
    float t_round;
    float cavity_decay_time;
    float pump_rate;
    float cavity_loss;

    // Saturable Absorber Parameters
    float N_tot_sa;
    float cross_ground;
    float cross_excited;
    float sa_decay_time;
    float sa_length;
    float n_sa;
};

#endif // SIMULATION_H
