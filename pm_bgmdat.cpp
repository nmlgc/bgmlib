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
	NewTrack->FN = GI->BGMFile;
	NewTrack->PosFmt = FMT_BYTE;

	return true;	// Read position info from parsed file
}

// Scanning
// --------
bool PM_BGMDat::CheckZWAV_dat(GameInfo* Target, FXFile& File)
{
	uchar FileID[2];

	File.position(8);
	File.readBlock(FileID, 2);
	
	return (FileID[0] == Target->ZWAVID[0]) && (FileID[1] == Target->ZWAVID[1]);
}

static bool CheckZWAV_dat_helper(GameInfo* Target)
{
	FXFile F(Target->BGMFile);
	bool Ret = PM_BGMDat::Inst().CheckZWAV_dat(Target, F);
	F.close();
	return Ret;
}

// Scans [Path] for an original BGM file
GameInfo* PM_BGMDat::ScanDat(const FXString& Path)
{
	GameInfo* NewGame = PF_Scan_BGMFile(Path, CheckZWAV_dat_helper);
	if(NewGame)	NewGame->HeaderSize = 0;
	return NewGame;
}

static bool Scan_OpenVorbis(char** Tag, FXFile* F, OggVorbis_File* VF, const FXString& FN)
{
	static const FXString ZWAV("ZWAV");

	if(F->isOpen())	ov_clear(VF);
	F->open(FN);
	if(ov_open_callbacks(F, VF, NULL, 0, OV_CALLBACKS_FXFILE))	F->close();

	vorbis_comment* vc = ov_comment(VF, -1);
	if(!vc)	return false;

	if(!(*Tag = vorbis_comment_query(vc, ZWAV.text(), 0)))	return false;
	if(strlen(*Tag) < 2)
	{
		*Tag = NULL;	// Security
		return false;
	}

	// Undo the replacements of the Vorbis compressor
	for(ushort c = 0; c < 2; c++)
	{
		if((*Tag)[c] == -1) (*Tag)[c] = 0;
	}
	return true;
}

// Scans [Path] for both original and Vorbis-compressed BGM files
GameInfo* PM_BGMDat::ScanFull(const FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString* Files = NULL;
	FXint FileCount = 0;
	FXString Search, Ext;
	bool TrgVorbis, Found;

	// ov_open can be a quite expensive operation when dealing with mulitple bitstreams,
	// so we're always keeping the last loaded file open, until the scan file name changes
	FXString LastSearchFN, LastVorbisFN;
	FXFile F;
	OggVorbis_File VF;
	char* Tag;

	memset(&VF, 0, sizeof(OggVorbis_File));
	
	ListEntry<GameInfo*>* CurGame = PMGame.First();
	while(CurGame)
	{
		NewGame = CurGame->Data;
		TrgVorbis = Found = false;

		if(NewGame->BGMFile != LastSearchFN)
		{
			SAFE_DELETE_ARRAY(Files);
			Search = FXPath::stripExtension(NewGame->BGMFile) + ".*";
			FileCount = FXDir::listFiles(Files, Path, Search, FXDir::NoDirs | FXDir::CaseFold | FXDir::HiddenFiles);
			LastSearchFN = NewGame->BGMFile;
		}

		for(ushort c = 0; c < FileCount; c++)
		{
			Ext = FXPath::extension(Files[c]);
			if(Ext == "dat")
			{
				TrgVorbis = false;
				FXFile File(Files[c]);
				Found = CheckZWAV_dat(NewGame, File);
				File.close();
			}
			else if(Ext == "ogg")
			{
				TrgVorbis = true;
				if(Files[c] != LastVorbisFN)
				{
					LastVorbisFN = Files[c];

					if(!Scan_OpenVorbis(&Tag, &F, &VF, Files[c]))	continue;
				}
				if(Tag)	Found = (Tag[0] == NewGame->ZWAVID[0]) && (Tag[1] == NewGame->ZWAVID[1]);
			}
			else continue;

			if(Found)
			{
				NewGame->BGMFile = Files[c];
				NewGame->HeaderSize = TrgVorbis;	// Let's cache the target Vorbis flag there...

				// Immediately return when we found a matching .dat file (because it will be lossless!)
				if(!TrgVorbis)	break;
			}
		}
		if(Found)	break;
		
		CurGame = CurGame->Next();
		NewGame = NULL;
	}
	SAFE_DELETE_ARRAY(Files);

	if(F.isOpen())	ov_clear(&VF);

	return NewGame;
}

bool PM_BGMDat::TrackData(GameInfo* GI)
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
}
// --------

OggVorbis_File* SF = NULL;

// Custom SeekTest
// ---------------
bool PM_BGMDat::SeekTest_Open(FXFile& In, GameInfo* GI)
{
	bool Ret = In.open(GI->BGMFile);
	if(GI->Vorbis)
	{
		SF = new OggVorbis_File;
		Ret = (ov_open_callbacks(&In, SF, NULL, 0, OV_CALLBACKS_FXFILE) == 0);
	}
	return Ret;
}

bool PM_BGMDat::SeekTest_Track(FXFile& In, GameInfo* GI, TrackInfo* TI)
{
	char Read;
	if(GI->Vorbis)
	{
		return (ov_pcm_seek(SF, TI->End - 1) == 0);
	}
	else
	{
		In.position(TI->End - 1);
		return (In.readBlock(&Read, 1) == 1);
	}
}

bool PM_BGMDat::SeekTest_Close(FXFile& In, GameInfo* GI)
{
	bool Ret;
	if(GI->Vorbis)
	{
		Ret = (ov_clear(SF) == 0);
		SAFE_DELETE(SF);
	}
	else
	{
		Ret = In.close();
	}
	return Ret;
}
// ---------------

// Custom SilenceScan
// ------------------

// Same code as the seek test
bool PM_BGMDat::SilenceScan_Open(FXFile& In, GameInfo* GI)	{return SeekTest_Open(In, GI);}
bool PM_BGMDat::SilenceScan_Close(FXFile& In, GameInfo* GI)	{return SeekTest_Close(In, GI);}

ulong PM_BGMDat::SilenceScan_Track(FXFile& In, GameInfo* GI, TrackInfo* TI, ulong* _Buf, ulong BufSize)
{
	if(!GI->Vorbis)	return PackMethod::SilenceScan_Track(In, GI, TI, _Buf, BufSize);

	// The naive approach to silence scanning won't work here, because any lossy encoder would smear the previous signal into the silence.
	// So, we are ignoring all audio data at the beginning of the track until we passed a huge block of real, digital silence,
	// and then begin the scanning from there.
	const long Threshold = 1024 * 4;	// around 23 ms
	const long Comp = 0;
	long c = 0, d = 0;	// current section of decoded audio
	long s = 0;	// scanning progress
	long f = 0;	// found bytes
	long Read;
	int Sec;

	char* Buf = (char*)_Buf;
	ov_pcm_seek(SF, TI->GetStart());

	while(c < (long)BufSize)
	{
		Read = MIN(BufSize - c, Threshold * 2);
		d = ov_read(SF, Buf + c, Read, 0, 2, 1, &Sec);
		if(d < 0)
		{
			// Something's wrong with the file...
			return false;
		}

		// This code is so brilliant, it gives me an orgasm every time I read it
		for(s; s < (c + d - Threshold); s += 4)
		{
			if( *((ulong*)(Buf + s)) == Comp)	f += 4;
			else if(f > Threshold)			return s >> 2;
			else							f = 0;
		}

		c += d;
	}
	return 0;
}
// ------------------
