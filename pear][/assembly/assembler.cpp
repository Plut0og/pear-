#include "assembler.h"
#include "opcodes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 256

assembler::assembler(unsigned int length, char* string){
    this->length = length;
    this->string = string;
}

void assembler::readline(char*** data, unsigned int* numLines){
    (*data) = (char**)malloc(sizeof(char*)*1);
    char* lineBuf = (char*)malloc(sizeof(char)*BUFFSIZE);
    unsigned int i = 0, c = 0, l = 0;
    while(i < length){
        if(string[i] == '\n'){
            (*data)[l] = (char*)malloc(sizeof(char)*(c+1));
            for(int a = 0; a < c; a++){
                (*data)[l][a] = lineBuf[a];
            }
            (*data)[l][c] = '\0';
            l++;
            c = 0;
            i++;
            (*data) = (char**)realloc((*data), sizeof(char*)*(l+1));
        } else {
            lineBuf[c] = string[i];
            c++;
            i++;
        }
    }
    (*numLines) = l;
}

void assembler::parseLines(line** lines, unsigned int* numL){
    //a readline wrapper that parses every line into TAG, INSTRUCTION, and OPERAND columns
    char** strLines;
    unsigned int numLines;
    readline(&strLines, &numLines);
    
    (*lines) = (line*)malloc(sizeof(line*)*(numLines*3));
    (*numL) = numLines;
    
    unsigned int c, l = 0, mc = 0;
    
    line* cLine;
    char** cBuff;
    unsigned int cMode;
    while(l < numLines){
        cMode = 0;
        
        cLine = &((*lines)[l]);
        cLine->tag = NULL; cLine->instruction = NULL; cLine->operand = NULL;
        
        //iterators used in the loop
        c = 0; //the character iterator
        mc = 0; //the mode character or cBuff pointer
        cBuff = &(cLine->tag); //the current buffer that contains tag, instruction, or operand data
        (*cBuff) = (char*)malloc(sizeof(char)*BUFFSIZE);
        memset((*cBuff), '\0', BUFFSIZE); //init the current buffer (tag buffer) to empty
        
        //this is the main line parser that will search for a tag, instruction, and operand
        //for all that it finds it will fill a value
        //anything missing will either be left NULL or empty
        while(strLines[l][c] != '\0'){
            //on a space
            if(cMode == 3){
                break;
            } else if(strLines[l][c] == ' '){
                cMode++; //enter the next column
                //consolidate and end the previous column data
                if(cBuff){
                    (*cBuff) = (char*)realloc((*cBuff), sizeof(char)*(mc + 1));
                    (*cBuff)[mc] = '\0';
                }
                //init the new column data
                mc = 0;
                switch(cMode){
                    case(0): cBuff = &(cLine->tag); break;
                    case(1): cBuff = &(cLine->instruction); break;
                    case(2): cBuff = &(cLine->operand); break;
                }
                (*cBuff) = (char*)malloc(sizeof(char)*BUFFSIZE);
                memset((*cBuff), '\0', BUFFSIZE);
            // on the ';' symbol enter comment mode if the previous column was consolidated
            } else if(strLines[l][c] == ';') {
                if((*cBuff)){
                    if((*cBuff)[0] != '\0') {
                        (*lines) = NULL; //invalid comment position(comment directly follows data column)
                        return;
                    } else {
                        cMode = 3;
                    }
                } else {
                    cMode = 3;
                }
                //if we are in comment mode, stop reading
            } else {
                (*cBuff)[mc] = strLines[l][c];
                mc++;
            }
            c++;
        }
        l++;
    }
    free(strLines);
}

line* assembler::assemble(unsigned int* numTokens){
    //variable that holds TAG, INSTRUCTION, OPERAND for every line
    unsigned int numLines;
    line* lines;
    parseLines(&lines, &numLines);
    
    //iterators used in every loop
    unsigned int i = 0;
    line* cLine;
    
    //initialize our 'tagTable' and 'tagTable' iterator
    bool operError, opcError;
    
    numTags = 1;
    tagTable = (tte**)malloc(sizeof(tte*));
    tte cTag;
    tte* cTagPtr;
    //initialize an array that stores lines that require tag processing
    bool* reqTagProc = (bool*)malloc(sizeof(bool)*numLines);
    
    //process all instructions and tags possible
    i = 0;
    hasCAddr = false;
    bool hasOperand = true;
    run = 1;
    while(i < numLines){
        printf("%i \n", run);
        cLine = &(lines[i]);
        cLine->isPreprocessor = false;
        cLine->p = nullptr;
        
        if(cLine->instruction){
            if(cLine->instruction[0] != '\0'){
                //get the operand if it exists
                hasOperand = true;
                if(cLine->operand){
                    if(cLine->operand[0] != '\0'){
                        decodeOperand(cLine, &operError);
                        if(operError){ // there was an error decoding the operand
                            printf("error getting operand\n");
                            return NULL;
                        }
                        if(cLine->addr_mode & 0x80){
                            reqTagProc[i] = true;
                            i++;
                            continue;
                        } else {
                            reqTagProc[i] = false;
                        }
                    } else {
                        reqTagProc[i] = false;
                        hasOperand = false;
                    }
                } else {
                    reqTagProc[i] = false;
                    hasOperand = false;
                }
                if(cLine->tag){ //if the line has a tag
                    if(cLine->tag[0] != '\0'){
                        if(isPreprocessor((*cLine))){
                            cLine->isPreprocessor = true;
                            cTag = processTag((*cLine), true);
                        } else {
                            cTag = processTag((*cLine), false);
                        }
                        cTagPtr = (tte*)malloc(sizeof(tte));
                        (*cTagPtr) = cTag;
                        tagTable = (tte**)realloc(tagTable, sizeof(tte*)*numTags);
                        tagTable[numTags - 1] = cTagPtr;
                        numTags++;
                    }
                }
                if(!cLine->isPreprocessor){
                    if(hasOperand){
                        cLine->opcode = getOpcode(cLine->instruction, cLine->addr_mode, &opcError);
                    } else {
                        cLine->size = 1;
                        cLine->opcode = getOpcode(cLine->instruction, 0xFF, &opcError);
                    }
                    if(opcError){
                        printf("error getting opcode\n");
                        printf("%#04X\n", cLine->addr_mode);
                        return NULL;
                    }
                }
            }
        }
        i++;
    }
    
    //process all the tags we couldn't get to
    i = 0;
    hasCAddr = true;
    cAddr = 0x0800; //the default starting position for programs
    run = 2;
    while(i < numLines){
        printf("%i \n", run);
        cLine = &(lines[i]);
        if(cLine->isPreprocessor){
            i++;
            continue;
        }
        bool isBeginning = false;
        if(cLine->tag){
            if(cLine->tag[0] != '\0'){
                tte* cTagPtr = queryTag(cLine->tag);
                if(cLine->tag[0] == '\3'){
                    isBeginning = true;
                }
                if(!isBeginning){
                    if(cTagPtr->hasVal){
                        //is preprocessor
                    } else {
                        //needs to be assigned a value
                        cTagPtr->value = cAddr;
                        cTagPtr->hasVal = true;
                    }
                } else {
                    isBeginning = false;
                }
            }
        }
        if(cLine->instruction){
            if(cLine->instruction[0] != '\0'){
                if(reqTagProc[i]){
                    reqTagProc[i] = false;
                    decodeOperand(cLine, &operError);
                    if(operError){
                        printf("there was a problem decoding the operand\n");
                        return NULL;
                    }
                    if(cLine->addr_mode & 0x80){
                        if(cLine->addr_mode & 0x40){
                            //the tag was an instruction tag and was not assigned an address value previously
                            reqTagProc[i] = true;
                        } else { //the tag was never defined in the program
                            return NULL;
                        }
                    } else {
                        cLine->opcode = getOpcode(cLine->instruction, cLine->addr_mode, &opcError);
                        if(opcError){
                            printf("there was a problem getting the opcode\n");
                            return NULL;
                        }
                    }
                }
                cLine->addr = cAddr;
                cAddr += cLine->size;
            }
        }
        i++;
    }
    
    i = 0;
    run = 3;
    while(i < numLines){
        printf("%i \n", run);
        
        cLine = &(lines[i]);
        
        if(reqTagProc[i]){
            decodeOperand(cLine, &operError);
            if(operError){
                return NULL;
            }
            if(cLine->addr_mode & 0x80){
                printf("unexepected error\n");
                return NULL;
            }
            cLine->opcode = getOpcode(cLine->instruction, cLine->addr_mode, &opcError);
            if(opcError){
                return NULL;
            }
        }
        i++;
    }
    i = 0;
    while(i < numLines){
        cLine = &(lines[i]);
        if(cLine->isPreprocessor){
            i++;
            continue;
        }
        if(cLine->instruction){
            if(cLine->instruction[0] != '\0'){
                printf("%04X: ", cLine->addr);
                printf("s: %i ", cLine->size);
                switch(cLine->size){
                    case(1):
                        printf("%02X \n", cLine->opcode);
                        break;
                    case(2):
                        printf("%02X %02X \n", cLine->opcode, cLine->p[0]);
                        break;
                    case(3):
                        printf("%02X %02X %02X \n", cLine->opcode, cLine->p[0], cLine->p[1]);
                        break;
                    case(4):
                        return NULL;
                }
            }
        }
        i++;
            
    }
    (*numTokens) = numLines;
    return lines;
}

tte* assembler::queryTag(char* tagname){
    for(int i = 0; i < numTags - 1; i++){
        if(strcmp(tagname, tagTable[i]->name) == 0){
            return tagTable[i];
        }
    }
    return NULL;
}

int assembler::evalTag(line l, char* tagname, unsigned long* p){
    tte* tagData = queryTag(tagname);
    if(tagData != NULL){
        if(tagData->isPreprocessor){
            //the tag is defined and has a value
            (*p) = tagData->value;
            return 3 + tagData->size;
        } else {
            bool isBranching = false;
            if(strcmp(l.instruction, "BNE") == 0
            || strcmp(l.instruction, "BEQ") == 0){
                isBranching = true;
            }
            if(tagData->hasVal){
                if(isBranching){
                    (*p) = (unsigned char) (tagData->value - cAddr);
                    return 3;
                } else {
                    (*p) = (unsigned short) tagData->value;
                    return 5;
                }
            } else {
                return 0 + (isBranching ? 1 : 2 );
            }
        }
    } else {
        return 0; //the tag is not defined
    }
}

bool assembler::isPreprocessor(line l){
    if(strcmp(l.instruction, "EQU") == 0){
        return true;
    } else if(strcmp(l.instruction, "EZP") == 0){
        return true;
    }
    return false;
}

tte assembler::processTag(line l, bool isDefined){
    char* name = l.tag;
    unsigned int size;
    unsigned short val;
    if(isDefined){
        val = l.p[0] + (l.p[1] << 8);
        if(strcmp(l.instruction, "EQU") == 0){size = 2;}
        else if(strcmp(l.instruction, "EZP") == 0){size = 1; val = (unsigned char) val;}
        else{
            printf("unexpected error1\n");
            return (tte){NULL};
        }
        return (tte){name, true, true, size, val};
    } else {
        size = 0;
        
        return (tte){name, false, false, size, 0x0000};
    }
    
}

void assembler::decodeOperand(line* l, bool* error ){
    (*error) = false;
    char* operand = l->operand;
    
    char isImmediate = 0x01, immType = 0x02, isAbsolute = 0x04, isIndirect = 0x08, isIndexed = 0x10, indexType = 0x20, isBranching = 0x40;
    
    char addr_mode_data = 0x00;
    char* strVal = (char*)malloc(sizeof(char)*BUFFSIZE);
    int c = 0, vc = 0;
    dataType type;
    opParseState state = oADDR_INDICATOR;
    
    bool indirectDidFinish = false;
    while(state != oEND){
        switch(state){
            case(oADDR_INDICATOR):
                switch(operand[c]){
                    case('('):;break;
                    case('#'):addr_mode_data |= isImmediate;break;
                    case('/'):addr_mode_data |= isImmediate; addr_mode_data |= immType ;break;
                    default:
                        state = oPREPEND;
                        continue;
                        break;
                }
                break;
            case(oPREPEND):
                switch(operand[c]){
                    //dataType indicators
                    case('$'):state = oVALUE; type = dHEX;break;
                    case('%'):state = oVALUE; type = dBINARY;break;
                    case('!'):state = oVALUE; type = dBASE10;break;
                    case('"'):state = oVALUE; type = dASCII;break;
                    default:
                        if(operand[c] <= '9' && operand[c] >= '0'){
                            state = oVALUE;
                            type = dBASE10;
                        } else {
                            type = dTAG;
                            state = oVALUE;
                        }
                        continue;
                        break;
                }
                break;
            case(oVALUE):
                strVal[vc] = operand[c];
                vc++;
                
                state = oAPPEND;
                break;
            case(oAPPEND):
                switch(operand[c]){
                    case('"'):;break;
                    case(')'):
                        if(addr_mode_data & isIndirect){
                            indirectDidFinish = true;
                        } else {
                            l->p = NULL;
                            return;
                        }
                        break;
                    case(','):addr_mode_data |= isIndexed ;break;
                    case('X'):
                        if(!(addr_mode_data & isIndexed)){
                            l->p = NULL;
                        }
                        ;break;
                    case('Y'):
                        if(addr_mode_data & isIndexed){
                            addr_mode_data |= indexType;
                        } else {
                            l->p = NULL;
                            return;
                        }
                        ;break;
                    case('\0'):state = oEND ;break;
                    default:
                        state = oVALUE;
                        continue;
                }
                break;
        }
        c++;
    }
    if(addr_mode_data & isIndirect && ! indirectDidFinish){
        (*error) = true;
        return;
    }
    //handle our new value
    strVal = (char*)realloc(strVal, sizeof(char)*(vc + 1));
    strVal[vc] = '\0';
    unsigned long int newVal = 0x0000000000000000;
    switch(type){
        case(dHEX): newVal = strtol(strVal, NULL, 16) ;break;
        case(dBINARY): newVal = strtol(strVal, NULL, 2) ;break;
        case(dBASE10):newVal = newVal = strtol(strVal, NULL, 10) ;break;
        case(dTAG): {
            int tagStatus = evalTag((*l), strVal, &newVal);
            switch(tagStatus){
                case(0):
                    type = dTAGNOTFOUND;
                    break;
                // cases 1 and 2 are tags that have no value yet
                case(1):
                    addr_mode_data |= isBranching;
                    type = dNOTAGVALUE;
                    break;
                case(2):
                    addr_mode_data |= isAbsolute;
                    type = dNOTAGVALUE;
                    break;
                // case 3 is a defined branching value
                case(3):
                    addr_mode_data |= isBranching;
                    break;
                // cases 4 and 5 are tags that have a value
                case(4):
                    break;
                case(5):
                    addr_mode_data |= isAbsolute;
                    break;
            }
            break;
        }
        case(dASCII): newVal = (unsigned char) strVal[0] ;break;
        default:
            (*error) = true;
            return;
    }
    
    l->p = (unsigned char*)malloc(sizeof(unsigned char*)*2);
    if(addr_mode_data & isImmediate){
        addr_mode_data &= ~(isAbsolute); //if the addressing mode is immediate it cannot be absolute
        //when a tag value is defined as abssolute its assembly interpretation can be changed when used on a line
        //preceded with a '#' or '/'
        l->size = 1;
        if(addr_mode_data & immType){
            l->p[0] = (unsigned char) ((newVal & 0xFF00) >> 8);
        } else {
            l->p[0] = (unsigned char) newVal;
        }
        l->p[1] = 0x00;
    } else if(addr_mode_data & isBranching){
        l->size = 1;
        if(type != dNOTAGVALUE){
            l->p[0] = newVal;
        }
    } else if(addr_mode_data & isAbsolute){
        l->size = 2;
        l->p[0] = (unsigned char) newVal;
        l->p[1] = (unsigned char) ((newVal & 0xFF00) >> 8);
    } else {
        if(newVal < 0x100){
            l->size = 1;
            l->p[0] = (unsigned char) newVal;
            l->p[1] = 0x00;
        } else if(newVal < 0x10000) {
            l->size = 2;
            addr_mode_data |= isAbsolute;
            l->p[0] = (unsigned char) newVal;
            l->p[1] = (unsigned char) ((newVal & 0xFF00) >> 8);
        } else {
            (*error) = true;
            return;
        }
    }
    
    //generate our id
    unsigned char a;
    switch(addr_mode_data){
        case(0x01):a = ID_I ;break; //lsn immediate
        case(0x03):a = ID_I ;break; //msn immediate
        case(0x18):a = ID_NX ;break; //zero-page, x-indexed, indirect
        case(0x38):a = ID_NY ;break; //zero-page, y-indexed, indirect
        case(0x40):a = ID_R ;break; //relative
        case(0x00):a = ID_Z ;break; //zero-page
        case(0x10):a = ID_ZX ;break; //zero-page, x-indexed
        case(0x30):a = ID_ZY ;break; //zero-page, y-indexed
        case(0x0C):a = ID_N ;break; //absolute indirect
        case(0x04):a = ID_A ;break; //absolute
        case(0x14):a = ID_AX ;break; //absolute, x-indexed
        case(0x34):a = ID_AY ;break; //absolute, y-indexed
        default:
            (*error) = true;
            return;
    }
    l->size += 1;
    if(type == dTAGNOTFOUND){l->addr_mode = a | 0x80; }
    else if(type == dNOTAGVALUE){l->addr_mode = a | 0xC0; }
    else {
        l->addr_mode = a;
    }
    
}

unsigned char assembler::getOpcode(char* instruction, unsigned char addr_mode_id, bool* error){
    (*error) = false;
    bool isEncoded;
    if(addr_mode_id == 0xFF){
        isEncoded = false;
    } else {
        isEncoded = true;
    }
    char aaa = 0x00;
    char bbb = 0x00;
    char cc  = 0x00;
    char opc = 0xFF;
    
    if(strcmp(instruction, "BNE") == 0){opc = 0xD0; isEncoded = false;}
    else if(strcmp(instruction, "BEQ") == 0){opc = 0xF0; isEncoded = false;}
    
    if(!isEncoded){
        if(strcmp(instruction, "BNE") == 0){opc = 0xD0; isEncoded = false;}
        else if(strcmp(instruction, "BEQ") == 0){opc = 0xF0; isEncoded = false;}
        else if(strcmp(instruction, "BRK") == 0){opc = 0x00;}
        else if(strcmp(instruction, "CLC") == 0){opc = 0x18;}
        else if(strcmp(instruction, "SEC") == 0){opc = 0x38;}
        else if(strcmp(instruction, "PHA") == 0){opc = 0x48;}
        else if(strcmp(instruction, "PLA") == 0){opc = 0x68;}
        else { (*error) = true; return 0;} //instruction doesn't exist
    } else if(isEncoded){
        //IF TO GET AAA AND CC PARTS OF OPCODE
        if(strcmp(instruction, "ADC") == 0){cc = SET1; aaa = ADC;}
        else if(strcmp(instruction, "STA") == 0){cc = SET1; aaa = STA;}
        else if(strcmp(instruction, "LDA") == 0){cc = SET1; aaa = LDA;}
        else if(strcmp(instruction, "CMP") == 0){cc = SET1; aaa = CMP;}
        else if(strcmp(instruction, "SBC") == 0){cc = SET1; aaa = SBC;}
        else if(strcmp(instruction, "DEC") == 0){cc = SET2; aaa = DEC;}
        else if(strcmp(instruction, "INC") == 0){cc = SET2; aaa = INC;}
        //ENCODE A JUMP EXCEPTION
        else if(strcmp(instruction, "JMP") == 0){cc = SET3; aaa = 0x08;}
        else { (*error) = true; return 0; } //instruction doesn't exist
        
        //GENERATE THE BBB PART GIVEN THE ADDR_MODE_ID
        switch(cc){
            case(SET1):
                switch(addr_mode_id){
                    case(ID_NX):bbb = ADDR_NZX_1;break;
                    case(ID_Z):bbb = ADDR_Z_1;break;
                    case(ID_I):bbb = ADDR_I_1;break;
                    case(ID_A):bbb = ADDR_A_1;break;
                    case(ID_NY):bbb = ADDR_NZY_1;break;
                    case(ID_ZX):bbb = ADDR_ZX_1;break;
                    case(ID_AY):bbb = ADDR_AY_1;break;
                    case(ID_AX):bbb = ADDR_AX_1;break;
                    default:
                        (*error) = true;
                        return 0;
                }
                break;
            case(SET2):
                switch(addr_mode_id){
                    case(ID_I):bbb = ADDR_I_2;break;
                    case(ID_Z):bbb = ADDR_Z_2;break;
                    case(ID_C):bbb = ADDR_C_2;break;
                    case(ID_A):bbb = ADDR_A_2;break;
                    case(ID_ZX):bbb = ADDR_ZX_2;break;
                    case(ID_AX):bbb = ADDR_AX_2;break;
                    default:
                        (*error) = true;
                        return 0;
                }
                break;
            case(SET3):
                switch(addr_mode_id){
                    case(ID_I):bbb = ADDR_I_3;break;
                        if(aaa == 0x08){
                            aaa = 0x03;
                            bbb = 0x03;
                        }
                    case(ID_Z):bbb = ADDR_Z_3;break;
                    case(ID_A):
                        if(aaa == 0x08){ //jump is special
                            aaa = 0x02;
                            bbb = 0x03;
                        }
                        bbb = ADDR_A_3;
                        break;
                    case(ID_ZX):bbb = ADDR_ZX_3;break;
                    case(ID_AX):bbb = ADDR_AX_3;break;
                    default:
                        (*error) = true;
                        return 0;
                }
                break;
        }
        
    }
    opc = isEncoded ? (aaa << 5) + (bbb << 2) + cc : opc;
    return opc;
}
