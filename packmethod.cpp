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

GameInfo* PackMethod::PF_Scan_BGMFile(const FXString& Path, bool (*SecondCheckFunction)(GameInfo*))
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

		if((FileCount > 0))
		{
			if(SecondCheckFunction)
			{
				if(SecondCheckFunction(NewGame))	return NewGame;
			}
			else	return NewGame;
		}
		CurGame = CurGame->Next();
	}

	return NULL;
}

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

		if((Track->FN) == FN)
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

FXString PackMethod::TrackFN(GameInfo* GI, TrackInfo* TI)
{
	return GI->BGMFile;
}

// SeekTest
// --------
bool PackMethod::SeekTest_Open(FXFile& In, GameInfo* GI)
{
	return In.open(GI->BGMFile);
}

bool PackMethod::SeekTest_Track(FXFile& In, GameInfo* GI, TrackInfo* TI)
{
	char Read;

	In.position(TI->GetStart());
	return In.readBlock(&Read, 1) == 1;
}

bool PackMethod::SeekTest_Close(FXFile& In, GameInfo* GI)
{
	return In.close();
}

bool PackMethod::SeekTest(GameInfo* GI)
{
	ushort Seek = 0, Found = 0;
	TrackInfo* TI;
	FXFile In;
	FXString Str;
	bool Ret;

	Ret = SeekTest_Open(In, GI);
	if(!Ret)	return false;
	else if(!In.isOpen())	return true;

	BGMLib::UI_Stat("Verifying track count...");

	ListEntry<TrackInfo>* CurTI = GI->Track.First();
	while(CurTI)
	{
		TI = &CurTI->Data;

		if(TI->GetStart() != 0)
		{
			if(SeekTest_Track(In, GI, TI))
			{
				Seek = TI->Number;
				Found++;
			}
			else	break;
		}

		CurTI = CurTI->Next();
	}
	SeekTest_Close(In, GI);

	Str.format("%d/%d\n", Found, GI->Track.Size());

	if(Seek == 0)
	{
		Str.append("BGM file (" + GI->BGMFile + ") is corrupted!\n");
		Ret = false;
	}
	else if(Seek != GI->Track.Size())
	{
		Str.append("This is most likely a trial version. Extraction is limited to the available tracks.\n");
	}
	BGMLib::UI_Stat(Str);

	GI->TrackCount = Seek;
	return Ret;
}
// --------

// SilenceScan
// -----------
bool PackMethod::SilenceScan_Open(FXFile& In, GameInfo* GI)
{
	if(GI->BGMFile.empty())	return false;
	else	return In.open(GI->BGMFile);
}

ulong PackMethod::SilenceScan_Track(FXFile &In, GameInfo *GI, TrackInfo *TI, ulong* Buf, ulong BufSize)
{
	const ulong Comp = 0;
	ulong c;

	In.position(TI->GetStart());
	In.readBlock(Buf, BufSize);

	BufSize >>= 2;
	for(c = 0; c < BufSize; c++)
	{
		if(Buf[c] != Comp)	break;
	}
	// Fix IaMP
	if(c == BufSize)	c = 0;
	return c << 2;
}

bool PackMethod::SilenceScan_Close(FXFile& In, GameInfo* GI)
{
	// Same code ;-)
	return SeekTest_Close(In, GI);
}

void PackMethod::SilenceScan(GameInfo* GI)
{
	if(GI->Scanned)	return;
	if(GI->Vorbis && GI->CryptKind)
	{
		GI->Scanned = true;
		return;
	}

	BGMLib::UI_Stat("Scanning track start values...");

	TrackInfo* TI;
	FXFile In;
	ulong* Buf = NULL;	// 32-bit elements
	ulong BufSize;

	ulong tl;

	// LARGE_INTEGER Time[2], Total;
	// QueryPerformanceCounter(&Time[0]);

	bool MultiFile = !SilenceScan_Open(In, GI);

	ListEntry<TrackInfo>* CurTI = GI->Track.First();

	for(ushort Track = 0; Track < GI->TrackCount; Track++)
	{
		TI = &CurTI->Data;
		TI->GetPos(FMT_BYTE, false, NULL, &tl);

		// Check for a maximum of 5 seconds
		BufSize = tl - TI->Start[0];
		BufSize = MIN(BufSize, TI->Freq * 4.0f * 5.0f);
		Buf = (ulong*)realloc(Buf, BufSize);

		if(MultiFile)	In.open(GI->TrackFN(TI), FXIO::Reading);

		TI->Start[1] = TI->Start[0] + SilenceScan_Track(In, GI, TI, Buf, BufSize);
		
		if(MultiFile)	In.close();
		CurTI = CurTI->Next();
	}
	free(Buf);	Buf = NULL;

	if(!MultiFile)	SilenceScan_Close(In, GI);

	GI->Scanned = true;

	// QueryPerformanceCounter(&Time[1]);
	// Total = Time[1] - Time[0];

	BGMLib::UI_Stat("done.\n");
}
// -----------

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
FXString PM_None::TrackFN(GameInfo*, TrackInfo*)	{return "";}
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
