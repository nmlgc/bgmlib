// Music Room BGM Library
// ----------------------
// pm_zun.h - ZUN's Pack Methods
// ----------------------
// "©" Nmlgc, 2010-2011

#ifndef BGMLIB_PM_ZUN_H
#define BGMLIB_PM_ZUN_H

// Pack Methods
#define BGMDIR  0x1	// BGM Directory containing multiple wave files (th06, Kioh Gyoku)
#define BGMDAT  0x2	// raw PCM data in a single file (usually thbgm.dat). Also supports Vorbis-compressed versions of this file.

// Needless base class
class PM_ZUN : public PackMethod
{
	FXString PMInfo(GameInfo* GI);
};

// PM_BGMDir
// ---------
class PM_BGMDir : public PM_ZUN
{
protected:
	PM_BGMDir()	{ID = BGMDIR;}

public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, ConfigParser* TS, TrackInfo* NewTrack);		// return true if position data should be read from config file

	bool CheckBGMDir(GameInfo* Target);	// Checks if [Dir] contains the BGM of [Target]

	FXString DiskFN(GameInfo* GI, TrackInfo* TI);	// Returns a track filename with prepended BGM directory
	GameInfo* Scan(const FXString& Path);	// Scans [Path] for a game packed with this method

	bool TrackData(GameInfo* GI);	// Removes the RIFF header from TI->Start[0] in case of Vorbis BGM

	SINGLETON(PM_BGMDir);
};
// ---------

// PM_BGMDat
// ---------
class PM_BGMDat : public PM_ZUN
{
protected:
	PM_BGMDat()	{ID = BGMDAT;}

public:
	bool ParseGameInfo(ConfigFile& NewGame, GameInfo* GI);
	bool ParseTrackInfo(ConfigFile& NewGame, GameInfo* GI, ConfigParser* TS, TrackInfo* NewTrack);		// return true if position data should be read from config file

	// Checks if [File] contains the ZWAV identification bytes of [Target]
	bool BGMFile_Check(GameInfo* GI);
#ifdef SUPPORT_VORBIS_PM
	bool BGMFile_Check_Vorbis(GameInfo* GI, FXString& FN, FXString& Ext, FXString& LastVorbisFN, bool* TrgVorbis, OggVorbis_File* VF);
#endif

	FXString DiskFN(GameInfo* GI, TrackInfo* TI);	// Returns the main BGM file name
	GameInfo* Scan(const FXString& Path);	// Scans [Path] for a game packed with this method

	// bool TrackData(GameInfo* GI);	// Another custom function after the tracks were parsed

	SINGLETON(PM_BGMDat);
};
// ---------

#endif /* BGMLIB_PM_ZUN_H */