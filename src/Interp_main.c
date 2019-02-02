#include "h/Interp_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define I_DEBUG Interp_DEBUG
char Interp_DEBUG = FALSE;

void Interp_isDebug(char d) {
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
	long int fpSize = ftell(fp);
	rewind(fp);
	
	IntDat_t InterpInfo;
	InterpInfo.pc = 0;
	//Assign memory
	InterpInfo.buffSize = fpSize*sizeof(char);
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

	//Read the content back to the user:
	if (I_DEBUG) {
		printf("Got file content:\n");
		printf("%s", InterpInfo.buff);
	}

	//Count number of lines
	int numLines = 0;
	for(int i=0; i<InterpInfo.buffSize; i++){
		if(InterpInfo.buff[i] == '\n')
			numLines++;
	}

	if (I_DEBUG)
		printf("%i lines in file\n", numLines);

	//Close file
	fclose(fp);
	fp = NULL;

	printf("***** START *****\n");
	//Enter interpreter loop
	Interp_exec(&InterpInfo, numLines);
	printf("***** END *****\n");

	//Free our buffer before we leave
	free(InterpInfo.buff);
}

void Interp_exec(IntDat_t* store, int numLines){
	char exit = FALSE;
	store->currentLine = 0;
	
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
	if (strcmp(opcode, "if") == 0) {
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

	//We don't know what the opcode meant, so we return ACTION_NONE
	return ACTION_NONE;
}

void Interp_next(IntDat_t* store){
	char* d = store->buff;
	
	//Move onto the ';' terminator of the current command
	do{
		store->pc++;
	}while(d[store->pc] != ';');
	
	//Move until we are off of \n or \r values
	do{
		store->pc++;
	}while(d[store->pc] != '\n');
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
	} while (d[store->pc] != ';');
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

char Interp_act(InterpAction_t action, IntDat_t* s, OpDat_t* op){
	char returnVal = 0;
	
	OpDat_t operand; operand.code = NULL;
	char regA = 0;
	char regB = 0;
	int valA = 0;
	int valB = 0;
	
	int operandLenAccumulator = op->len;
	
	switch(action){

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

	case ACTION_SET_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Convert operand <1> to a char number (number is in base 10)
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		valA = (int)strtol(operand.code, NULL, 10);

		if(I_DEBUG)
			printf("SET_REG, regA: %i, valA: %i", regA, valA);
		//Check on write access registers
		if (regA >= 9 && regA <= 15) {
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
		if (regA >= 0 && regA <= 15 && regB >= 0 && regB <= 15) {
			if (s->reg[regA] != s->reg[regB]) {
				//Jump forward or backwards
				Interp_execJmp(valA, s);
			}
		}
		break;

	case ACTION_ADD_REG:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regA = (char)strtol(operand.code, NULL, 10); //Register 1
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + operandLenAccumulator);
		regB = (int)strtol(operand.code, NULL, 10); //Register 2

		if(I_DEBUG)
			printf("ADD_REG, regA: %i, regB: %i", regA, regB);

		//Check write access on regA, then read access on regB
		if (regA >= 9 && regA <= 15 && regB >= 0 && regB <= 15) {
			//We can write:
			s->reg[regA] += s->reg[regB];
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
