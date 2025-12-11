#include "CPUThread.hpp"
#include <QMessageBox>
#include <chrono>
#include <cstdint>
#include <string>
#include <thread>
#include <QFile>
#define ACTIVE_LOW_MASK ((uint64_t)0x0000002EFF000E7F)

// Variables externas
extern void errorMessage(std::string message);
extern uint8_t mem[65536];
extern uint8_t io[65536];

CPUThread::CPUThread(){
    // Cargar firmware
    QString firmwareLocation = ":/firmware/";
    QByteArray archivos[5];

    for (int i = 0; i < 5; ++i) {
        QString filename = firmwareLocation + "firmware_" + QString::number(i) + ".bin";
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly)) {
            errorMessage("No se pudo abrir el archivo " + filename.toStdString());
            break;
        }

        archivos[i] = file.readAll();
    }

    // Se cargan todas las palabras de control
    for (int i = 0; i < FIRM_SIZE; i++) {
        uint8_t byte_0, byte_1, byte_2, byte_3, byte_4;

        byte_0 = archivos[0].at(i);
        byte_1 = archivos[1].at(i);
        byte_2 = archivos[2].at(i);
        byte_3 = archivos[3].at(i);
        byte_4 = archivos[4].at(i);

        firmware[i] =
            (uint64_t(byte_4) << 32) |
            (uint64_t(byte_3) << 24) |
            (uint64_t(byte_2) << 16) |
            (uint64_t(byte_1) << 8)  |
            (uint64_t(byte_0));
    }

    // Inicializar periodo del ciclo de reloj
    periodNs = 1000;
}

uint16_t CPUThread::get_bus_dir(){
    uint16_t value;

    if(sig_pc_oe){
        value = (((uint16_t)PCH)<<8) | (uint16_t)PCL;
    }else if(sig_d_oe){
        value = (((uint16_t)DH)<<8) | (uint16_t)DL;
    }else if(sig_sp_oe){
        value = (((uint16_t)SPH)<<8) | (uint16_t)SPL;
    }else{
        value = 0;
    }

    return value;
}

uint8_t CPUThread::read_mem(){
    uint16_t direccion = get_bus_dir();
    if(!sig_mem_io){    // Seleccionado espacio de memoria principal
        return mem[direccion];
    }else{              // Seleccionado espacio de E/S
        return io[direccion];
    }
}

uint8_t CPUThread::get_bus_dat(){
    int seleccion = (sig_s_dat_2<<2) | (sig_s_dat_1<<1) | sig_s_dat_0;
    enum bus_data_t {AUX_OE, AC_OE, DIRL_OE, DIRH_OE, MEM_BUS, EST_OE, FILL_BUS};

    switch(seleccion){
    default:
    case AUX_OE:
        return AUX;
        break;
    case AC_OE:
        return AC;
        break;
    case DIRL_OE:
        return get_bus_dir()&0xFF;
        break;
    case DIRH_OE:
        return (get_bus_dir()>>8)&0xFF;
        break;
    case MEM_BUS:
        return read_mem();
        break;
    case EST_OE:
        return (H<<4) | (Z<<3) | (V<<2) | (S<<1) | C; // Registro de estado
        break;
    case FILL_BUS:
        if(sig_fill_bit){
            return 0b11111111;
        }else{
            return 0b00000000;
        }
        break;
    }
}

uint8_t CPUThread::get_bus_mem_dat(){
    if(sig_mem_oe){
        return read_mem();
    }else if(sig_mem_we){
        return get_bus_dat();
    }else{
        return 0;
    }
}

uint8_t CPUThread::get_bus_ci(){
    int seleccion = (sig_mux_ci_2<<2) | (sig_mux_ci_1<<1) | sig_mux_ci_0;
    switch(seleccion){
    case 0:
        return 0;
        break;
    case 1:
        return 1;
        break;
    case 2:
        return C;
        break;
    case 3:
        return (AC>>7)&1;
        break;
    default:
    case 4:
        return AC&1;
        break;
    }
}

uint8_t CPUThread::get_bus_ac(){
    int seleccion = (sig_s_ac_2<<2) | (sig_s_ac_1<<1) | sig_s_ac_0;
    enum bus_ac_t {ADD_OE, SHR_OE, AND_OE, OR_OE, NOT_OE, BUS_AC};

    switch(seleccion){
    default:
    case ADD_OE:{
        uint16_t suma;
        if(!sig_mux_add){
            suma = (uint16_t)AC + (uint16_t)get_bus_dat() + (uint16_t)get_bus_ci();
        }else{
            suma = (uint16_t)AC + ~((uint16_t)get_bus_dat()) + (uint16_t)get_bus_ci();
        }

        return suma&0xFF;
        break;
    }
    case SHR_OE:
        return (AC>>1) + (get_bus_ci()<<7);
        break;
    case AND_OE:
        return AC & get_bus_dat();
        break;
    case OR_OE:
        return AC | get_bus_dat();
        break;
    case NOT_OE:
        return ~AC;
        break;
    case BUS_AC:
        return get_bus_dat();
        break;
    }
}

void CPUThread::clearMemory(){
    for(int i = 0; i < 65536; i++){
        mem[i] = 0;
        io[i] = 0;
    }
}

uint8_t CPUThread::get_bus_c(){
    int seleccion = (sig_mux_c_1<<1) | sig_mux_c_0;
    uint16_t suma;

    switch(seleccion){
    case 0:
        return get_bus_dat()&1;
        break;
    case 1:
        return (get_bus_dat()>>7)&1;
        break;
    default:
    case 2:
        uint16_t suma;
        if(!sig_mux_add){
            suma = (uint16_t)AC + (uint16_t)get_bus_dat() + (uint16_t)get_bus_ci();
            return suma>0xFF;
        }else{
            suma = (uint16_t)AC + ~((uint16_t)get_bus_dat()) + (uint16_t)get_bus_ci();
            return suma<=0xFF;
        }

    break;
    }
}

uint8_t CPUThread::get_bus_zos(){
    uint8_t temp_dat = get_bus_dat();

    if(sig_mux_zos){
        // Entrada desde la ALU
        uint8_t temp_z, temp_v, temp_s;

        // Z
        temp_z = get_bus_ac() == 0;

        // S
        temp_s = ((AC + temp_dat)>>7)&1;

        // V
        uint8_t ac_7 = (AC>>7)&1;
        uint8_t dat_7 = (temp_dat>>7)&1;
        temp_v = (ac_7 & dat_7 & !temp_s) | (!ac_7 & !dat_7 & temp_s);

        return (temp_z<<2) | (temp_v<<1) | temp_s;
    }else{
        // Entrada desde el bus de datos interno
        return (temp_dat>>1)&3;
    }
}

void CPUThread::write_mem(){
    uint16_t direccion = get_bus_dir();

    if(!sig_mem_io){ // Seleccionado espacio de memoria principal
        mem[direccion] = get_bus_mem_dat();
    }else{ // Seleccionado espacio de E/S
        io[direccion] = get_bus_mem_dat();
    }
}

void CPUThread::step(){
    // Primero se consigue la palabra de control
    controlReg = (uint16_t)0 | RCF | (C<<4) | (S<<5) | (V<<6) | (Z<<7) | (H<<8) | (BRQ<<9) | (IRQ<<10) | (IFETCH<<11);
    uint64_t controlInput = (uint64_t)0 | (controlReg<<6) | RI;
    uint64_t controlWord = firmware[controlInput];

    // Negar las señales activas a nivel bajo
    controlWord ^= ACTIVE_LOW_MASK;

    // Extraer cada una de las señales de control
    sig_ifetch = (controlWord>>39)&1;
    sig_rcf_clr = (controlWord>>38)&1;
    sig_pc_oe = (controlWord>>37)&1;
    sig_mem_io = (controlWord>>36)&1;
    sig_f_pc = (controlWord>>35)&1;
    sig_sp_oe = (controlWord>>34)&1;
    sig_sp_load = (controlWord>>33)&1;
    sig_mux_add = (controlWord>>32)&1;
    sig_d_oe = (controlWord>>31)&1;
    sig_f_dl = (controlWord>>30)&1;
    sig_f_dh = (controlWord>>29)&1;
    sig_mem_we = (controlWord>>28)&1;
    sig_mem_oe = (controlWord>>27)&1;
    sig_d_clr = (controlWord>>26)&1;
    sig_fill_bit = (controlWord>>25)&1;
    sig_mux_zos = (controlWord>>24)&1;
    sig_mux_c_1 = (controlWord>>23)&1;
    sig_mux_c_0 = (controlWord>>22)&1;
    sig_s_dat_2 = (controlWord>>21)&1;
    sig_s_dat_1 = (controlWord>>20)&1;
    sig_s_dat_0 = (controlWord>>19)&1;
    sig_s_ac_2 = (controlWord>>18)&1;
    sig_s_ac_1 = (controlWord>>17)&1;
    sig_s_ac_0 = (controlWord>>16)&1;
    sig_bus_dis = (controlWord>>15)&1;
    sig_mux_ci_2 = (controlWord>>14)&1;
    sig_mux_ci_1 = (controlWord>>13)&1;
    sig_mux_ci_0 = (controlWord>>12)&1;
    sig_f_i = (controlWord>>11)&1;
    sig_f_zos = (controlWord>>10)&1;
    sig_f_c = (controlWord>>9)&1;
    sig_iack = (controlWord>>8)&1;
    sig_back = (controlWord>>7)&1;
    sig_f_pc_up = (controlWord>>6)&1;
    sig_f_sp_up = (controlWord>>5)&1;
    sig_f_d_up = (controlWord>>4)&1;
    sig_f_ri = (controlWord>>3)&1;
    sig_f_aux = (controlWord>>2)&1;
    sig_f_ac = (controlWord>>1)&1;
    sig_f_sp_down = (controlWord>>0)&1;

    // ############### Para asignar todas las señales a la vez al final del ciclo ###############################
    uint8_t next_BRQ = BRQ, next_IRQ = IRQ, next_IFETCH = IFETCH;

    // Registros
    uint8_t next_DL = DL, next_DH = DH, next_PCL = PCL, next_PCH = PCH;
    uint8_t next_SPL = SPL, next_SPH = SPH, next_RCF = RCF, next_RI = RI, next_AC = AC, next_AUX = AUX;

    // Biestables
    uint8_t next_H = H, next_Z = Z, next_V = V, next_S = S, next_C = C;
    // ###########################################################################################################

    if(sig_f_ri){
        next_RI = get_bus_mem_dat();
    }

    if(sig_f_ac){
        next_AC = get_bus_ac();
    }

    if(sig_f_aux){
        next_AUX = AC;
    }

    if(sig_f_c){
        next_C = get_bus_c();
    }

    if(sig_d_clr){
        next_DL = 0;
        next_DH = 0;
    }

    if(sig_f_d_up){
        uint16_t next_D = ((uint16_t)(DH)<<8) | DL;
        next_D++;
        next_DL = next_D&0xFF;
        next_DH = (next_D>>8)&0xFF;
    }

    if(sig_f_dh){
        next_DH = get_bus_dat();
    }

    if(sig_f_dl){
        next_DL = get_bus_dat();
    }

    if(sig_f_i){
        next_H = (get_bus_dat()>>4)&1;
    }

    if(sig_f_pc){
        next_PCL = DL;
        next_PCH = DH;
    }

    if(sig_f_zos){
        uint8_t valor = get_bus_zos();

        next_Z = (valor>>2)&1;
        next_V = (valor>>1)&1;
        next_S = valor&1;
    }

    if(sig_sp_load){
        next_SPL = DL;
        next_SPH = DH;
    }

    if(sig_mem_we){
        write_mem();
    }

    if(sig_ifetch){
        next_IFETCH = 1;
    }else{
        next_IFETCH = 0;
    }

    if(sig_f_sp_down){
        uint16_t next_SP = ((uint16_t)(SPH)<<8) | SPL;
        next_SP--;
        next_SPL = next_SP&0xFF;
        next_SPH = (next_SP>>8)&0xFF;
    }else if(sig_f_sp_up){
        uint16_t next_SP = ((uint16_t)(SPH)<<8) | SPL;
        next_SP++;
        next_SPL = next_SP&0xFF;
        next_SPH = (next_SP>>8)&0xFF;
    }



    if(sig_f_pc_up){
        uint16_t next_PC = ((uint16_t)(PCH)<<8) | PCL;
        next_PC++;
        next_PCL = next_PC&0xFF;
        next_PCH = (next_PC>>8)&0xFF;
    }

    if(sig_rcf_clr){
        next_RCF = 0;
    }else{
        next_RCF++;
    }

    // ################### Actualizar los registros al final #####################
    // Señales de control de BRQ, IRQ e IFETCH
    BRQ = next_BRQ;
    IRQ = next_IRQ;
    IFETCH = next_IFETCH;

    // Biestables
    H = next_H;
    Z = next_Z;
    V = next_V;
    S = next_S;
    C = next_C;

    // Registros
    DL = next_DL;
    DH = next_DH;
    PCL = next_PCL;
    PCH = next_PCH;
    SPL = next_SPL;
    SPH = next_SPH;
    RCF = next_RCF;
    RI = next_RI;
    AC = next_AC;
    AUX = next_AUX;
}

void CPUThread::reset(){
    BRQ = 0; IRQ = 0; IFETCH = 0;
    DL = 0xA0; DH = 0xA5; PCL = 0; PCH = 0; SPL = 0x16; SPH = 0x3A; RCF = 0; RI = 0; AC = 0x5F; AUX = 0x9D;
    H = 0; Z = 0; V = 0; S = 0; C = 0;
}

void CPUThread::run() {
    using namespace std::chrono;

    running = true;
    while (running) {
        auto start = high_resolution_clock::now();

        step();

        auto end = high_resolution_clock::now();
        auto elapsed = duration_cast<nanoseconds>(end - start).count();
        long remaining = periodNs - elapsed;

        if (remaining > 0) {
            std::this_thread::sleep_for(nanoseconds(remaining));
        }
    }
}

void CPUThread::setPeriodNs(double periodNs){
    this->periodNs = periodNs;
}

double CPUThread::getPeriodNs(){
    return periodNs;
}

void CPUThread::stepInstruction(){
    do{
        step();
    }while(RCF != 0);
}

void CPUThread::pause(){
    running = false;
}
