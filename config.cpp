// Legacy Engine
// -------------
// config.cpp - Parsing structures for simple cross-platform reading and writing of configuration files
// -------------
// "©" Nmlgc, 2008-2011

#include "platform.h"

#include <FXSystem.h>
#include <FXFile.h>
#include "list.h"
#include "config.h"

#ifdef WIN32
#include <ctype.h>
#endif

// Utils
// -----

// Returns [Str]'s number base and removes 0x if necessary
FXint BaseCheck(FXString* Str)
{
	FXint Base = 10;
	if(Str->left(2) == "0x")
	{
		Str->erase(0, 2);
		Base = 16;
	}
	return Base;
}

// Transforms a string to lowercase
static bool LowerString(char* Str)
{
	if(!Str) return false;

	bool Lock = false;

	size_t Len = strlen(Str) + 1;
	for(size_t c = 0; c < Len; c++)
	{
		if(Str[c] == '"')	Lock = !Lock;

		// This function isn't used for any other purpose, so we can safely do this...
		if(Str[c] == '=' || Str[c] == '#')
		{
			return true;
		}

		Str[c] = tolower(Str[c]);
	}
	return true;
}
// -----

// ConfigKey
// ---------
ConfigKey::ConfigKey()
{
	Data = NULL;
	DataType = 0;
	Saved = false;
}

ConfigKey::ConfigKey(FXString& _Key_, const ushort& _DataType_, void* _Data_)
{
	SetInfo(_Key_, _DataType_, _Data_);
}

bool ConfigKey::Link(const ushort& _DataType_, void* _Data_, bool Get)
{
	Data = _Data_;
	DataType = _DataType_;
	if(Get)	return GetData(DataType, Data);
	else	return true;
}

void ConfigKey::SetInfo(FXString& _Key_, const ushort& _DataType_, void* _Data_)
{
	if(!_Key_ || !_Data_)	return;

	Key = _Key_.lower();
	Data = _Data_;
	DataType = _DataType_;
}

bool ConfigKey::GetData(const ushort& pd, void* p, FXString* NewLine)
{
	if(!p || pd == 0)	return false;

	FXString* L = NewLine ? NewLine : &Line;

	switch(pd)
	{
	case TYPE_BOOL:	
		if(*L == "true")		*(bool*)p = true;
		else if(*L == "false")	*(bool*)p = false;
		else                    *(bool*)p = atoi(L->text()) != 0;
		break;

	case TYPE_SHORT:	*(short*)p = (short)L->toInt(BaseCheck(L));     break;
	case TYPE_USHORT:	*(ushort*)p = (ushort)L->toUInt(BaseCheck(L));  break;
	case TYPE_INT:		*(int*)p = L->toInt(BaseCheck(L));              break;
	case TYPE_UINT:		*(uint*)p = L->toUInt(BaseCheck(L));            break;
	case TYPE_LONG:		*(long*)p = L->toLong(BaseCheck(L));            break;
	case TYPE_ULONG:	*(ulong*)p = L->toULong(BaseCheck(L));          break;
	case TYPE_FLOAT:	*(float*)p = (float)L->toFloat();				break;
	case TYPE_DOUBLE:	*(double*)p = (double)L->toDouble();            break;
	case TYPE_UCHAR:	*(uchar*)p = (uchar)L->toUInt(BaseCheck(L));    break;
	case TYPE_STRING:
		FXString* s = (FXString*)p;
		L->substitute("\\n", 2, &LineBreak[1], 1, true);	// Translate line breaks
		L->substitute("\\\"", 2, "\"", 1, true);
		// Remove quotation marks
		FXint len = L->length() - 1;
		if(L->at(0) == '"')
		{
			len = L->rfind('"');
			s->assign(L->text() + 1, len - 1);
		}
		else s->assign(*L);
		break;
	}
	return true;
}

bool ConfigKey::SaveData(FXString* FileLine)
{
	if(!Data || DataType == 0 || !FileLine)	return false;

	switch(DataType)
	{
	case TYPE_BOOL:
		if(*(bool*)Data)  FileLine->format("%s = true", Key);
		else		  FileLine->format("%s = false", Key);
		break;

	case TYPE_SHORT:	FileLine->format("%s = %d", Key, *(short*)Data);   break;
	case TYPE_USHORT:	FileLine->format("%s = %d", Key, *(ushort*)Data);  break;
	case TYPE_INT:		FileLine->format("%s = %d", Key, *(int*)Data);     break;
	case TYPE_UINT:		FileLine->format("%s = %d", Key, *(uint*)Data);    break;
	case TYPE_LONG:		FileLine->format("%s = %ld",Key, *(long*)Data);    break;
	case TYPE_ULONG:	FileLine->format("%s = %lu",Key, *(ulong*)Data);   break;
	case TYPE_FLOAT:	FileLine->format("%s = %f", Key, *(float*)Data);   break;
	case TYPE_DOUBLE:	FileLine->format("%s = %f", Key, *(double*)Data);  break;
	case TYPE_UCHAR:	FileLine->format("%s = %d", Key, *(uchar*)Data);   break;
	case TYPE_STRING:
		*FileLine = *(FXString*)Data;
		if(FileLine->empty())	break;

		FileLine->substitute("\"", 1, "\\\"", 2, true);	// Enquote
		FileLine->append('\"');
		FileLine->substitute(&LineBreak[1], 1, "\\n", 2, true);	// Translate line breaks
		FileLine->prepend(Key + " = \"");
		break;
	}
	return Saved = true;
}
// ConfigParser
// ------------
ConfigParser::ConfigParser()
{
}

void ConfigParser::SetCaption(FXString& New)
{
	Caption = New.lower();
}

#define ADD_KEY_IMP(tn, tc)	ConfigKey* ConfigParser::AddKey(FXString& Key, tn* Data)   {ConfigKey NewKey; NewKey.SetInfo(Key, tc, (void*)Data); return &Keys.Add(&NewKey)->Data;}

ADD_KEY_IMP(bool, TYPE_BOOL)
ADD_KEY_IMP(short, TYPE_SHORT)
ADD_KEY_IMP(ushort, TYPE_USHORT)
ADD_KEY_IMP(int, TYPE_INT)
ADD_KEY_IMP(uint, TYPE_UINT)
ADD_KEY_IMP(long, TYPE_LONG)
ADD_KEY_IMP(ulong, TYPE_ULONG)
ADD_KEY_IMP(float, TYPE_FLOAT)
ADD_KEY_IMP(double, TYPE_DOUBLE)
ADD_KEY_IMP(FXString, TYPE_STRING)

ConfigKey* ConfigParser::CreateKey(const FXString& Name)
{
	ConfigKey* New = &(Keys.Add()->Data);
	New->Key = Name;
	return New;
}

ConfigKey* ConfigParser::FindKey(const FXString& Name)
{
	ListEntry<ConfigKey>* CurKey = Keys.First();

	while(CurKey)
	{
		if(CurKey->Data.Key == Name)	return &CurKey->Data;
		CurKey = CurKey->Next();
	}
	return NULL;
}

bool ConfigParser::GetValue(const FXString& KeyStr, const ushort& DataType, void* Value)
{
	ConfigKey* Key = FindKey(KeyStr);
	if(!Key)	return false;
	else		return Key->GetData(DataType, Value);
}

bool ConfigParser::LinkValue(const FXString& KeyStr, const ushort& DataType, void* Value, bool GetData)
{
	ConfigKey* Key = FindKey(KeyStr);
	if(!Key)	Key = CreateKey(KeyStr);
	return Key->Link(DataType, Value, GetData);
}

void ConfigParser::Clear()
{
	Keys.Clear();
}

ConfigParser::~ConfigParser()
{
	Clear();
}
// ------------

// ConfigFile
// ----------
void ConfigFile::SetFN(const FXString& FN)
{
	ConfigFN.clear();
	ConfigFN = FN;
}

bool ConfigFile::BufferFile()
{
	char bom[3];
	long Len = 0;
	FXlong Size;

	FXFile Config;
	if(!Config.open(ConfigFN, FXIO::Reading)) return false;//Log.Error("Config file not found!");

	// Skip byte order mark, if necessary
	Config.readBlock(bom, 3);
	if(memcmp(bom, utf8bom, 3))	Config.position(0);

	Size = Config.size() - Config.position();
	
	char* Buf = (char*)malloc(Size + 1);
	Config.readBlock(Buf, Size);
	Buf[Size] = LineBreak[1];
	Size += 1;
	Config.close();

	char* Line = Buf;
	for(long c = 0; c < Size; c++)
	{
		if(Line[c] == LineBreak[0])
		{
			Line[c] = '\0';
			Len = ++c;
		}
		if(Line[c] == LineBreak[1])
		{
			Line[c] = '\0';
			Len = c + 1;
		}

		if(Len > 0)
		{
			if(Line[0] == '\0')	FileBuffer.Add(Line, 0);
			else if(Line[0] != '#')
			{
				LowerString(Line);
				FileBuffer.Add(Line, Len);
			}
			else	FileBuffer.Add(Line, Len);
			Line += Len;
			Size -= Len;
			Len = c = -1;
		}
	}
	free(Buf);
	Config.close();
	return true;
}

// (Old version, new one is a bit faster)
/* bool ConfigFile::BufferFile()
{
	char CurLine[2048];

	while( (Len = ReadLineFromFile(CurLine, 2048 * sizeof(char), Config)) != -1)
	{
		if(CurLine[0] == '\0')	FileBuffer.Add(CurLine, 0);
		else if(CurLine[0] != '#')
		{
			LowerString(CurLine);
			FileBuffer.Add(CurLine, Len);
		}
		else	FileBuffer.Add(CurLine, Len);
		CurLine[0] = '\0';
	}
}*/

PListEntry<char>* ConfigFile::InsertLine(PListEntry<char>* PrevLine, char* NewLine)
{
	if(PrevLine)
	{
		PrevLine = FileBuffer.Insert(PrevLine, NewLine, 256);
	}
	else	FileBuffer.Add(NewLine, strlen(NewLine + 1));
	return PrevLine;
}

ConfigParser* ConfigFile::FindSection(const FXString& Name)
{
	ListEntry<ConfigParser>* Cur = Sect.First();

	while(Cur)
	{
		if(Cur->Data.Caption == Name)	return &Cur->Data;
		Cur = Cur->Next();
	}

	return NULL;
}

ConfigParser* ConfigFile::CheckSection(const FXString& Line)
{
	ConfigParser* New = FindSection(Line);
	if(!New)
	{
		// Create new section
		New = &(Sect.Add()->Data);
		New->Caption.assign(Line);

		FXString Name;
		Name.format("[%s]", Line);

		New->LastLine = FileBuffer.Add(Name.text(), Name.length() + 1);
	}
	return New;
}

bool ConfigFile::Load(const FXString& FN)
{
	SetFN(FN);
	if(!BufferFile())	return false;
	Load();
	return true;
}

bool ConfigFile::Load()
{
// LARGE_INTEGER Time[2], Diff[3], TimeTotal, SubDiff[3];

	char*         CurLine = NULL;
	ConfigParser* CurSection = NULL;

	FXString Key;
	FXString Value;

	ConfigKey*	TrgKey;

	int CurLineNum = -1;
	LineLink* NewLL;

	PListEntry<char>* BufferLine = FileBuffer.First();
	if(!BufferLine)	return false;

	do
	{
	// QueryPerformanceCounter(&Time[0]);

		CurLineNum++;
		CurLine = BufferLine->Data;
		if(!CurLine)	continue;

		// Comments
		     if(CurLine[0] == '#')	 continue;
		else if(CurLine[0] == '[')	// Section Name
		{
			Key.assign(CurLine + 1);
			Key.tail() = '\0';
			CurSection = FindSection(Key);
			if(!CurSection)
			{
				// Create new section
				CurSection = &(Sect.Add()->Data);
				CurSection->Caption.assign(Key);
			}
			continue;
		}

	// QueryPerformanceCounter(&Diff[0]);

		if(!CurSection)	continue;

		// Scanning...
		uint c;
		for(c = 0; c < BufferLine->Size; c++)
		{
			if(CurLine[c] == '=')
			{
				Key.assign(CurLine, c);	Key.trim();
				Value.assign(CurLine + c + 1);	Value.trim();
				break;
			}
		}

	// QueryPerformanceCounter(&Diff[1]);

		if(c < BufferLine->Size)
		{
			TrgKey = CurSection->FindKey(Key);

		// QueryPerformanceCounter(&SubDiff[0]);
					
			if(!TrgKey)
			{
				TrgKey = CurSection->CreateKey(Key);
				TrgKey->Line = Value;
			}
			else TrgKey->GetData(TrgKey->DataType, TrgKey->Data);

		// QueryPerformanceCounter(&SubDiff[1]);

			NewLL = &(Link.Add()->Data);
			NewLL->Key = TrgKey;
			NewLL->Line = CurLineNum;
			CurSection->LastLine = BufferLine;

		// QueryPerformanceCounter(&SubDiff[2]);
		}

	/*QueryPerformanceCounter(&Diff[2]);
	
	QueryPerformanceCounter(&Time[1]);

	SubDiff[2] = SubDiff[2] - SubDiff[1];
	SubDiff[1] = SubDiff[1] - SubDiff[0];
	SubDiff[0] = SubDiff[0] - Diff[1];

	Diff[2] = Diff[2] - Diff[1];
	Diff[1] = Diff[1] - Diff[0];
	Diff[0] = Diff[0] - Time[0];

	TimeTotal = Time[1] - Time[0];*/
	}
	while(BufferLine = BufferLine->Next());
	return true;
}

// Why wasn't I implementing something like this in the first place?
// Simple, fast and bullshit-free saving at the cost of 16 bytes per key.

bool ConfigFile::Save()
{
	PListEntry<char>* BufferLine = FileBuffer.First();	if(!BufferLine)	return false;
	ListEntry<LineLink>* CurLL = Link.First();

	char* CurLine = NULL;
	LineLink* LL;
	ConfigKey* Key;
	int CurLineNum = 0;

	bool Changed = false;

	FXString Save;

	while(BufferLine && ( BufferLine = BufferLine->Next()) && CurLL)
	{
		CurLineNum++;
		if(CurLL->Data.Line != CurLineNum)	continue;

		LL = &CurLL->Data;
		CurLL = CurLL->Next();

		CurLine = BufferLine->Data;
		if(!CurLine)	continue;

		if(LL->Key->SaveData(&Save))
		{
			if(Save.empty())
			{
				BufferLine = FileBuffer.Delete(BufferLine);
				CurLineNum++;
				Changed = true;
			}
			else if(Save != BufferLine->Data)
			{
				BufferLine->SetData(Save.text(), Save.length() + 1);
				Changed = true;
			}
		}
	}

	// Check for keys not present in the file yet
	ListEntry<ConfigKey>* CurKey;
	ListEntry<ConfigParser>* CurSect = Sect.First();
	while(CurSect)
	{
		CurKey = CurSect->Data.Keys.First();
		while(CurKey)
		{
			Key = &CurKey->Data;
			if(!Key->Saved && Key->SaveData(&Save) && !Save.empty())
			{
				FileBuffer.Insert(CurSect->Data.LastLine, Save.text(), Save.length() + 1);
				Changed = true;
			}

			CurKey = CurKey->Next();
		}
		CurSect = CurSect->Next();
	}

	if(Changed)	return WriteBuffer();	// No changes, no save.
	return Changed;
}

ConfigKey* ConfigFile::FindKey(const FXString& SectStr, const FXString& KeyStr)
{
	ConfigParser* Section = FindSection(SectStr);
	if(!Section)	return NULL;

	return Section->FindKey(KeyStr);
}

bool ConfigFile::GetValue(const FXString& SectStr, const FXString& KeyStr, const ushort& DataType, void* Value)
{
	ConfigParser* Section = FindSection(SectStr);
	if(!Section)	return false;
	else			return Section->GetValue(KeyStr, DataType, Value);
	return true;
}

bool ConfigFile::LinkValue(const FXString& SectStr, const FXString& KeyStr, const ushort& DataType, void* Value, bool GetData)
{
	ConfigParser* Section = FindSection(SectStr);
	if(!Section)	return false;
	else			return Section->LinkValue(KeyStr, DataType, Value, GetData);
}

bool ConfigFile::WriteBuffer()
{
	static const FXString TmpFN = FXSystem::getTempDirectory() + SlashString + "legacy_tmp.cfg";

	PListEntry<char>* CurLine = FileBuffer.First();
	if(!CurLine) return false;

	FXFile Config;
	if(!Config.open(TmpFN, FXIO::Writing))	return false;

	// Always write UTF8
	Config.writeBlock(utf8bom, 3);

	// We have to do it this way, unless we want to append a line each time we save the file.
	if(CurLine->Data)	Config.writeBlock(CurLine->Data, strlen(CurLine->Data));
	CurLine = CurLine->Next();
	while(CurLine)
	{
		Config.writeBlock(LineBreak, sizeof(LineBreak));
		if(CurLine->Data)	Config.writeBlock(CurLine->Data, strlen(CurLine->Data));
		CurLine = CurLine->Next();
	}
	Config.close();
	
	bool Ret = FXFile::moveFiles(TmpFN, ConfigFN, true);
	if(!Ret)	FXFile::removeFiles(TmpFN);
	return Ret;
}

void ConfigFile::Clear()
{
	FileBuffer.Clear();
	Sect.Clear();
	ConfigFN.clear();
}

ConfigFile::ConfigFile()
{
}

ConfigFile::ConfigFile(const FXString& FN)
{
	SetFN(FN);
	BufferFile();
}

ConfigFile::~ConfigFile()
{
	Clear();
}
// ----------
