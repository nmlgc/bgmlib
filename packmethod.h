// Music Room BGM Library
// ----------------------
// packmethod.h - Pack Method Base Class
// ----------------------
// "©" Nmlgc, 2010-2011

#pragma once

#ifndef BGMLIB_PACKMETHOD_H
#define BGMLIB_PACKMETHOD_H

#undef DecryptFile	// ...Win32

// Forward declarations
class ConfigParser;
class ConfigFile;
struct TrackInfo;
struct GameInfo;

// Pack Method Base Class
// ----------------------
class PackMethod
{
	friend struct GameInfo;

protected:
	short ID;	// Number identifier of this pack method

	List<GameInfo*>	PMGame;

	PackMethod();

	// Prefabricated general purpose functionality for the virtual methods
	// -------------

	// ParseGameInfo
	bool PF_PGI_BGMFile(ConfigFile& NewGame, GameInfo* GI);
	GameInfo* PF_Scan_BGMFile(const FXString& Path, bool (*SecondCheckFunction)(GameInfo*) = NULL);

	// TrackData

	// Reads necessary track data from an archive file based on its extension.
	// Calls <AudioData> on [AudioExt], or <MetaData> on [MetaExt].
	TrackInfo* PF_TD_ParseArchiveFile(GameInfo *GI, FXFile& In, const FXString &_FN, const FXString &AudioExt, const FXString &MetaExt, const ulong &CFPos, const ulong &CFSize);
	// -------------

	// (Required for PF_TD_ParseArchiveFile function!)
	virtual void MetaData(GameInfo* GI, FXFile& In, const ulong& Pos, const ulong& Size, TrackInfo* TI) {}	// Reads track meta data in the method's meta format
	virtual void AudioData(GameInfo* GI, FXFile& In, const ulong& Pos, const ulong& Size, TrackInfo* TI); 	// (Default implementation:) Sets track start/end values

public:
	const short& GetID()	{return ID;}

	// Decryption function, called by <Dump> and the extractor. Returns the number of source bytes read from the file (important if encryption changes file size!)
	virtual ulong DecryptFile(GameInfo* GI, FXFile& In, char* Out, const ulong& Pos, const ulong& Size, volatile FXulong* p = NULL) {return 0;}

	bool Dump(GameInfo* GI, FXFile& In, const ulong& Pos, const ulong& Size, const FXString& DumpFN, volatile FXulong* p = NULL);

	virtual bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI) = 0;
	virtual bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, ConfigParser* TS, TrackInfo* NewTrack) = 0;		// return true if position data should be read from config file

	virtual GameInfo* Scan(const FXString& Path) = 0;	// Scans [Path] for a game packed with this method.
	virtual bool TrackData(GameInfo* GI) {return true;}	// Another custom function after the tracks were parsed

	virtual void DisplayName(FXString& Name, GameInfo* GI)	{}	// Allows modification of the (delimited) game display name
	virtual FXString DiskFN(GameInfo* GI, TrackInfo* TI);	// Returns the file name where the track of [TI] is stored

	virtual FXString	PMInfo(GameInfo* GI);	// Displays an info string after a game was loaded
};
// ----------------------

// PM_None (Dummy class)
// -------
class PM_None : public PackMethod
{
protected:
	PM_None()	{ID = 0;}

public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, ConfigParser* TS, TrackInfo* NewTrack);

	GameInfo* Scan(const FXString& Path);

	FXString DiskFN(GameInfo* GI, TrackInfo* TI);	// Returns an empty string, because we have no files

	void DisplayName(FXString& Name, GameInfo* GI);

	SINGLETON(PM_None);
};
// -------

#endif /* BGMLIB_PACKMETHOD_H */