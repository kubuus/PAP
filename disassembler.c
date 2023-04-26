#include <stdint.h>
#include <stdio.h>

const char* regTable[16] =
{
    "AL", "CL", "DL", "BL", "AH", "CH", "DH", "BH",
    "AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
};

const char* displacementTable[8] =
{
    "BX + SI", "BX + DI", "BP + SI", "BP + DI", "SI", "DI", "BP", "BX"
};

void RegMemToReg (uint8_t bytes[2], FILE* file)
{
    uint8_t dw =  bytes[0] & 3;
    uint8_t mod = (bytes[1] >> 6) & 3;
    uint8_t reg = (bytes[1] >> 3) & 7;
    uint8_t rm = bytes[1] & 7;

    uint8_t source = dw & 2 ? rm : reg;
    uint8_t dest   = dw & 2 ? reg : rm;

    switch(mod)
    {
    case(0b11):
        printf("%s", regTable[dest + (8 * (dw & 1))]);
        printf(", ");

        printf("%s", regTable[source + (8 * (dw & 1))]);
        printf("\n");
        break;
    case(0b10):
        {
        uint8_t displacement[2];
        fread(displacement, sizeof(uint8_t), 2, file);
        
        uint16_t disp = displacement[0] | (displacement[1] << 8);
        char eac[32]; 
        sprintf(eac, "[%s + %d]", displacementTable[rm], disp);
        if (dw & 2)
            printf("%s, %s\n", eac, regTable[reg + (8 * (dw & 1))]);
        else
            printf("%s, %s\n", regTable[reg + (8 * (dw & 1))], eac);
        break;
        }
    case(0b01):
        {
        uint8_t displacement;
        fread(&displacement, sizeof(uint8_t), 1, file);
        
        char eac[32]; 
        sprintf(eac, "[%s + %d]", displacementTable[rm], displacement);
        if (dw & 2)
            printf("%s, %s\n", eac, regTable[reg + (8 * (dw & 1))]);
        else
            printf("%s, %s\n", regTable[reg + (8 * (dw & 1))], eac);
        break;
        }
    case(0b00):
        {
        char eac[32];
        if (rm == 0b110)
        {
            uint8_t displacement[2];
            fread(displacement, sizeof(uint8_t), 2, file);

            uint16_t disp = displacement[0] | (displacement[1] << 8);
            sprintf(eac, "[%d]", disp);
        }
        else 
            sprintf(eac, "[%s]", displacementTable[rm]);
        
        if (dw & 2)
            printf("%s, %s\n", eac, regTable[reg + (8 * (dw & 1))]);
        else
            printf("%s, %s\n", regTable[reg + (8 * (dw & 1))], eac);
        
        break;
        }
    }
}

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
    
    uint8_t instruction = 0;
    while(fread(&instruction, sizeof(uint8_t), 1, input))
    {
        uint8_t opcode = instruction & 0b11111100;
        if (opcode == 0b10001000)
        {
            uint8_t bytes[2];
            bytes[0] = instruction;
            fread(bytes + 1, sizeof(uint8_t), 1, input);
            RegMemToReg(bytes, input);
        }

        // We only want to read specific mov instruction
        else if(opcode == 0b1100)
            continue;
        
                
        if (feof(input))
            break;
    }

    fclose(input);
}


