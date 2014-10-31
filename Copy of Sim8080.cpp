// Sim8080.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "123456789"

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









void execute(BYTE opcode)
{
	BYTE ms2bits;

	ms2bits 
	switch (opcode)
	{
	case 0x00:	// NOP - no operation
		break;

	case 0x37:  // STC - set carry flag
		Flags = Flags | FLAG_C;
		break;

	case 0x76:  // HLT - halt microprocessor
		halt = true;
		break;

	default:
		break;
	}
}


void emulate_8080()
{
	BYTE opcode;

	halt = false;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
	}

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

			if (strcmp(buffer, "Testing complete") == 0)
			{
				testing_complete = true;
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
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 8080

				emulate_8080();

				// Send results

				sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %04X", Registers[REGISTER_A], Flags, Registers[REGISTER_B], Registers[REGISTER_C], Registers[REGISTER_D], Registers[REGISTER_E], Registers[REGISTER_H], Registers[REGISTER_L], StackPointer);
				sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
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

