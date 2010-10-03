/* Value Spitter (TH06/Kioh Gyoku version)
   -------------
   Spits out nicely formatted BGM position data from a bunch of th06_??.pos files.
   -------------
*/

#include <stdio.h>

#ifdef KIOH_GYOKU
#define POS_MASK "kog_%2d"
#define START_FILE 0
#else
#define POS_MASK "th06_%2d.pos"
#define START_FILE 1
#endif

void strfmt(char* str);

int main(int argc, char** argv)
{
	FILE *In, *Out;

	short FileID = START_FILE;
	char Name[16], Fmt[128];
	unsigned long Loop, End;

	sprintf(Name, POS_MASK, FileID++);
	strfmt(Name);
	In = fopen(Name, "rb");
	if(!In)
	{
		printf("(Because I'm lazy,) This program requires the th06_**.pos files directly extracted from th06(e)_MD.dat in the same directory.");
		return 0;		
	}

	if(argc < 2)	Out = fopen("values.txt", "wt");
	else			Out = fopen(argv[2], "wt");
	if(!Out)
	{
		printf("Error opening output file.\n");
		return 0;		
	}

	while(In)
	{
		fread(&Loop, 4, 1, In);	Loop *= 4;
		fread(&End, 4, 1, In);	End *= 4;

		strcpy(&Name[8], "wav");
		sprintf(Fmt, "filenameß=ß\"%s\"\nrel_loopß=ß0x%8x\nrel_endß=ß0x%8x\n\n", Name, Loop, End);

		strfmt(Fmt);

		fputs(Fmt, Out);

		fclose(In);
		sprintf(Name, POS_MASK, FileID++);
		strfmt(Name);
		In = fopen(Name, "rb");
	}

	fclose(Out);
	return 0;
}
