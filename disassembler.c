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
    
    if(mod == 0b11)
    {
        printf("%s", regTable[dest + (8 * (dw & 1))]);
        printf(", ");

        printf("%s", regTable[source + (8 * (dw & 1))]);
        printf("\n");
        return;
    }

    char eac[32];

    switch(mod)
    {
    case(0b10):
        {
        uint8_t displacement[2];
        fread(displacement, sizeof(uint8_t), 2, file);
        
        uint16_t disp = displacement[0] | (displacement[1] << 8);
        if(disp != 0)
            sprintf(eac, "[%s + %d]", displacementTable[rm], disp);
        else
            sprintf(eac, "[%s]", displacementTable[rm]);

        break;
        }
    case(0b01):
        {
        uint8_t displacement;
        fread(&displacement, sizeof(uint8_t), 1, file);
        
        if(displacement != 0)
            sprintf(eac, "[%s + %d]", displacementTable[rm], displacement);
        else
            sprintf(eac, "[%s]", displacementTable[rm]);

       break;
        }
    case(0b00):
        {
        if (rm == 0b110)
        {
            uint8_t displacement[2];
            fread(displacement, sizeof(uint8_t), 2, file);

            uint16_t disp = displacement[0] | (displacement[1] << 8);
            sprintf(eac, "[%d]", disp);
        }
        else 
            sprintf(eac, "[%s]", displacementTable[rm]);
        
       
        break;
        }
    }

    if (dw & 2)
        printf("%s, %s\n", regTable[reg + (8 * (dw & 1))], eac);
    else
        printf("%s, %s\n", eac, regTable[reg + (8 * (dw & 1))]);
 
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
        printf("mov ");
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


