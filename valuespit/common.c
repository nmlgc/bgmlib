/* Value Spitter
   -------------
   Common functions
   -------------
*/

#include <stdio.h>

// Nicely format the string
void strfmt(char* str)
{
	short Upper;
	int Len, c;

	Upper = 0;
	Len = strlen(str);

	for(c = 0; c < Len; c++)
	{
		// Replace 'ß' with space, and spaces with zeroes
		if(str[c] == 'ß')
		{
			Upper = 1;
			str[c] = ' ';
		}
		else if(str[c] == '\n')	Upper = 0;
		else if(str[c] == ' ')	str[c] = '0';
		else if(Upper && str[c] != 'x')		str[c] = toupper(str[c]);
	}
}
