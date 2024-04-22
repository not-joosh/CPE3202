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
        // printf("\n\nPress Enter to continue...\n");
        // getchar();

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
        else if(inst_code == 0x11) // BRLT // NEEDS TO BE IMPLEMENTED
        {
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            if(Memory)
                BUS = MBR;
            ALU();
            printf("Instruction \t: BRLT \n");
            printf("Swap data of MBR and IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code == 0x12) // BRGT // NEEDS TO BE IMPLEMENTED 
        {
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            if(Memory)
                BUS = MBR;
            ALU();
            printf("Instruction \t: BRGT \n");
            printf("Swap data of MBR and IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if (inst_code == 0x13) // BNRE // NEEDS TO BE IMPLEMENTED
        {
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            if(Memory)
                BUS = MBR;
            ALU();
            printf("Instruction \t: BNRE \n");
            printf("Swap data of MBR and IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            printf("MBR \t\t\t: 0x%02x \n", MBR);
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
        }
        else if(inst_code == 0x14) // BRE   // NEEDS TO BE IMPLEMENTED
        {
            Fetch = 0;
            Memory = 1;
            IO = 0;
            CONTROL = inst_code;
            if(Memory)
                BUS = MBR;
            ALU();
            printf("Instruction \t: BRE \n");
            printf("Swap data of MBR and IOBR....\n");
            printf("IOBR \t\t\t: 0x%02x \n", IOBR);
            printf("MBR \t\t\t: 0x%02x \n", MBR);
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
            printf("\n\nBitch we are leaving right now \n\n");
            getch();
            result = 1;
            printf("Instruction \t: EOP \n");
            printf("Program Ended....\n");
            displayData(PC, MAR, IOAR, IOBR, IR, inst_code, CONTROL, BUS, ADDR, operand); // New Changes to displayData call
            // Added IR, inst_code, control, bus, addr
            isEOP = true;
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
    IOM = 1, RW = 1, OE = 1;
    // dataMemory[0x000] = 0b00110000; //opcode for WB (Write to MBR)
    // dataMemory[0x001] = 0b00010101; //operand for WB (0x15)
    // dataMemory[0x002] = 0b00001100; 
    // dataMemory[0x003] = 0b00000000; 
    // dataMemory[0x004] = 0b00110000;
    // dataMemory[0x005] = 0b00000101;
    // dataMemory[0x006] = 0b01001000;
    // dataMemory[0x007] = 0b00000000;
    // dataMemory[0x008] = 0b00110000;
    // dataMemory[0x009] = 0b00001000;
    // dataMemory[0x00A] = 0b11110000;
    // dataMemory[0x00B] = 0b00000000;
    // dataMemory[0x00C] = 0b00010100;
    // dataMemory[0x00D] = 0b00000000;
    // dataMemory[0x00E] = 0b11011000;
    // dataMemory[0x00F] = 0b00000000;
    // dataMemory[0x010] = 0b01011000;
    // dataMemory[0x011] = 0b00000000;
    // dataMemory[0x012] = 0b00001100;
    // dataMemory[0x013] = 0b00000001;
    // dataMemory[0x014] = 0b00111000;
    // dataMemory[0x015] = 0b00001011;
    // dataMemory[0x016] = 0b00101000;
    // dataMemory[0x017] = 0b00000000;
    // dataMemory[0x018] = 0b00110000;
    // dataMemory[0x019] = 0b00010000;
    // dataMemory[0x01A] = 0b11101000;
    // dataMemory[0x01B] = 0b00000000;
    // dataMemory[0x01C] = 0b01011000;
    // dataMemory[0x01D] = 0b00000000;
    // dataMemory[0x01E] = 0b00101000;
    // dataMemory[0x01F] = 0b00000001;
    // dataMemory[0x020] = 0b10110000;
    // dataMemory[0x021] = 0b00000000;
    // dataMemory[0x022] = 0b10110000;
    // dataMemory[0x023] = 0b00000000;
    // dataMemory[0x024] = 0b10101000;
    // dataMemory[0x025] = 0b00000000;
    // dataMemory[0x026] = 0b00010100;
    // dataMemory[0x027] = 0b00000001;
    // dataMemory[0x028] = 0b11001000;
    // dataMemory[0x029] = 0b00000000;
    // dataMemory[0x02A] = 0b11000000;
    // dataMemory[0x02B] = 0b00000000;
    // dataMemory[0x02C] = 0b00100000;
    // dataMemory[0x02D] = 0b00000001;
    // dataMemory[0x02E] = 0b01110000;
    // dataMemory[0x02F] = 0b00000000;
    // dataMemory[0x030] = 0b10111000;
    // dataMemory[0x031] = 0b00000000;
    // dataMemory[0x032] = 0b00110000;
    // dataMemory[0x033] = 0b11111111;
    // dataMemory[0x034] = 0b11010000;
    // dataMemory[0x035] = 0b00000000;
    // dataMemory[0x036] = 0b00010100;
    // dataMemory[0x037] = 0b00000001;
    // dataMemory[0x038] = 0b10100000;
    // dataMemory[0x039] = 0b00111100; 
    // dataMemory[0x03A] = 0b00110000; //WB
    // dataMemory[0x03B] = 0b11110000; //0XF0
    // dataMemory[0x03C] = 0b10010000;
    // dataMemory[0x03D] = 0b01000000;
    // dataMemory[0x03E] = 0b10001000;
    // dataMemory[0x03F] = 0b01000100;
    // dataMemory[0x040] = 0b00110000;
    // dataMemory[0x041] = 0b00000000;
    // dataMemory[0x042] = 0b01001000;
    // dataMemory[0x043] = 0b00000000;
    // dataMemory[0x044] = 0b00110000;
    // dataMemory[0x045] = 0b00000011;
    // dataMemory[0x046] = 0b01001000;
    // dataMemory[0x047] = 0b00000000;
    // dataMemory[0x048] = 0b00110000;
    // dataMemory[0x049] = 0b00000000;
    // dataMemory[0x04A] = 0b10100000;
    // dataMemory[0x04B] = 0b01010010;
    // dataMemory[0x04C] = 0b00110000;
    // dataMemory[0x04D] = 0b00000001;
    // dataMemory[0x04E] = 0b11101000;
    // dataMemory[0x04F] = 0b00000000;
    // dataMemory[0x050] = 0b00011000;
    // dataMemory[0x051] = 0b01001000;
    // dataMemory[0x052] = 0b11111000;
    // dataMemory[0x053] = 0b00000000;



    ADDR = 0x000; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB write 0x15 to MBR
    ADDR = 0x001; BUS = 0x15; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x002; BUS = 0x0C; dataMemory[ADDR] = BUS; MainMemory(); // WM
    ADDR = 0x003; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x004; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB write 0x05 to MBR
    ADDR = 0x005; BUS = 0x05; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x006; BUS = 0x48; dataMemory[ADDR] = BUS; MainMemory(); // Write 0x05 to WACC
    ADDR = 0x007; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); // Maybe not 0x05 WACC
    
    ADDR = 0x008; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB write 0x08 to MBR
    ADDR = 0x009; BUS = 0x08; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x00A; BUS = 0xF0; dataMemory[ADDR] = BUS; MainMemory(); // ADD ACC 
    ADDR = 0x00B; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();    
    
    ADDR = 0x00C; BUS = 0x14; dataMemory[ADDR] = BUS; MainMemory(); // RM 
    ADDR = 0x00D; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x00E; BUS = 0xD8; dataMemory[ADDR] = BUS; MainMemory(); // Multiply ACC x MBR
    ADDR = 0x00F; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x010; BUS = 0x58; dataMemory[ADDR] = BUS; MainMemory(); // RACC
    ADDR = 0x011; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x012; BUS = 0x0C; dataMemory[ADDR] = BUS; MainMemory(); // WM 0x401
    ADDR = 0x013; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x014; BUS = 0x38; dataMemory[ADDR] = BUS; MainMemory(); // WIB 0x0B
    ADDR = 0x015; BUS = 0x0B; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x016; BUS = 0x28; dataMemory[ADDR] = BUS; MainMemory(); // WIO 0x000
    ADDR = 0x017; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x018; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB 0x10
    ADDR = 0x019; BUS = 0x10; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x01A; BUS = 0xE8; dataMemory[ADDR] = BUS; MainMemory(); // SUB ACC = (0x11) - (0x10) = 0x01
    ADDR = 0x01B; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x01C; BUS = 0x58; dataMemory[ADDR] = BUS; MainMemory(); // RACC
    ADDR = 0x01D; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();  
    
    ADDR = 0x01E; BUS = 0x28; dataMemory[ADDR] = BUS; MainMemory(); // WIO 0x001
    ADDR = 0x01F; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x020; BUS = 0xB0; dataMemory[ADDR] = BUS; MainMemory(); // SHL ACC = (0x01) << 1 = 0x02
    ADDR = 0x021; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x022; BUS = 0xB0; dataMemory[ADDR] = BUS; MainMemory(); // SHL ACC = (0x02) << 1 = 0x04
    ADDR = 0x023; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();  

    ADDR = 0x026; BUS = 0xA8; dataMemory[ADDR] = BUS; MainMemory(); // SHR ACC = (0x04) >> 1 = 0x02
    ADDR = 0x027; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();  
    
    ADDR = 0x024; BUS = 0x14; dataMemory[ADDR] = BUS; MainMemory(); // RM 0x401 
    ADDR = 0x025; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x028; BUS = 0xC8; dataMemory[ADDR] = BUS; MainMemory(); // OR ACC = (0x02) OR (0x11) = 0x13
    ADDR = 0x029; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 

    ADDR = 0x02A; BUS = 0xC0; dataMemory[ADDR] = BUS; MainMemory(); // NOT
    ADDR = 0x02B; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x02C; BUS = 0x20; dataMemory[ADDR] = BUS; MainMemory(); // RIO 0x01
    ADDR = 0x02D; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory();

    ADDR = 0x02E; BUS = 0x70; dataMemory[ADDR] = BUS; MainMemory(); // SWAP
    ADDR = 0x02F; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();

    ADDR = 0x030; BUS = 0xB8; dataMemory[ADDR] = BUS; MainMemory(); // XOR
    ADDR = 0x031; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x032; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB 0xFF
    ADDR = 0x033; BUS = 0xFF; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x034; BUS = 0xD0; dataMemory[ADDR] = BUS; MainMemory(); // AND 
    ADDR = 0x035; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x036; BUS = 0x14; dataMemory[ADDR] = BUS; MainMemory(); // RM 0x401 
    ADDR = 0x037; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x038; BUS = 0xA0; dataMemory[ADDR] = BUS; MainMemory(); // BRE 0x03C
    ADDR = 0x039; BUS = 0x3C; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x03A; BUS = 0x0C; dataMemory[ADDR] = BUS; MainMemory(); // WM 0xF0
    ADDR = 0x03B; BUS = 0xF0; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x03C; BUS = 0x90; dataMemory[ADDR] = BUS; MainMemory(); // BRGT 0x040	
    ADDR = 0x03D; BUS = 0x40; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x03E; BUS = 0x88; dataMemory[ADDR] = BUS; MainMemory(); // BRLT 0x044	
    ADDR = 0x03F; BUS = 0x44; dataMemory[ADDR] = BUS; MainMemory();


    ADDR = 0x040; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB 0x00
    ADDR = 0x041; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x042; BUS = 0x48; dataMemory[ADDR] = BUS; MainMemory(); // WACC 
    ADDR = 0x043; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory();
    
    ADDR = 0x044; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB 0x03 
    ADDR = 0x045; BUS = 0x03; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x046; BUS = 0x48; dataMemory[ADDR] = BUS; MainMemory(); //  WACC 
    ADDR = 0x047; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 


    
    // CONTROLLED LOOP
    ADDR = 0x048; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); //  WB 0x00
    ADDR = 0x049; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x04A; BUS = 0xA0; dataMemory[ADDR] = BUS; MainMemory(); // BRE 0x052
    ADDR = 0x04B; BUS = 0x52; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x04C; BUS = 0x30; dataMemory[ADDR] = BUS; MainMemory(); // WB 0x01
    ADDR = 0x04D; BUS = 0x01; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x04E; BUS = 0xE8; dataMemory[ADDR] = BUS; MainMemory(); // SUB
    ADDR = 0x04F; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x050; BUS = 0x18; dataMemory[ADDR] = BUS; MainMemory(); // BR 0x048
    ADDR = 0x051; BUS = 0x48; dataMemory[ADDR] = BUS; MainMemory(); 
    
    ADDR = 0x052; BUS = 0xF8; dataMemory[ADDR] = BUS; MainMemory(); // EOP 
    ADDR = 0x053; BUS = 0x00; dataMemory[ADDR] = BUS; MainMemory(); 

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

/*===============================================
*   FUNCTION    :   ALU
*   DESCRIPTION :   ALU FUNCTION
*   ARGUMENTS   :   UNSIGNED CHAR, UNSIGNED CHAR, UNSIGNED CHAR
*   RETURNS     :   INT
 *==============================================*/
int ALU(void)
{
    /* setting ACC and flags to initial values */
    static unsigned int ACC = 0x0000; 
    unsigned char temp_ACC = 0x0000;
    unsigned char temp_OP2;
    SF=0, CF=0, ZF=0, OF=0;

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
        temp_ACC = (int) ACC + temp_OP2; 
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
        temp_OP2 = BUS;
        ACC = ACC & temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nAND <--- ALU\n");
    }
    else if(CONTROL == OR) 
    {
        // Performing OR
        temp_OP2 = BUS;
        ACC = ACC | temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nOR <--- ALU\n");
    }
    else if(CONTROL == NOT)
    {
        // Performing NOT
        temp_OP2 = BUS;
        ACC = ~ACC;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nNOT <--- ALU\n");
    }
    else if(CONTROL == XOR)
    {
        // Performing XOR
        temp_OP2 = BUS;
        ACC = ACC ^ temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nXOR <--- ALU\n");
    }
    else if(CONTROL == shift_left)
    {
        // Performing Shift Left
        temp_OP2 = BUS;
        ACC = ACC << temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nSHIFT LEFT <--- ALU\n");
    }
    else if(CONTROL == shift_right)
    {
        // Performing Shift Right
        temp_OP2 = BUS;
        ACC = ACC >> temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nSHIFT RIGHT <--- ALU\n");
    }
    else if(CONTROL == WACC)
    {
        // Write data on BUS to ACC
        ACC = BUS;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nWACC <--- ALU\n");
    }
    else if(CONTROL == RACC)
    {
        // Move ACC data to BUS
        BUS = ACC;
        printf("\nACC = "); printBin(ACC, 16);
        printf("\nRACC <--- ALU\n");
    }
    else
    {
        printf("\nInvalid Control Signal");
    }
    printf("\nACC = "); printBin(ACC, 16);
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
    // Check if ACC is zero
    if ((ACC & 0x00FF)  == 0x0000) ZF = 1; 
    else ZF = 0; 
    // Check if ACC is negative (MSB is 1)
    if (ACC & 0x0080) SF = 1; 
    else SF = 0; 
    // Check if there is an overflow
    if (ACC > 0x007F) OF = 1;
    else OF = 0; 
    // Check if there is a carry
    if (ACC > 0xFF && ZF == 0) CF = 1; 
    else CF = 0; 
    printf("\nZF = %d, CF = %d, SF = %d, OF = %d", ZF, CF, SF, OF);
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
