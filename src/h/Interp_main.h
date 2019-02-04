/*

Interp_main.h

Header file for Interp_main.c

*/

#define INTERP_MEM_SIZE 4096
#define INTERP_STACK_SIZE 16

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
	ACTION_SUB_NUM, //subn
	ACTION_MV_REG, //mv
	ACTION_SET_REG, //set
	ACTION_SV_MEM, //svm
	ACTION_RD_MEM, //rdm
	ACTION_JMP, //jmp
	ACTION_JMP_IF, //ife
	ACTION_JMP_NIF, //ifn
	ACTION_JMP_LT, //ifl
	ACTION_JMP_MT, //ifm
	ACTION_DIV_REG, //div
	ACTION_DIV_NUM, //divn
	ACTION_MUL_REG, //mul
	ACTION_MUL_NUM, //muln
	ACTION_BSH_POS, //bpl
	ACTION_BSN_POS, //bnp
	ACTION_BSH_NEG, //bmi
	ACTION_BSN_NEG, //bnm
	ACTION_MOD_REG, //mod
	ACTION_MOD_NUM, //modn
	ACTION_JMP_REL, //jmpr
	ACTION_PUSH, //push
	ACTION_POP, //pop
	ACTION_PEAK, //peak
	ACTION_JMPL, //jmpl
	ACTION_RET, //ret
	ACTION_FUNC, //func
	ACTION_ECHOC, //conc
	ACTION_ECHOV, //conv
	ACTION_IF, //if
};

typedef struct OpcodeData OpDat_t;
struct OpcodeData{
	char* code;
	int len;
};

typedef struct ProgramLabel ProgLab_t;
struct ProgramLabel {
	char name[65]; //Labels may be 64 characters in length (+1 for \0)
	int name_len;
	int lineNum;
	int pcPos;
};

typedef struct InterpData IntDat_t;
struct InterpData{
	char* buff;
	int buffSize;

	ProgLab_t* labels;
	int labelsLength;

	short memory[INTERP_MEM_SIZE]; //8kB of memory, 2 bytes per space
	short stack[INTERP_STACK_SIZE];
	short reg[16]; //16 registers, 2 Bytes each
	int pc; //Program counter
	int currentLine;
	int sp;
};

void Interp_setDebug(char);

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

/* Executes a jump forwards or backwards by <int> opcodes (techincally lines) */
void Interp_execJmp(int, IntDat_t*);

/* Checks for write permission on register */
char Interp_regWritePerm(char reg);

/* Checks for read permission on register */
char Interp_regReadPerm(char reg);

/* Operates on memory, reads the value of mem[r[1]] into reg*/
void Interp_memOpRead(IntDat_t*, char);

/* Operates on memory, writes the value of reg to mem[r[1]]*/
void Interp_memOpWrite(IntDat_t*, char);

/* Push the value onto the stack */
void Interp_pushStack(IntDat_t*, short);

/* Pop a value off the stack */
short Interp_popStack(IntDat_t*);

/* Peak at the value on the stack */
short Interp_peakStack(IntDat_t*);

/* Returns the pc of the End of the Instruction (';' terminated) */
int Interp_pcEOI(IntDat_t*);

/* Returns the pc of the End of the Line ('\n' terminated) */
int Interp_pcEOI(IntDat_t*);

/* Returns the pc of the Start of the Instruction (eliminates spaces, '\n' and '\r') */
int Interp_pcEOI(IntDat_t*);

/* Searches the labels and finds the requisite one */
ProgLab_t* Interp_getLabel(IntDat_t*, char*, int);

/* Searches the labels and finds the requisite one after the line specified */
ProgLab_t* Interp_getLabelAfter(IntDat_t*, char*, int, int);