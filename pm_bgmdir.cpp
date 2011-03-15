// Music Room BGM Library
// ----------------------
// pm_bgmdir.cpp - Parsing for BGM Directory games (th06, Kioh Gyoku)
// ----------------------
// "©" Nmlgc, 2010-2011

#include "platform.h"
#include <FXFile.h>
#include <FXDir.h>
#include "infostruct.h"
#include "packmethod.h"
#include "config.h"
#include "pm_zun.h"

bool PM_BGMDir::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "bgmdir", TYPE_STRING, &GI->BGMDir);

	GI->BGMFile.clear();

	PMGame.Add(&GI);

	return true;
}

bool PM_BGMDir::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, ConfigParser* TS, TrackInfo *NewTrack)
{
	TS->GetValue("filename", TYPE_STRING, &NewTrack->FN);
	NewTrack->Start[0] = GI->HeaderSize;
	NewTrack->PosFmt = FMT_BYTE;

	return true;	// Read position info from parsed file
}

FXString PM_BGMDir::TrackFN(GameInfo* GI, TrackInfo* TI)
{
	return GI->BGMDir + PATHSEP + TI->FN;
}

// Cancel seek testing
bool PM_BGMDir::SeekTest_Open(FXFile& In, GameInfo* GI)
{
	GI->TrackCount = GI->Track.Size();
	return true;
}

// Scanning
// --------
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

		if(DirCount == 1)	return GI;
		CurGame = CurGame->Next();
	}
	return NULL;
}

// --------
