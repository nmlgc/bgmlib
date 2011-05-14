// Music Room BGM Library
// ----------------------
// packmethod.cpp - Pack Method Base Class
// ----------------------
// "©" Nmlgc, 2010-2011

#include "platform.h"

#include <FXFile.h>
#include <FXDir.h>
#include "list.h"
#include "config.h"
#include "infostruct.h"
#include "bgmlib.h"
#include "ui.h"
#include "packmethod.h"
#ifdef SUPPORT_VORBIS_PM
#include <FXPath.h>
#include "utils.h"
#include "libvorbis.h"
#endif

// Pack Methods
// ------------
PackMethod::PackMethod()
{
	ID = -1;
	ADD_THIS(BGMLib::PM, PackMethod);
}

bool PackMethod::PF_PGI_BGMFile(ConfigFile& NewGame, GameInfo* GI)
{
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, &GI->BGMFile);

	PMGame.Add(&GI);
	return true;
}

// Scans for GameInfo::BGMFile in [Path]
GameInfo* PackMethod::PF_Scan_BGMFile(const FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString* Files = NULL;
	FXint FileCount = 0;
	
	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		NewGame = CurGame->Data;

		FileCount = FXDir::listFiles(Files, Path, NewGame->BGMFile, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
		SAFE_DELETE_ARRAY(Files);

		if((FileCount > 0) && BGMFile_Check(NewGame))
		{
			return NewGame;
		}
		CurGame = CurGame->Next();
	}
	return NULL;
}

#ifdef SUPPORT_VORBIS_PM

// Scans for original and Vorbis versions of GameInfo::BGMFile in [Path]
GameInfo* PackMethod::PF_Scan_BGMFile_Vorbis(const FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString* Files = NULL;
	FXint FileCount = 0;
	FXString Search, Ext;
	bool TrgVorbis, Found;

	// ov_open can be a quite expensive operation when dealing with mulitple bitstreams,
	// so we're always keeping the last loaded file open, until the scan file name changes
	FXString LastSearchFN, LastVorbisFN;
	OggVorbis_File VF;

	memset(&VF, 0, sizeof(OggVorbis_File));
	
	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		NewGame = CurGame->Data;
		TrgVorbis = Found = false;

		if(NewGame->BGMFile != LastSearchFN)
		{
			SAFE_DELETE_ARRAY(Files);
			Search = replaceExtension(NewGame->BGMFile, "*");
			FileCount = FXDir::listFiles(Files, Path, Search, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
			LastSearchFN = NewGame->BGMFile;
		}

		for(ushort c = 0; c < FileCount; c++)
		{
			Ext = FXPath::extension(Files[c]);
			Found = BGMFile_Check_Vorbis(NewGame, Files[c], Ext, LastVorbisFN, &TrgVorbis, &VF);
			
			if(Found)
			{
				// NewGame->BGMFile = Files[c];	// Moved to DiskFN, we should not change that value
				NewGame->Vorbis = TrgVorbis;	// Let's cache the target Vorbis flag there...

				// Immediately return when we found a matching .dat file (because it will be lossless!)
				if(!TrgVorbis)	break;
			}
		}
		if(Found)	break;
		
		CurGame = CurGame->Next();
		NewGame = NULL;
	}
	SAFE_DELETE_ARRAY(Files);
	ov_clear(&VF);
	return NewGame;
}

// Helper function test-opening a new Vorbis file
bool PackMethod::PF_Scan_TestVorbis(OggVorbis_File* VF, const FXString& FN)
{
	// File handle has to be static because vorbisfile will crash on ov_clear if it's already freed
	// And since we always close it here, nothing will leak
	static FXFile F;
	int Ret;

	ov_clear(VF);
	F.open(FN);
	Ret = ov_test_callbacks(&F, VF, NULL, 0, OV_CALLBACKS_FXFILE);
	// VF now has everything we want, so we can close already
	F.close();
	return Ret == 0;
}


#endif

// Reads necessary track data from an archive file based on it's extension.
// Sets track start/end values and returns true if [AudioExt], or calls MetaData function below and returns false on [MetaExt].
TrackInfo* PackMethod::PF_TD_ParseArchiveFile(GameInfo *GI, FXFile& In, const FXString &_FN, const FXString &AudioExt, const FXString &MetaExt, const ulong &CFPos, const ulong &CFSize)
{
	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* Track;

	FXString FNExt = _FN.after('.');
	FXString FN = _FN.before('.');

	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		Track = &CurTrack->Data;

		if((Track->NativeFN) == FN)
		{
			if(FNExt == AudioExt)		AudioData(GI, In, CFPos, CFSize, Track);
			else if(FNExt == MetaExt)	MetaData(GI, In, CFPos, CFSize, Track);
			return Track;
		}
		else CurTrack = CurTrack->Next();
	}
	return NULL;
}

void PackMethod::AudioData(GameInfo* GI, FXFile& In, const ulong& Pos, const ulong& Size, TrackInfo* TI)
{
	TI->Start[0] = TI->Start[1] = Pos;
	TI->FS = Size;
}

bool PackMethod::Dump(GameInfo* GI, FXFile& In, const ulong& Pos, const ulong& Size, const FXString& DumpFN, volatile FXulong* p)
{
	FXFile Dec;
	if(!Dec.open(DumpFN, FXIO::Writing))
	{
		BGMLib::UI_Error("Couldn't open temporary Vorbis file!\nExtraction from this game won't be possible...\n");
		return false;
	}

	char* DecBuf = new char[Size];

	DecryptFile(GI, In, DecBuf, Pos, Size, p);
	Dec.writeBlock(DecBuf, Size);
	SAFE_DELETE_ARRAY(DecBuf);

	Dec.close();

	return true;
}

FXString PackMethod::DiskFN(GameInfo* GI, TrackInfo* TI)
{
	return GI->BGMFile;
}

FXString PackMethod::PMInfo(GameInfo* GI)
{
	return FXString("To get the original music files as used by the game without re-encoding,\n"
					"select the OGG format, a loop count of 1, and 0 second fades.");
}
// ------------

// PM_None
// -------
bool PM_None::ParseGameInfo(ConfigFile&, GameInfo*)		{return true;}
bool PM_None::ParseTrackInfo(ConfigFile&, GameInfo*, ConfigParser*, TrackInfo*)	{return false;}
GameInfo* PM_None::Scan(const FXString&)	{return NULL;}
FXString PM_None::DiskFN(GameInfo*, TrackInfo*)	{return "";}
void PM_None::DisplayName(FXString& Name, GameInfo*)		{Name.append(" (tagging only)");}
// -------

PackMethod* BGMLib::FindPM(const short& PMID)
{
	ListEntry<PackMethod*>*	CurPM = PM.First();
	while(CurPM)
	{
		if(CurPM->Data->GetID() == PMID)	return CurPM->Data;
		CurPM = CurPM->Next();
	}
	// return &PM_None::Inst();
	return NULL;
}
