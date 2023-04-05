#include <stdint.h>
#include <stdio.h>

const char* regTable[16] =
{
    "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
    "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

int main(int numArgs, const char** args)
{
	if (numArgs < 2)
	{
		printf("Please input an assembled file\n");
		return 1;
	}

	FILE* input;
    fopen_s(&input, args[1], "rb");
	if (!input)
	{
		printf("Could not open the specified file!");
		return 1;
	}
    printf("bits 16\n\n");
    
    unsigned char instruction[2] = {0, 0};
    while(fread(instruction, sizeof(char), 2, input))
    {
        char opcode = instruction[0] & 0b11111100;
        if (instruction[0] & 0b10001000)
            printf("mov ");
        // We only want to read specific mov instruction
        else
            continue;
        
        char dw = instruction [0] & 3;
        char reg = (instruction[1] >> 3) & 7;
        char reg2 = instruction[1] & 7;

        char source = dw & 2 ? reg2 : reg;
        char dest   = dw & 2 ? reg : reg2;
        printf("%s", regTable[dest + (8 * (dw & 1))]);
        printf(", ");

        printf("%s", regTable[source + (8 * (dw & 1))]);
        printf("\n");
        
        if (feof(input))
            break;
    }

    fclose(input);
}
