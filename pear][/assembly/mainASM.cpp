#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "assembler.h"

char* readFile(const char* fname, unsigned int* size){
    FILE *f = fopen(fname, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *string = (char *)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);
    
    string[fsize] = 0;
    *size = (int)fsize;
    return string;
}


int main(){
    
    line* strings;
    char** cLine;
    unsigned int fileLength;
    unsigned int numTokens;
    char* prog = readFile("/Users/sebastianwyngaard/Documents/pear][/pear][/assembly/test.my_asm", &fileLength);
    
    assembler* l = new assembler(fileLength, prog);
    strings = l->assemble(&numTokens);
    if(strings){
        /*
        char* tag; char* instr; char* oper;
        for(int i = 0; i < numTokens; i++){
            cLine = &(strings[i * 3]);
            tag = cLine[0][0] ? cLine[0] : (char*)"NONE\0";
            instr = cLine[1] ? (cLine[1][0] ? cLine[1] : (char*)"NONE\0") : (char*)"NONE\0";
            oper = cLine[2] ? (cLine[2][0] ? cLine[2] : (char*)"NONE\0") : (char*)"NONE\0";
            printf("%s, %s, %s\n", tag, instr, oper);
        }
        */
    } else {
        printf("AN ERROR OCCURED IN THE ASSEMBLY\n");
    }
    return 0;
}

