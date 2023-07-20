#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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
        
        int16_t disp = displacement[0] | (displacement[1] << 8);
        if(disp != 0)
        {
            if (disp > 0)
                sprintf(eac, "[%s + %d]", displacementTable[rm], disp);
            else
                sprintf(eac, "[%s - %d]", displacementTable[rm], -disp);

        }
        else
            sprintf(eac, "[%s]", displacementTable[rm]);

        break;
        }
    case(0b01):
        {
        int8_t displacement;
        fread(&displacement, sizeof(uint8_t), 1, file);
        
        if(displacement != 0)
        {     
            if (displacement > 0)
                sprintf(eac, "[%s + %d]", displacementTable[rm], displacement);
            else
                sprintf(eac, "[%s - %d]", displacementTable[rm], -displacement);
        }

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

            int16_t disp = displacement[0] | (displacement[1] << 8);
            if (disp > 0)
                sprintf(eac, "[%s + %d]", displacementTable[rm], disp);
            else
                sprintf(eac, "[%s - %d]", displacementTable[rm], -disp);
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

void ImmediateToRegMem(uint8_t bytes, FILE* file, bool isWide)
{
    // unfinished
    return;
    uint16_t displacement;
    uint16_t data;

    if(isWide)
    {
        uint8_t dispBytes[2];
        fread(dispBytes, sizeof(uint8_t), 2, file);
        displacement = dispBytes[0] | (dispBytes[1] << 8);
        
        fread(&data, sizeof(uint16_t), 1, file);
    }

    else 
    {
        fread(&displacement, sizeof(uint8_t), 2, file);
        displacement &= 0x8; // Clear any garbage data
        fread(&data, sizeof(uint8_t), 1, file);
    }

    uint8_t mod = (bytes >> 6) & 3;
    uint8_t rm = bytes & 7;
    
    //if(displacement != 0)
    //       printf("%s, [%s + %d]", displacementTable[rm], displacement);
    //    else
    //        sprintf(eac, "[%s]", displacementTable[rm]);

}

void ImmediateToReg(uint8_t instruction, FILE* file)
{
    uint8_t w = (instruction >> 3) & 1;
    uint8_t reg = instruction & 7;
    int16_t data;

    if(w)
    {
        uint8_t displacement[2];
        fread(displacement, sizeof(uint8_t), 2, file);

        data = displacement[0] | (displacement[1] << 8);
    }
    else 
    {
        int8_t temp;
        fread(&temp, sizeof(uint8_t), 1, file);
        data = temp;
    }
    
    printf("%s, %d\n", regTable[reg + (8 * w)], data);

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
        
        // Register/memory to/from register
        if (opcode == 0b10001000)
        {
            uint8_t bytes[2];
            bytes[0] = instruction;
            fread(bytes + 1, sizeof(uint8_t), 1, input);
            RegMemToReg(bytes, input);
            continue;
        }
        
        // Immediate to register/memory
        if(opcode == 0b11000100)
        {
            uint8_t bytes;
            fread(&bytes, sizeof(uint8_t), 1, input);
            ImmediateToRegMem(bytes, input, instruction & 1);
        }
        
        // Immediate to register
        if(opcode & 0b10110000)
        {
            ImmediateToReg(instruction, input);
        }

                
        if (feof(input))
            break;
    }

    fclose(input);
}


