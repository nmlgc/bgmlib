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
	short Upper;
	char Name[16], Fmt[64];
	unsigned long Start, Loop, End;
	int Len, c;

	if(argc < 2)	In = fopen("thbgm.fmt", "rb");
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
		if(Name[0] == '\0')	return;

		fread(&Start, 4, 1, In);
		fseek(In, 4, SEEK_CUR);
		fread(&Loop, 4, 1, In);
		fread(&End, 4, 1, In);
		fread(Fmt, 20, 1, In);

		sprintf(Fmt, "%s:ß0x%8x,ß0x%8x,ß0x%8x\n", Name, Start, Loop, End);
		strfmt(Fmt);

		fputs(Fmt, Out);
	}
	fclose(In);
	fclose(Out);
	return 0;
}
