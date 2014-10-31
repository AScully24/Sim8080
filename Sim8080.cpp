// Sim8080.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "13021034"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   500

SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;


char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];

//////////////////////////
// Intel 8080 Registers //
//////////////////////////

#define REGISTER_B	0
#define REGISTER_C	1
#define REGISTER_D	2
#define REGISTER_E	3
#define REGISTER_H	4
#define REGISTER_L	5
#define REGISTER_M	6
#define REGISTER_A	7

#define FLAG_S	0x80
#define FLAG_Z	0x40
#define FLAG_A	0x10
#define FLAG_P	0x04
#define FLAG_C	0x01

BYTE Registers[8];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;

////////////
// Memory //
////////////

#define K_1			1024
#define MEMORY_SIZE	K_1

BYTE Memory[MEMORY_SIZE];

///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;

///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][12] =
{
	"NOP        ", 
	"LXI B,data ", 
	"STAX B     ", 
	"INX B      ", 
	"INR B      ", 
	"DCR B      ", 
	"MVI B,data ", 
	"RLC        ", 
	".BYTE 0x08 ", 
	"DAD B      ", 
	"LDAX B     ", 
	"DCX B      ", 
	"INR C      ", 
	"DCR C      ", 
	"MVI C,data ", 
	"RRC        ", 
	".BYTE 0x10 ", 
	"LXI D,data ", 
	"STAX D     ", 
	"INX D      ", 
	"INR D      ", 
	"DCR D      ", 
	"MVI D,data ", 
	"RAL        ", 
	".BYTE 0x18 ", 
	"DAD D      ", 
	"LDAX D     ", 
	"DCX D      ", 
	"INR E      ", 
	"DCR E      ", 
	"MVI E,data ", 
	"RAR        ", 
	"RIM        ", 
	"LXI H,data ", 
	"SHLD       ", 
	"INX H      ", 
	"INR H      ", 
	"DCR H      ", 
	"MVI H,data ", 
	"DAA        ", 
	".BYTE 0x28 ", 
	"DAD H      ", 
	"LHLD       ", 
	"DCX H      ", 
	"INR L      ", 
	"DCR L      ", 
	"MVI L,data ", 
	"CMA        ", 
	"SIM        ", 
	"LXI SP,data", 
	"STA        ", 
	"INX SP     ", 
	"INR M      ", 
	"DCR M      ", 
	"MVI M,data ", 
	"STC        ", 
	".BYTE 0x38 ", 
	"DAD SP     ", 
	"LDA        ", 
	"DCX SP     ", 
	"INR A      ", 
	"DCR A      ", 
	"MVI A,data ", 
	"CMC        ", 
	"MOV B,B    ", 
	"MOV B,C    ", 
	"MOV B,D    ", 
	"MOV B,E    ", 
	"MOV B,H    ", 
	"MOV B,L    ", 
	"MOV B,M    ", 
	"MOV B,A    ", 
	"MOV C,B    ", 
	"MOV C,C    ", 
	"MOV C,D    ", 
	"MOV C,E    ", 
	"MOV C,H    ", 
	"MOV C,L    ", 
	"MOV C,M    ", 
	"MOV C,A    ", 
	"MOV D,B    ", 
	"MOV D,C    ", 
	"MOV D,D    ", 
	"MOV D,E    ", 
	"MOV D,H    ", 
	"MOV D,L    ", 
	"MOV D,M    ", 
	"MOV D,A    ", 
	"MOV E,B    ", 
	"MOV E,C    ", 
	"MOV E,D    ", 
	"MOV E,E    ", 
	"MOV E,H    ", 
	"MOV E,L    ", 
	"MOV E,M    ", 
	"MOV E,A    ", 
	"MOV H,B    ", 
	"MOV H,C    ", 
	"MOV H,D    ", 
	"MOV H,E    ", 
	"MOV H,H    ", 
	"MOV H,L    ", 
	"MOV H,M    ", 
	"MOV H,A    ", 
	"MOV L,B    ", 
	"MOV L,C    ", 
	"MOV L,D    ", 
	"MOV L,E    ", 
	"MOV L,H    ", 
	"MOV L,L    ", 
	"MOV L,M    ", 
	"MOV L,A    ", 
	"MOV M,B    ", 
	"MOV M,C    ", 
	"MOV M,D    ", 
	"MOV M,E    ", 
	"MOV M,H    ", 
	"MOV M,L    ", 
	"HLT        ", 
	"MOV M,A    ", 
	"MOV A,B    ", 
	"MOV A,C    ", 
	"MOV A,D    ", 
	"MOV A,E    ", 
	"MOV A,H    ", 
	"MOV A,L    ", 
	"MOV A,M    ", 
	"MOV A,A    ", 
	"ADD B      ", 
	"ADD C      ", 
	"ADD D      ", 
	"ADD E      ", 
	"ADD H      ", 
	"ADD L      ", 
	"ADD M      ", 
	"ADD A      ", 
	"ADC B      ", 
	"ADC C      ", 
	"ADC D      ", 
	"ADC E      ", 
	"ADC H      ", 
	"ADC L      ", 
	"ADC M      ", 
	"ADC A      ", 
	"SUB B      ", 
	"SUB C      ", 
	"SUB D      ", 
	"SUB E      ", 
	"SUB H      ", 
	"SUB L      ", 
	"SUB M      ", 
	"SUB A      ", 
	"SBB B      ", 
	"SBB C      ", 
	"SBB D      ", 
	"SBB E      ", 
	"SBB H      ", 
	"SBB L      ", 
	"SBB M      ", 
	"SBB A      ", 
	"ANA B      ", 
	"ANA C      ", 
	"ANA D      ", 
	"ANA E      ", 
	"ANA H      ", 
	"ANA L      ", 
	"ANA M      ", 
	"ANA A      ", 
	"XRA B      ", 
	"XRA C      ", 
	"XRA D      ", 
	"XRA E      ", 
	"XRA H      ", 
	"XRA L      ", 
	"XRA M      ", 
	"XRA A      ", 
	"ORA B      ", 
	"ORA C      ", 
	"ORA D      ", 
	"ORA E      ", 
	"ORA H      ", 
	"ORA L      ", 
	"ORA M      ", 
	"ORA A      ", 
	"CMP B      ", 
	"CMP C      ", 
	"CMP D      ", 
	"CMP E      ", 
	"CMP H      ", 
	"CMP L      ", 
	"CMP M      ", 
	"CMP A      ", 
	"RNZ        ", 
	"POP B      ", 
	"JNZ        ", 
	"JMP        ", 
	"CNZ        ", 
	"PUSH B     ", 
	"ADI        ", 
	"RST 0      ", 
	"RZ         ", 
	"RET        ", 
	"JZ         ", 
	".BYTE 0xCB ", 
	"CZ         ", 
	"CALL       ", 
	"ACI        ", 
	"RST 1      ", 
	"RNC        ", 
	"POP D      ", 
	"JNC        ", 
	"OUT        ", 
	"CNC        ", 
	"PUSH D     ", 
	"SUI        ", 
	"RST 2      ", 
	"RC         ", 
	".BYTE 0xD9 ", 
	"JC         ", 
	"IN         ", 
	"CC         ", 
	".BYTE 0xDD ", 
	"SBI        ", 
	"RST 3      ", 
	"RPO        ", 
	"POP H      ", 
	"JPO        ", 
	"XTHL       ", 
	"CPO        ", 
	"PUSH H     ", 
	"ANI        ", 
	"RST 4      ", 
	"RPE        ", 
	"PCHL       ", 
	"JPE        ", 
	"XCHG       ", 
	"CPE        ", 
	".BYTE 0xED ", 
	"XRI        ", 
	"RST 5      ", 
	"RP         ", 
	"POP PSW    ", 
	"JP         ", 
	"DI         ", 
	"CP         ", 
	"PUSH PSW   ", 
	"ORI        ", 
	"RST 6      ", 
	"RM         ", 
	"SPHL       ", 
	"JM         ", 
	"EI         ", 
	"CM         ", 
	".BYTE 0xFD ", 
	"CPI        ", 
	"RST 7      "
};


////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (Start)                 //
////////////////////////////////////////////////////////////////////////////////


BYTE fetch()
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}


// Add any instruction implementing routines here...

/*
* Author: Anthony Scully (13021034)
* Created: 17/10/2013
* Revised: 26/11/2013
* Revisions:
*	Added comments
*	Implemented instruction sets from week 13
*	Achieved maximum score of 60 .
* User advice:
*	Read over the custom method listed below.
*	Descriptions of the methods are available when hovering over them throughout the method name.
*/

/*
* NOTE:
* The methods below are pieces of code that were re-used regularly.
* Pointers are primarily used for consistency. (Not all functions require the variable to be pointed to.)
*/

// | (or) activates the flag
// & (and) (0xFF - FLAG_MARKER) disables the flag
// Sets flags Z, S and P based on the value set by regA
void set_flags(BYTE regA)
{
	BYTE regACheck = 0x01;
	int  bit_set_count;

	if (regA == 0) Flags = Flags | FLAG_Z;
	else Flags = Flags & (0xFF - FLAG_Z);

	if ((regA & 0x80) != 0) Flags = Flags | FLAG_S;
	else Flags = Flags & (0xFF - FLAG_S);

	
	bit_set_count = 0;
	for (int i = 0; i < 8; i++)
	{
		if ((regA & regACheck) != 0) bit_set_count++;
		regACheck = regACheck << 1;
	}
	
	if ((bit_set_count & 0x01) != 1) Flags = Flags | FLAG_P;
	else Flags = Flags & (0xFF - FLAG_P);
}


/*
Uses a value (usually temp_word) to either clear or set FLAG_C in variable Flags.
To set Flag C to 0, input a value less than 0x100. Else any other value will set Flag C to 1.
*/
void flag_c_handler(WORD temp_word)
{
	if (temp_word >= 0x100) Flags = Flags | FLAG_C;
	else Flags = Flags & (0xFF - FLAG_C); 
}

/*
Handles storing and loading of instructions from a single registry.
If the bool isLoadingToRegister parameter is true, the value in the memory address will be stored to the register.
If false, the value in the register will be stored to the memory address.
*/
void single_registry_store_and_load(BYTE * Reg1, WORD address, bool isLoadingToRegister)
{
	if (isLoadingToRegister)
	{
		if ((address >= 0) && (address < MEMORY_SIZE)) 
			*Reg1 = Memory[address];
		else *Reg1 = 0;
	}
	else
	{
		if ((address >= 0) && (address < MEMORY_SIZE))
			Memory[address] = *Reg1;
	}
}

/*
Handles storing and loading of instructions from registry pairs.
If the bool isLoadingToRegister parameter is true, the value in the memory address will be stored to the the two registers.
If false, the value in the two registers will be stored to the memory address.
*/
void pair_registry_store_and_load(BYTE * Reg1,BYTE * Reg2, WORD address, bool isLoadingToRegister)
{
	if (isLoadingToRegister)
	{
		if ((address >= 0) && (address < MEMORY_SIZE-1))
		{
			*Reg1 = Memory[address];
			*Reg2 = Memory[address+1];
		}
		else
		{
			*Reg1 = 0;
			*Reg2 = 0;
		}
	}
	else
	{
		if ((address >= 0) && (address < MEMORY_SIZE-1))
		{
			Memory[address] = *Reg1;
			Memory[address+1] = *Reg2;
		}
	}
}

// Pushes the contents of the registries into the memory pointed to by the StackPointer (PUSH)
void push_stack_pointer(BYTE * Reg1, BYTE * Reg2) 
{
	//Pointers are used as the Flags variable would be difficult to check for using variables.
	if ((StackPointer >=2) && (StackPointer < (MEMORY_SIZE+1))) //Uses >=2 and +1 because we will be decrementing
	{
		StackPointer--;
		Memory[StackPointer] = *Reg1;
		StackPointer--;
		Memory[StackPointer] = *Reg2;
	}
}

// Pops the contents from memory pointed to by the StackPointer into the registries. (POP)
void pop_stack_pointer(BYTE * Reg1, BYTE * Reg2)
{
	if ((StackPointer >=0) && (StackPointer < (MEMORY_SIZE - 2 ))) //Uses -2 as we will be incrementing twice.
	{
		*Reg1 = Memory[StackPointer];
		StackPointer++;
		*Reg2 = Memory[StackPointer];
		StackPointer++;
	}
}

// Pushes the program counter on to the stack and puts the address into the program counter. (CALL)
void call_handler(WORD address)
{
	//Checks if the memory address is valid
	if ((address >=0) && (address < MEMORY_SIZE))
	{
		//Checks if the address held in the stack pointer is valid
		if ((StackPointer >=2) && (StackPointer <(MEMORY_SIZE+1)))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			ProgramCounter = address;
		}
	}
}
/*
Takes the data in memory pointed to by the stack pointer 
and sets it to the program counter using lb and hb. (RET)
*/
void ret_handler(BYTE * lb, BYTE * hb)
{
	WORD address;
	if ((StackPointer >=0) && (StackPointer < (MEMORY_SIZE-2)))
	{
		*lb = Memory[StackPointer];
		StackPointer++;
		*hb = Memory[StackPointer];
		StackPointer++;
		address = ((WORD)*hb << 8) + (WORD)*lb;
		
		//Checks if the memory address is valid
		if ((address >=0) && (address < MEMORY_SIZE))
			ProgramCounter = ((WORD)*hb << 8) + (WORD)*lb;
	}
}

// Jumps to the address specified by lb and hb. (JMP)
void jmp_handler(BYTE * lb, BYTE * hb)
{
	WORD address = ((WORD)*hb << 8) + (WORD)*lb;
	if ((address >=0) && (address < MEMORY_SIZE))
		ProgramCounter = address;
}

/*
Increments or decrements registry pair based on the isIncrement bool.
If isIncrement is true, the registry pair is increased by 1 (INX)
Else is false, the registry pair is decreased by 1 (DCX)
*/
void inx_dcx_handler(BYTE * Reg1, BYTE * Reg2, bool isIncrement)
{	
	WORD temp_word;
	temp_word = ((WORD)*Reg1 << 8) + (WORD)*Reg2;

	if (isIncrement) temp_word++;
	else temp_word--;

	*Reg1 = (BYTE)((temp_word >> 8) & 0xFF);
	*Reg2 = (BYTE)(temp_word & 0xFF);
}

/*
Stores the program counter into the memory pointed to by the stack pointer
The program counter value is then set to destination multiplied by 8 found in the opcode
ProgramCounter = destination * 8
*/
void rst_handler(BYTE opcode)
{
	BYTE destination = (opcode >> 3) & 0x07;
	if ((StackPointer >= 2) && (StackPointer < MEMORY_SIZE + 1))
	{
		StackPointer--;
		Memory[StackPointer] = ((BYTE)(ProgramCounter >> 8) & 0xFF);
		StackPointer--;
		Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
	}
	ProgramCounter = destination * 8;
}

// Swaps the values of the two registries
void register_swap(BYTE * Reg1, BYTE * Reg2)
{
	*Reg1 = *Reg1 ^ *Reg2;
	*Reg2 = *Reg1 ^ *Reg2;
	*Reg1 = *Reg1 ^ *Reg2;
}


void block_00_instructions(BYTE opcode)
{
	BYTE ls3bits;
	BYTE destination;
	WORD address;
	BYTE lb;
	BYTE hb;
	long temp;
	long hl;
	WORD temp_word;
	BYTE temp_byte;

	ls3bits = opcode & 0x07;

	switch (ls3bits)
	{
	case 0x00:	// NOP - No instruction is called.
		break;

	case 0x01:	// LXI and DAD
		if ((opcode & 0x08) == 0) //LXI - Load register pair immediate
		{
			lb = fetch();
			hb = fetch();
			switch (opcode)
			{
			case 0x01: //LXI BC
				Registers[REGISTER_B] = hb;
				Registers[REGISTER_C] = lb;
				break;

			case 0x11: //LXI DE
				Registers[REGISTER_D] = hb;
				Registers[REGISTER_E] = lb;
				break;

			case 0x21: //LXI HL
				Registers[REGISTER_H] = hb;
				Registers[REGISTER_L] = lb;
				break;

			default:
				StackPointer = ((WORD)hb << 8) + (WORD)lb;
				break;
			}

		} 
		else //DAD - Add register pair to HL (16 bit add)
		{
			temp_word = (((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]);
			switch (opcode)
			{
			case 0x09: //BC
				temp_word += (((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C]);
				break;

			case 0x19: //DE
				temp_word += (((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E]);
				break;

			case 0x29: //HL
 				temp_word += temp_word;
				break;

			case 0x39: //StackPointer
				temp_word += StackPointer;
				break;
				
			}
			flag_c_handler(temp_word & 0x10000); // Activates flag C if temp_word is larger than 16 bits
			Registers[REGISTER_H] = (BYTE)((temp_word >> 8) & 0xFF);
			Registers[REGISTER_L] = (BYTE)(temp_word & 0xFF);
		}
		break;

	case 0x02:	// Load and Store instructions
		switch (opcode)
		{
		case 0x3A: // LDA - Load A from memory
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			single_registry_store_and_load(&Registers[REGISTER_A],address,true);
			break;

		case 0x2A: // LHLD - Load H:L from memory
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb) + (WORD)lb;
			pair_registry_store_and_load(&Registers[REGISTER_L],&Registers[REGISTER_H],address,true);
			break;

		case 0x32: //STA - Store A to memory
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			single_registry_store_and_load(&Registers[REGISTER_A],address,false);
			break;

		case 0x22: // SHLD - Store H:L to memory
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			pair_registry_store_and_load(&Registers[REGISTER_L],&Registers[REGISTER_H],address,false);
			break; 

		case 0x02: //STAX B - Store indirect through BC
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			single_registry_store_and_load(&Registers[REGISTER_A],address,false);
			break;

		case 0x12: //STAX D - Store indirect through DE
			address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
			single_registry_store_and_load(&Registers[REGISTER_A],address,false);
			break;

		case 0x0A: //LDAX B - Load indirect through BC
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			single_registry_store_and_load(&Registers[REGISTER_A],address,true);
			break;

		case 0x1A: //LDAX D - Load indirect through DE
			address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
			single_registry_store_and_load(&Registers[REGISTER_A],address,true);
			break;

		default:
			break;

		}

		break;

	case 0x03:	// INX and DCX
		if ((opcode & 0x08) == 0) // INX - Increment register pair
		{
			switch (opcode)
			{
			case 0x03: // BC
				inx_dcx_handler(&Registers[REGISTER_B],&Registers[REGISTER_C],true);
				break;

			case 0x13: // DE
				inx_dcx_handler(&Registers[REGISTER_D],&Registers[REGISTER_E],true);
				break;

			case 0x23: // HL
				inx_dcx_handler(&Registers[REGISTER_H],&Registers[REGISTER_L],true);
				break;

			case 0x33: // Stack Pointer 
				StackPointer++;
				break;

			}
		}
		else // DCX - Decrement register pair
		{
			switch (opcode)
			{
			case 0x0B: // BC
				inx_dcx_handler(&Registers[REGISTER_B],&Registers[REGISTER_C],false);
				break;

			case 0x1B: // DE
				inx_dcx_handler(&Registers[REGISTER_D],&Registers[REGISTER_E],false);
				break;

			case 0x2B: // HL
				inx_dcx_handler(&Registers[REGISTER_H],&Registers[REGISTER_L],false);
				break;

			case 0x3B: // Stack Pointer
				StackPointer--;
				break;

			}
		}
		break;

	case 0x04:	// INR D - Increment register
		destination = (opcode >> 3) & 0x07;
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			if ((address >=0) && (address < MEMORY_SIZE))
			{
				Memory[address]++;
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]++;
			set_flags(Registers[destination]);
		}
		break;

	case 0x05:	// DCR D - Decrement register
		destination = (opcode >> 3) & 0x07;
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			if ((address >=0) && (address < MEMORY_SIZE))
			{
				Memory[address]--;
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]--;
			set_flags(Registers[destination]);
		}
		break;

	case 0x06:	// MVI - Move immediate to register
		destination  = (opcode >> 3) & 0x07;
		Registers[destination] = fetch();
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			single_registry_store_and_load(&Registers[REGISTER_M],address,false);
		}
		break;

	default:	// Rotates,DAA and carry instructions
		switch (opcode)
		{
		case 0x07: // RLC - Rotate A left
			temp_word = (WORD)Registers[REGISTER_A];
			temp_word = temp_word << 1;
			flag_c_handler(temp_word);
			if ((Flags & FLAG_C) != 0)
			{
				temp_word = temp_word | 0x01;
			}
			Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
			break;

		case 0x17: // RAL - Rotate A left through carry
			temp_word = (WORD)Registers[REGISTER_A];
			temp_word = temp_word << 1;
			if ((Flags & FLAG_C) == FLAG_C)
			{
				temp_word = temp_word | 0x01;
			}
			flag_c_handler(temp_word);
			Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
			break;
		
		case 0x0F: // RRC - Rotate A right
			temp_word = (WORD)Registers[REGISTER_A];
			//Checks if the the lowest bit is 1 or 0
			if ((temp_word & 0x01) != 0)
			{
				temp_word = temp_word >> 1;
				temp_word = temp_word | 0x80;
				flag_c_handler(0x100);
			}
			else
			{
				temp_word = temp_word >> 1;
				flag_c_handler(0); 
			}
			Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);

			break;

		case 0x1F: // RAR - Rotate A right through carry
			temp_word = (WORD)Registers[REGISTER_A];
			if ((Flags & FLAG_C) != 0)
			{
				temp_word = temp_word | 0x100;
			}

			//Checks if the lowest bit is 1 or 0. Sets FLAG_C equal to the result
			if ((temp_word & 0x01) != 0)
				flag_c_handler(0x100);
			else
				flag_c_handler(0);

			temp_word = temp_word >> 1;
			Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
			break;

		case 0x2F: // CMA -Compliment A
			Registers[REGISTER_A] = (BYTE)((WORD)Registers[REGISTER_A] ^ 0xFF);
			break;

		case 0x37: // STC - Set Carry flag
			Flags = Flags | FLAG_C;
			break;

		case 0x3F: // CMC - Compliment Carry flag
			Flags = Flags ^ FLAG_C;
			break;

		default:
			break;
		}
		break;
	}
}



void MOV_and_HLT_instructions(BYTE opcode)
{
	BYTE source;
	BYTE destination;
	BYTE temp;
	WORD address;

	if (opcode == 0x76)
		halt = true;  // HLT - halt microprocessor
	else // MOV - Move register to register
	{
		source = opcode & 0x07;
		destination = (opcode >> 3) & 0x07;

		if (source == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			single_registry_store_and_load(&Registers[REGISTER_M],address,true);
		}

		Registers[destination] = Registers[source];

		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			single_registry_store_and_load(&Registers[REGISTER_M],address,false);
		}
	}
}



void block_10_instructions(BYTE opcode)
{
	BYTE  source;
	BYTE  instruction_type;
	WORD temp_word = 0;
	WORD  address;

	instruction_type = (opcode >> 3) & 0x07;
	source = opcode & 0x07;
	
	if (source == REGISTER_M)
	{
		address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
		single_registry_store_and_load(&Registers[REGISTER_M],address,true);
	}

	temp_word = (WORD)Registers[source];

	switch (instruction_type)
	{
	case 0x00: // ADD S - Add register to A
		temp_word  = (WORD)Registers[REGISTER_A] + temp_word;
		break;
		
	case 0x01: // ADC - Add register to A with carry
		temp_word  = (WORD)Registers[REGISTER_A] + temp_word;
		if ((Flags & FLAG_C) !=0)
			temp_word++;
		break;

	case 0x03: // SBB - Subtract register from A with borrow
		temp_word  = (WORD)Registers[REGISTER_A] - temp_word;
		if ((Flags & FLAG_C) !=0)
			temp_word--;
		break;

	case 0x02: // SUB S - Subtract register from A
		temp_word  = (WORD)Registers[REGISTER_A] - temp_word;
		break;
	
	case 0x04: // ANA - AND register with A
		temp_word  = (WORD)Registers[REGISTER_A] & temp_word;
		break;

	case 0x05: // XRA
		temp_word  = (WORD)Registers[REGISTER_A] ^ temp_word;
		break;
	case 0x06: // ORA - OR register with A
		temp_word  = (WORD)Registers[REGISTER_A] | temp_word;
		break;

	case 0x07: // CMP - Compare register with A (Does not set Register A, only the flags)
		temp_word  = (WORD)Registers[REGISTER_A] - temp_word;
		break;

	}
	// Does not assign temp_word to Register A when the instruction code is CMP
	if (instruction_type != 0x07)
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
	
	flag_c_handler(temp_word);
	set_flags((BYTE)(temp_word & 0xFF));
}



void block_11_instructions(BYTE opcode)
{
	WORD address;
	BYTE lb;
	BYTE hb;
	BYTE temp;
	WORD temp_word;

	switch (opcode)
	{
	case 0xC0:	// RNZ - Calls return from sub-routine (RET) when the Zero Flag is inactive
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_Z) == 0)
		{
			ret_handler(&lb,&hb);
		}
		break;

	case 0xC1:	// POP B - Pop register pair BC from the stack
		pop_stack_pointer(&Registers[REGISTER_C],&Registers[REGISTER_B]);
		break;

	case 0xC2:	// JNZ - Calls unconditional jump (JMP) when the Zero Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) == 0)
			jmp_handler(&lb,&hb);
		break;

	case 0xC3:	// JMP - Unconditional jump
		lb = fetch();
		hb = fetch();
		jmp_handler(&lb,&hb);
		break;

	case 0xC4:	// CNZ - Calls call sub-routine (CALL) when the Zero Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) == 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xC5:	// PUSH B - push register BC on the stack
		push_stack_pointer(&Registers[REGISTER_B],&Registers[REGISTER_C]);
		break;

	case 0xC6:	// ADI - Add immediate to A
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		flag_c_handler(temp_word);
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xC7:	// RST 0 - Restart (Call 000*8)
		rst_handler(opcode);
		break;

	case 0xC8:	// RZ - Calls return from sub-routine (RET) when the Zero Flag is active
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_Z) != 0)
			ret_handler(&lb,&hb);
		break;

	case 0xC9:	// RET - Return from subroutine
		lb = fetch();
		hb = fetch();
		ret_handler(&lb,&hb);
		break;

	case 0xCA:	// JZ - Calls unconditional jump (JMP) when the Zero Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) != 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xCB:	// .BYTE 0xCB  
		break;

	case 0xCC:	// CZ - Calls call sub-routine (CALL) when the Zero Flag is active      
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) != 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xCD:	// CALL - Subroutine call
		lb = fetch();
		hb = fetch();
		address = (WORD)(hb << 8) + (WORD)lb;
		call_handler(address);
		break;

	case 0xCE:	// ACI - Add immediate to A with carry
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if ((Flags & FLAG_C) !=0)
			temp_word++;

		flag_c_handler(temp_word);
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xCF:	// RST 1 - Restart (Call 001*8)
		rst_handler(opcode);
		break;

	case 0xD0:	// RNC - Calls return from sub-routine (RET) when the Carry Flag is inactive
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_C) == 0)
			ret_handler(&lb,&hb);

		break;

	case 0xD1:	// POP D - Pop register pair DE from the stack
		pop_stack_pointer(&Registers[REGISTER_E],&Registers[REGISTER_D]);
		break;

	case 0xD2:	// JNC - Calls unconditional jump (JMP) when the Carry Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_C) == 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xD3:	// OUT - TO CHECK
		break;

	case 0xD4:	// CNC - Calls call sub-routine (CALL) when the Carry Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_C) == 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xD5:	// PUSH D - push register DE on the stack
		push_stack_pointer(&Registers[REGISTER_D],&Registers[REGISTER_E]);
		break;

	case 0xD6:	// SUI - Subtract immediate from A
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		flag_c_handler(temp_word);
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xD7:	// RST 2 - Restart (Call 010*8)
		rst_handler(opcode);
		break;

	case 0xD8:	// RC - Calls return from sub-routine (RET) when the Carry Flag is active
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_C) != 0)
			ret_handler(&lb,&hb);

		break;

	case 0xD9:	// .BYTE 0xD9 - TO CHECK
		break;

	case 0xDA:	// JC - Calls unconditional jump (JMP) when the Carry Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_C) != 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xDB:	// IN - TO CHECK   
		break;

	case 0xDC:	// CC - Calls call sub-routine (CALL) when the Carry Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_C) != 0)
		{		
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xDD:	// .BYTE 0xDD - TO CHECK
		break;

	case 0xDE:	// SBI - Subtract immediate from A with borrow
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if ((Flags & FLAG_C) !=0)
			temp_word--;

		flag_c_handler(temp_word);
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xDF:	// RST 3 - Restart (Call 011*8)
		rst_handler(opcode);
		break;

	case 0xE0:	// RPO - Calls return from sub-routine (RET) when the Parity Flag is inactive
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_P) == 0)
			ret_handler(&lb,&hb);

		break;

	case 0xE1:	// POP H - pop register HL from the stack
		pop_stack_pointer(&Registers[REGISTER_L],&Registers[REGISTER_H]);
		break;

	case 0xE2:	// JPO - Calls unconditional jump (JMP) when the Parity Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_P) == 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xE3:	// XTHL        
		register_swap(&Registers[REGISTER_L], &Memory[StackPointer]);
		StackPointer++;
		register_swap(&Registers[REGISTER_H], &Memory[StackPointer]);
		StackPointer--;
		
		break;

	case 0xE4:	// CPO - Calls call sub-routine (CALL) when the Parity Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_P) == 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xE5:	// PUSH H - push register HL onto stack
		push_stack_pointer(&Registers[REGISTER_H],&Registers[REGISTER_L]);
		break;

	case 0xE6:	// ANI - AND immediate with A - CHANGED 19/11/2013
		lb = fetch();
		//temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;
		Flags = Flags & (0xFF - FLAG_C); //Clear FLAG_C
		Registers[REGISTER_A] = (BYTE)((WORD)Registers[REGISTER_A] & (WORD)lb);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xE7:	// RST 4 - Restart (Call 100*8)
		rst_handler(opcode);
		break;

	case 0xE8:	// RPE - Calls return from sub-routine (RET) when the Carry Flag is active
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_P) != 0)
			ret_handler(&lb,&hb);
		
		break;

	case 0xE9:	// PCHL - IN PROGRESS
		jmp_handler(&Registers[REGISTER_L],&Registers[REGISTER_H]);
		break;

	case 0xEA:	// JPE - Calls unconditional jump (JMP) when the parity Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_P) != 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xEB:	// XCHG - Exchange DE and HL content
		register_swap(&Registers[REGISTER_H], &Registers[REGISTER_D]);
		register_swap(&Registers[REGISTER_L], &Registers[REGISTER_E]);

		break;

	case 0xEC:	// CPE - Calls call sub-routine (CALL) when the Parity Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_P) != 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xED:	// .BYTE 0xED - TO CHECK
		break;

	case 0xEE:	// XRI - Exclusive OR immediate with A - CHANGED RECENTLY - 19/11/2013
		lb = fetch();
		// Clear flags
		Flags = Flags & (0xFF - FLAG_C); //Clear FLAG_C
		Flags = Flags & (0xFF - FLAG_A); //Clear FLAG_A
		Registers[REGISTER_A] = (BYTE)((WORD)Registers[REGISTER_A] ^ (WORD)lb);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xEF:	// RST 5 - Restart (Call 101*8)
		rst_handler(opcode);
		break;

	case 0xF0:	// RP - Calls return from sub-routine (RET) when the Sign Flag is inactive
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_S) == 0)
			ret_handler(&lb,&hb);

		break;

	case 0xF1:	// POP PSW - pop register A and flags from the stack
		pop_stack_pointer(&Flags,&Registers[REGISTER_A]);
		break;

	case 0xF2:	// JP - Calls unconditional jump (JMP) when the Sign Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_S) == 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xF3:	// DI - TO CHECK
		break;

	case 0xF4:	// CP - Calls call sub-routine (CALL) when the Sign Flag is inactive
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_S) == 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xF5:	// PUSH PSW  - push register A and flags into stack
		push_stack_pointer(&Registers[REGISTER_A],&Flags);
		break;

	case 0xF6:	// ORI SCULLY - OR immediate with A - CHANGED RECENTLY - 19/11/2013
		lb = fetch();
		// Clear flags
		Flags = Flags & (0xFF - FLAG_C);
		Flags = Flags & (0xFF - FLAG_A);
		Registers[REGISTER_A] = (BYTE)((WORD)Registers[REGISTER_A] | (WORD)lb);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xF7:	// RST 6 - Restart (Call 110*8)
		rst_handler(opcode);
		break;

	case 0xF8:	// RM - Calls return from sub-routine (RET) when the Sign Flag is active
		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_S) != 0)
			ret_handler(&lb,&hb);

		break;

	case 0xF9:	// SPHL - Set Stack Pointer to content of Regesiter pair HL
		StackPointer = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
		break;

	case 0xFA:	// JM - Calls unconditional jump (JMP) when the Sign Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_S) != 0)
			jmp_handler(&lb,&hb);

		break;

	case 0xFB:	// EI - Not Needed
		break;

	case 0xFC:	// CM - Calls call sub-routine (CALL) when the Sign Flag is active
		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_S) != 0)
		{
			address = (WORD)(hb << 8) + (WORD)lb;
			call_handler(address);
		}
		break;

	case 0xFD:	// .BYTE 0xFD - Not Needed
		break;

	case 0xFE:	// CPI - Compare immediate with A
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		flag_c_handler(temp_word);
		set_flags((BYTE)(temp_word & 0xFF));
		break;

	case 0xFF:	// RST 7 - Restart (Call 111*8)
		rst_handler(opcode);
		break;

	default:
		break;
	}
}



void execute(BYTE opcode)
{
	BYTE ms2bits;

	ms2bits = (opcode >> 6) & 0x03;

	switch (ms2bits)
	{
	case 0:
		block_00_instructions(opcode);
		break;

	case 1:		// MOV and HALT
		MOV_and_HLT_instructions(opcode);
		break;

	case 2:
		block_10_instructions(opcode);
		break;

	default:
		block_11_instructions(opcode);
		break;
	}
}


void emulate_8080()
{
	BYTE opcode;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                  A  B  C  D  E  H  L   SP\n");

	while ((!halt) && (memory_in_range))
	{
		printf("%04X ", ProgramCounter);           // Print current address

		opcode = fetch();
		execute(opcode);

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);     // Print Accumulator
		printf("%02X ", Registers[REGISTER_B]);     // Print Register B
		printf("%02X ", Registers[REGISTER_C]);     // Print Register C
		printf("%02X ", Registers[REGISTER_D]);     // Print Register D
		printf("%02X ", Registers[REGISTER_E]);     // Print Register E
		printf("%02X ", Registers[REGISTER_H]);     // Print Register H
		printf("%02X ", Registers[REGISTER_L]);     // Print Register L

		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_S) == FLAG_S)	            // Print Sign Flag
		{
			printf("S=1 ");
		}
		else
		{
			printf("S=0 ");
		}

		if ((Flags & FLAG_Z) == FLAG_Z)	            // Print Zero Flag
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}

		if ((Flags & FLAG_A) == FLAG_A)	            // Print Auxillary Carry Flag
		{
			printf("A=1 ");
		}
		else
		{
			printf("A=0 ");
		}

		if ((Flags & FLAG_P) == FLAG_P)	            // Print Parity Flag
		{
			printf("P=1 ");
		}
		else
		{
			printf("P=0 ");
		}

		if ((Flags & FLAG_C) == FLAG_C)	            // Print Carry Flag
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}




////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (End)                   //
////////////////////////////////////////////////////////////////////////////////













void initialise_filenames()
{
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}

int find_dot_position(char *filename)
{
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.') 
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
{
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) 
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) 
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);

		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}





void load_and_run()
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	ln = 0;
	chr = '\0';
	while (chr != '\n')
	{
		chr = getchar();

		switch(chr)
		{
		case '\n':
			break;
		default:
			if (ln < MAX_FILENAME_SIZE)
			{
				hex_file [ln] = chr;
				trc_file [ln] = chr;
				ln++;
			}
			break;
		}
	}

	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;
		for (i=0; i<8; i++)
		{
			Registers[i] = 0;
		}
		for (i=0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) 
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate the 8080

		emulate_8080();
	}
	else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}




void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));

	while (!testing_complete)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 0)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);
			}
			else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else
			{
				// Clear Registers and Memory

				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<8; i++)
				{
					Registers[i] = 0;
				}
				for (i=0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
							}
							load_at++;
						}
						else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 8080

				if (load_at > 1)
				{
					emulate_8080();

					// Send results

					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %04X", Registers[REGISTER_A], Flags, Registers[REGISTER_B], Registers[REGISTER_C], Registers[REGISTER_D], Registers[REGISTER_E], Registers[REGISTER_H], Registers[REGISTER_L], StackPointer);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Intel 8080 Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1 (2013-14)\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
		// Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//   //printf("Bind failed! \n");  // Bind failed!
	//}



	chr = '\0';
	while ((chr != 'e') && (chr != 'E')) 
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		printf("Enter option: ");
		
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run();
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}