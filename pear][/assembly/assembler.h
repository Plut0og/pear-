#ifndef ASSEMBLER_H
#define ASSEMBLER_H

enum opParseState {
    oADDR_INDICATOR,
    oPREPEND,
    oVALUE,
    oAPPEND,
    oEND
};

enum dataType {
    dHEX,
    dBINARY,
    dBASE10,
    dTAG,
    dTAGNOTFOUND,
    dNOTAGVALUE,
    dASCII,
    
};

typedef struct tagTableElement {
    char* name;
    bool isPreprocessor;
    bool hasVal;
    unsigned int size;
    unsigned short value;
} tte;

typedef struct line {
    unsigned int size;
    bool isPreprocessor;
    char* tag;
    char* instruction;
    char* operand;
    unsigned short addr;
    unsigned char opcode;
    unsigned char addr_mode;
    unsigned char* p;
} line;



class assembler {
public:
    line* assemble(unsigned int* numTokens);
    assembler(unsigned int length, char* string);
private:
    
    //initial run-throughs for reading the source
    void readline(char*** data, unsigned int* numLines);
    void parseLines(line** lines, unsigned int* numL);
    //tag handling
    tte* queryTag(char* tagname);
    int evalTag(line l, char* tagname, unsigned long* p);
    tte processTag(line l, bool isDefined);
    bool isPreprocessor(line l);
    //main functions for generating binary
    void decodeOperand(line* l, bool* error);
    unsigned char getOpcode(char* instruction, unsigned char addr_mode_id, bool* error);
    //source files information
    unsigned int length;
    char* string;
    //tagTable
    unsigned int numTags;
    tte** tagTable;
    //the current address in the binary
    bool hasCAddr;
    unsigned short cAddr;
    
    int run;
    
};

#endif
