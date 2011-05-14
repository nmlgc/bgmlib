// Music Room BGM Library
// ----------------------
// infostruct.h - GameInfo and TrackInfo structures
// ----------------------
// "©" Nmlgc, 2010-2011

#ifndef BGMLIB_INFOSTRUCT_H
#define BGMLIB_INFOSTRUCT_H

#include "list.h"

using namespace FX;

// Languages
// ---------
struct LangInfo
{
	FXString Display;	// Display String (e.g. "English")
	FXString GUILang;	// Display String in GUI language (= English for now)
	FXString Code2;	// 2-byte language code (e.g. "en")
	FXString Code3;	// 3-byte language code (e.g. "eng") (ISO 639-2)

	void Clear();
};

#define LANG_JP	0
#define LANG_EN 1
#define LANG_COUNT 2

struct IntString
{
	FXString s[LANG_COUNT];
	
	IntString()	{}
	IntString(const FXString& a, const FXString& b)	{s[0].assign(a); s[1].assign(b);}
	FXString& operator [] (const ushort& l)	{return s[l];}
};
// ---------

#define FMT_BYTE   0x0
#define FMT_SAMPLE 0x1

struct TrackInfo
{
	friend class PackMethod;
	friend struct GameInfo;

	void	PosFmtConvert(const bool& TrgFmt);

public:
	// Track Positions. Clear() guarantees those to be zero before anything is filled in.
	// Since those are in [PosFmt], do _not_ access them directly outside where pack method track data parsing, use GetPos() instead!
	ulong	Start[2];	// (with or without silence)
	ulong	Loop;
	ulong	End;
	bool	PosFmt;	// Format of the position data (bytes or samples)

	// Individual track filename, as used by the pack method.
	// Use GameInfo::DiskFN or PackMethod::DiskFN for the real one!
	FXString	NativeFN;

	ushort		Number; // Track Number (starting with 1!)
	IntString	Name; // Track name
	IntString	Comment; // Music room comment
	List<IntString>	Afterword;	// Supplementary comment
	ushort	CmpID;	// Composer ID of this track  - if the whole soundtrack was composed by one artist, the element of GameInfo is used instead

	ulong	FS;		// File size for archived tracks
	float	Freq;	// Individual track sampling rate

	void Clear();

	// Gets the requested position values in the requested format
	void GetPos(const bool& Fmt, const bool& SilRem, ulong* Start = NULL, ulong* Loop = NULL, ulong* End = NULL);
	ulong GetStart(const bool& Fmt, const bool& SilRem);	// By popular demand
	ulong GetStart();	// Returns digital start point
	bool& GetPosFmt()	{return PosFmt;}

	// Returns length of this track with [LoopCnt] loops, [FadeDur] second fades and enabled/disabled silence removal
	ulong GetByteLength(const bool& SilRem, const ushort& LoopCnt, const float& FadeDur);

	FXString LengthString(const ulong& ByteLen);	// Converts [ByteLen] into a nice minute:second format

	FXString GetComment(const ushort& Lang);
	FXString GetNumber();
	
	TrackInfo();
	~TrackInfo();
};

// Forward declarations
namespace FX
{
	class FXIcon;
}
class ConfigFile;

struct GameInfo
{
	bool	HaveTrackData;
	bool	Scanned;

	bool	SilenceScan;	// Silence scanning override flag

	FXIcon*	Icon;	// Optional game icon
	
	FXString	InfoFile;
	FXString	WikiPage;
	ulong		WikiRev;	// Wiki revision ID of the info file

	IntString	Name;	// Game name in both languages
	ushort	Year;
	PackMethod*	PM;
	FXString	GameNum;	// Game Number (e.g. "12.5")
	FXString	BGMFile;	// Main (not used with BGMDIR) Never refer this value, use DiskFN instead!
	FXString	BGMDir;	// BGM Subdirectory (only used with BGMDIR)
	IntString	Artist;	// Composer of the whole soundtrack - if there are multiple composers, the element of TrackInfo is used instead
	IntString	Circle;
	List<IntString> Composer;
	List<TrackInfo>	Track;
	ushort	TrackCount;	// Actual number of active tracks. Gets adjusted for trial versions.

	ushort	HeaderSize;	// Header size of each BGM file (only used with BGMDIR)
	char	ZWAVID[2];	// 0x8 and 0x9 in thbgm.dat for this game (only used with BGMDAT)
	uchar	CryptKind;	// BGM File Encryption kind. Set to a nonzero value to turn on encryption handling!
	ushort	EntrySize;	// Size of a junk-filled entry (only used with encryption version 1)

	bool	Vorbis;	// Is the BGM Vorbis-compressed? 
	FXuint	PatchClass;	// (only used by the Touhou Vorbis Compressor) Patch Class Hash

	FXString	Path;	// Contains the valid local path to this game. Saved in LGDFile.
	
	bool ParseGameData(const FXString& InfoFile);	// Reads necessary data to identify the game
	bool ParseTrackData();	// Reads all the rest, and then calls ParseTrackDataEx() for further processing (e.g. wiki updating)

	bool ParseTrackDataEx(ConfigFile& NewGame);

	// Name display
	FXString	DelimName(const ushort& Lang);	// returns <GNDelim[0]><Name[Lang]><GNDelim[1]> <PackMethod->DisplayName()> 
	FXString	FullName(const ushort& Lang);	// returns "<Name[Lang]>", followed by a trial version identifier, if applicable
	FXString	NumName(const ushort& Lang);	// returns "<GameNum> <Name[Lang]>"
	FXString	NamePlusInfoFN(const ushort& Lang);	// returns "<Name[Lang]> (<InfoFile>)"

	FXString DiskFN(TrackInfo* TI);	// Returns the file name where the track of [TI] is stored
	
	bool	OpenBGMFile(FXFile& File, TrackInfo* TI);	// Opens [TI]

	bool Init(const FXString& Path);	// Calls initializing parse functions after [Path] was verified to contain this game. Returns the result of SeekTest
	void Clear();

	GameInfo();
	~GameInfo();
};

FXString GetToken(ushort ID);
FXString PatternFN(TrackInfo* Track);

#endif /* BGMLIB_INFOSTRUCT_H */
