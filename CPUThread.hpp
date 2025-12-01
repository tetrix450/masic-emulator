#ifndef CPUTHREAD_H
#define CPUTHREAD_H

#define FIRM_SIZE 262144
#include <QThread>
#include <cstdint>

extern uint8_t vram[8192];

class CPUThread : public QThread {
    Q_OBJECT
private:
    bool running = false;
    void run() override;
    double periodNs; // Periodo de reloj, por defecto 1MHz (1us)
    uint64_t firmware[FIRM_SIZE]; // Aquí se almacenarán todas las palabras de control del firmware

    // Señales, registros, biestables
    uint8_t BRQ = 0, IRQ = 0, IFETCH = 0;
    uint8_t DL = 0xA0, DH = 0xA5, PCL = 0, PCH = 0, SPL = 0x16, SPH = 0x3A, RCF = 0, RI = 0, AC = 0x5F, AUX = 0x9D;
    uint16_t controlReg;
    uint8_t H = 0, Z = 0, V = 0, S = 0, C = 0;

    // Señales de control
    uint8_t sig_ifetch, sig_rcf_clr, sig_pc_oe, sig_mem_io, sig_f_pc, sig_sp_oe, sig_sp_load, sig_mux_add, sig_d_oe, sig_f_dl, sig_f_dh,
        sig_mem_we, sig_mem_oe, sig_d_clr, sig_fill_bit, sig_mux_zos, sig_mux_c_1, sig_mux_c_0, sig_s_dat_2, sig_s_dat_1, sig_s_dat_0, sig_s_ac_2,
        sig_s_ac_1, sig_s_ac_0, sig_bus_dis, sig_mux_ci_2, sig_mux_ci_1, sig_mux_ci_0, sig_f_i, sig_f_zos, sig_f_c, sig_iack, sig_back, sig_f_pc_up,
        sig_f_sp_up, sig_f_d_up, sig_f_ri, sig_f_aux, sig_f_ac, sig_f_sp_down;

    // Buses
    uint16_t get_bus_dir();
    uint8_t read_mem();
    uint8_t get_bus_dat();
    uint8_t get_bus_mem_dat();
    uint8_t get_bus_ci();
    uint8_t get_bus_ac();
    uint8_t get_bus_c();
    uint8_t get_bus_zos();
    void write_mem();

public:
    CPUThread();
    void reset();
    void step();
    void stepInstruction();
    void pause();
    uint16_t getPC(){return ((PCH<<8)|PCL);};
    void setPeriodNs(double periodNs);
    double getPeriodNs();
    bool isRunning(){return running;};
};

#endif // CPUTHREAD_H
