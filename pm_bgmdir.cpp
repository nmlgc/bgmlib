// Music Room BGM Library
// ----------------------
// pm_bgmdir.cpp - Parsing for BGM Directory games (th06, Kioh Gyoku)
// ----------------------
// "©" Nmlgc, 2010-2011

#include "platform.h"
#include <FXFile.h>
#include <FXPath.h>
#include <FXDir.h>
#include <FXStat.h>
#include "infostruct.h"
#include "packmethod.h"
#include "config.h"
#include "pm_zun.h"
#include "bgmlib.h"
#include "utils.h"

bool PM_BGMDir::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmdir", TYPE_STRING, &GI->BGMDir);

	GI->BGMFile.clear();

	PMGame.Add(&GI);

	return true;
}

bool PM_BGMDir::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, ConfigParser* TS, TrackInfo *NewTrack)
{
	TS->GetValue("filename", TYPE_STRING, &NewTrack->NativeFN);
	NewTrack->Start[0] = GI->HeaderSize;
	NewTrack->PosFmt = FMT_BYTE;

	return true;	// Read position info from parsed file
}

FXString PM_BGMDir::DiskFN(GameInfo* GI, TrackInfo* TI)
{
	if(GI->Vorbis)	return GI->BGMDir + PATHSEP + replaceExtension(TI->NativeFN, "ogg");
	else			return GI->BGMDir + PATHSEP + TI->NativeFN;
}

// Scanning
// --------
bool PM_BGMDir::CheckBGMDir(GameInfo* Target)
{
	ListEntry<TrackInfo>* First;
	TrackInfo* TI;
	TrackInfo Temp;
	
	if(!Target)	return false;

	if(!Target->HaveTrackData)
	{
		// Quickly get the first track FN
		FXString Str;
		ConfigParser* TS;

		Str = BGMLib::InfoPath + Target->InfoFile;

		ConfigFile NewGame(Str);
		NewGame.Load();

		// Track Info
		// ---------
		TI = &Temp;
		TI->Number = 1;

		TS = NewGame.FindSection(TI->GetNumber());
		if(TS)	ParseTrackInfo(NewGame, Target, TS, TI);
		NewGame.Clear();
		if(!TS)	return false;		
	}
	else
	{
		First = Target->Track.First();
		if(!First)	return false;
		TI = &First->Data;
	}

	if(FXStat::exists(DiskFN(Target, TI)))	return true;

#ifdef SUPPORT_VORBIS_PM
	Target->Vorbis = true;
	if(FXStat::exists(DiskFN(Target, TI)))	return true;
	Target->Vorbis = false;
#endif
	return false;
}

GameInfo* PM_BGMDir::Scan(const FXString& Path)
{
	FXString* Dirs = NULL;
	GameInfo* GI;
	FXint DirCount = 0;

	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		GI = CurGame->Data;
		DirCount = FXDir::listFiles(Dirs, ".", GI->BGMDir, FXDir::NoFiles | FXDir::CaseFold | FXDir::HiddenDirs);
		SAFE_DELETE_ARRAY(Dirs);

		if(DirCount == 1 && CheckBGMDir(GI))	return GI;
		CurGame = CurGame->Next();
	}
	return NULL;
}

bool PM_BGMDir::TrackData(GameInfo *GI)
{
	if(GI->Vorbis)
	{
		ListEntry<TrackInfo>* CurTI = GI->Track.First();
		if(!CurTI)	return true;
		do
		{
			CurTI->Data.Start[0] -= GI->HeaderSize;
			CurTI->Data.Loop -= GI->HeaderSize;
			CurTI->Data.End -= GI->HeaderSize;
		}
		while(CurTI = CurTI->Next());
	}
	return true;
}
// --------
