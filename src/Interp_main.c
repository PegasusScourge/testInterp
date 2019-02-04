#include "h/Interp_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TRUE 1
#define FALSE 0

#define I_DEBUG Interp_DEBUG
char Interp_DEBUG = FALSE;

void Interp_setDebug(char d) {
	I_DEBUG = d;
}

void Interp_run(char* f){
	printf("Starting interpreter: opening file...\n");
	//check the file exists
	FILE *fp;
	if(!(fp = fopen(f, "r"))){
		//File failed to open
		printf("Failed to open file!\n");
		return; //Don't continue
	}

	//Do things
	
	//Get the size of the file
	fseek(fp, 0L, SEEK_END);
	long fpSize = (long)ftell(fp);
	rewind(fp);

	if (fpSize > INT_MAX) {
		printf("File has size %li; above INT_MAX! Unable to load file.\n", fpSize);
		//Close the file we were using and return
		fclose(fp);
		return;
	}
	
	IntDat_t InterpInfo;
	//Assign memory
	InterpInfo.buffSize = (int)fpSize*sizeof(char);
	InterpInfo.buff = malloc(InterpInfo.buffSize);

	if(InterpInfo.buff == NULL){
		printf("Failed to allocate %i bytes of memory for file buffer\n", InterpInfo.buffSize);
		//Close the file we were using and return
		fclose(fp);
		return;
	}

	if (I_DEBUG)
		printf("Buffer of %i bytes prepared\n", InterpInfo.buffSize);

	//Read content to buff
	for(int i=0; i < InterpInfo.buffSize; i++){
		InterpInfo.buff[i] = fgetc(fp);
	}
	InterpInfo.buff[InterpInfo.buffSize - 1] = '\0';

	//Close file
	fclose(fp);
	fp = NULL;

	//Read the content back to the user:
	if (I_DEBUG) {
		printf("Got file content:\n");
		printf("%s", InterpInfo.buff);
	}

	//Count number of lines
	int numLines = 0;
	InterpInfo.labelsLength = 0;
	for(int i=0; i<InterpInfo.buffSize; i++){
		if(InterpInfo.buff[i] == '\n')
			numLines++;
		if (InterpInfo.buff[i] == ':')
			InterpInfo.labelsLength++;
	}

	if (I_DEBUG)
		printf("%i lines in file, %i labels\n", numLines, InterpInfo.labelsLength);

	//Assign memory for labels:
	InterpInfo.labels = malloc(InterpInfo.labelsLength * sizeof(ProgLab_t));
	//Iterate though and assign labels
	int startPC = -1; //Set to -1 to indicate no start of label
	int currentLab = 0;
	int currentLne = 0;
	for (int i = 0; i < InterpInfo.buffSize; i++) {
		if (InterpInfo.buff[i] == '\n')
			currentLne++;

		if (InterpInfo.buff[i] == ':')
			startPC = i+1;

		if ((InterpInfo.buff[i] == ';' || InterpInfo.buff[i] == ' ') && startPC != -1) {
			//Trigger the setup of a new label
			InterpInfo.labels[currentLab].pcPos = startPC;
			InterpInfo.labels[currentLab].lineNum = currentLne;
			InterpInfo.labels[currentLab].name_len = i - startPC + 1;

			//Check that we won't run out of name storage
			if (InterpInfo.labels[currentLab].name_len <= 65) {
				memcpy(InterpInfo.labels[currentLab].name, &InterpInfo.buff[startPC], InterpInfo.labels[currentLab].name_len);
				//Null terminate the string
				InterpInfo.labels[currentLab].name[InterpInfo.labels[currentLab].name_len - 1] = '\0';
			}

			startPC = -1;
			currentLab++;
		}
	}
	if (startPC != -1) {
		//We somehow managed to not finish a label?
		printf("A label declaration didn't finish? Continuing anyway\n");
	}

	if (I_DEBUG) {
		printf("Got labels:\n");
		for (int i = 0; i < InterpInfo.labelsLength; i++) {
			printf("Name: \"%s\" (len:%i), line: %i, pc: %i\n", InterpInfo.labels[i].name, InterpInfo.labels[i].name_len, InterpInfo.labels[i].lineNum, InterpInfo.labels[i].pcPos);
		}
	}

	printf("***** START *****\n");
	//Enter interpreter loop
	Interp_exec(&InterpInfo, numLines);
	printf("***** END *****\n");

	//Free our buffers before we leave
	free(InterpInfo.buff);
	free(InterpInfo.labels);
}

void Interp_exec(IntDat_t* store, int numLines){
	char exit = FALSE;
	
	//Init the store values
	store->currentLine = 0;
	store->pc = 0;
	store->sp = 0;
	store->reg[0] = 0;
	
	OpDat_t op; op.code = NULL;
	InterpAction_t action;
	while(!exit && store->pc < store->buffSize){
		Interp_getOpcode(&op, store->buff, store->pc);

		if(I_DEBUG)
			printf("STATUS: [pc=%i,line=%i] Got opcode %s, len %i | OUTPUT: ", store->pc, store->currentLine, op.code, op.len);

		//Do things with opcode
		//Get the action the opcode string represents
		action = Interp_opcode(op.code);
		
		if(Interp_act(action, store, &op) == ACTION_EXIT){
			//We recieved an exit code:
			exit = TRUE;
		}

		//Clean up:
		//Free opcode
		if(op.code){
			free(op.code);
			op.code = NULL;
		}

		//Increment pc
		Interp_next(store);
		//We moved to the next line of the file
		store->currentLine++;

		if(I_DEBUG)
			printf("\n");
	}
}

void Interp_getOpcode(OpDat_t* opdat, char* d, int s){
	int e=0;
	while(d[s+e] != ' ' && d[s+e] != ';'){
		e++;
	}
	//Add space for null terminator
	e++;
	if(opdat->code){
		free(opdat->code);
		opdat->code = NULL;
	}
	opdat->code = malloc(e*sizeof(char));
	memcpy(opdat->code, &d[s], e-1);
	opdat->code[e-1] = '\0';
	opdat->len = e;
	//printf("Opcode len %i\n", e);
}

InterpAction_t Interp_opcode(char* opcode){
	if(strcmp(opcode, "hello") == 0){
		return ACTION_HI;
	}
	if(strcmp(opcode, "exit") == 0){
		return ACTION_EXIT;
	}
	if (strcmp(opcode, "con") == 0) {
		return ACTION_ECHO;
	}
	if (strcmp(opcode, "add") == 0) {
		return ACTION_ADD_REG;
	}
	if (strcmp(opcode, "sub") == 0) {
		return ACTION_SUB_REG;
	}
	if (strcmp(opcode, "mv") == 0) {
		return ACTION_MV_REG;
	}
	if (strcmp(opcode, "set") == 0) {
		return ACTION_SET_REG;
	}
	if (strcmp(opcode, "svm") == 0) {
		return ACTION_SV_MEM;
	}
	if (strcmp(opcode, "rdm") == 0) {
		return ACTION_RD_MEM;
	}
	if (strcmp(opcode, "jmp") == 0) {
		return ACTION_JMP;
	}
	if (strcmp(opcode, "ife") == 0) {
		return ACTION_JMP_IF;
	}
	if (strcmp(opcode, "ifn") == 0) {
		return ACTION_JMP_NIF;
	}
	if (strcmp(opcode, "ifl") == 0) {
		return ACTION_JMP_LT;
	}
	if (strcmp(opcode, "ifm") == 0) {
		return ACTION_JMP_MT;
	}
	if (strcmp(opcode, "div") == 0) {
		return ACTION_DIV_REG;
	}
	if (strcmp(opcode, "mul") == 0) {
		return ACTION_MUL_REG;
	}
	if (strcmp(opcode, "bpl") == 0) {
		return ACTION_BSH_POS;
	}
	if (strcmp(opcode, "bmi") == 0) {
		return ACTION_BSH_NEG;
	}
	if (strcmp(opcode, "conr") == 0) {
		return ACTION_ECHOR;
	}
	if (strcmp(opcode, "mod") == 0) {
		return ACTION_MOD_REG;
	}
	if (strcmp(opcode, "jmpr") == 0) {
		return ACTION_JMP_REL;
	}
	if (strcmp(opcode, "addn") == 0) {
		return ACTION_ADD_NUM;
	}
	if (strcmp(opcode, "subn") == 0) {
		return ACTION_SUB_NUM;
	}
	if (strcmp(opcode, "divn") == 0) {
		return ACTION_DIV_NUM;
	}
	if (strcmp(opcode, "muln") == 0) {
		return ACTION_MUL_NUM;
	}
	if (strcmp(opcode, "modn") == 0) {
		return ACTION_MOD_NUM;
	}
	if (strcmp(opcode, "bnp") == 0) {
		return ACTION_BSN_POS;
	}
	if (strcmp(opcode, "bnm") == 0) {
		return ACTION_BSN_NEG;
	}
	if (strcmp(opcode, "push") == 0) {
		return ACTION_PUSH;
	}
	if (strcmp(opcode, "pop") == 0) {
		return ACTION_POP;
	}
	if (strcmp(opcode, "peak") == 0) {
		return ACTION_PEAK;
	}
	if (strcmp(opcode, "jmpl") == 0) {
		return ACTION_JMPL;
	}
	if (strcmp(opcode, "ret") == 0) {
		return ACTION_RET;
	}
	if (strcmp(opcode, "func") == 0) {
		return ACTION_FUNC;
	}
	if (strcmp(opcode, "cons") == 0) {
		return ACTION_ECHOC;
	}
	if (strcmp(opcode, "conv") == 0) {
		return ACTION_ECHOV;
	}
	if (strcmp(opcode, "if") == 0) {
		return ACTION_IF;
	}

	if (opcode[0] == '#') {
		//We detected a comment!
		return ACTION_NONE;
	}
	if (opcode[0] == ':') {
		//We detected a label!
		return ACTION_NONE;
	}

	//We don't know what the opcode meant, so we return ACTION_NONE
	return ACTION_NONE;
}

int Interp_pcEOI(IntDat_t* s) {
	int currentP = s->pc;

	//Increments up to the character before the ;
	while (s->pc < s->buffSize && s->buff[s->pc] != ';') {
		currentP++;
	}
	currentP++; //Put ourselves on the ;

	return currentP;
}

int Interp_pcSOI(IntDat_t* s) {
	int currentP = s->pc;

	//Increments past ' ' characters
	while (s->pc < s->buffSize && (s->buff[s->pc] == ' ' || s->buff[s->pc] == '\n' || s->buff[s->pc] == '\r')) {
		currentP++;
	}

	return currentP;
}

int Interp_pcEOL(IntDat_t* s) {
	int currentP = s->pc;

	//Increments up to the character before the \n
	while (s->pc < s->buffSize && s->buff[s->pc] != '\n') {
		currentP++;
	}
	currentP++; //Put ourselves on the \n

	return currentP;
}

void Interp_next(IntDat_t* store){
	char* d = store->buff;
	
	//Move onto the ';' terminator of the current command
	do{
		store->pc++;
	}while(store->pc < store->buffSize && d[store->pc] != ';');
	
	//Move until we are off of \n or \r values
	while (store->pc < store->buffSize && d[store->pc] != '\n') {
		store->pc++;
	}
	store->pc++;
	
	//We should now be at the next opcode
}

void Interp_last(IntDat_t* store) {
	char* d = store->buff;

	//Get back before the last ;, at worst case there are 3 characters between us
	store->pc -= 3;

	//Move onto the ';' terminator of the last command
	do {
		store->pc--;
	} while (d[store->pc] != ';' && store->pc >= 0);
	store->pc++;

	//We should now be at the last opcode
}

int Interp_getNextOperand(OpDat_t* opdat, char* d, int s) {
	int e = 0;
	while (d[s + e] != ' ' && d[s + e] != ';') {
		e++;
	}
	//Add space for null terminator
	e++;
	if (opdat->code) {
		free(opdat->code);
		opdat->code = NULL;
	}
	opdat->code = malloc(e * sizeof(char));
	memcpy(opdat->code, &d[s], e - 1);
	opdat->code[e - 1] = '\0';
	opdat->len = e;

	return e;
}

void Interp_execJmp(int jmpTo, IntDat_t* s) {
	if (jmpTo < s->currentLine) {
		//Go backwards
		if (I_DEBUG)
			printf(" | -ve");

		while (s->currentLine != jmpTo) {
			Interp_last(s);
			s->currentLine--;

			if (I_DEBUG)
				printf(",%i", s->currentLine);
		}
	}
	else {
		//Go forwards
		if (I_DEBUG)
			printf(" | +ve");

		while (s->currentLine != jmpTo) {
			Interp_next(s);
			s->currentLine++;

			if (I_DEBUG)
				printf(",%i", s->currentLine);
		}
	}
}

char Interp_regWritePerm(char reg) {
	if (reg == 1 || (reg >= 9 && reg <= 15))
		return TRUE;
	return FALSE;
}

char Interp_regReadPerm(char reg) {
	if (reg >= 0 && reg <= 15)
		return TRUE;
	return FALSE;
}

void Interp_pushStack(IntDat_t* s, short val) {
	s->sp++;
	if (s->sp < 0) {
		//Throw a stack underflow
	}
	else if (s->sp >= INTERP_STACK_SIZE) {
		//Throw a stack overflow error
	}
	else {
		s->stack[s->sp] = val;
	}
}

short Interp_popStack(IntDat_t* s) {
	if (s->sp < 0) {
		//Throw a stack underflow
	}
	else if (s->sp >= INTERP_STACK_SIZE) {
		//Throw a stack overflow error
	}
	else {
		return s->stack[s->sp];
	}
	s->sp--;
	return 0;
}

short Interp_peakStack(IntDat_t* s) {
	if (s->sp < 0) {
		//Throw a stack underflow
	}
	else if (s->sp >= INTERP_STACK_SIZE) {
		//Throw a stack overflow error
	}
	else {
		return s->stack[s->sp];
	}
	return 0;
}

void Interp_memOpRead(IntDat_t* s, char reg) {
	//Check that the memory position selected is valid
	if (s->reg[1] >= 0 && s->reg[1] < INTERP_MEM_SIZE) {
		//Perform read into reg
		s->reg[reg] = s->memory[s->reg[1]];
	}
}

void Interp_memOpWrite(IntDat_t* s, char reg) {
	//Check that the memory position selected is valid
	if (s->reg[1] >= 0 && s->reg[1] < INTERP_MEM_SIZE) {
		//Perform write to memory
		s->memory[s->reg[1]] = s->reg[reg];
	}
}

ProgLab_t* Interp_getLabel(IntDat_t* s, char* st, int sLen) {
	ProgLab_t* label = NULL;

	for (int i = 0; i < s->labelsLength; i++) {
		if (strcmp(s->labels[i].name, st) == 0) {
			label = &s->labels[i];
		}
	}

	return label;
}

ProgLab_t* Interp_getLabelAfter(IntDat_t* s, char* st, int sLen, int line) {
	ProgLab_t* label = NULL;

	for (int i = 0; i < s->labelsLength; i++) {
		if (strcmp(s->labels[i].name, st) == 0 && s->labels[i].lineNum >= line) {
			label = &s->labels[i];
		}
	}

	return label;
}

char Interp_act(InterpAction_t action, IntDat_t* s, OpDat_t* op){
	char returnVal = 0;
	
	OpDat_t operand; operand.code = NULL;
	char valS[65];
	char regA = 0;
	char regB = 0;
	int valA = 0;
	//int valB = 0;
	
	int operandLenAccumulator = op->len;
	
	switch (action) {

	case ACTION_JMPL: {
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		memcpy(valS, operand.code, (operand.len + 1)); //+1 for the \0 character
		valS[operand.len] = '\0';

		if (I_DEBUG)
			printf("JMPL, valS: %s", valS);

		ProgLab_t* label = Interp_getLabel(s, valS, (operand.len + 1));
		if (label) {
			valA = label->lineNum;
			Interp_execJmp(valA - 1, s);
		}
		else {
			//Throw label not found exception
		}
	}	break;

	case ACTION_IF: {
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		memcpy(valS, operand.code, (operand.len + 1)); //+1 for the \0 character
		valS[operand.len] = '\0';
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (char)strtol(operand.code, NULL, 10); //Register 2

		char validIfFunc = FALSE;
		char skip = TRUE;
		//Skip is set to the inverse of the statement we are testing for; therefore if we are looking for ==, we set skip to the evaluation of !=
		if (strcmp(valS, "e") == 0 && Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			validIfFunc = TRUE;
			skip = (s->reg[regA] != s->reg[regB]);
		}
		if (strcmp(valS, "n") == 0 && Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			validIfFunc = TRUE;
			skip = (s->reg[regA] == s->reg[regB]);
		}
		if (strcmp(valS, "l") == 0 && Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			validIfFunc = TRUE;
			skip = (s->reg[regA] >= s->reg[regB]);
		}
		if (strcmp(valS, "m") == 0 && Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			validIfFunc = TRUE;
			skip = (s->reg[regA] <= s->reg[regB]);
		}

		if (validIfFunc && skip) {
			//Skip to the next :endif
			ProgLab_t* lab = Interp_getLabelAfter(s, "endif\0", 6, s->currentLine);
			if (lab) {
				Interp_execJmp(lab->lineNum - 1, s);
			}
		}
	}	break;

	case ACTION_FUNC: {
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		memcpy(valS, operand.code, (operand.len + 1)); //+1 for the \0 character
		valS[operand.len] = '\0';

		ProgLab_t* label = Interp_getLabel(s, valS, (operand.len + 1));

		//Check we didn't get a null pointer, thus no label
		if (label) {
			valA = label->lineNum;

			if (I_DEBUG)
				printf("FUNC, valS: %s, ln: %i, pc: %i, tgtLn: %i", valS, s->currentLine, s->pc, valA);

			//Before we jump, push the line number with +1
			Interp_pushStack(s, s->currentLine + 1);

			Interp_execJmp(valA - 1, s);
		}
		else {
			//Throw label not found?
		}
	}	break;

	case ACTION_RET:
		//Pop the value off the stack:
		valA = Interp_popStack(s);

		if (I_DEBUG)
			printf("RET, ln: %i, pc: %i, tgtLn: %i", s->currentLine, s->pc, valA);

		Interp_execJmp(valA - 1, s);
		break;

	case ACTION_PEAK:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("STACK_PEAK, regA: %i", regA);

		if (Interp_regWritePerm(regA)) {
			s->reg[regA] = Interp_peakStack(s);
		}
		break;

	case ACTION_POP:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("STACK_POP, regA: %i", regA);

		if (Interp_regWritePerm(regA)) {
			s->reg[regA] = Interp_popStack(s);
		}
		break;

	case ACTION_PUSH:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("STACK_PUSH, regA: %i", regA);

		if (Interp_regReadPerm(regA)) {
			 Interp_pushStack(s, s->reg[regA]);
		}
		break;

	case ACTION_HI:
		//Get the person we are saying hi to:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		printf("Hi to %s!", operand.code);

		if (!I_DEBUG)
			printf("\n");
		break;

	case ACTION_ECHO: {
		//Loop to find the end of the statement
		int end = op->len + s->pc;
		while (s->buff[end] != ';') {
			end++;
		}

		if(I_DEBUG)
			printf("ECHO\n");

		//Having discovered the end of the statement, now print the content
		for (int i = op->len + s->pc; i < end; i++) {
			printf("%c", s->buff[i]);
		}

		if (!I_DEBUG)
			printf("\n");
	}	break;

	case ACTION_ECHOR:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("ECHOR\n");

		if(Interp_regReadPerm(regA))
			printf("Register %i has value %i", regA, s->reg[regA]);

		if (!I_DEBUG)
			printf("\n");
		break;

	case ACTION_ECHOV:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("ECHOV\n");

		if (Interp_regReadPerm(regA))
			printf("%i", s->reg[regA]);
		break;

	case ACTION_ECHOC:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)

		if (I_DEBUG)
			printf("ECHOC\n");

		if (Interp_regReadPerm(regA)) {
			if ((s->reg[regA] & 0xFF00) >> 8 == 0) {
				//Print just the lower part of the reg value
				printf("%c", s->reg[regA] & 0x00FF);
			}
			else {
				printf("%c", (s->reg[regA] & 0xFF00) >> 8);
				printf("%c", s->reg[regA] & 0x00FF);
			}
		}
			
		break;

	case ACTION_SET_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10);

		if(I_DEBUG)
			printf("SET_REG, regA: %i, valA: %i", regA, valA);
		//Check on write access registers
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] = valA;
		}
		break;

	case ACTION_JMP_NIF:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (char)strtol(operand.code, NULL, 10); //Register 2

		if(I_DEBUG)
			printf("JMP_NIF, valA: %i, regA: %i, regB: %i", valA, regA, regB);

		//Check for all valid registers
		if (Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			if (s->reg[regA] != s->reg[regB]) {
				//Jump forward or backwards
				Interp_execJmp(valA, s);
			}
		}
		break;

	case ACTION_JMP_IF:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (char)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("JMP_IF, valA: %i, regA: %i, regB: %i", valA, regA, regB);

		//Check for all valid registers
		if (Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			if (s->reg[regA] == s->reg[regB]) {
				//Jump forward or backwards
				Interp_execJmp(valA, s);
			}
		}
		break;

	case ACTION_JMP_LT:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (char)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("JMP_LT, valA: %i, regA: %i, regB: %i", valA, regA, regB);

		//Check for all valid registers
		if (Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			if (s->reg[regA] < s->reg[regB]) {
				//Jump forward or backwards
				Interp_execJmp(valA, s);
			}
		}
		break;

	case ACTION_JMP_MT:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (char)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("JMP_MT, valA: %i, regA: %i, regB: %i", valA, regA, regB);

		//Check for all valid registers
		if (Interp_regReadPerm(regA) && Interp_regReadPerm(regB)) {
			if (s->reg[regA] > s->reg[regB]) {
				//Jump forward or backwards
				Interp_execJmp(valA, s);
			}
		}
		break;

	case ACTION_RD_MEM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1

		if (I_DEBUG)
			printf("MEM_RD, regA: %i", regA);

		if (Interp_regWritePerm(regA)) {
			Interp_memOpRead(s, regA);
		}
		break;

	case ACTION_SV_MEM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1

		if (I_DEBUG)
			printf("MEM_SV, regA: %i", regA);

		if (Interp_regReadPerm(regA)) {
			Interp_memOpWrite(s, regA);
		}
		break;

	case ACTION_JMP:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed

		if (I_DEBUG)
			printf("JMP, valA: %i", valA);

		Interp_execJmp(valA, s);
		break;

	case ACTION_JMP_REL:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10) - 1; //Jmp amount, accounting for the fact that the pc will be incremented after this is processed

		if (I_DEBUG)
			printf("JMP_REL, valA: %i", valA);

		Interp_execJmp(s->currentLine + valA, s);
		break;

	case ACTION_ADD_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if(I_DEBUG)
			printf("ADD_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] += s->reg[regB];
		}
		break;

	case ACTION_ADD_NUM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("ADD_NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] += valA;
		}
		break;

	case ACTION_SUB_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("SUB_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] -= s->reg[regB];
		}
		break;

	case ACTION_SUB_NUM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("SUB_NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] -= valA;
		}
		break;

	case ACTION_DIV_NUM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("DIV_NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] /= valA;
		}
		break;

	case ACTION_MUL_NUM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("MUL_NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] *= valA;
		}
		break;

	case ACTION_MOD_NUM:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("MOD_NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] %= valA;
		}
		break;

	case ACTION_BSN_POS:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("BSH_P-NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] = (s->reg[regA] << s->reg[regB]);
		}
		break;

	case ACTION_BSN_NEG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("BSH_N-NUM, regA: %i, valA: %i", regA, valA);

		//Check write access on regA
		if (Interp_regWritePerm(regA)) {
			//We can write:
			s->reg[regA] = (s->reg[regA] >> s->reg[regB]);
		}
		break;

	case ACTION_MV_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("MV_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] = s->reg[regB];
		}
		break;

	case ACTION_DIV_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("DIV_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] /= s->reg[regB];

			//Set the overflow reg to regA % regB
			s->reg[2] = s->reg[regA] % s->reg[regB];
		}
		break;

	case ACTION_MUL_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("MUL_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] *= s->reg[regB];
		}
		break;

	case ACTION_MOD_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("MOD_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] %= s->reg[regB];
		}
		break;

	case ACTION_BSH_POS:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("BSH_POS, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] = (s->reg[regA] << s->reg[regB]);
		}
		break;

	case ACTION_BSH_NEG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if (I_DEBUG)
			printf("BSH_NEG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (Interp_regWritePerm(regA) && Interp_regReadPerm(regB)) {
			//We can write:
			s->reg[regA] = (s->reg[regA] >> s->reg[regB]);
		}
		break;
	
	case ACTION_EXIT:
		if(I_DEBUG)
			printf("EXIT");

		returnVal = ACTION_EXIT;
		break;

	case ACTION_NONE:
		//Do nothing as instructed
		break;

	default:
		if (I_DEBUG)
			printf("Opcode not implemented yet");
		break;

	}
	
	//Free operand if not already
	if(operand.code){
		free(operand.code);
		operand.code = NULL;
	}
	
	return returnVal;
}
