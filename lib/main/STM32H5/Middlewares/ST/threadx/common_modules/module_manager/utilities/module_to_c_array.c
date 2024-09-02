#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Define the file handles.  */

FILE	*source_file;
FILE    *array_file;


#define ELF_ID_STRING_SIZE	    16
#define ELF_ARM_MACHINE_TYPE    40
#define ELF_EXECUTABLE          2


typedef struct ELF_HEADER_STRUCT 
{
    unsigned char   elf_header_id_string[ELF_ID_STRING_SIZE];
    unsigned short  elf_header_file_type;
    unsigned short  elf_header_machinge_type;
    unsigned long   elf_header_version;
    unsigned long   elf_header_entry_address;
    unsigned long   elf_header_program_header_offset;
    unsigned long   elf_header_section_header_offset;
    unsigned long   elf_header_processor_flags;
    unsigned short  elf_header_size;
    unsigned short  elf_header_program_header_size;
    unsigned short  elf_header_program_header_entries;
    unsigned short  elf_header_section_header_size;
    unsigned short  elf_header_section_header_entries;
    unsigned short  elf_header_section_string_index;
} ELF_HEADER;


typedef struct ELF_PROGRAM_HEADER_STRUCT
{
    unsigned long   elf_program_header_type;
    unsigned long   elf_program_header_offset;
    unsigned long   elf_program_header_virtual_address;
    unsigned long   elf_program_header_physical_address;
    unsigned long   elf_program_header_file_size;
    unsigned long   elf_program_header_memory_size;
    unsigned long   elf_program_header_flags;
    unsigned long   elf_program_header_alignment;
} ELF_PROGRAM_HEADER;


typedef struct ELF_SECTION_HEADER_STRUCT
{
    unsigned long   elf_section_header_name;
    unsigned long   elf_section_header_type;
    unsigned long   elf_section_header_flags;
    unsigned long   elf_section_header_address;
    unsigned long   elf_section_header_offset;
    unsigned long   elf_section_header_size;
    unsigned long   elf_section_header_link;
    unsigned long   elf_section_header_info;
    unsigned long   elf_section_header_alignment;
    unsigned long   elf_section_header_entry_size;
} ELF_SECTION_HEADER;


typedef struct ELF_SYMBOL_TABLE_ENTRY_STRUCT
{
    unsigned long   elf_symbol_table_entry_name;
    unsigned long   elf_symbol_table_entry_address;
    unsigned long   elf_symbol_table_entry_size;
    unsigned char   elf_symbol_table_entry_info;
    unsigned char   elf_symbol_table_entry_other;
    unsigned short  elf_symbol_table_entry_shndx;

} ELF_SYMBOL_TABLE_ENTRY;


typedef struct CODE_SECTION_ENTRY_STRUCT
{
    unsigned long   code_section_index;
    unsigned long   code_section_address;
    unsigned long   code_section_size;
} CODE_SECTION_ENTRY;


/* Define global variables.  */

ELF_HEADER              header;
ELF_PROGRAM_HEADER      *program_header;
ELF_SECTION_HEADER      *section_header;
unsigned char           *section_string_table;
unsigned char           *main_string_table;
unsigned long           total_symbols;
ELF_SYMBOL_TABLE_ENTRY  *symbol_table;
unsigned long           total_functions;
ELF_SYMBOL_TABLE_ENTRY  *function_table;
CODE_SECTION_ENTRY		*code_section_array;


/* Define helper functions.  */

int elf_object_read(unsigned long offset, void *object_address, int object_size)
{

int             i;
int             alpha;
unsigned char   *buffer;

    /* Setup the buffer pointer.  */
    buffer =  (unsigned char *) object_address;

    /* Seek to the proper position in the file.  */
    fseek(source_file, offset, SEEK_SET);

    /* Read the ELF object.  */
    for (i = 0; i < object_size; i++)
    {
        alpha =  fgetc(source_file);
        
        if (alpha == EOF)
            return(1);
            
        buffer[i] =  (unsigned char) alpha;
    }    

    /* Return success.  */
    return(0);
}


int main(int argc, char* argv[])
{

unsigned long           i, j, k;
unsigned long           current_total;
unsigned long           address;
unsigned long           size;
unsigned long           column;
unsigned char           *code_buffer;
unsigned long			code_section_index;
CODE_SECTION_ENTRY		code_section_temp;


    /* Determine if the proper number of files are provided.  */
    if (argc != 3) 
    {

        /* Print an error message out and wait for user key hit.  */
		printf("module_to_c_array.exe - Copyright (c) Microsoft Corporation v5.8\n");
        printf("**** Error: invalid input parameter for module_to_c_array.exe **** \n");
        printf("     Command Line Should be:\n\n");
        printf("     > module_to_c_array source_elf_file c_array_file <cr> \n\n");
        return(1);
    }

    /* Attempt to open the source file for reading.  */
    source_file =  fopen(argv[1], "rb");

    /* Determine if the source file was opened properly.  */
    if (source_file == NULL)
    {

        /* Print an error message out and wait for user key hit.  */
        printf("**** Error: open failed on source elf file **** \n");
        printf("            File: %s   ", argv[1]);
        return(2);
    }
    
    /* Attempt to open the dump file for writing.  */
    array_file =  fopen(argv[2], "w");

    /* Determine if the dump file was opened properly.  */
    if (array_file == NULL)
    {

        /* Print an error message out and wait for user key hit.  */
        printf("**** Error: open failed on C array file **** \n");
        printf("            File: %s   ", argv[2]);
        return(3);
    }

    /* Read the ELF header.  */
    elf_object_read(0, &header, sizeof(header));

    fprintf(array_file, "/**************************** Module-to-C-array Utility *****************************************/\n");
	fprintf(array_file, "/*                                                                                              */\n");
	fprintf(array_file, "/* Copyright (c) Microsoft Corporation                      Version 5.8, build date: 03-01-2018 */\n");
	fprintf(array_file, "/*                                                                                              */\n");
	fprintf(array_file, "/************************************************************************************************/\n\n");
    fprintf(array_file, "/* \n");
	fprintf(array_file, "   Input ELF file:      %30s\n", argv[1]);
	fprintf(array_file, "   Output C Array file: %30s\n", argv[2]);
	fprintf(array_file, "*/\n\n");

    /* Allocate memory for the program header(s).  */
    program_header =  malloc(sizeof(ELF_PROGRAM_HEADER)*header.elf_header_program_header_entries);
	
    /* Read the program header(s).  */
    elf_object_read(header.elf_header_program_header_offset, program_header, (sizeof(ELF_PROGRAM_HEADER)*header.elf_header_program_header_entries));

    /* Allocate memory for the section header(s).  */
    section_header =  malloc(sizeof(ELF_SECTION_HEADER)*header.elf_header_section_header_entries);

    /* Read the section header(s).  */
    elf_object_read(header.elf_header_section_header_offset, section_header, (sizeof(ELF_SECTION_HEADER)*header.elf_header_section_header_entries));


    /* Alocate memory for the section string table.  */
    section_string_table =  malloc(section_header[header.elf_header_section_string_index].elf_section_header_size);   

    /* Read the section string table.  */
    elf_object_read(section_header[header.elf_header_section_string_index].elf_section_header_offset, section_string_table, section_header[header.elf_header_section_string_index].elf_section_header_size);

    /* Allocate memory for the code section array.  */
    code_section_array =  malloc(sizeof(CODE_SECTION_ENTRY)*header.elf_header_section_header_entries);
	code_section_index =  0;

    /* Print out the section header(s).  */
    for (i = 0; i < header.elf_header_section_header_entries; i++)
    {

		/* Determine if this section is a code section and there is a size.  */
		if ((section_header[i].elf_section_header_type == 1) && (section_header[i].elf_section_header_size))
		{

            /* Check for an-instruction area.  */
            if ((section_header[i].elf_section_header_flags & 0x4) || (section_header[i].elf_section_header_flags & 0x2))
			{
                /* Determine if this new section overlaps with an existing section.  */
				for (j = 0; j < code_section_index; j++)
				{
					/* Is there an overlap?  */
					if ((section_header[i].elf_section_header_address >= code_section_array[j].code_section_address) &&
						((section_header[i].elf_section_header_address+section_header[i].elf_section_header_size + section_header[i].elf_section_header_offset) < (code_section_array[j].code_section_address+code_section_array[j].code_section_size)))
					{
						/* New section is within a current section, just disregard it.  */
						break;
					}
				}

				/* Determine if we have an overlap.  */
				if (j == code_section_index)
				{

				    /* Yes, we have a code section... save it!  */
				    code_section_array[code_section_index].code_section_index =  i;
				    code_section_array[code_section_index].code_section_address =  section_header[i].elf_section_header_address;
				    code_section_array[code_section_index].code_section_size =     section_header[i].elf_section_header_size;

				    /* Move to next code section.  */
				    code_section_index++;
				}
			}
		}
	}

	/* Check for no code sections.  */
	if (code_section_index == 0)
	{

        /* Print an error message out.  */
        printf("**** Error: No code sections found! **** \n");

		fprintf(array_file, "unsigned char  module_code[] = {0x00};\n\n");
		
		/* Close files.  */
		fclose(source_file);
		fclose(array_file);

		return(4);
 	}

	/* One or more code sections have been found... let's put them in the correct order by address.  */
	i = 0;
	while (i+1 < code_section_index)
	{

		/* Make the "ith" entry the lowest address.  */
		j = i + 1;
		do
		{
			/* Is there a new lowest address?  */
			if (code_section_array[j].code_section_address < code_section_array[i].code_section_address)
			{
				/* Yes, swap them!  */
				code_section_temp =  code_section_array[i];
				code_section_array[i] =  code_section_array[j];
				code_section_array[j] = code_section_temp;
			}

			/* Move the inner index.  */
			j++;
		} while (j < code_section_index);

		/* Move top index.  */
		i++;
	}

	/* Now print out the sections in a C array.  */
	fprintf(array_file, "unsigned char  module_code[] = {\n\n");
	fprintf(array_file, "/* Address                                            Contents                                        */\n\n");

	address =  code_section_array[0].code_section_address;
	column =   0;

	for (i = 0; i < code_section_index; i++)
	{

		/* Determine if there is any fill characters between sections.  */
		while (address < code_section_array[i].code_section_address)
		{

			/* Print out a character with a leading comma, except on the first character.  */
			if (column == 0)
				fprintf(array_file, "/* 0x%08X */   0x00", address);
			else
  				fprintf(array_file, ", 0x00");

			/* Move column forward.  */
			column++;

			/* Are we at the end of the column?  */
			if (column >= 16)
			{
				fprintf(array_file, ",\n");
				column =  0;
			}

			/* Move address forward.  */
			address++;
		}

		/* Now allocate memory for the code section.  */
		code_buffer =  malloc(code_section_array[i].code_section_size);

		/* Read in the code area.  */
		j =  code_section_array[i].code_section_index;
		elf_object_read(section_header[j].elf_section_header_offset, code_buffer, code_section_array[i].code_section_size);

		/* Write out the contents of this program area.  */
		size =  code_section_array[i].code_section_size;
		
		j =  0;
		k =  0;
		while (size)
		{

			/* Print out a character with a leading comma, except on the first character.  */
			if (column == 0)
				fprintf(array_file, "/* 0x%08X */   0x%02X", address, (unsigned int) code_buffer[j]);
			else
  				fprintf(array_file, ", 0x%02X", (unsigned int) code_buffer[j]);

			/* Move column forward.  */
			column++;

			/* Are we at the end of the column?  */
			if (column >= 16)
			{

				/* Is this the last byte of the image?  */
				if ((size != 1) || (i+1 != code_section_index))
				{
					if (k == 0)
					{
						k = code_section_array[i].code_section_index;
					    fprintf(array_file, ",      /* SECTION: %s  */\n", &section_string_table[section_header[k].elf_section_header_name]);
					}
					else
  					    fprintf(array_file, ",\n");
				}
				column =  0;
			}

			/* Move address forward.  */
			address++;
			
			/* Decrement size.  */
			size--;

			/* Move index into buffer.  */
			j++;
		}
	}

	/* Finally, finish the C array containing the module code.  */
	fprintf(array_file, "};\n\n");

	/* Close files.  */
	fclose(source_file);
    fclose(array_file);

	return 0;
}
