/*

Interp_main.h

Header file for Interp_main.c

*/

typedef enum InterpAction InterpAction_t;
enum InterpAction{
	ACTION_NONE, //No action
	ACTION_EXIT, //Exit, "exit"
	ACTION_HI, //Send a hi to <operand>, "hello <operand>"
	ACTION_ECHO, //Echo the following text to the user, "echo <text>"
	ACTION_ADD_REG, //Add reg <2> to <1> and store in <1>, "add <1> <2>"
	ACTION_SUB_REG, //Subtract reg <2> from <1> and store in <1>, "sub <1> <2>"
	ACTION_MV_REG, //Move value of register <1> to register <2>, "mv <1> <2>"
	ACTION_SET_REG, //Set the value of register <1> to <val>, "set <1> <val>"
	ACTION_SV_MEM, //Save the value of register <1> to currently selected memory location (register #1), "svm <1>"
	ACTION_RD_MEM, //Read the value of selected memory location (register #1) and set register <1> to that value, "rdm <1>"
	ACTION_JMP, //Jump to line <ln>, "jmp <ln>"
	ACTION_JMP_IF, //Jump to line <ln> if register <1> == register <2>, "if <ln> <1> <2>"
	ACTION_JMP_NIF, //Jump to line <ln> if register <1> != register <2>, "ifn <ln> <1> <2>"
	ACTION_JMP_LT, //Jump to line <ln> if register <1> < register <2>, "ifl <ln> <1> <2>"
	ACTION_JMP_MT, //Jump to line <ln> if register <1> > register <2>, "ifm <ln> <1> <2>"
	ACTION_DIV_REG, //Divide reg <1> by reg <2>, store in reg <1> and put remainder in register #2, "div <1> <2>"
	ACTION_MUL_REG, //Multiply reg <1> by reg <2> and store in reg <1>, "mul <1> <2>"
	ACTION_BSH_POS, //Bitshift register <1> by value of <2> bits, "bpl <1> <2>"
	ACTION_BSH_NEG //Bitshift register <1> by - value of <2> bits, "bmi <1> <2>"
};

typedef struct OpcodeData OpDat_t;
struct OpcodeData{
	char* code;
	int len;
};

typedef struct InterpData IntDat_t;
struct InterpData{

	char* buff;
	long int buffSize;
	int memory[4096]; //8kB of memory, 2 bytes per space
	
	int reg[16]; //16 registers, 2 Bytes each
	/*
		0 = always 0
		1 = memory address currently set
		2 = overflow
		3 = Error register
		4 - 8 = reserved for future use
		9 - 15 = code use
	*/
	int pc; //Program counter

	int currentLine;
};

/* Loads and runs the interpreter */
void Interp_run(char*);

/* Executes interpreter */
void Interp_exec(IntDat_t*, int);

/* Takes a code and converts to an action enum */
InterpAction_t Interp_opcode(char*);

/* Isolates the opcode
mallocs the retured 'code' content! */
void Interp_getOpcode(OpDat_t*, char*, int);

/* Gets the next pc point offset */
void Interp_next(IntDat_t*);

/* Gets the last pc point offset */
void Interp_last(IntDat_t*);

/* Does the actual execution of opcodes */
char Interp_act(InterpAction_t, IntDat_t*, OpDat_t*);

/* Gets the next operand in the current instruction, returns the length of the operand (also stored in OpDat_t) */
int Interp_getNextOperand(OpDat_t*, char*, int);
