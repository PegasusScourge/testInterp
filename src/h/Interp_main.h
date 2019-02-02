/*

Interp_main.h

Header file for Interp_main.c

*/

#define INTERP_MEM_SIZE 4096

typedef enum InterpAction InterpAction_t;
enum InterpAction{
	ACTION_NONE, //No action
	ACTION_EXIT, //exit
	ACTION_HI, //hello
	ACTION_ECHO, //con
	ACTION_ECHOR, //conr
	ACTION_ADD_REG, //add
	ACTION_ADD_NUM, //addn
	ACTION_SUB_REG, //sub
	ACTION_MV_REG, //mv
	ACTION_SET_REG, //set
	ACTION_SV_MEM, //svm -
	ACTION_RD_MEM, //rdm -
	ACTION_JMP, //jmp
	ACTION_JMP_IF, //ife
	ACTION_JMP_NIF, //ifn
	ACTION_JMP_LT, //ifl
	ACTION_JMP_MT, //ifm
	ACTION_DIV_REG, //div
	ACTION_MUL_REG, //mul
	ACTION_BSH_POS, //bpl
	ACTION_BSH_NEG, //bmi
	ACTION_MOD_REG, //mod
	ACTION_JMP_REL //jmpr
};

typedef struct OpcodeData OpDat_t;
struct OpcodeData{
	char* code;
	int len;
};

typedef struct InterpData IntDat_t;
struct InterpData{
	char* buff;
	long buffSize;
	short memory[INTERP_MEM_SIZE]; //8kB of memory, 2 bytes per space
	short reg[16]; //16 registers, 2 Bytes each
	long pc; //Program counter
	int currentLine;
};

void Interp_isDebug(char);

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

/* Executes a jump forwards or backwards by <int> opcodes (techincally lines too) */
void Interp_execJmp(int, IntDat_t*);

/* Checks for write permission on register */
char Interp_regWritePerm(char reg);

/* Checks for read permission on register */
char Interp_regReadPerm(char reg);

/* Operates on memory, reads the value of mem[r[1]] into reg*/
void Interp_memOpRead(IntDat_t*, char);

/* Operates on memory, writes the value of reg to mem[r[1]]*/
void Interp_memOpWrite(IntDat_t*, char);