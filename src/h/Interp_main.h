typedef enum InterpAction InterpAction_t;

enum InterpAction{
	ACTION_NONE,
	ACTION_EXIT,
	ACTION_HI
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
		1 = memory address currently read
		2 = overflow
		3 - 8 = reserved for future use
		9 - 15 = code use
	*/
	int pc; //Program counter
};

/* Loads and runs the interpreter */
void Interp_run(char*);

/* Executes interpreter */
void Interp_exec(struct InterpData*);

/* Takes a code and converts to an action enum */
InterpAction_t Interp_opcode(char*);

/* Isolates the opcode
mallocs the retured char! */
OpDat_t* Interp_getOpcode(char*, int);

/* Gets the next pc point offset */
int Interp_next(char*, int);

/* Does the actual execution of opcodes */
void Interp_act(InterpAction_t, struct InterpData*);
