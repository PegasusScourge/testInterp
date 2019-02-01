#include "h/Interp_main.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
	
	struct InterpData InterpInfo;
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
	fgets(InterpInfo.buff, InterpInfo.buffSize, fp);
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

	//Enter interpreter loop
	Interp_exec(&InterpInfo);

	//Free our buffer before we leave
	free(InterpInfo.buff);
}

void Interp_exec(struct InterpData* store){
	char exit = 0;
	OpDat_t* op;
	InterpAction_t action;
	while(!exit && store->pc < store->buffSize){
		op = Interp_getOpcode(store->buff, store->pc);
		printf("Got opcode %s, len %i\n", op->code, op->len);
		//Do things with opcode
		//Get the action the opcode string represents
		action = Interp_opcode(op->code);
		
		
		Interp_act(action, store);

		//Clean up:
		//Free opcode
		free(op->code);
		free(op);

		//Increment pc
		int pcInc = Interp_next(store->buff, store->pc);
		if(pcInc == 0)
			exit = 1;

		store->pc += pcInc;

		printf("pc=%i, pcInc=%i\n", store->pc, pcInc);
		sleep(3);
	}
}

OpDat_t* Interp_getOpcode(char* d, int s){
	int e=0;
	while(d[s+e] != ' '){
		e++;
	}
	//Add space for null terminator
	e++;
	OpDat_t* dat = malloc(sizeof(OpDat_t));
	dat->code = malloc(e*sizeof(char));
	memcpy(dat->code, &d[s], e-1);
	dat->code[e-1] = '\0';
	dat->len = e;
	//printf("Opcode len %i\n", e);
	return dat;
}

InterpAction_t Interp_opcode(char* opcode){
	if(strcmp(opcode, "hello") == 0){
		return ACTION_HI;
	}

	return ACTION_NONE;
}

int Interp_next(char* d, int s){
	int e=0;
	while(d[s+e] != '\n'){
		e++;
	}
	return e;
}

void Interp_act(InterpAction_t action, struct InterpData* s){
	switch(action){

	case ACTION_HI:
		printf("Hi!\n");
	break;

	default:
	break;

	}
}
