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
*   09 March, 2024: V1.1 - Implemented CU, main, displayDataData, and initMemory functions
*   13 March, 2024: V1.2 - Resolved Bugs and mistyped variables
*   13 March, 2024: V1.3 - Documented the code with labels and comments
*   16 March, 2024: V2.0 - File name changed to CUv2
*   16 March, 2024: V2.1 - Adjusted Logical Errors in Comments, Added MainMemory Function
*   17 March, 2024: V2.2 - Added IOMemory Function, Adjusted CU logic. Addressed Bugs. 
*   17 March, 2024: V2.3 - Added displayDataData function, Adjusted CU logic. Addressed Bugs.
*   18 March, 2024: V2.4 - Finalized Code.
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
unsigned char BUS = 0x00;  // 8 bit bus
unsigned int ADDR = 0x00;  
unsigned char CONTROL = 0x00; // 8 bit control signal
bool IOM = 0;
bool RW = 0;
bool OE = 0;



/*==================================
FUNCTION PROTOTYPES
==================================*/
int CU();
void initMemory();
void displayData(unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR, unsigned int IR, unsigned int inst_code, unsigned int CONTROL, unsigned int BUS, unsigned int ADDR, unsigned int operand); // New Changes to displayData call
void MainMemory(void);
void IOMemory(void);

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
    int result = 0, i;
    
    // Stuff
    bool Fetch, IO, Memory, Increment;
    // Instruction Code 4 | 3 | 2 | 1 | 0
    // Instruction code is 5 bits wide... 

    
    while(inst_code != 0x1F)
    {
        printf("**************************\n");
        printf("PC \t\t\t\t: 0x%03x \n", PC);

        /* setting external control signals */
        CONTROL = inst_code; // setting the control signals 
        IOM = 1; // Main Memory access 
        RW = 0; // read operation (fetch) 
        OE = 1; // allow data movement to/from memory
        
        /* Fetching Instruction (2 cycle) */ 
        Fetch = 1; // set local control signal Fetch to 1 to signify fetch operation
        IO = 0;
        Memory = 0;
        
        /* fetching the upper byte */
        ADDR = PC;
        MainMemory(); //fetch upper byte

        if(Fetch == 1)
        {
            IR = (int) BUS; // load instruction to IR
            IR = IR << 8; // shift IR 8 bits to the left
            PC++; // points to the lower byte
            ADDR = PC; // update address bus 
        }
        
        /* fetching the lower byte */
        MainMemory(); // fetch lower byte
        if(Fetch==1)
        {
            IR = IR | BUS; // load the instruction on bus to lower
                            // 8 bits of IR
            PC++; // points to the next instruction
        }
        /* Instruction Decode */ 
        printf("Fetching Instructions...\n");
        printf("IR  \t\t    : 0x%04x \n", IR);
        //get 5 bit instruction code
        inst_code = IR>>11;
        //get 11 bit operand
        operand = IR & 0x07FF;
        printf("Instruction Code: 0x%02x\n", inst_code);
        printf("Operand \t\t: 0x%03x \n", operand);

    
        if(inst_code==0x01) // Write to Memory
        {
            MAR = operand; // load the operand to MAR (address)
            /* setting local control signals */
            Fetch = 0;
            Memory = 1; // accessing memory
            IO = 0;
            /* setting external control signals */
            CONTROL = inst_code; // setting the control signals
            IOM = 1; // Main Memory access
            RW = 1; // write operation
            OE = 1; // allow data movement to/from memory
            ADDR = MAR; // load MAR to Address Bus
            
            if(Memory)
                BUS = MBR; // MBR owns the bus since control signal Memory is 1
            MainMemory(); // write data in data bus to memory
            printf("Instruction \t: WM \n");
            printf("Writing information into memory....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            OE = 0; // disable data movement to/from memory
        }
        else if(inst_code==0x02) // Read from memory
        {
            MAR = operand; // load the operand to MAR (address)
            /* setting local control signals */
            Fetch = 0;
            Memory = 1; // accessing memory
            IO = 0;
            /* setting external control signals */
            CONTROL = inst_code; // setting the control signals
            IOM = 1; // Main Memory access
            RW = 0; // reading operation
            OE = 1; // allow data movement to/from memory
            ADDR = MAR; // load MAR to Address Bus
            
            if(Memory)
                MBR = BUS; 
            MainMemory(); // write data in data bus to memory
            printf("Instruction \t: WM \n");
            printf("Writing information into memory....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            OE = 0; // disable data movement to/from memory
        }
        else if (inst_code==0x03) // Branch 
        {
            PC = operand;
            printf("Instruction \t: BR \n");
            printf("Branching to 0x%03x to the next cycle.\n", PC);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
        }
        else if (inst_code==0x04) // Read from IO Buffer
        {
            IOAR=operand;
            /* setting local control signals */
            Fetch = 0;
            Memory = 0; 
            IO = 1;
            /* setting external control signals */
            CONTROL = inst_code; // setting the control signals
            IOM = 0; // Main Memory access
            RW = 1;
            OE = 1; // allow data movement to/from memory
            ADDR = IOAR;
            if(Memory)
               BUS = IOBR; 
            IOMemory();

            printf("Instruction \t: RIO \n");
            printf("Fetching information from the I/O buffer...\n");
            printf("IOBR \t\t: 0x%02x \n", IOBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
        }
        else if (inst_code==0x05) // write to IO buffer
        {
            IOAR = operand;
            /* setting local control signals */
            Fetch = 0;
            Memory = 0; 
            IO = 1;
            /* setting external control signals */
            CONTROL = inst_code; // setting the control signals
            IOM = 0; // Main Memory access
            RW = 0;
            OE = 1; // allow data movement to/from memory
            ADDR = IOAR;
            if(Memory)
               IOBR = BUS; 
            IOMemory();
            printf("Instruction \t: WIO \n");
            printf("Storing information into memory....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
        }
        else if(inst_code==0x06) // write data to MBR
        {
            MBR = operand;
            printf("Instruction \t: WB \n");
            printf("Loading Data to MBR....\n");
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
        }
        else if(inst_code==0x07) // write data to IOBR
        {
            IOBR = operand;
            printf("Instruction \t: WIB \n");
            printf("Loading Data to IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
        }
        else if (inst_code==0x1F) // End of Program
        {
            result = 1;
            printf("Instruction \t: EOP \n");
            printf("Program Ended....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
            break;
        }
    }
    return result;
}

/*===============================================
*   FUNCTION    :   displayDataData
*   DESCRIPTION :   This function displayDatas the data in the CU.
*   ARGUMENTS   :   unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR
*   RETURNS     :   VOID
 *==============================================*/
void displayData(unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR, unsigned int IR, unsigned int inst_code, unsigned int CONTROL, unsigned int BUS, unsigned int ADDR, unsigned int operand)
{
    printf("\n\t\tData \n");
    printf("MAR \t\t\t: 0x%03x \n", MAR);
    printf("PC \t\t\t\t: 0x%02x \n", PC);
    printf("IOAR \t\t\t: 0x%02x \n", IOAR);
    printf("IOBR \t\t\t: 0x%02x \n", IOBR);
    printf("IR \t\t\t\t: 0x%04x \n", IR);
    printf("Control \t\t: 0x%02x \n", CONTROL);
    printf("BUS \t\t\t: 0x%02x \n", BUS);
    printf("ADDR \t\t\t: 0x%03x \n", ADDR);
    printf("Instruction Code: 0x%02x\n", inst_code);
    printf("Operand \t\t: 0x%03x \n", operand);
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
*   DESCRIPTION :   This function reads or writes from or onto MainMemory.
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
            dataMemory[ADDR] = BUS;
        }        
    }
}

/*===============================================
*   FUNCTION    :   IOMemory
*   DESCRIPTION :   This function reads or writes from or onto IOMemory.
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
void IOMemory(void)
{
    if(IOM == 0)
    {
        if(RW == 0 && OE == 1) // memory Read
            BUS = ioBuffer[ADDR];
        else if (RW == 1 && OE == 1) // memory write
        {
            ioBuffer[ADDR] = BUS;
        }        
    }
}