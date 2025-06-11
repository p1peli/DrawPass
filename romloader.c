#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//https://www.nesdev.org/wiki/INES
//https://forums.nesdev.org/viewtopic.php?t=15649
//https://www.nesdev.org/wiki/CPU_power_up_state

typedef struct {
    uint8_t A, X, Y;     // Registers
    uint8_t SP;          // Stack Pointer
    uint8_t status;      // Processor Status
    uint16_t PC;         // Program Counter
} CPU;

typedef struct {
    uint8_t status;
    int scanline;
    int cycle;
} PPU;

PPU ppu;

uint8_t cpu_memory[0x10000] = {0}; // 64KB main CPU address space


uint8_t cpu_read(uint16_t addr) {
    if (addr == 0x2002) {
        uint8_t result = ppu.status;
        ppu.status &= ~0x80;  // Clear VBlank bit on read
        return result;
    }
    return cpu_memory[addr];
}


void ppu_step() {
    ppu.cycle++;
    if (ppu.cycle >= 341) {
        ppu.cycle = 0;
        ppu.scanline++;

        if (ppu.scanline == 241) {
            // Enter vblank
            ppu.status |= 0x80; // Set bit 7
        }

        if (ppu.scanline >= 262) {
            // New frame
            ppu.scanline = 0;
            ppu.status &= ~0x80; // Clear vblank
        }
    }
}



void cpu_reset(CPU *cpu, uint8_t *memory) {
    cpu->PC = memory[0xFFFC] | (memory[0xFFFD] << 8); //These two bytes form the 16-bit start address where the CPU jumps to begin execution.
    cpu->SP = 0xFD;
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->status = 0x24; // Default flags: IRQ disabled
}

void update_zn_flags(CPU *cpu, uint8_t value) {
    // Zero flag
    if (value == 0)
        cpu->status |= (1 << 1); // Set Z
    else
        cpu->status &= ~(1 << 1); // Clear Z

    // Negative flag
    if (value & 0x80)
        cpu->status |= (1 << 7); // Set N
    else
        cpu->status &= ~(1 << 7); // Clear N
}

// CPU Program Counter
uint16_t pc = 0xF000;  // Example starting PC


void cpu_step(CPU *cpu, uint8_t *memory) {
    uint8_t opcode = cpu_read(cpu->PC++);
    printf("Executing opcode: %02X at PC: %04X\n", opcode, cpu->PC - 1);

    //http://www.6502.org/tutorials/6502opcodes.html
    //https://www.nesdev.org/wiki/Status_flags

    switch(opcode) {
        case 0xEA: //NOP
            break; 

        case 0x00: // BRK (stop execution)
            printf("BRK encountered. Halting.\n");
            exit(0);


        //LDA (LoaD Accumulator) 
        //Affects Flags: N Z 
        case 0xA9: { // LDA immediate
            cpu->A = cpu_read(cpu->PC++); //read the next byte and store it in the accumulator register
            update_zn_flags(cpu, cpu->A);
            break;
        }
        case 0xA5: { //LDA Zero Page
            uint8_t addr = cpu_read(cpu->PC++);
            cpu->A = cpu_read(addr); 
            update_zn_flags(cpu, cpu->A);
            break;
        }
        case 0xB5: { //LDA Zero Page, X
            uint8_t addr = (cpu_read(cpu->PC++) + cpu->X) & 0xFF;
            cpu->A = cpu_read(addr);
            update_zn_flags(cpu, cpu->A);
            break;
        }
        case 0xAD: { // LDA Absolute
            uint16_t addr = cpu_read(cpu->PC) | (cpu_read(cpu->PC + 1) << 8);
            cpu->PC += 2;
            uint8_t value = cpu_read(addr);
            cpu->A = value;

            // Set flags
            cpu->status = (cpu->status & ~((1 << 7) | (1 << 1))); // Clear N and Z
            if (value == 0) cpu->status |= (1 << 1);               // Set Z if A == 0
            if (value & 0x80) cpu->status |= (1 << 7);             // Set N if A bit 7 is set

            //printf("LDA $%04X = 0x%02X\n", addr, value);
            break;
        }
        case 0xBD: { //LDA Absolute, X
            uint16_t addr = cpu_read(cpu->PC) | (cpu_read(cpu->PC + 1) << 8);
            cpu->PC += 2;
            cpu->A = cpu_read(addr + cpu->X);
            update_zn_flags(cpu, cpu->A);
            break;
        }
        case 0xB9: { //LDA Absolute, Y
            uint16_t addr = cpu_read(cpu->PC) | (cpu_read(cpu->PC + 1) << 8);
            cpu->PC += 2;
            cpu->A = cpu_read(addr + cpu->Y);
            update_zn_flags(cpu, cpu->A);
            break;
        }


        //LDX (LoaD X register) 
        //Affects Flags: N Z 
        case 0xA2:  // LDX immediate
            cpu->X = cpu_read(cpu->PC++);
            break;









        
        case 0x78: // SEI
            cpu->status |= 0x04; // Set Interrupt Disable flag (bit 2)
            break;

        case 0x9A: // TXS
            cpu->SP = cpu->X; //Copies the value in the X register into the Stack Pointer (SP).
            break;

        //Register Instructions: 
        //Affect Flags: N (bit 7) Z (bit 1)
        case 0xAA: //TAX (Transfer A to X)
            cpu->X = cpu->A; 
            update_zn_flags(cpu, cpu->X);
            break;
        
        case 0x8A: //TXA (Transfer X to A)
            cpu->A = cpu->X; 
            update_zn_flags(cpu, cpu->A);
            break;

        case 0xCA: //DEX (Decrement X)
            cpu->X--; 
            update_zn_flags(cpu, cpu->X);
            break;

        case 0xE8: //INX (Increment X)
            cpu->X++; 
            update_zn_flags(cpu, cpu->X);
            break;

        case 0xA8: //TAY (Transfer A to Y)
            cpu->Y = cpu->A; 
            update_zn_flags(cpu, cpu->Y);
            break;
        
        case 0x98: //TYA (Transfer Y to A)
            cpu->A = cpu->Y; 
            update_zn_flags(cpu, cpu->A);
            break;
        
        case 0x88: //DEY (Decrement Y)
            cpu->Y--; 
            update_zn_flags(cpu, cpu->Y);
            break;
        
        case 0xC8: //INY (Increment Y) 
            cpu->Y++; 
            update_zn_flags(cpu, cpu->Y);
            break;

        //STX (STore X register): 
        case 0x86: { // STX Zero Page
            uint8_t addr = cpu_read(cpu->PC++);
            memory[addr] = cpu->X;
            break;
        }

        case 0x96: { // STX Zero Page,Y
            uint8_t base = memory[cpu->PC++];
            uint8_t addr = (base + cpu->Y) & 0xFF; // wrap around zero page
            memory[addr] = cpu->X;
            break;
        }

        case 0x8E: { // STX absolute
            uint16_t addr = memory[cpu->PC] | (memory[cpu->PC + 1] << 8);
            cpu->PC += 2;
            memory[addr] = cpu->X;
            break;
        }



        //Branch Instructions
        case 0x10: { // BPL - Branch if Positive (Negative flag clear)
            //printf("A=0x%02X, N flag=%d\n", cpu->A, (cpu->status & 0x80) != 0);

            int8_t offset = (int8_t)cpu_read(cpu->PC++);
            //printf("Offset: %d (0x%02X)\n", offset, (uint8_t)offset);

            if ((cpu->status & (1 << 7)) == 0) { // If N flag is clear
                cpu->PC += offset;
            }
            break;
        }



        default:
            printf("Unknown opcode: %02X\n", opcode);
            pc++;

            for (int i = 0; i < 3; i++) {
                ppu_step();
            }


            exit(1);
    }
}





int validate_header(const uint8_t *header) {
    return header[0] == 78 && header[1] == 69 && header[2] == 83 && header[3] == 26; 
}

int load_prg_rom(FILE *f, const uint8_t *header, uint8_t *cpu_memory) {
    int prg_size; 
    int offset = 16; 
    uint8_t *ptr1; 

    if (header[6] & (1 << 2)) { //check if second bit is set in byte 6
        offset += 512; //leaving space for trainer data
    }

    if (header[4] == 0) {
        return 1; 
    }

    prg_size = 16 * 1024 * header[4]; //Byte 4: Size of PRG ROM in 16 KB units 

    ptr1 = (uint8_t *)malloc(prg_size);
    if (!ptr1) {
        return 1; 
    }

    fseek(f, offset, SEEK_SET); //move the file pointer to the start of the PRG ROM data inside the .nes file

    if (fread(ptr1, 1, prg_size, f) != prg_size) {
        free(ptr1);
        return 1; // Read error
    }


    if (header[4] == 1) {
        memcpy(cpu_memory + 0x8000, ptr1, 16 * 1024); //16KB
        memcpy(cpu_memory + 0xC000, ptr1, 16 * 1024); //16KB Mirror
    } else if (header[4] == 2) {
        memcpy(cpu_memory + 0x8000, ptr1, 32 * 1024); //32KB
    } else {
        free(ptr1); 
        return 1; 
    }


    free(ptr1); 
    return 0; 
}

uint8_t *load_chr_data(FILE *f, const uint8_t *header, int prg_size) {
    int chr_size = 0; 
    int offset = 16; 

    if (header[6] & (1 << 2)) { //check if second bit is set in byte 6
        offset += 512; //leaving space for trainer data
    }

    offset += prg_size; 

    if (header[5] == 0) {
        //Allocate CHR Ram
        chr_size = 8 * 1024;
        uint8_t *chr_ram = (uint8_t *)calloc(1, chr_size); //use calloc to set all bytes to 0 -> no garbage memory
        return chr_ram;
    } else {
        //Allocate CHR Rom
        chr_size = header[5] * 1024 * 8; 
        uint8_t *chr_rom = (uint8_t *)malloc(chr_size); //use malloc to allocate a buffer to hold CHR Rom
        if (!chr_rom) return NULL; //NULL if if allocation fails

        fseek(f, offset, SEEK_SET); //move file pointer to at the beginning of the rom (at the end of prg)

        size_t bytes_read = fread(chr_rom, 1, chr_size, f); //reading the files from the NES (f) into the pointer

        if (bytes_read != chr_size) { //free memory if reading fails
            free(chr_rom);
            return NULL;
        }

        return chr_rom;

    }

    return 0; 
}

int main() {

    FILE *fptr; 
    fptr = fopen("NeSnake.nes", "rb");
    uint8_t header[16];

    if(fptr == NULL) {
        printf("Not able to open the file.\n");
        return 1; 
    }


    fread(header,sizeof(header),1,fptr);


    if (!validate_header(header)) {
        printf("Invalid file format.\n");
        return 1;
    }

    int prg_size = 16 * 1024 * header[4];

    if (load_prg_rom(fptr, header, cpu_memory)) { //load prg rom into our CPU memory
        printf("Failed to load PRG ROM\n");
        return 1;
    }

    uint8_t *ppu_memory = load_chr_data(fptr, header, prg_size); //load chr data into ppu memory
    if (!ppu_memory) {
        printf("Failed to load CHR data\n");
        return 1;
    }


    for(int i = 0; i < 16; i++)
        printf("%x ", header[i]);

    printf("\n"); 

    //Start CPU
    CPU cpu;
    cpu_reset(&cpu, cpu_memory);

    int cycle_count = 0;

    for (int i = 0; i < 100000; i++) {
        cpu_step(&cpu, cpu_memory);
        cycle_count++;

        if (cycle_count >= 29780) {
            ppu.status |= 0x80;  // Set VBlank flag
            cycle_count = 0;
            printf("VBlank set!\n");
        }
    }





    return 0;
} 


