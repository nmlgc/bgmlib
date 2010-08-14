/* Value Spitter (Tasofro games)
   -------------
   Spits out nicely formatted BGM position data from a thbgm.fmt file.
   -------------
*/

#include <stdio.h>
#include <stdlib.h>

void strfmt(char* str);

const unsigned short EntrySize = 0x6C;

int main(int argc, char** argv)
{
	FILE *In = NULL, *Out = NULL;
	char Name[16], Fmt[64];
	char* hdr;
	unsigned long Start, Loop, End;

	unsigned short Files, hdrSize;
	unsigned short k, t, c;

	if(argc >= 2)	In = fopen(argv[1], "rb");
	if(!In)
	{
		printf("Error opening input file.\nPlease specify a Tasofro game BGM file.");
		return 0;
	}

	if(argc < 3)	Out = fopen("values.txt", "wt");
	else		Out = fopen(argv[2], "wt");
	if(!Out)
	{
		printf("Error opening output file.\n");
		return 0;		
	}

	fread(&Files, 2, 1, In);
	hdrSize = Files * EntrySize;

	hdr = malloc(hdrSize);

	fread(hdr, hdrSize, 1, In);
	fclose(In);

	// Decrypt...
	k = 0x64, t = 0x64;
	for(c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += t; t += 0x4D;
	}

	fwrite(hdr, hdrSize, 1, Out);
	fclose(Out);

	free(hdr);
	hdr = NULL;

	return 0;

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

	fclose(Out);
	return 0;
}
