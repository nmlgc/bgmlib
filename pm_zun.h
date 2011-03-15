// Music Room BGM Library
// ----------------------
// pm_zun.h - ZUN's Pack Methods
// ----------------------
// "©" Nmlgc, 2010-2011

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

	bool SeekTest_Open(FXFile& In, GameInfo* GI);	// Cancels seek testing

	FXString TrackFN(GameInfo* GI, TrackInfo* TI);	// Returns a track filename with prepended BGM directory

	GameInfo* Scan(const FXString& Path);	// Scans [Path] for a game packed with this method

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

	bool CheckZWAV_dat(GameInfo* Target, FXFile& File);	// Checks if [File] contains the ZWAV identification bytes of [Target]

	GameInfo* ScanDat(const FXString& Path);	// Scans [Path] for an original BGM file
	GameInfo* ScanFull(const FXString& Path);	// Scans [Path] for both original and Vorbis-compressed BGM files

	GameInfo* Scan(const FXString& Path);	// Scans [Path] for a game packed with this method

	bool TrackData(GameInfo* GI);	// Another custom function after the tracks were parsed

	// Custom SeekTest implementation (needed for thbgm.ogg)
	bool SeekTest_Open(FXFile& In, GameInfo* GI);	// Open the game's BGM file. Should return a valid file handle in [In] and true on opening success, false on failure
	bool SeekTest_Track(FXFile& In, GameInfo* GI, TrackInfo* TI);	// return if a single track is present
	bool SeekTest_Close(FXFile& In, GameInfo* GI);	// Close the game's BGM file.

	// Custom SilenceScan implementation
	bool SilenceScan_Open(FXFile& In, GameInfo* GI);	// Open the game's BGM file. Should return a valid file handle in [In] and true on opening success, false on failure
	ulong SilenceScan_Track(FXFile& In, GameInfo* GI, TrackInfo* TI, ulong* _Buf, ulong BufSize);	// Returns the amount of silence at the start of [TI]
	bool SilenceScan_Close(FXFile& In, GameInfo* GI);	// Close the game's BGM file.

	SINGLETON(PM_BGMDat);
};
// ---------
