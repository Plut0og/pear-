//
//  vCPU.cpp
//  pear][
//
//  Created by Sebastian Wyngaard on 7/5/18.
//  Copyright © 2018 Sebastian Wyngaard. All rights reserved.
//

#include <stdio.h>

char *to_bin(int v, unsigned n) {
    char* s = new char[n+1];
    for(int i = 0; i < n; i++){
        s[i] = ((v & (1 << (n-i-1))) >> (n-i-1)) ? '1' : '0';
    }
    s[n] = '\0';
    return s;
}


#include "vCPU.h"

vCPU::vCPU(char* mem){
    this->mem = mem;
    
    this->status = 0x00;
    
    this->a = 0x00;
    this->x = 0x00;
    this->y = 0x00;
    
    this->pc = PROGS;
    this->sp = 0xFF;
}

void vCPU::start(){
    while((~this->status) & FLAG_BREAK){
        printf("INIT:\n");
        printf("NV-BDIZC,     PC,  ACC,    X,    Y,  \n");
        printf("%s, %#06x, %#04x, %#04x, %#04x, \n", to_bin(this->status, 8), this->pc, this->a, this->x, this->y);
        this->fetch();
        this->decode();
        printf("POST FETCH:\n");
        printf("NV-BDIZC,     PC,  OPC, PARAMS,  ACC,    X,    Y,  \n");
        printf("%s, %#06x, %#04x, %#06x, %#04x, %#04x, %#04x, \n", to_bin(this->status, 8), this->pc, this->i.base, this->i.params, this->a, this->x, this->y);
        this->eval();
        printf("POST_EVAL:\n");
        printf("NV-BDIZC,     PC,  ACC,    X,    Y,  \n");
        printf("%s, %#06x, %#04x, %#04x, %#04x, \n", to_bin(this->status, 8), this->pc, this->a, this->x, this->y);
        printf("\n\n\n");
    }
}

void vCPU::fetch(){
    unsigned char size;
    unsigned char addr_mode_id;
    
    unsigned char opc = this->mem[this->pc];
    unsigned char set = opc & 0x03;
    unsigned char addr_mode = (opc & 0x1C) >> 2;
    unsigned char base = (opc & 0xE0) >> 5;
    switch(set){
        case(SET1): {
            switch(addr_mode){
                case(ADDR_NZX_1): size = SIZE_NX; addr_mode_id = ID_NX;break;
                case(ADDR_Z_1): size = SIZE_Z; addr_mode_id = ID_Z;break;
                case(ADDR_I_1): size = SIZE_I; addr_mode_id = ID_I;break;
                case(ADDR_A_1): size = SIZE_A; addr_mode_id = ID_A;break;
                case(ADDR_NZY_1): size = SIZE_NY; addr_mode_id = ID_NY;break;
                case(ADDR_ZX_1): size = SIZE_ZX; addr_mode_id = ID_ZX;break;
                case(ADDR_AY_1): size = SIZE_AY; addr_mode_id = ID_AY;break;
                case(ADDR_AX_1): size = SIZE_AX; addr_mode_id = ID_AX;break;
            }
        }
            break;
        case(SET2): {
            switch(addr_mode){
                case(ADDR_I_2): size = SIZE_I; addr_mode_id = ID_I;break;
                case(ADDR_Z_2): size = SIZE_Z; addr_mode_id = ID_Z;break;
                case(ADDR_C_2): size = SIZE_C; addr_mode_id = ID_C;break;
                case(ADDR_A_2): size = SIZE_A; addr_mode_id = ID_A;break;
                case(ADDR_ZX_2): size = SIZE_ZX; addr_mode_id = ID_ZX;break;
                case(ADDR_AX_2): size = SIZE_AX; addr_mode_id = ID_AX;break;
            }
        }
            break;
        case(SET3): {
            if(opc == 0){
                size = 1;
                addr_mode_id = ID_C;
                break;
            }
            switch(addr_mode){
                case(ADDR_I_3): size = SIZE_I; addr_mode_id = ID_I;break;
                case(ADDR_Z_3): size = SIZE_Z; addr_mode_id = ID_Z;break;
                case(ADDR_A_3): size = SIZE_A; addr_mode_id = ID_A;break;
                case(ADDR_ZX_3): size = SIZE_ZX; addr_mode_id = ID_ZX;break;
                case(ADDR_AX_3): size = SIZE_AX; addr_mode_id = ID_AX;break;
                case(BRANCH): size = SIZE_R; addr_mode_id = ID_R;break;
                case(OTHER1):
                case(OTHER2): printf("ehllo"); size = SIZE_M; addr_mode_id = ID_M;break;
            }
            break;
        }
    }
    unsigned short params;
    switch(size){
        case(1):
            params = 0x0000;
            break;
        case(2):
            params = (unsigned char)(this->mem[this->pc + 1]);
            break;
        case(3):
            params = (this->mem[this->pc + 1] << 4) + this->mem[this->pc + 2];
            break;
    }
    this->pc += size;
    this->i = (t_instr){size, base, set, addr_mode_id, params};
}

void vCPU::decode(){
    unsigned short params;
    switch(this->i.addr_mode){
        case(ID_M):
            params = 0x0000;
            break;
        case(ID_C):
            params = 0x0000;
            break;
        case(ID_I):
            params = this->i.params;
            break;
        case(ID_NX):
            params = this->mem[(unsigned char)this->mem[(unsigned char)(this->i.params + this->x)]];
            break;
        case(ID_NY):
            params = this->mem[(unsigned char)(this->mem[this->i.params] + this->y)];
            break;
        case(ID_R):
            params = this->i.params;
            break;
        case(ID_Z):
            params = this->mem[this->i.params];
            break;
        case(ID_ZX):
            params = this->mem[(unsigned char)(this->i.params + (unsigned char)this->x)];
            break;
        case(ID_ZY):
            params = this->mem[(unsigned char)(this->i.params + (unsigned char)this->y)];
            break;
        case(ID_N):
            params = this->mem[this->mem[this->i.params] + (this->mem[this->i.params + 1] << 4)];
            break;
        case(ID_A):
            params = this->mem[this->i.params];
            break;
        case(ID_AX):
            params = this->mem[this->i.params + (unsigned char)this->x];
            break;
        case(ID_AY):
            params = this->mem[this->i.params + (unsigned char)this->y];
            break;
    }
    this->i = {this->i.size, this->i.base, this->i.set, this->i.addr_mode, params};
}

void vCPU::eval(){
    unsigned char base = this->i.base;
    switch(this->i.set){
        case(SET1): {
            switch(base){
                case(ADC):instrADC();break;
                case(STA):instrSTA();break;
                case(LDA):instrLDA();break;
                case(CMP):instrCMP();break;
                case(SBC):instrSBC();break;
            }
        }
        case(SET2): {
            switch(base){
                case(DEC):instrDEC();break;
                case(INC):instrINC();break;
            }
        }
        case(SET3): {
            switch(this->i.addr_mode){
                //branching
                case(ID_R):
                    switch(base){
                        case(BNE):instrBNE();break;
                        case(BEQ):instrBEQ();break;
                    }
                //aka other
                case(ID_M):
                    switch(base){
                        case(CLC):instrCLC();break;
                        case(SEC):instrSEC();break;
                        case(PHA):instrPHA();break;
                        case(PLA):instrPLA();break;
                    }
                //aka special other
                case(ID_C):
                    switch(base){
                        case(BRK):instrBRK();break;
                    }
                //aka 3rd set
                default:
                    switch(base){
                        case(JMP_N):;break;
                        case(JMP_A):;break;
                    }
            }
        }
    }
}

void vCPU::instrADC(){
    char c = this->status & FLAG_CARRY;
    char sum = this->a + (char)i.params + c;
    //set the overflow
    char v = ~(a ^ i.params) & (a ^ sum) & 0x80;
    this->status = this->status | (v >> 1);
    
    //set the carry
    char newC = ((((a | i.params) & ~(sum)) | (a & i.params)) & 0x80) >> 7;
    printf("%#04x\n", newC);
    this->status = this->status | (newC);
    
    //set the negative
    this->status = this->status | (sum & 0x80);
    
    //set the zero
    if(sum == 0){
        this->status = this->status | FLAG_ZERO;
    }
    
    this->a = sum;
}
void vCPU::instrBEQ(){
    if((this->status & FLAG_ZERO) == 0){
        this->pc += (char)i.params;
    }
}
void vCPU::instrBNE(){
    if(this->status & FLAG_ZERO){
        this->pc += (char)i.params;
    }
}
void vCPU::instrBRK(){
    this->status = this->status | FLAG_BREAK; //set the interrupt
}
void vCPU::instrCLC(){
    this->status = this->status & ~(FLAG_CARRY);
}

void vCPU::instrCMP(){
    char temp = this->a - (char)i.params;
    
    
    
    if(temp == 0){
        this->status = this->status | FLAG_CARRY; //set the carry
        this->status = this->status | FLAG_ZERO; //set the z-flag
    } else if(temp < 0) {
        this->status = this->status | FLAG_NEGATIVE; //set negative
        this->status = this->status & ~(FLAG_CARRY); //clear the carry
        this->status = this->status & ~(FLAG_ZERO); //clear the z-flag
    } else if(temp > 0){
        this->status = this->status | FLAG_CARRY; //set the carry
        this->status = this->status & ~(FLAG_ZERO); //clear the z-flag
    }
}
void vCPU::instrDEC(){
    
}
void vCPU::instrINC(){
    this->mem[i.params] = this->mem[i.params] + 1;
}
void vCPU::instrJMP(){
    this->pc = i.params;
}
void vCPU::instrLDA(){
    this->a = i.params;
}
void vCPU::instrPHA(){
    mem[STACK + this->sp] = this->a;
    this->sp--;
}
void vCPU::instrPLA(){
    this->a = mem[STACK + this->sp];
}
void vCPU::instrSBC(){
    char c = this->status & FLAG_CARRY;
    char sum = this->a + ~(i.params) + c;
    
    //set the overflow
    char v = ~(a ^ i.params) & (a ^ sum) & 0x80;
    this->status = this->status | (v >> 1);
    
    //set the carry
    char newC = (((a | i.params) & ~(sum)) | (a & i.params)) & FLAG_CARRY;
    this->status = this->status | (newC >> 7);
    
    //set the negative
    this->status = this->status | (sum & 0x80);
    
    //set the zero
    if(sum == 0){
        this->status = this->status | FLAG_ZERO;
    }
    
    this->a = sum;
}
void vCPU::instrSEC(){
    this->status = this->status | FLAG_CARRY;
}
void vCPU::instrSTA(){
    mem[i.params] = this->a;
}


