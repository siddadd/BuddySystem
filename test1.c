#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "memalloc.h"
#include <stdbool.h>
//***********************************************************************************
int main()
{
	FILE * pFile;
	pFile = fopen ("test1_output.txt","w");
    int size;
    int RAM_SIZE=1<<20;
    void* RAM=malloc(RAM_SIZE);//1024*1024
	setup(3,RAM_SIZE,RAM);//Buddy System, Memory size=1024*1024, Start of memory=RAM

    //test 1
    size=0.5*1024;
	void* a=my_malloc(size);//We have 4 bytes header to save the size of that chunk in memory so the output starts at 4
    if ((int)a==-1)
        fprintf(pFile, "This size can not be allocated!\n");
    else
    {
    	fprintf(pFile, "start of the chunk a: %f K\n",(float)((int)a-(int)RAM)/1024);
    	fprintf(pFile, "End of the chunk a: %f K\n\n",(float)((int)a+size-(int)RAM)/1024);
    }

    my_free(a);

    size=200*1024;
	void* b=my_malloc(size);
    if ((int)b==-1)
        fprintf(pFile, "This size can not be allocated!\n");
    else
    {
    	fprintf(pFile, "start of the chunk b: %f K\n",(float)((int)b-(int)RAM)/1024);
    	fprintf(pFile, "End of the chunk b: %f K\n\n",(float)((int)b+size-(int)RAM)/1024);
    }

    size=900*1024;
	void* c=my_malloc(size);
    if ((int)c==-1)
        fprintf(pFile, "This size can not be allocated!\n");
    else
    {
    	fprintf(pFile, "start of the chunk c: %f K\n",(float)((int)c-(int)RAM)/1024);
    	fprintf(pFile, "End of the chunk c: %f K\n\n",(float)((int)c+size-(int)RAM)/1024);
    }
    fclose (pFile);
	printf("Done: Output written to %s\n", "test1_output.txt");
    return 0;

}

