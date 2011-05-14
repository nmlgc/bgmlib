// Music Room BGM Library
// ----------------------
// pm_bgmdat.cpp - Parsing for thbgm.dat games (Team Shanghai Alice games starting with th07)
// ----------------------
// "©" Nmlgc, 2010-2011

#include "platform.h"
#include <FXFile.h>
#include <FXPath.h>
#include <FXIO.h>
#include <FXDir.h>
#include "infostruct.h"
#include "packmethod.h"
#include "config.h"
#include "pm_zun.h"
#include "utils.h"
#include "libvorbis.h"

FXString PM_ZUN::PMInfo(GameInfo* GI)
{
	return FXString("Activate Ogg bitstream chaining in the settings to generate Ogg output\n"
					"files without reencoding. Those might not work with every player, though.");
}

bool PM_BGMDat::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	NewGame.GetValue("game", "zwavid_08", TYPE_UCHAR, &GI->ZWAVID[0]);
	NewGame.GetValue("game", "zwavid_09", TYPE_UCHAR, &GI->ZWAVID[1]);
	NewGame.GetValue("game", "bgmfile", TYPE_STRING, &GI->BGMFile);

	PMGame.Add(&GI);
	return true;
}

bool PM_BGMDat::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, ConfigParser* TS, TrackInfo *NewTrack)
{
	NewTrack->PosFmt = FMT_BYTE;

	return true;	// Read position info from parsed file
}

FXString PM_BGMDat::DiskFN(GameInfo* GI, TrackInfo* TI)
{
	if(GI->Vorbis)	return replaceExtension(GI->BGMFile, "ogg");
	else			return GI->BGMFile;
}

// Scanning
// --------
bool PM_BGMDat::BGMFile_Check(GameInfo* GI)
{
	char FileID[2];
	FXFile F(GI->BGMFile);
	bool Ret;

	F.position(8);
	F.readBlock(FileID, 2);
	
	Ret = (FileID[0] == GI->ZWAVID[0]) && (FileID[1] == GI->ZWAVID[1]);
	F.close();
	return Ret;
}

/*bool PM_BGMDat::TrackData(GameInfo* GI)
{
	// (HeaderSize holds cached Vorbis flag from ::Scan)
	bool TrgVorbis = (GI->HeaderSize != 0);
	// Well, maybe the user has multiple copies of this game installed, and one may have lossless BGM...
	if(GI->Vorbis != TrgVorbis)
	{
		ListEntry<TrackInfo>* CurTrack = GI->Track.First();
		while(CurTrack)
		{
			CurTrack->Data.PosFmtConvert(TrgVorbis);
			CurTrack = CurTrack->Next();
		}
		GI->Vorbis = TrgVorbis;
	}
	return true;
}*/
// --------

#ifdef SUPPORT_VORBIS_PM

bool PM_BGMDat::BGMFile_Check_Vorbis(GameInfo* GI, FXString& FN, FXString& Ext, FXString& LastVorbisFN, bool* TrgVorbis, OggVorbis_File* VF)
{
	if(Ext == "dat")
	{
		*TrgVorbis = false;
		return BGMFile_Check(GI);
	}
	else if(Ext == "ogg")
	{
		static const FXString ZWAV("ZWAV");
		static char* Tag = NULL;

		*TrgVorbis = true;
		if((FN != LastVorbisFN) || !Tag)
		{
			vorbis_comment* vc;
			LastVorbisFN = FN;

			if(!PF_Scan_TestVorbis(VF, FN))	return false;
			vc = ov_comment(VF, -1);
			if(!vc)	return false;
			if(!(Tag = vorbis_comment_query(vc, ZWAV.text(), 0)))	return false;
			if(strlen(Tag) < 2)
			{
				Tag = NULL;	// Security
				return false;
			}

			// Undo the replacements of the Vorbis compressor
			for(ushort c = 0; c < 2; c++)
			{
				if(Tag[c] == -1) Tag[c] = 0;
			}
		}
		if(Tag)	return (Tag[0] == GI->ZWAVID[0]) && (Tag[1] == GI->ZWAVID[1]);
	}
	return false;
}

// Scans [Path] for both original and Vorbis-compressed BGM files
GameInfo* PM_BGMDat::Scan(const FXString& Path)
{
	GameInfo* NewGame = PF_Scan_BGMFile_Vorbis(Path);
	if(NewGame)	NewGame->HeaderSize = 0;
	return NewGame;
}

#else

// Scans [Path] for an original BGM file
GameInfo* PM_BGMDat::Scan(const FXString& Path)
{
	GameInfo* NewGame = PF_Scan_BGMFile(Path, CheckZWAV_dat_helper);
	if(NewGame)	NewGame->HeaderSize = 0;
	return NewGame;
}

#endif	/* SUPPORT_VORBIS_PM */
