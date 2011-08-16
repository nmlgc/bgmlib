/* Value Spitter (TH07 and above)
   -------------
   Spits out nicely formatted BGM position data from a thbgm.fmt file.
   -------------
*/

#include <stdio.h>

void strfmt(char* str);

int main(int argc, char** argv)
{
	FILE *In, *Out;
	char Name[16], Fmt[256];
	unsigned long Start, Loop, End, Freq;
	int c = 1;

	if(argc < 2)
	{
		In = fopen("thbgm.fmt", "rb");
		if(!In)	In = fopen("thbgm_tr.fmt", "rb");
	}
	else		In = fopen(argv[1], "rb");
	if(!In)
	{
		printf("Error opening input file.\n");
		return 0;
	}

	if(argc < 3)	Out = fopen("values.txt", "wt");
	else		Out = fopen(argv[2], "wt");
	if(!Out)
	{
		printf("Error opening output file.\n");
		return 0;		
	}

	while(!feof(In))
	{
		fread(Name, 16, 1, In);
		if(Name[0] == '\0')	break;

		fread(&Start, 4, 1, In);
		fseek(In, 4, SEEK_CUR);
		fread(&Loop, 4, 1, In);
		fread(&End, 4, 1, In);
		fseek(In, 4, SEEK_CUR);
		fread(&Freq, 4, 1, In);
		fread(Fmt, 12, 1, In);

		sprintf(Fmt, "[%02d]\n"
				"name_jp = \"\"\n"
				"filename = \"%s\"\n"
				"position = \"%#08x, %#08x, %#08x\"\n"
				"frequency = %d\n\n", c, Name, Start, Loop, End, Freq);

		fputs(Fmt, Out);
		c++;
	}
	fclose(In);
	fclose(Out);
	return 0;
}
