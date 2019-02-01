#include "h/Interp_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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
	printf("Buffer of %i bytes prepared\n", InterpInfo.buffSize);
	//Read content to buff
	for(int i=0; i < InterpInfo.buffSize; i++){
		InterpInfo.buff[i] = fgetc(fp);
	}
	InterpInfo.buff[InterpInfo.buffSize - 1] = '\0';
	//Read the content back to the user:
	printf("Got file content:\n");
	printf("%s", InterpInfo.buff);

	//Count number of lines
	int numLines = 0;
	for(int i=0; i<InterpInfo.buffSize; i++){
		if(InterpInfo.buff[i] == '\n')
			numLines++;
	}
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
	char exit = 0;
	int currentLine = 0;
	
	OpDat_t op; op.code = NULL;
	InterpAction_t action;
	while(!exit && store->pc < store->buffSize){
		Interp_getOpcode(&op, store->buff, store->pc);
		printf("[pc=%i] Got opcode %s, len %i", store->pc, op.code, op.len);
		//Do things with opcode
		//Get the action the opcode string represents
		action = Interp_opcode(op.code);
		
		if(Interp_act(action, store, &op, currentLine) == 1){
			//We recieved an exit code:
			exit = 1;
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
		currentLine++;
		
		printf("\n");
		sleep(3);
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

char Interp_act(InterpAction_t action, IntDat_t* s, OpDat_t* op, int currentLine){
	char returnVal = 0;
	
	OpDat_t operand; operand.code = NULL;
	
	int operandLenAccumulator = 0;
	
	switch(action){

	case ACTION_HI:
		//Get the person we are saying hi to:
		operandLenAccumulator += Interp_getNextOperand(&operand, s->buff, s->pc + op->len);
		printf(" | Hi to %s !", operand.code);
	break;
	
	case ACTION_EXIT:
		returnVal = 1;
	break;

	default:
	break;

	}
	
	//Free operand if not already
	if(operand.code){
		free(operand.code);
		operand.code = NULL;
	}
	
	return returnVal;
}

int Interp_getNextOperand(OpDat_t* opdat, char* d, int s){
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
	
	return e;
}