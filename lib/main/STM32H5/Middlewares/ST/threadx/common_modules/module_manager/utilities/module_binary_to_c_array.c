#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Define the file handles.  */

FILE	*source_file;
FILE    *array_file;


int main(int argc, char* argv[])
{

int             alpha;
int             alpha1;
int             alpha2;
int             alpha3;
unsigned long   address;
unsigned long   column;


    /* Determine if the proper number of files are provided.  */
    if (argc != 3) 
    {

        /* Print an error message out and wait for user key hit.  */
		printf("module_binary_to_c_array.exe - Copyright (c) Microsoft Corporation v5.8\n");
        printf("**** Error: invalid input parameter for module_binary_to_c_array.exe **** \n");
        printf("     Command Line Should be:\n\n");
        printf("     > module_binary_to_c_array source_binary_file c_array_file <cr> \n\n");
        return(1);
    }

    /* Attempt to open the source file for reading.  */
    source_file =  fopen(argv[1], "rb");

    /* Determine if the source file was opened properly.  */
    if (source_file == NULL)
    {

        /* Print an error message out and wait for user key hit.  */
        printf("**** Error: open failed on binary source file **** \n");
        printf("            File: %s   ", argv[1]);
        return(2);
    }
    
	/* Determine if the binary file is a valid ThreadX module.  */
	alpha =   fgetc(source_file);
	alpha1 =  fgetc(source_file);
	alpha2 =  fgetc(source_file);
	alpha3 =  fgetc(source_file);

	if ((alpha != 0x4D && alpha != 0x55) || (alpha1 != 0x4F && alpha1 != 0x44) || (alpha2 != 0x44 && alpha2 != 0x4F) || (alpha3 != 0x55 && alpha3 != 0x4D))
	{

        /* Print an error message out and wait for user key hit.  */
        printf("**** Error: invalid format of binary input file **** \n");
        printf("            File: %s   ", argv[1]);
        return(3);
	}

	/* Attempt to open the dump file for writing.  */
    array_file =  fopen(argv[2], "w");

    /* Determine if the dump file was opened properly.  */
    if (array_file == NULL)
    {

        /* Print an error message out and wait for user key hit.  */
        printf("**** Error: open failed on C array file **** \n");
        printf("            File: %s   ", argv[2]);
        return(4);
    }

    fprintf(array_file, "/**************************** Module-Binary-to-C-array Utility **********************************/\n");
	fprintf(array_file, "/*                                                                                              */\n");
	fprintf(array_file, "/* Copyright (c) Microsoft Corporation                      Version 5.4, build date: 03-01-2018 */\n");
	fprintf(array_file, "/*                                                                                              */\n");
	fprintf(array_file, "/************************************************************************************************/\n\n");
    fprintf(array_file, "/* \n");
	fprintf(array_file, "   Input Binary file:   %30s\n", argv[1]);
	fprintf(array_file, "   Output C Array file: %30s\n", argv[2]);
	fprintf(array_file, "*/\n\n");

	/* Now print out the sections in a C array.  */
	fprintf(array_file, "unsigned char  module_code[] = {\n\n");
	fprintf(array_file, "/* Address                                            Contents                                        */\n\n");

	/* Seek to the beginning of the source file. */
    fseek(source_file, 0, SEEK_SET);

	/* Initialize the variables.  */
	address =  0;
	column =   0;

	do 
	{

		/* Get character from the input file.  */
		alpha =   fgetc(source_file);

		/* Have we reached EOF?  */
		if (alpha == EOF)
			break;

		/* Print out character with a leading comma, except on the first character.  */
		if (column == 0)
		{
			if (address != 0)
				fprintf(array_file, ",\n");
			fprintf(array_file, "/* 0x%08X */   0x%02X", address, (unsigned int) alpha);
		}
		else
			fprintf(array_file, ", 0x%02X", (unsigned int) alpha);

		/* Move column forward.  */
		column++;

		/* Are we at the end of the column?  */
		if (column >= 16)
		{

			column =  0;
    	}

		/* Move address forward.  */
		address++;
	} while (alpha != EOF);

	/* Finally, finish the C array containing the module code.  */
	fprintf(array_file, "};\n\n");

	/* Close files.  */
	fclose(source_file);
    fclose(array_file);

	return 0;
}
