// Music Room Interface
// --------------------
// utils.cpp - Random utility functions
// --------------------
// "©" Nmlgc, 2011

#include "platform.h"

#include <FXString.h>
#include <FXFile.h>
#include <fxascii.h>
#include "utils.h"

// Performs memcpy and increases [src] by [size]
void* memcpy_advance(void* dest, char** src, size_t size)
{
	void* ret = memcpy(dest, *src, size);
	*src += size;
	return ret;
}

// Performs memcpy and increases [dest] by [size]
void* memcpy_advance(char** dest, const void* src, size_t size)
{
	void* ret = memcpy(*dest, src, size);
	*dest += size;
	return ret;
}

/*ulong SplitString(const char* String, const char Delimiter, PList<char>* Result)
{
	if(!Result || !String || !Delimiter)	return 0;

	char* TempString = new char[strlen(String) + 1];
	strcpy(TempString, String);
	char* CurToken = strtok(TempString, &Delimiter);

	while(CurToken)
	{
		Result->Add(CurToken, strlen(CurToken) + 1);
		CurToken = strtok(NULL, &Delimiter);
	}
	SAFE_DELETE_ARRAY(TempString);
	return Result->Size();
}*/

// Cross platform text-mode line reading function
/*int ReadLineFromFile(char* String, int MaxChars, FXFile& File)
{
	int c;
	FXival Pos = File.position();
	FXival Len = File.readBlock(String, MaxChars);

	if(Len == 0)	return -1;

	for(c = 0; c < Len; c++)
	{
		if(String[c] == LineBreak[0])
		{
			String[c] = '\0';
			Len = ++c;
		}
		if(String[c] == LineBreak[1])
		{
			String[c] = '\0';
			Len = c + 1;
			break;	// <- Difference!
		}
	}
	String[c] = '\0';
	File.position(Pos + Len);
	return Len;
}*/

bool WriteByteBlock(FXFile& File, const long& Count, const FXchar Byte)
{
	if(Count < 0)	return false;

	char* PAD = new char[Count];
	memset(PAD, Byte, Count);
	File.writeBlock(PAD, Count);
	SAFE_DELETE_ARRAY(PAD);
	return true;
}

// Removes all occurrences of a given substring
// No idea why there isn't a function like this already.
// --------------------------------------------
FXString& remove_sub(FXString& str, const FXchar* org, FXint olen)
{
  if(0<olen){
	  register FXint pos = 0;
    while(pos<=str.length()-olen){
      if(compare(str.text()+pos,org,olen)==0)
	  {
		  str.erase(pos,olen);
        continue;
		}
      pos++;
      }
    }
  return str;
  }

FXString& remove_sub(FXString& str, const FXchar& org)
{
  return remove_sub(str, &org, 1);
}

FXString& remove_sub(FXString& str, const FXString& org)
{
	return remove_sub(str, org.text(), org.length());
}
// --------------------------------------------

// Return the string value of "[ValName][Assign]" in [Source], terminated by [End]
FXString NamedValue(const FXString& Str, const FXString& ValName, const FXString& Assign, const FXString& End)
{
	FXint v = 0, a, e;
	FXString Ret;

	while(1)
	{
		if((v = Str.find(ValName, v)) == -1)					return "";	
		if((a = Str.find(Assign, v + ValName.length()))  == -1)	return "";
		// Check if that is _really_ only ValName, and nothing more!
		bool r = true;
		// Start
		if(v != 0)
		{
			if(Ascii::isAlphaNumeric(Str[v-1]) && Str[v-1] != End.tail())
			{
				v++;
				continue;
			}
		}
		v += ValName.length();
		// End
		for(FXint c = v; (c < a) && r; c++)
		{
			r = FX::Ascii::isSpace(Str[c]);
		}
		if(r)	break;
	}
	a += Assign.length();
	if((e = Str.find(End, a))     == -1)	return "";
	Ret = Str.mid(a, e-a);
	while(LineBreak[1] == Ret.head())	Ret.erase(0);
	Ret.trimEnd();
	return Ret;
}

// Random endianess value classes
// ------------------------------

// 16-bit
ushort EndianSwap(const ushort& x)
{
	return ((x & 0x00ff) << 8) |
		   ((x & 0xff00) >> 8);
}

ushort& u16::swap()
{
	d = EndianSwap(d);
	e = !e;
	return d;
}

// 32-bit
inline ulong EndianSwap(const ulong& x)
{
	return ((x & 0x000000ff) << 24) |
		   ((x & 0x0000ff00) << 8) |
		   ((x & 0x00ff0000) >> 8) |
		   ((x & 0xff000000) >> 24);
}

// Ha! Same code!
ulong& u32::swap()
{
	d = EndianSwap(d);
	e = !e;
	return d;
}
// ------------------------------
