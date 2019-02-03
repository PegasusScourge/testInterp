#include "h/main.h"
#include "h/Interp_main.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
	printf("Got argv:\n");

	for(int i=0; i < argc; i++){
		printf("%i: %s\n", i, argv[i]);
	}

	if(argc == 2){
		//We got the second arg which specifies our file to interpret
		Interp_run(argv[1]);
	} else if(argc == 3){
		//Got a third argument, so set the debug flag
		Interp_setDebug((char)strtol(argv[2], NULL, 10));
		Interp_run(argv[1]);
	}
	else if(argc < 2){
		printf("Not enough args!\n");
	}
	else {
		printf("Too many args! I can't do your taxes\n");
	}

	return 0;
}
