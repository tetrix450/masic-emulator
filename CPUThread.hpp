#ifndef CPUTHREAD_H
#define CPUTHREAD_H

#define FIRM_SIZE 262144
#include <QThread>
#include <cstdint>

class CPUThread : public QThread {
    Q_OBJECT
private:
    bool running = false;
    void run() override;
    double periodNs; // Periodo de reloj, por defecto 1MHz (1us)
    uint64_t firmware[FIRM_SIZE]; // Aquí se almacenarán todas las palabras de control del firmware

    // Señales, registros, biestables
    uint8_t BRQ = 0, IRQ = 0, IENT = 0;
    uint8_t DL = random()%256, DH = random()%256, PCL = 0, PCH = 0, SPL = 0, SPH = 0, RCF = 0, RI = 0, AC = random()%256, AUX = random()%256;
    uint16_t controlReg;
    uint8_t H = 0, Z = 0, V = 0, S = 0, C = 0;

    // Señales de control
    uint8_t sig_ient, sig_rcf_clr, sig_pc_oe, sig_mem_io, sig_f_pc, sig_sp_oe, sig_sp_load, sig_mux_add, sig_d_oe, sig_f_dl, sig_f_dh,
        sig_mem_we, sig_mem_oe, sig_d_clr, sig_fill_bit, sig_mux_zos, sig_mux_c_1, sig_mux_c_0, sig_s_dat_2, sig_s_dat_1, sig_s_dat_0, sig_s_ac_2,
        sig_s_ac_1, sig_s_ac_0, sig_bus_dis, sig_mux_ci_2, sig_mux_ci_1, sig_mux_ci_0, sig_f_h, sig_f_zos, sig_f_c, sig_iack, sig_back, sig_f_pc_up,
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
    void randomRAM();

    // Getters
    uint16_t getPC(){return ((PCH<<8)|PCL);};
    uint16_t getSP(){return ((SPH<<8)|SPL);};
    uint16_t getD(){return ((DH<<8)|DL);};
    uint8_t getAC(){return AC;};
    uint8_t getS(){return S;};
    uint8_t getC(){return C;};
    uint8_t getV(){return V;};
    uint8_t getZ(){return Z;};
    uint8_t getH(){return H;};
    uint8_t getAUX(){return AUX;};
    uint8_t getRCF(){return RCF;};
    uint8_t getRI(){return RI;};

    // Setters
    void setPC(uint16_t value){PCL = value&0xFF; PCH = (value>>8)&0xFF;};
    void setSP(uint16_t value){SPL = value&0xFF; SPH = (value>>8)&0xFF;};
    void setD(uint16_t value){DL = value&0xFF; DH = (value>>8)&0xFF;};
    void setAC(uint8_t value){AC = value;};
    void setS(uint8_t value){S = value;};
    void setC(uint8_t value){C = value;};
    void setV(uint8_t value){V = value;};
    void setZ(uint8_t value){Z = value;};
    void setH(uint8_t value){H = value;};
    void setAUX(uint8_t value){AUX = value;};
    void setRCF(uint8_t value){RCF = value;};
    void setRI(uint8_t value){RI = value;};

    void setPeriodNs(double periodNs);
    double getPeriodNs();
    bool isRunning(){return running;};
};

#endif // CPUTHREAD_H
