#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#define ROWMAX 4096
#define COLMAX 16
#define MAX_LINE 300

int buffer[ROWMAX][COLMAX];

int dataSetUp(FILE *fpointer);
void dataInitialize();
int dataFilter(char *inputStr, int hexLine);
void dataPrint(int influRange);
void printAddress(int add);
int verifyDisplay(int displayedChar);
void userAsk();

int hexStringToInteger(char *inputStr, int startPos, int digitNum);
int hexCharToInt(char inputChar);

int main(int argc, char *argv[])
{
    char sourceFile[MAX_LINE];
    if (argc > 1)
        strcpy(sourceFile, argv[1]);
    FILE *fpointer = fopen(sourceFile, "r");
    if (fpointer == NULL)
    {
        printf("ERROR: %s could not be opened.", sourceFile);
        exit(1);
    }
    dataPrint(dataSetUp(fpointer));
    return 0;
}

int dataSetUp(FILE *fpointer)
{                            // argument: the pointer to the hex source file
    dataInitialize();        // set all the data of buffer to 255
    int influRange = 0;      // store the last effected address
    int hexLine = 1;         // input hex line number
    char inputStr[MAX_LINE]; // store each line of hex code
    while (fgets(inputStr, MAX_LINE, fpointer) != NULL && dataFilter(inputStr, hexLine) == 1)
    {
        hexLine++;
        if (inputStr[8] == '0')
        { // record type: data
            int dataLen = hexStringToInteger(inputStr, 1, 2);
            int address = hexStringToInteger(inputStr, 3, 4);
            if (influRange < address + dataLen)
                influRange = address + dataLen; // find the last effected address

            int i = 9, row = address / 16, col = address % 16;
            while (i <= (7 + 2 * dataLen))
            {                                // take all data from inputStr
                if (buffer[row][col] == 255) // if this is empty address then set the value for it
                    buffer[row][col] = hexStringToInteger(inputStr, i, 2);
                i += 2;
                if (col <= 14)
                    col++; // assign value for all collumns
                else
                { // then consider the next rows
                    col = 0;
                    row++;
                    if (row >= ROWMAX)
                    {
                        printf("Buffer size limit exceeded. ");
                        break;
                    }
                }
            }
        }
    }
    fclose(fpointer);
    return influRange; // return the last effected (by hex code) address
}

void dataInitialize()
{ // set all the data of buffer to 255
    for (int row = 0; row < ROWMAX; row++)
    {
        for (int col = 0; col < COLMAX; col++)
        {
            buffer[row][col] = 255;
        }
    }
}

// argument 1: the string that stores the information of 1 hex source code
// argument 2: the line number of inputStr
int dataFilter(char *inputStr, int hexLine)
{
    if (strlen(inputStr) == 1)
    { // if inputStr is empty string
        printf("Line %d has no information, the program skips this line.\n", hexLine);
        return 1;
    }
    int byteCount = hexStringToInteger(inputStr, 1, 2);
    if (inputStr[0] != ':')
    { // verify hex code
        printf("Line %d is not an intel hex code.", hexLine);
        return 0;
    }
    if (strlen(inputStr) != byteCount * 2 + 12)
    { // verify length validity
        printf("Line %d has invalid length.", hexLine);
        return 0;
    }
    int checkSum = 0; // verify checksum
    for (int i = 1; i <= hexStringToInteger(inputStr, 1, 2) * 2 + 9; i += 2)
    { // sums up all the value
        checkSum += hexStringToInteger(inputStr, i, 2);
    }
    if (checkSum % 256 != 0)
    {
        printf("Line %d has invalid checksum.", hexLine);
        return 0;
    }
    if (inputStr[8] == '1')
    { // check end of file
        printf("Line %d signals the end of hex code.", hexLine);
        return 0;
    }
    return 1; // default value
}

void dataPrint(int influRange)
{ // argument: the last address that has information, from this address on, data is all empty
    printf(" Press any key to start printing.\n");
    getch();
    for (int row = 0; row <= influRange / 16 && row < ROWMAX; row++)
    { // for each row in buffer[][]
        printAddress(16 * row);
        printf("\t"); // print address
        for (int col = 0; col < 16; col++)
        { // print hex
            if (buffer[row][col] < 16)
                printf("0");
            printf("%X ", buffer[row][col]);
        }
        printf("\t");
        for (int col = 0; col < 16; col++)
        { // print char
            if (verifyDisplay(buffer[row][col]))
                printf("%c", buffer[row][col]);
            else
                printf(".");
        }
        if (row < (influRange / 16) && row < (ROWMAX - 1)) //'\n' is not printed in the last line
            printf("\n");
        if (row % 25 == 0 && row != 0)
            userAsk(); // after each 25 lines
    }
    printf("\nThis is the end, the remaining storage is empty.");
    getch();
}

void printAddress(int add)
{ // argument: the address of first col in the considered row.
    const char HEX[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
    char str[] = {'0', '0', '0', '0', '0', '0', '0', '0'};
    for (int j = 7, rmd = add % 16; add != 0 && j >= 0; add /= 16, rmd = add % 16, j--) // find rmd and convert to character
        str[j] = HEX[rmd];
    for (int i = 0; i < 8; i++)
        printf("%c", str[i]); // print string of address
}

int verifyDisplay(int displayedChar)
{ // used to verify the input character can be displayed or not
    for (int i = 0; i <= 32; i++)
        if (displayedChar == i)
            return 0;
    return 1;
}

void userAsk()
{ // used to ask user whether or not to keep printing
    printf("Print next 25 lines of data? Press 'N' to stop or press any other key to continue.");
    char ans;
    scanf("%c", &ans);
    if (ans == 'N' || ans == 'n')
        exit(0);
    else
        system("cls");
}

int hexCharToInt(char inputChar)
{ // conver character to integer
    if (inputChar >= 'a' && inputChar <= 'f')
        return inputChar - 97 + 10;
    if (inputChar >= 'A' && inputChar <= 'F')
        return inputChar - 65 + 10;
    if (inputChar >= '0' && inputChar <= '9')
        return inputChar - 48;
}

// argument 1: the string that contains 1 line of hex code
// argument 2: the start position of the string
// argument 3: the number of digits
// for example: inputStr = ":0204", startPos = 3, digitNum = 2 will return integer value of 4
int hexStringToInteger(char *inputStr, int startPos, int digitNum)
{
    int integer = 0;
    for (int i = startPos, j = 0; i < startPos + digitNum; i++, j--)
        integer += pow(16, digitNum - 1 + j) * hexCharToInt(inputStr[i]);
    return integer;
}