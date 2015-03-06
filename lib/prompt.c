
#include "prompt.h"

#define MAX_LINE_SIZE 256
#define MAX_FIELD_SIZE 20
char * _buffer;

void prompt_init()
{
	_buffer = malloc( MAX_LINE_SIZE );
}

void prompt_read_command(struct prompt_t * prompt)
{
	char * ptr;
	size_t len;
	char c;

	ptr = _buffer;
	prompt->command = NULL;
	prompt->parameters = NULL;
	
	printf("orden?# ");

	if( scanf("%s", ptr) == 1)
	{
		len = strlen(ptr);
		prompt->command = ptr;
		ptr[len] = '\0';
		
		while( (c = getc(stdin)) == ' ' && c != EOF);

		if(c != EOF && c != '\n')
		{
			ptr = ptr + len + 1;
			ptr[0] = c;

			if( fgets(ptr + 1, MAX_LINE_SIZE - len, stdin) != NULL )
			{
				len = strlen(ptr);
				ptr[len - 1] = '\0';
				prompt->parameters = ptr;
			}
		}
	}

}

void prompt_print(char *title, char *data)
{
	int i = 0, x = 0,lenline = PROMPT_MAX_LINE_SIZE;
	char c;

	

	if (title != NULL)
	{
		printf("\n %s\n", title);
	}

	printf("->\n");
	if (data != NULL)
	{
		lenline = PROMPT_MAX_LINE_SIZE;
		putchar(' ');
		while( (c = data[i++]) != '\0' )
		{
			if (c == '\n' || --lenline == 0)
			{
				putchar('\n');
				putchar(' ');
				lenline = PROMPT_MAX_LINE_SIZE;
				x = 0;
				if (c != '\n') putchar(c);
			}
			else if( c == '|')
			{
				while(x++ < MAX_FIELD_SIZE) putchar(' ');
				putchar(c);
				putchar(' ');
				x = 0;
			}
			else
			{
				putchar(c);
				x++;
			}
				
		}
		putchar('\n');
	}
	putchar('\n');	
}

void prompt_destroy(struct prompt_t * prompt)
{
	free(_buffer);
}