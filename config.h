// Legacy Engine
// -------------
// config.h - Parsing structures for simple cross-platform reading and writing of configuration files
// -------------
// "©" Nmlgc, 2008-2011

#pragma once

#ifndef LEGACY_CONFIG_H
#define LEGACY_CONFIG_H

#define TYPE_BOOL	0x1
#define TYPE_SHORT	0x2
#define TYPE_USHORT	0x3
#define TYPE_INT	0x4
#define TYPE_UINT	0x5
#define TYPE_LONG	0x6
#define TYPE_ULONG	0x7
#define TYPE_FLOAT	0x8
#define TYPE_UCHAR	0x9
#define TYPE_STRING	0xA

struct ConfigKey
{
	friend class ConfigParser;
	friend class ConfigFile;

protected:
	FXString Key;
	ushort	DataType;
	void*	Data;
	bool	Saved;

	FXString Line;	// Storage for the value read from the config file

	bool	SaveData(FXString* SaveLine);

public:
	ConfigKey();
	ConfigKey(FXString& Key, const ushort& DataType, void* Data);

	bool	Link(const ushort& DataType, void* Data, bool GetData = true);	// Links this key to the [DataType] variable [Data], and updates [Data] if [GetData] is true. On ConfigFile::Save, the value of [Data] gets automatically written
	void	SetInfo(FXString& Key, const ushort& DataType, void* Data);

	bool	GetData(const ushort& DataType, void* Data, FXString* Line = NULL);	// Parses the value of [Line] and writes it to the [DataType] variable [Data]
};

template class List<ConfigKey>;

#define ADD_KEY(Type)	ConfigKey* AddKey(FXString& Key, Type* Data)

class ConfigParser
{
	friend class ConfigFile;

protected:
	FXString	Caption;	// Section Name
	PListEntry<char>*	LastLine;

	List<ConfigKey>	Keys;

public:
	void	Save();
	void	Load();

	void	SetCaption(FXString& Caption);

	ADD_KEY(bool);
	ADD_KEY(short);
	ADD_KEY(ushort);
	ADD_KEY(int);
	ADD_KEY(uint);
	ADD_KEY(long);
	ADD_KEY(ulong);
	ADD_KEY(float);
	ADD_KEY(FXString);

	ConfigKey*	CreateKey(const FXString& Name);
	ConfigKey*	FindKey(const FXString& Name);

	bool	 GetValue(const FXString& Key, const ushort& DataType, void* Value);	// Stores the parsed value of [Key] in the [DataType] variable [Value]. Should only be used if [Value] is local or you're not going to save anyway
	bool	LinkValue(const FXString& Key, const ushort& DataType, void* Value, bool GetData = true);	// Links [Key] to the [DataType] variable [Value]. On ConfigFile::Save, the current value of [Value] gets automatically written

	void	Clear();

	ConfigParser();
	~ConfigParser();
};

struct LineLink
{
	int Line;
	ConfigKey* Key;
};

class ConfigFile
{
	friend class ConfigParser;

protected:
	PList<char>	FileBuffer;
	List<ConfigParser> Sect;	// Automatically created sections
	List<LineLink> Link;	// Sequentially stored line-key links
	FXString	ConfigFN;

	void	SetFN(const FXString& FN);
	bool	BufferFile();
	bool	WriteBuffer();

	PListEntry<char>*	InsertLine(PListEntry<char>* PrevLine, char* NewLine);	// Inserts a new line into the buffer
	
public:
	bool	Load();
	bool	Load(const FXString& FN);

	FXString&	GetFN()	{return ConfigFN;}

	ConfigKey* FindKey(const FXString& Section, const FXString& Key);
	ConfigParser*	FindSection(const FXString& Name);
	ConfigParser*	CheckSection(const FXString& Name);	// Tries to find a section called [Name], and creates a new one if it doesn't exist

	bool	 GetValue(const FXString& Section, const FXString& Key, const ushort& DataType, void* Value);	// Stores the parsed value of [Key] in [Section] in the [DataType] variable [Value]. Should only be used if [Value] is local or you're not going to save anyway
	bool	LinkValue(const FXString& Section, const FXString& Key, const ushort& DataType, void* Value, bool GetData = true);	// Links [Key] in [Section] to the [DataType] variable [Value]. On ConfigFile::Save, the current value of [Value] gets automatically written

	bool	Save();

	void	Clear();

	ConfigFile();
	ConfigFile(const FXString& FN);
	~ConfigFile();
};

// Utils
// -----
FXint BaseCheck(FXString* Str);	// Returns [Str]'s number base and removes 0x if necessary
// -----

#endif /* LEGACY_CONFIG_H */