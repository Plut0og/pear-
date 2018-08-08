//
//  opcodes.h
//  pear][
//
//  Created by Sebastian Wyngaard on 7/5/18.
//  Copyright © 2018 Sebastian Wyngaard. All rights reserved.
//

#ifndef opcodes_h
#define opcodes_h

#define SIZE_M 1
#define ID_M 0x00
#define SIZE_C 1 //accumulator
#define ID_C 0x01

#define SIZE_I 2
#define ID_I 0x02
#define SIZE_NX 2
#define ID_NX 0x03
#define SIZE_NY 2
#define ID_NY 0x04
#define SIZE_R 2
#define ID_R 0x05
#define SIZE_Z 2
#define ID_Z 0x06
#define SIZE_ZX 2
#define ID_ZX 0x07
#define SIZE_ZY 2
#define ID_ZY 0x08

#define SIZE_N 3
#define ID_N 0x09

#define SIZE_A 3
#define ID_A 0x0A
#define SIZE_AX 3
#define ID_AX 0x0B
#define SIZE_AY 3
#define ID_AY 0x0C


#define SET1 0x01

#define ADC 0x03
#define STA 0x04
#define LDA 0x05
#define CMP 0x06
#define SBC 0x07

#define ADDR_NZX_1 0x00
#define ADDR_Z_1 0x01
#define ADDR_I_1 0x02
#define ADDR_A_1 0x03
#define ADDR_NZY_1 0x04
#define ADDR_ZX_1 0x05
#define ADDR_AY_1 0x06
#define ADDR_AX_1 0x07

#define SET2 0x02

#define DEC 0x06
#define INC 0x07

#define ADDR_I_2 0x00
#define ADDR_Z_2 0x01
#define ADDR_C_2 0x02
#define ADDR_A_2 0x03
#define ADDR_ZX_2 0x05
#define ADDR_AX_2 0x07

#define SET3 0x00

#define JMP_N 0x02
#define JMP_A 0x03

#define ADDR_I_3 0x00
#define ADDR_Z_3 0x01
#define ADDR_A_3 0x03
#define ADDR_ZX_3 0x05
#define ADDR_AX_3 0x07

/* BRANCHING INSTRUCTIONS */

#define BRANCH 0x04

#define BNE 0x06
#define BEQ 0x07

/*SPECIAL OTHER INSTRUCTIONS */

#define BRK 0x00

/* OTHER INSTRUCTIONS */

#define OTHER1 0x06
#define OTHER2 0x02

/* FLAG INSTRUCTIONS */

#define CLC 0x00
#define SEC 0x01

/* STACK INSTRUCTIONS */

#define PHA 0x02
#define PLA 0x03


#endif /* opcodes_h */
