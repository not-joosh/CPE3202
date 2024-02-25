/*=============================================== 
 *   HEADER FILES
 *==============================================*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*=============================================== 
 *   DEFINITIONS AND CONSTANTS
 *==============================================*/
#define addition 0x01
#define subtraction 0x02
#define multiplication 0x03
#define AND 0x04
#define OR 0x05
#define NOT 0x06
#define XOR 0x07
#define shift_left 0x09
#define shift_right 0x08

unsigned char SF, CF, ZF, OF; // Flags 

/*=============================================== 
 *   FUNCTION PROTOTYPES
 *==============================================*/
int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signal);
unsigned char twosComp(unsigned char operand);
void printBin(int data, unsigned char data_width);
void setFlags(unsigned int ACC);
void boothsAlogrithm(unsigned char *A, unsigned char Q, unsigned char M);
void displayStep(unsigned char *A, unsigned char Q, unsigned char Q_N1, unsigned char M, int n);

/*===============================================
*   FUNCTION    :   MAIN
*   DESCRIPTION :   MAIN FUNCTION
*   ARGUMENTS   :   VOID
*   RETURNS     :   VOID
 *==============================================*/
void main(void)
{
    ALU(0b01000110, 0b00000010, multiplication);   
    // ALU(0x03,0x05, subtraction);       // 3 - 5 = -2 => 11111110 <--- 00000010
    // ALU(0x88,0x85, addition);          // 136 + 133 = 269 => 100001101
    ALU(0xC0, 0x0A, multiplication);   
    // ALU(0x0A,0x0A, AND);               // 10 & 10 = 10 => 00001010
    // ALU(0x0A,0x0A, OR);                // 10 | 10 = 10 => 00001010
    // ALU(0x0A,0x0A, NOT);               // ~10 = -11 => 11110101
    // ALU(0x0A,0x0A, XOR);               // 10 ^ 10 = 0 => 00000000
    // ALU(0x0A,0x0A, shift_left);        // 10 << 1 = 20 => 0000000000010100
    // ALU(0x0A,0x0A, shift_right);       // 10 >> 1 = 5 => 0000000000000101
}

/*===============================================
*   FUNCTION    :   ALU
*   DESCRIPTION :   ALU FUNCTION
*   ARGUMENTS   :   UNSIGNED CHAR, UNSIGNED CHAR, UNSIGNED CHAR
*   RETURNS     :   INT
 *==============================================*/
int ALU(unsigned char operand1, unsigned char operand2, unsigned char control_signal)
{
    printf("\n*************************");
    printf("\nFetching operands...");
    printf("\nOP1 = "); printBin(operand1, 8);
    printf("\nOP2 = "); printBin(operand2, 8);

    /* setting ACC and flags to initial values */
    static unsigned char ACC;
    unsigned char temp_OP1=0x00, temp_OP2=0x00;
    ACC = 0x0000; SF=0, CF=0, ZF=0, OF=0;

    if(control_signal == subtraction || control_signal == addition) // Checking if t
    {
        temp_OP1 = operand1;
        if(control_signal == subtraction)
        {
            printf("\nOperation = SUB");
            printf("\n2's complement OP2");
            temp_OP2 = twosComp(operand2); //000 0000 0010 00110
        }
        else // Addition
        {
            printf("\nOperation = ADD");
            temp_OP2 = operand2;
        }
        /*8 bit adder*/
        printf("\nAdding OP1 & OP2...");
        ACC = temp_OP1 + temp_OP2;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == multiplication) // Multiplication 
    { // Implementing Booths algorithm
        printf("\nOperation = MUL");
        boothsAlogrithm(&ACC, operand1, operand2);
    }
    else if(control_signal == AND)
    {
        // Performing AND
        printf("\nOperation = AND");
        ACC = operand1 & operand2;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == OR) 
    {
        // Performing OR
        printf("\nOperation = OR");
        ACC = operand1 | operand2;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == NOT)
    {
        // Performing NOT
        printf("\nOperation = NOT");
        ACC = ~operand1;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == XOR)
    {
        // Performing XOR
        printf("\nOperation = XOR");
        ACC = operand1 ^ operand2;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == shift_left)
    {
        // Performing Shift Left
        printf("\nOperation = Shift Left");
        ACC = operand1 << 1;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else if(control_signal == shift_right)
    {
        // Performing Shift Right
        printf("\nOperation = Shift Right");
        ACC = operand1 >> 1;
        printf("\nACC = "); printBin(ACC, 16);
    }
    else
    {
        printf("\nInvalid Control Signal");
    }
    setFlags(ACC);
}


/*===============================================
*   FUNCTION    :   boothsAlogrithm
*   DESCRIPTION :   Performs multiplication using Booth's algorithm
*   ARGUMENTS   :   UNSIGNED INT, UNSIGNED CHAR, UNSIGNED CHAR
*   RETURNS     :   VOID
 *==============================================*/
void boothsAlogrithm(unsigned char *A, unsigned char M, unsigned char Q) { 
    int n;
    unsigned char Q_N1 = 0;
    // unsigned char LSB_Q = Q & 0x01;
    printf("\nA\t\t\tQ\t\t\tQn-1\tM\t    Cycle\n");
    for(n = 0; n < 8; n++){
        displayStep(A, Q, Q_N1, M, n);
        unsigned char MSB_A;
        unsigned char LSB_Q = Q & 0x01;
        unsigned char LSB_A = *A & 0x01;
        // Check if the LSB of Q and Q_N1 are different
        if(LSB_Q == 1 && Q_N1 == 0)     // 10
            *A = *A + twosComp(M);
        else if(LSB_Q == 0 && Q_N1 == 1) // 01
            *A = *A + M;
        MSB_A = *A & 0x80; 
        // Shift Right, Add Zero to MSB OF A
        *A >>= 1;           // Shift A one bit to the Right
        *A &= 0x7F;         // Clear the MSB of A
        // Set the MSB of A based on the saved MSB_A
        if (MSB_A != 0) *A |= 0x80; 
        // Shift Q one bit to the Right
        Q >>= 1;
        Q |= (LSB_A << 7); // Set the LSB of Q to the LSB of A
        Q_N1 = LSB_Q; // Set Q_N1 to the LSB of Q for next cycle
    }
    displayStep(A, Q, Q_N1, M, 8);
    // Lastly we merge A and Q to get the result and then print the binary of 16 bits
    unsigned int result = (*A << 8) | Q;
    printf("ACC = ");
    printBin(result, 16);
}
void displayStep(unsigned char *A, unsigned char Q, unsigned char Q_N1, unsigned char M, int n)
{
    printBin(*A, 8);
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
    if (ACC == 0x0000) ZF = 1; 
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

