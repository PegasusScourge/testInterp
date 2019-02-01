#include "h/main.h"
#include "h/Interp_main.h"

#include <stdio.h>

int main(int argc, char** argv){
	printf("Got argv:\n");

	for(int i=0; i < argc; i++){
		printf("%i: %s\n", i, argv[i]);
	}

	if(argc == 2){
		//We got the second arg which specifies our file to interpret
		Interp_run(argv[1]);
	} else {
		printf("Not enough args!\n");
	}

	return 0;
}
