//
//  vCPU.h
//  pear][
//
//  Created by Sebastian Wyngaard on 7/4/18.
//  Copyright © 2018 Sebastian Wyngaard. All rights reserved.
//

#ifndef vCPU_h
#define vCPU_h

#include "opcodes.h"
#include "memLayout.h"

#define FLAG_NEGATIVE 0x80
#define FLAG_V 0x40
#define FLAG_BREAK 0x10
#define FLAG_DECIMAL 0x08
#define FLAG_INTERRUPT 0x40
#define FLAG_ZERO 0x02
#define FLAG_CARRY 0x01


typedef struct instruction {
    unsigned char size;
    unsigned char base;
    unsigned char set;
    unsigned char addr_mode;
    unsigned short params;
} t_instr;



class vCPU {
public:
    
    vCPU(char* mem);
    
    void start();
private:
    //main cpu run functions
    void fetch();
    void decode();
    void eval();
    
    //buses
    char* mem;
    
    t_instr i;
    
    //8-bit general-purpose registers
    char a;
    char x;
    char y;
    
    //contains flags
    unsigned char status; // NV-BDIZC
    // Negative, Overflow, Ignored, Break, Decimal, Interrupt, Zero, Carry
    
    unsigned char sp;
    unsigned short pc;
    
    //instruction implementations
    void instrADC();
    void instrBEQ();
    void instrBNE();
    void instrBRK();
    void instrCLC();
    void instrCMP();
    void instrDEC();
    void instrINC();
    void instrJMP();
    void instrLDA();
    void instrPHA();
    void instrPLA();
    void instrSBC();
    void instrSEC();
    void instrSTA();
};


#endif /* vCPU_h */
