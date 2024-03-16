 /*======================================================================================================
* FILE        : CUv2.c
* AUTHOR      : Josh Ratificar (Hardware Lead)
*               Ben Cesar Cadungog (Software Lead)
*               Jeddah Laine Luceñara  (Research Lead)
*               Harold Marvin Comendador (Documentation Lead)
* DESCRIPTION : This program simulates the Control Unit of the CPU...
* COPYRIGHT   : 13 March, 2024
* REVISION HISTORY:
*   08 March, 2024: V1.0 - File Created
*   09 March, 2024: V1.1 - Implemented CU, main, displayData, and initMemory functions
*   13 March, 2024: V1.2 - Resolved Bugs and mistyped variables
*   13 March, 2024: V1.3 - Documented the code with labels and comments
*   16 March, 2024: V2.0 - File name changed to CUv2
*   16 March, 2024: V2.1 - Adjusted Logical Errors in Comments, Added MainMemory Function
======================================================================================================*/

/*==================================
HEADER FILES
==================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*==================================
GLOBAL VARIABLES AND CONSTANTS
==================================*/
unsigned char ioBuffer[32];
unsigned char dataMemory[2048];
bool IOM = 0;
bool RW = 0;
bool OE = 0;


/*==================================
FUNCTION PROTOTYPES
==================================*/
int CU();
void initMemory();
void displayData(unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR);
void MainMemory(void);

/*===============================================
*   FUNCTION    :   MAIN
*   DESCRIPTION :   This function is the entry point of the program.
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
int main()
{
    initMemory();
    if (CU()==1)
        printf("\nProgram ran successfully!");
    else
        printf("\nThe program was terminated after encountering an error.");
    return 0;
}

/*===============================================
*   FUNCTION    :   CU
*   DESCRIPTION :   This function is the Control Unit of the CU.
*   ARGUMENTS   :   VOID
*   RETURNS     :   INT
 *==============================================*/
int CU()
{
    unsigned int PC=0, IR=0, MAR=0, MBR=0, IOAR=0, IOBR=0, inst_code=0, operand=0;
    int result = 0,i;

    for (i=0; i<2048; i++){
        printf("**************************\n");
        printf("PC \t\t\t\t: 0x%03x \n", PC);
        //fetching upper byte
        IR = dataMemory[PC];
        IR = IR << 8;
        PC++;
        //fetching lower byte
        IR = IR | dataMemory[PC];
        //pointing to next instruction
        PC++;
        printf("Fetching Instructions...\n");
        printf("IR  \t\t    : 0x%04x \n", IR);
        //get 5 bit instruction code
        inst_code = IR>>11;
        //get 11 bit operand
        operand = IR & 0x07FF;
        printf("Instruction Code: 0x%02x\n", inst_code);
        printf("Operand \t\t: 0x%03x \n", operand);

        if(inst_code==0x01)
        {
            MAR = operand;
            dataMemory[MAR]= MBR;

            printf("Instruction \t: WM \n");
            printf("Writing information into memory....\n");
            displayData(PC,MAR,IOAR, IOBR);
        }
        else if(inst_code==0x02)
        {
            MAR=operand;
            MBR=dataMemory[MAR];
            printf("Instruction \t: RM \n");
            printf("MBR \t\t\t: 0x%o2x \n", MBR);
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if (inst_code==0x03)
        {
            PC = operand;
            printf("Instruction \t: BR \n");
            printf("Branching to 0x%03x to the next cycle.\n", PC);
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if (inst_code==0x04){
            IOAR=operand;
            IOBR=ioBuffer[IOAR];
            printf("Instruction \t: RIO \n");
            printf("Fetching information from the I/O buffer...\n");
            printf("IOBR \t\t: 0x%02x \n", IOBR);
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if (inst_code==0x05){
            IOAR = operand;
            ioBuffer[IOAR]=IOBR;
            printf("Instruction \t: WIO \n");
            printf("Storing information into memory....\n");
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if(inst_code==0x06){
            MBR = operand;
            printf("Instruction \t: WB \n");
            printf("Loading Data to MBR....\n");
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if(inst_code==0x07){
            IOBR = operand;
            printf("Instruction \t: WIB \n");
            printf("Loading Data to IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            displayData(PC,MAR,IOAR,IOBR);
        }
        else if (inst_code==0x1F){
            result = 1;
            printf("Instruction \t: EOP \n");
            printf("Program Ended....\n");
            displayData(PC,MAR,IOAR,IOBR);
            break;
        }
    }
    return result;
}

/*===============================================
*   FUNCTION    :   displayData
*   DESCRIPTION :   This function displays the data in the CU.
*   ARGUMENTS   :   unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR
*   RETURNS     :   VOID
 *==============================================*/
void displayData(unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR)
{
    printf("\n\t\tData \n");
    printf("MAR \t\t\t: 0x%03x \n", MAR);
    printf("PC \t\t\t\t: 0x%02x \n", PC);
    printf("IOAR \t\t\t: 0x%02x \n", IOAR);
    printf("IOBR \t\t\t: 0x%02x \n", IOBR);
}

/*===============================================
*   FUNCTION    :   initMemory
*   DESCRIPTION :   This function initializes the main memory of the CU.
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
void initMemory()
{
    printf("Initializing Main Memmory...\n\n");

    dataMemory[0x000] = 0x30;
    dataMemory[0x001] = 0xFF;
    dataMemory[0x002] = 0x0C;
    dataMemory[0x003] = 0x00;
    dataMemory[0x004] = 0x14;
    dataMemory[0x005] = 0x00;
    dataMemory[0x006] = 0x19;
    dataMemory[0x007] = 0x2A;
    dataMemory[0x12A] = 0x38;
    dataMemory[0x12B] = 0x05;
    dataMemory[0x12C] = 0x28;
    dataMemory[0x12D] = 0x0A;
    dataMemory[0x12E] = 0xF8;
    dataMemory[0x12F] = 0x00;
}

/*===============================================
*   FUNCTION    :   MainMemory
*   DESCRIPTION :   This function initializes the main memory of the CU.
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
void MainMemory(void)
{
    if(IOM == 1)
    {
        if(RW == 0 && OE == 1) // memory Read
            BUS = dataMemory[ADDR];
        else if (RW == 1 && OE == 1) // memory write
        {
            MainMemory[ADDR] = BUS;
        }        
    }
}