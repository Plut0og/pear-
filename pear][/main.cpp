//
//  main.c
//  pear][
//
//  Created by Sebastian Wyngaard on 6/25/18.
//  Copyright © 2018 Sebastian Wyngaard. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "vCPU.h"

#define RAM 0xFFFF

bool loadProg(const char* fname, char* mem);
/*
int main(){
    
    char* mem = (char*)malloc(sizeof(char)*RAM);
    vCPU* cpu = new vCPU(mem);
    if(loadProg("/Users/sebastianwyngaard/Documents/pear][/pear][/testing/prog", mem)){
        return 1;
    }
    
    cpu->start();
    
    free(mem);
    return 0;
}
*/
bool loadProg(const char* fname, char* mem){
    FILE* f = fopen(fname, "rb");
    if(f == NULL){
        printf("Could not find the file!\n");
        return true;
    }
    //get the file length
    fseek(f, 0, SEEK_END);
    unsigned long int length = ftell(f);
    if(length > 0x1000){
        printf("File is too large!\n");
        return true;
    }
    fseek(f, 0, SEEK_SET);
    fread(&(mem[PROGS]), sizeof(char)*length, 1, f);
    return false;
}
