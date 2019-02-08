/******************************************************************************
* 
* sing.c
* 
* This program generates a lot of text output.
*
* Usage: Just run the program.
*
* Written Sept 6, 2015 by David Ferry
******************************************************************************/

#include <stdio.h>  //For printf()
#include <stdlib.h> //For atoi() and exit()

int main( int argc, char* argv[] ){

	int i = 0;
	int iterations = 0;
	
	if( argc != 2 ){
		printf("Usage: ./sing <number of verses>\n");
		exit(0);
	}

	iterations = atoi(argv[1]);

	for( i = 0; i < iterations; i++){
		printf("The Road goes ever on and on\n");
		printf("Down from the door where it began.\n");
		printf("Now far ahead the Road has gone,\n");
		printf("And I must follow, if I can,\n");
		printf("Pursuing it with eager feet,\n");
		printf("Until it joins some larger way,\n");
		printf("Where many paths and errands meet.\n");
		printf("And whither then? I cannot say.\n");
		printf("-Bilbo, The Lord of the Rings\n");
	}

	return 0;
}
