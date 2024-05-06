 /*======================================================================================================
* FILE        : Cadungog_Comendador_Lucenara_Ratificar_CPU-MEM.c
* AUTHOR      : Josh Ratificar (Hardware Lead)
*               Ben Cesar Cadungog (Software Lead)
*               Jeddah Laine Luceñara  (Research Lead)
*               Harold Marvin Comendador (Documentation Lead)
* DESCRIPTION : This program simulates CPU and Memory operations. 
* COPYRIGHT   : 17 March, 2024
* REVISION HISTORY:
*   20 April, 2024: V1.0 - File Created
*   21 April, 2024: V1.1 - Addressed bugs in CU and Flags.
*   22 April, 2024: V1.2 - Finalized the program. Created New MainMemory changes with chip modules 
======================================================================================================*/
/*=============================================== 
 *   HEADER FILES
 *==============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*=============================================== 
 *   DEFINITIONS AND CONSTANTS
 *==============================================*/
// ALU Constants
#define addition 0x1E
#define subtraction 0x1D
#define multiplication 0x1B
#define AND 0x1A
#define OR 0x19
#define NOT 0x18
#define XOR 0x17
#define shift_left 0x16
#define shift_right 0x15
#define WACC 0x09
#define RACC 0x0B

unsigned int FLAGS = 0x00; // Flags
unsigned char SF, CF, ZF, OF; // Flags
unsigned char CONTROL = 0;

// Control Unit Constants
unsigned char ioBuffer[32];
unsigned char dataMemory[2048];
unsigned char BUS = 0x00;  // 8 bit bus
unsigned int ADDR = 0x00;  
bool IOM = 0;
bool RW = 0;
bool OE = 0;

// Memory Constants
/* memory chip declaration */
long A1[32], A2[32], A3[32], A4[32], A5[32], A6[32], A7[32], A8[32]; // chip group A
long B1[32], B2[32], B3[32], B4[32], B5[32], B6[32], B7[32], B8[32]; // chip group B


/*=============================================== 
 *   FUNCTION PROTOTYPES
 *==============================================*/
// ALU prototypes
int ALU(void);
unsigned char twosComp(unsigned char operand);
void printBin(int data, unsigned char data_width);
void setFlags(unsigned int ACC);
void boothsAlogrithm(unsigned char Q, unsigned char M);
void displayStep(unsigned char A, unsigned char Q, unsigned char Q_N1, unsigned char M, int n);


// CU prototypes
int CU();
void initMemory();
void displayData(unsigned int PC, unsigned int MAR, unsigned int IOAR, unsigned int IOBR, unsigned int IR, unsigned int inst_code, unsigned int CONTROL, unsigned int BUS, unsigned int ADDR, unsigned int operand); // New Changes to displayData call
void MainMemory(void);
void IOMemory(void);

// Memory prototypes
unsigned char reconstruct(int col, int row, int cs);
int getBit(long num, int pos);
void writeToMemory(int* binary, int col, int row, short int cs);
void setBit(long* num, int pos, int value);
int* charToBinary(unsigned char data);
void displayMemory(void);

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
    bool isEOP = false; // End of Program
    bool Fetch, IO, Memory, Increment;
    // Instruction Code 4 | 3 | 2 | 1 | 0
    // Instruction code is 5 bits wide...
    int row, col, cs;
    PC = 0x000;

    MainMemory();

    while(isEOP == false)
    {
        // Debugging purposes, just loading getchar() to pause the program
        // printf("\n\nMemory: \n");
        // displayMemory();
        printf("\n\nPress Enter to continue...\n");
        getchar();

        printf("\n**************************\n");
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
            /* decoding address data */
            col = ADDR & 0x001F;
            row = (ADDR >> 5) & 0x001F;
            cs = ADDR >> 10; 

            IR = reconstruct(col, row, cs); // reconstruct the instruction
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
        else if (inst_code == 0x09) // Write data on BUS to ACC
        {
            // Write data on BUS to ACC
            CONTROL = inst_code;
            Fetch = 0;
            Memory = 1;
            IO = 0;

            // When an instruction needs to perform a register-register operation
            // - REG1 ← REG2 (Example: ACC ←MBR)
            // - Control signals: IOM = x, RW = x, OE = x
            if(cs == 1)
    {
        B1[0] = BUS && 0x01;
        B2[1] = BUS && 0x02;
        B3[2] = BUS && 0x03;
        B4[3] = BUS && 0x04;
        B5[4] = BUS && 0x05;
        B6[5] = BUS && 0x06;
        B7[6] = BUS && 0x07;
        B8[7] = BUS && 0x08;
    } 
    else if(cs == 0)
    {
        A1[0] = BUS && 0x01;
        A2[1] = BUS && 0x02;
        A3[2] = BUS && 0x03;
        A4[3] = BUS && 0x04;
        A5[4] = BUS && 0x05;
        A6[5] = BUS && 0x06;
        A7[6] = BUS && 0x07;
        A8[7] = BUS && 0x08;
    }
            if(Memory)
                BUS = MBR;
            ALU(); // ALU
            printf("Instruction \t: WACC \n");
            printf("Write data on BUS to ACC....\n");
            printf("BUS \t\t\t: 0x%02x \n", BUS);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if (inst_code == 0x0B) // Move ACC data to BUS
        {
            // Write data on BUS to ACC
            CONTROL = inst_code;
            Fetch = 0;
            Memory = 1;
            IO = 0;


            if(Memory)
                MBR = BUS;
            ALU(); // ALU
            printf("Instruction \t: RACC \n");
            printf("Move ACC data to BUS....\n");
            printf("BUS \t\t\t: 0x%02x \n", BUS);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code == 0x0E) // Swap data of MBR and IOBR
        {
            CONTROL = inst_code;
            Fetch = 0;
            Memory = 1;
            IO = 0;
            
            unsigned char tempIOBR = IOBR;
            MBR = tempIOBR;
            IOBR = MBR;
            printf("Instruction \t: SWAP \n");
            printf("Swap data of MBR and IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        } 
        else if(inst_code==0x11) //BRLT
        {
            // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”
            CONTROL = subtraction; // setup the Control Signals
            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            if ((FLAGS & SF) == SF)
                PC = operand;
            printf("Instruction \t: ADD \n");
            printf("Adding ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call

        }
        else if(inst_code==0x12) //BRGT
        {
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access throug
            CONTROL = inst_code;

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte    
            ALU();
            if ((FLAGS & SF) == 0)
                PC = operand;
            printf("Instruction \t: SUBTRACT \n");
            printf("Subtracting ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x13) //BRNE
        {
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            CONTROL = inst_code;

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU();
             if ((FLAGS & ZF) == 0)
                PC = operand;
            printf("Instruction \t: MULTIPLY \n");
            printf("Multiplying ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if (inst_code==0x14) //BRE
        {
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            CONTROL = inst_code;

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU();
            if ((FLAGS & ZF) == ZF)
                PC = operand;
            printf("Instruction \t: ADD \n");
            printf("Adding ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        
        else if(inst_code==0x15) // Shift the value of ACC 1 bit to the right, CF will
        {                        // receive LSB of ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: Shift Right \n");
            printf("Shift Right....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x16) // Shift the value of ACC 1 bit to the left,
        {                        // CF will receive MSB of ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: Shift left \n");
            printf("Shift Left....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x17) // XOR the value of ACC and BUS, result stored
        {                        // to ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: XOR \n");
            printf("XOR operation....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x18) // Complement the value of ACC, result stored to
        {                        // ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: NOT \n");
            printf("NOT operation....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x19) // OR the value of ACC and BUS, result stored to
        {                        // ACC 
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: OR \n");
            printf("OR operation....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x1A) // AND the value of ACC and BUS, result stored
        {                        // to ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: AND \n");
            printf("AND operation....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x1B) // Multiply the value of ACC to BUS, product
        {                        // stored to ACC
             // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: MULTIPLY \n");
            printf("Multiplying ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x1D) // Subtract the data on the BUS from the 
        {                        // ACC register, difference stored to ACC 
            // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: SUBTRACT \n");
            printf("Subtracting ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code==0x1E) // Adds the data on the BUS to ACC register, sum stored to ACC 
        {
            // ALU
            Fetch = 0; Memory = 1; IO = 0; // operation is bus access through MBR
            /* Setting global control signals */
            CONTROL = inst_code; // setup the Control Signals
            IOM = 0; RW = 0; OE = 0; // operation neither "write" or “read”

            if(Memory)
                BUS = MBR; // load data on BUS to MBR (ACC high byte
            ALU(); 
            printf("Instruction \t: ADD \n");
            printf("Adding ACC and BUS....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if (inst_code==0x1F) // End of Program
        {
            result = 1;
            printf("Instruction \t: EOP \n");
            printf("Program Ended....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
            isEOP = true;
            getchar();
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
    /* setting the global control signals */
    IOM = 1, RW = 1, OE = 1;

    /* Format ADDR=<program memory address>; BUS=<instruction>; MainMemory() */
    /* Calling MainMemory() writes the instruction to memory */
    displayMemory(); getchar();
    ADDR = 0x000; BUS = 0x30; MainMemory(); // write to MBR 0x15
    ADDR = 0x001; BUS = 0x15; MainMemory();
    displayMemory(); getchar();
    ADDR = 0x002; BUS = 0x0C; MainMemory(); // write to Main Memory at 0x400
    ADDR = 0x003; BUS = 0x00; MainMemory(); 
    displayMemory(); getchar();
    ADDR = 0x004; BUS = 0xF8; MainMemory(); // EOP 
    ADDR = 0x005; BUS = 0x00; MainMemory(); 
    displayMemory(); getchar();
}

/*===============================================
*   FUNCTION    :   MainMemory
*   DESCRIPTION :   This function reads or writes from or onto MainMemory.
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
void MainMemory(void)
{
    int row, col, i;
	short int cs; // chip select
	if(OE && IOM == 1)
    {
        /* decoding address data */ 
        col = ADDR & 0x001F; 
        row = (ADDR >> 5) & 0x001F; 
        cs = ADDR >> 10; 
        if(RW == 0) // memory read 
            BUS = reconstruct(col, row, cs); // reconstruct the data from memory
        else if(RW == 1) 
        {
            int* binary = charToBinary(BUS);
            // Pass in the binary, col, row, cs
            writeToMemory(binary, col, row, cs);
        }
	}	
}

/*===============================================
*   FUNCTION    :   reconstruct
*   DESCRIPTION :   This function reconstructs the data from memory.
*   ARGUMENTS   :   int, int, short int
*   RETURNS     :   UNSIGNED CHAR
 *==============================================*/
unsigned char reconstruct(int col, int row, int cs)
{
	int i;
	unsigned char result[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	unsigned char final = 0;
	if(!cs)
    {
		result[7] = getBit(A1[row], col);
		result[6] = getBit(A2[row], col);
		result[5] = getBit(A3[row], col);
		result[4] = getBit(A4[row], col);
		result[3] = getBit(A5[row], col);
		result[2] = getBit(A6[row], col);
		result[1] = getBit(A7[row], col);
		result[0] = getBit(A8[row], col);
		for (i = 0; i < 8; i++) 
        	final |= result[i] << (8 - i - 1);
	    return final;
	}
	result[7] = getBit(B1[row], col);
    result[6] = getBit(B2[row], col);
    result[5] = getBit(B3[row], col);
    result[4] = getBit(B4[row], col);
    result[3] = getBit(B5[row], col);
    result[2] = getBit(B6[row], col);
    result[1] = getBit(B7[row], col);
    result[0] = getBit(B8[row], col);
    for (i = 0; i < 8; i++)
        final |= result[i] << (8 - i - 1);
    return final;
}

/*===============================================
*   FUNCTION    :   setBit
*   DESCRIPTION :   This function sets the bit at the given position.
*   ARGUMENTS   :   long*, int, int
*   RETURNS     :   VOID
 *==============================================*/
void setBit(long* num, int pos, int value) 
{
    if (value == 0)
        *num &= ~(1u << pos); // Clear the bit at the given position
    else 
        *num |= (1u << pos);  // Set the bit at the given position
}

/*===============================================
*   FUNCTION    :   getBit
*   DESCRIPTION :   This function gets the bit at the given position.
*   ARGUMENTS   :   long, int
*   RETURNS     :   INT
 *==============================================*/
int getBit(long num, int pos)
{
    return (num >> pos) & 1;
}


/*===============================================
*   FUNCTION    :   writeToMemory
*   DESCRIPTION :   This function sets the binary data to memory.
*   ARGUMENTS   :   int*, int, int, short int
*   RETURNS     :   VOID
 *==============================================*/
void writeToMemory(int* binary, int col, int row, short int cs)
{
	if(!cs)
    {
		setBit(&A1[row], col, binary[0]);
		setBit(&A2[row], col, binary[1]);
		setBit(&A3[row], col, binary[2]);
		setBit(&A4[row], col, binary[3]);
		setBit(&A5[row], col, binary[4]);
		setBit(&A6[row], col, binary[5]);
		setBit(&A7[row], col, binary[6]);
		setBit(&A8[row], col, binary[7]);
		return;
	}
    else
    {
        setBit(&B1[row], col, binary[0]);
        setBit(&B2[row], col, binary[1]);
        setBit(&B3[row], col, binary[2]);
        setBit(&B4[row], col, binary[3]);
        setBit(&B5[row], col, binary[4]);
        setBit(&B6[row], col, binary[5]);
        setBit(&B7[row], col, binary[6]);
        setBit(&B8[row], col, binary[7]);
    }
}

/*===============================================
*   FUNCTION    :   charToBinary
*   DESCRIPTION :   This function converts a char to binary.
*   ARGUMENTS   :   UNSIGNED CHAR
*   RETURNS     :   INT*
 *==============================================*/
int* charToBinary(unsigned char num) 
{
	// Convert a char to binary
    int i;
    int* bits = malloc(8 * sizeof(int)); // Allocate memory for 8 bits
    if (bits == NULL) 
    {
        printf("Error: memory allocation failed\n");
        return NULL;
    }
    
    for (i = 7; i >= 0; i--) 
        bits[i] = (num >> i) & 1; // Get the bit at the ith position
    return bits;
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

void displayMemory(void)
{
    // Displaying the chip memory
    // Displaying the row, col, and cs
    printf("\n\n\t\tChip Memory\n");
    printf("A1 \t\t\t: 0x%02x \n", A1[0]);
    printf("A2 \t\t\t: 0x%02x \n", A2[0]);
    printf("A3 \t\t\t: 0x%02x \n", A3[0]);
    printf("A4 \t\t\t: 0x%02x \n", A4[0]);
    printf("A5 \t\t\t: 0x%02x \n", A5[0]);
    printf("A6 \t\t\t: 0x%02x \n", A6[0]);
    printf("A7 \t\t\t: 0x%02x \n", A7[0]);
    printf("A8 \t\t\t: 0x%02x \n", A8[0]);
    printf("B1 \t\t\t: 0x%02x \n", B1[0]);
    printf("B2 \t\t\t: 0x%02x \n", B2[0]);
    printf("B3 \t\t\t: 0x%02x \n", B3[0]);
    printf("B4 \t\t\t: 0x%02x \n", B4[0]);
    printf("B5 \t\t\t: 0x%02x \n", B5[0]);
    printf("B6 \t\t\t: 0x%02x \n", B6[0]);
    printf("B7 \t\t\t: 0x%02x \n", B7[0]);
    printf("B8 \t\t\t: 0x%02x \n", B8[0]);
    printf("\n");
    // printBin(A1[0], 8);printf("  ");
    // printBin(A2[0], 8);printf("  ");
    // printBin(A3[0], 8);printf("  ");
    // printBin(A4[0], 8);printf("  ");
    // printBin(A5[0], 8);printf("  ");
    // printBin(A6[0], 8);printf("  ");
    // printBin(A7[0], 8);printf("  ");
    // printBin(A8[0], 8);printf("  ");
    // printf("\n");
    // printBin(B1[0], 8);printf("  ");
    // printBin(B2[0], 8);printf("  ");
    // printBin(B3[0], 8);printf("  ");
    // printBin(B4[0], 8);printf("  ");
    // printBin(B5[0], 8);printf("  ");
    // printBin(B6[0], 8);printf("  ");
    // printBin(B7[0], 8);printf("  ");
    // printBin(B8[0], 8);printf("  ");
    printf("\n");

    // int i;
    // printf("\n\n\t\tMain Memory\n");
    // for(i = 0; i < 32; i++)
    // {
    //     printf("0x%03x \t\t: 0x%02x\n", i, dataMemory[i]);
    // }
    // printf("\n\n\t\tI/O Buffer\n");
    // for(i = 0; i < 32; i++)
    // {
    //     printf("0x%02x \t\t: 0x%02x\n", i, ioBuffer[i]);
    // }
}

/*===============================================
*   FUNCTION    :   ALU
*   DESCRIPTION :   ALU FUNCTION
*   ARGUMENTS   :   UNSIGNED CHAR, UNSIGNED CHAR, UNSIGNED CHAR
*   RETURNS     :   INT
 *==============================================*/
int ALU(void)
{
    printf("\n");
    /* setting ACC and flags to initial values */
    static unsigned int ACC = 0x0000; 
    unsigned char temp_ACC = 0x0000;
    unsigned char temp_OP1, temp_OP2, temp_prod;
    unsigned int n = 0, Q_n1 = 0;
    SF=0, CF=0, ZF=0, OF=0;

    // printf("\nACC = "); printBin(ACC, 16);
    if(CONTROL == subtraction || CONTROL == addition) // Checking if addition or subtraction
    {
        if(CONTROL == subtraction)
        {
            temp_OP2 = BUS;
            temp_OP2 = twosComp(BUS); //000 0000 0010 00110
            printf("\n SUBTRACTION <--- ALU\n");
        }
        else // Addition
        {
            temp_OP2 = BUS;
            printf("\nADDITION <--- ALU\n");
        }
        temp_ACC = (0x00FF & ACC) + temp_OP2; 
        ACC = (unsigned char) temp_ACC;
    }
    else if(CONTROL == multiplication) // Multiplication 
    { // Implementing Booths algorithm
        boothsAlogrithm(ACC, BUS);
        printf("\nMULTIPLICATION <--- ALU\n");
    }
    else if(CONTROL == AND)
    {
        // Performing AND
        ACC = ACC & BUS;
        if (ACC == 0)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nAND <--- ALU\n");
    }
    else if(CONTROL == OR) 
    {
        // Performing OR
        ACC = ACC | BUS;
        if (ACC == 0)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nOR <--- ALU\n");
    }
    else if(CONTROL == NOT)
    {
        // Performing NOT
        ACC = ~ACC;
        if (ACC == 0)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nNOT <--- ALU\n");
    }
    else if(CONTROL == XOR)
    {
        // Performing XOR
        temp_OP2 = BUS;
        ACC = ACC ^ temp_OP2;
        if (ACC == 0)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nXOR <--- ALU\n");
    }
    else if(CONTROL == shift_left)
    {
        // Performing Shift Left
        FLAGS = FLAGS & ~CF;    //Clearing CF Flag
        if ((ACC & 0x8000) == 0x8000)
            FLAGS = FLAGS | CF;     //Set CF Flag
        else 
            FLAGS = FLAGS & ~CF; //Clear CF Flag
        ACC = ACC << 1;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nSHIFT LEFT <--- ALU\n");
    }
    else if(CONTROL == shift_right)
    {
        // Performing Shift Right
        FLAGS = FLAGS & ~CF; 
        if (0x01 & ACC) 
            FLAGS = FLAGS | CF; 
        else
            FLAGS = FLAGS & ~CF; 
        ACC = ACC >> 1;       
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nSHIFT RIGHT <--- ALU\n");
    }
    else if(CONTROL == WACC)
    {
        // Write data on BUS to ACC
        ACC = (ACC & 0xFF00) | BUS;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nWACC <--- ALU\n");
    }
    else if(CONTROL == RACC)
    {
        // Move ACC data to BUS
        BUS = ACC & 0x00FF;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nRACC <--- ALU\n");
    }
    else
    {
        printf("\nInvalid Control Signal");
    }
    printf("\nACC = "); printBin(ACC, 16);
    printf("\n");
    setFlags(ACC);
}


/*===============================================
*   FUNCTION    :   boothsAlogrithm
*   DESCRIPTION :   Performs multiplication using Booth's algorithm
*   ARGUMENTS   :   UNSIGNED INT, UNSIGNED CHAR, UNSIGNED CHAR
*   RETURNS     :   VOID
 *==============================================*/
void boothsAlogrithm(unsigned char M, unsigned char Q) {  // Q Multiplier and M Multiplicand
    int n;
    unsigned char Q_N1 = 0;
    unsigned char A = 0x00;
    // unsigned char LSB_Q = Q & 0x01;
    printf("\nA\t\t\tQ\t\t\tQn-1\tM\t    Cycle\n");
    for(n = 0; n < 8; n++){
        displayStep(A, Q, Q_N1, M, n);
        unsigned char MSB_A;
        unsigned char LSB_Q = Q & 0x01;
        unsigned char LSB_A = A & 0x01;
        // Check if the LSB of Q and Q_N1 are different
        if(LSB_Q == 1 && Q_N1 == 0)     // 10 Q LSB_Q
            A = A + twosComp(M);
        else if(LSB_Q == 0 && Q_N1 == 1) // 01
            A = A + M;
        MSB_A = A & 0x80; 
        // Shift Right, Add Zero to MSB OF A
        A >>= 1;           // Shift A one bit to the Right
        A &= 0x7F;         // Clear the MSB of A
        // Set the MSB of A based on the saved MSB_A
        if (MSB_A != 0) A |= 0x80; 
        // Shift Q one bit to the Right
        Q >>= 1;
        Q |= (LSB_A << 7); // Set the LSB of Q to the LSB of A
        Q_N1 = LSB_Q; // Set Q_N1 to the LSB of Q for next cycle
    }
    displayStep(A, Q, Q_N1, M, 8);
    // Lastly we merge A and Q to get the result and then print the binary of 16 bits
    unsigned int result = (A << 8) | Q;
    printf("ACC = ");
    printBin(result, 16);
}
/*===============================================
*   FUNCTION    :   displayStep
*   DESCRIPTION :   Displays the current state of the registers
*   ARGUMENTS   :   UNSIGNED CHAR, UNSIGNED CHAR, UNSIGNED CHAR, UNSIGNED CHAR, INT
*   RETURNS     :   VOID
 *==============================================*/
void displayStep(unsigned char A, unsigned char Q, unsigned char Q_N1, unsigned char M, int n)
{
    printBin(A, 8);
    printf("\t");
    printBin(Q, 8);
    printf("\t");
    printBin(Q_N1, 1);
    printf("\t");
    printBin(M, 8);
    printf("\t");
    printf("%d", n);
    printf("\n");
}

/*===============================================
*   FUNCTION    :   setFlags
*   DESCRIPTION :   Sets the flags based on the result of the operation
*   ARGUMENTS   :   UNSIGNED INT
*   RETURNS     :   VOID
 *==============================================*/
void setFlags(unsigned int ACC)
{    
     unsigned char tmp_ACC = ACC;
    if (CONTROL == addition || CONTROL == subtraction){
        //check if zero flag
        if (ACC == 0x0000)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;

        //check if sign flag
        if ((ACC & 0x8000) == 0x8000)
            FLAGS = FLAGS | SF;
        else
            FLAGS = FLAGS & ~SF;
        
        //check if overflow flag
        if (ACC > 0x7FFF)
            FLAGS = FLAGS | OF;
        else
            FLAGS = FLAGS & ~OF;
        
        //check if carry flag
        if (ACC > 0xFFFF)
            FLAGS = FLAGS | CF;
        else
            FLAGS = FLAGS & ~CF;
        
        
    } else if (CONTROL == multiplication){

        //check zero flag
        if (ACC == 0x0000)
            FLAGS = FLAGS | ZF;
        else
            FLAGS = FLAGS & ~ZF;
        
        //check sign flag
        if ((ACC & 0x8000) == 0x8000)
            FLAGS = FLAGS | SF;
        else
            FLAGS = FLAGS & ~SF;
        
        //check overflow flag
        if (ACC > 0xFF)
            FLAGS = FLAGS | OF;
        else
            FLAGS = FLAGS & ~OF;
        
        //check carry flag
        if (ACC > 0xFF)
            FLAGS = FLAGS | CF;
        else
            FLAGS = FLAGS & ~CF;
    }
}

/*===============================================
*   FUNCTION    :   twosComp
*   DESCRIPTION :   Returns the two's complement of a number
*   ARGUMENTS   :   UNSIGNED CHAR
*   RETURNS     :   UNSIGNED CHAR
 *==============================================*/
unsigned char twosComp(unsigned char operand)
{
    return ~operand + 1;
}

/*===============================================
*   FUNCTION    :   printBin
*   DESCRIPTION :   Prints the binary representation of a number
*   ARGUMENTS   :   INT, UNSIGNED CHAR
*   RETURNS     :   VOID
 *==============================================*/
void printBin(int data, unsigned char data_width)
{
    for(int i = data_width-1; i >= 0; i--)
    {
        printf("%d", (data >> i) & 0x01);
    }
}
