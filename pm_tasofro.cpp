// Music Room BGM Library
// ----------------------
// pm_tasofro.cpp - Parsing for Tasofro archives
// ----------------------
// "©" Nmlgc, 2010

#include "platform.h"
#include <FXSystem.h>
#include <FXFile.h>
#include "infostruct.h"
#include "ui.h"
#include "config.h"
#include "packmethod.h"
#include "utils.h"
#include "pm_tasofro.h"
#include "mt.hpp"

#ifdef SUPPORT_VORBIS_PM
#include "libvorbis.h"
#endif 

// Info
// ----
bool PM_Tasofro::CheckCryptKind(ConfigFile& NewGame, const uchar& CRKind)
{
	if(CRKind == 0 || CRKind > CR_TENSHI)
	{
		FXString Str;
		Str.format("Invalid encryption kind specified in %s!\n", NewGame.GetFN());
		BGMLib::UI_Error(Str);
		return false;
	}
	return true;
}

// Returns GI->BGMFile. Tasofro always stores BGM in one file
FXString PM_Tasofro::DiskFN(GameInfo* GI, TrackInfo* TI)
{
	return GI->BGMFile;
}

bool PM_BMWav::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	PF_PGI_BGMFile(NewGame, GI);
	return CheckCryptKind(NewGame, GI->CryptKind);
}

bool PM_BMWav::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, ConfigParser* TS, TrackInfo *NewTrack)
{
	TS->GetValue("filename", TYPE_STRING, &NewTrack->NativeFN);
	NewTrack->Start[0] = GI->HeaderSize;
	NewTrack->PosFmt = FMT_BYTE;

	return false;	// Read position info from archive
}

bool PM_BMOgg::ParseGameInfo(ConfigFile &NewGame, GameInfo *GI)
{
	PF_PGI_BGMFile(NewGame, GI);
	return CheckCryptKind(NewGame, GI->CryptKind);
}

bool PM_BMOgg::ParseTrackInfo(ConfigFile &NewGame, GameInfo *GI, ConfigParser* TS, TrackInfo *NewTrack)
{
	TS->GetValue("filename", TYPE_STRING, &NewTrack->NativeFN);
	NewTrack->PosFmt = FMT_SAMPLE;
	GI->Vorbis = true;

	return false;	// Read position info from archive
}

// Decryption
// ----------
ulong PM_Tasofro::HeaderSizeV2(FX::FXFile& In)
{
	ulong Ret;
	In.readBlock(&Ret, 4);
	return Ret;
}

bool PM_Tasofro::IsValidHeader(char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	char* p = hdr;
	uchar FNLen;
	for(ushort f = 0; f < Files; f++)
	{
		p += 8;
		memcpy(&FNLen, p, 1); p++;
		p += FNLen;
		if( (p - hdr) > (FXint)hdrSize)	return false;
	}
	return (p - hdr) == hdrSize;
}

void PM_Tasofro::DecryptHeaderV1(char* hdr, const FXuint& hdrSize)
{
	FXushort k = 0x64, l = 0x64;

	for(ushort c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += l; l += 0x4D;
	}
}

void PM_Tasofro::DecryptHeaderV2(char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	RNG_MT mt(hdrSize + 6);
	for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= mt.next_int32() & 0xFF;
	}

	// Check header validity. If we can read the data already, we're done.
	if(IsValidHeader(hdr, hdrSize, Files))	return;

    FXuchar k = 0xC5, t = 0x83;
    for(ulong c = 0; c < hdrSize; ++c)
	{
		hdr[c] ^= k;
		k += t; t +=0x53;
    }
}

// Calling functions
ulong PM_Tasofro::HeaderSize(GameInfo* GI, FX::FXFile& In, const FXushort& Files)
{
	switch(GI->CryptKind)
	{
	case CR_SUIKA:	return Files * GI->EntrySize;
	case CR_TENSHI:	return HeaderSizeV2(In);
	}
	return 0;
}

bool PM_Tasofro::DecryptHeader(GameInfo* GI, char* hdr, const FXuint& hdrSize, const FXushort& Files)
{
	switch(GI->CryptKind)
	{
	case CR_SUIKA:	DecryptHeaderV1(hdr, hdrSize);	return true;
	case CR_TENSHI:	DecryptHeaderV2(hdr, hdrSize, Files);	return true;
	}
	return false;
}

// ----------

// Data
// ----
void PM_BMWav::GetPosData(GameInfo* GI, FX::FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize)
{
	char* p = hdr;

	const FXushort hdrJunkSize = GI->EntrySize - 4;

	char* FNTemp = new char[hdrJunkSize];
	FXString FN;

	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* Track;

	FXuint DataSize;

	for(ushort c = 0; c < Files; c++)
	{
		strcpy(FNTemp, p);	FN = FNTemp;
		p += hdrJunkSize;

		CurTrack = GI->Track.First();
		while(CurTrack)
		{
			Track = &CurTrack->Data;

			if((Track->NativeFN + ".wav") == FN)
			{
				memcpy_advance(&Track->Start[0], &p, 4);
				CurTrack = NULL;
			}
			else	CurTrack = CurTrack->Next();
		}
	}

	// Get loop and end data...
	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		Track = &CurTrack->Data;

		In.position(Track->Start[0] + 40);
		In.readBlock(&DataSize, 4);	// Wave data size

		In.position(DataSize + 16, FXIO::Current);

		In.readBlock(&Track->Loop, 4);	// Loop in samples

		In.position(40, FXIO::Current);
		In.readBlock(&Track->End, 4);	// End in samples, relative to loop

		Track->Loop *= 4;
		Track->End *= 4;
		Track->End += Track->Loop;

		Track->Start[0] += GI->HeaderSize;
		Track->End += Track->Start[0];
		Track->Loop += Track->Start[0];

		if(Track->Loop == Track->Start[0])	Track->Loop = Track->End;

		CurTrack = CurTrack->Next();
	}
	SAFE_DELETE(FNTemp);
}
// ----

// Data
// ----
#ifdef BGMLIB_LIBVORBIS_H
ulong PM_BMOgg::DecryptBuffer(const uchar& CryptKind, char* Out, const ulong& Pos, const ulong& Size)
{
	unsigned char k = (Pos >> 1);
	
	switch(CryptKind)
	{
	case CR_SUIKA:	k |= 0x08;	break;	// I found that one on my own! I'm really proud of myself :-)
	case CR_TENSHI:	k |= 0x23;	break;
	}

	for(ulong i = 0; i < Size; ++i)
	{
		Out[i] ^= k;
	}

	return Size;
}

ulong PM_BMOgg::DecryptFile(GameInfo* GI, FXFile& In, char* Out, const ulong& Pos, const ulong& Size, volatile FXulong* p)
{
	if(!Out)	return NULL;

	if(!In.position(Pos))	return 0;
	ulong Ret = In.readBlock(Out, Size);

	DecryptBuffer(GI->CryptKind, Out, Pos, Ret);
	if(p)	*p = Ret;

	return Ret;
}

void PM_BMOgg::MetaData(GameInfo* GI, FX::FXFile& In, const ulong& Pos, const ulong& Size, TrackInfo* TI)
{
	char* SFL = new char[Size];
	DecryptFile(GI, In, SFL, Pos, Size);

	char* p = SFL + 28;

	memcpy(&TI->Loop, p, 4); p += 4 + 40;
	memcpy(&TI->End, p, 4);

	TI->End += TI->Loop;

	SAFE_DELETE_ARRAY(SFL);
}

void PM_BMOgg::GetPosData(GameInfo* GI, FXFile& In, FXushort& Files, char* hdr, FXuint& hdrSize)
{
	char* p = hdr;

	char FNTemp[128];
	ulong CFPos = 0, CFSize = 0;
	uchar FNLen;

	ListEntry<TrackInfo>* CurTrack;
	TrackInfo* TI;

	for(ushort c = 0; c < Files; c++)
	{
		if(GI->CryptKind == CR_SUIKA)
		{
			memcpy( &CFPos, p + GI->EntrySize - 4, 4);
			memcpy(&CFSize, p + GI->EntrySize - 8, 4);
			FNLen = strlen(p);
		}
		else
		{
			memcpy_advance( &CFPos, &p, 4);
			memcpy_advance(&CFSize, &p, 4);
			memcpy_advance( &FNLen, &p, 1);
		}

		// Split filename in name + extension
		strncpy(FNTemp, p, FNLen);	p += FNLen;
		FNTemp[FNLen] = '\0';

		PF_TD_ParseArchiveFile(GI, In, FNTemp, "ogg", "sfl", CFPos, CFSize);

		if(GI->CryptKind == CR_SUIKA) p += GI->EntrySize - FNLen;
	}

	CurTrack = GI->Track.First();
	while(CurTrack)
	{
		TI = &CurTrack->Data;
		if(TI->Start[0] == 0)
		{
			TI->FS = -1;
			CurTrack = CurTrack->Next();
			continue;
		}

		if(TI->Loop == 0)
		{
			// LARGE_INTEGER Time[2], Total;
			// QueryPerformanceCounter(&Time[0]);

			// Right. This is indeed faster than the CryptFile solution.
			VFile Dec(TI->FS);
			DecryptFile(GI, In, Dec.Buf, TI->GetStart(), Dec.Size, &Dec.Write);

			OggVorbis_File SF;
			if(ov_open_callbacks(&Dec, &SF, NULL, 0, OV_CALLBACKS_VFILE))
			{
				sprintf(FNTemp, "Error decrypting %s!\n", TI->NativeFN);
				BGMLib::UI_Stat(FNTemp);
			}
			else
			{
				TI->Loop = TI->End = ov_pcm_total(&SF, -1);
				ov_clear(&SF);
			}
			Dec.Clear();

			// QueryPerformanceCounter(&Time[1]);
			// Total = Time[1] - Time[0];
		}
		CurTrack = CurTrack->Next();
	}

	FXSystem::setCurrentDirectory(GI->Path);

	// We don't silence scan those
	GI->Scanned = true;
}
#endif
// ----

bool PM_Tasofro::TrackData(GameInfo* GI)
{
	FXFile In;
	FXushort Files;

	FXuint hdrSize;
	char* hdr;

	In.open(GI->BGMFile, FXIO::Reading);

	In.readBlock(&Files, 2);
	hdrSize = HeaderSize(GI, In, Files);

	hdr = new char[hdrSize];

	In.readBlock(hdr, hdrSize);
	DecryptHeader(GI, hdr, hdrSize, Files);

	// Get track offsets...
	GetPosData(GI, In, Files, hdr, hdrSize);

	SAFE_DELETE_ARRAY(hdr);

	// ... done!
	In.close();

	// GI->Scanned gets set by SilenceScan
	return true;
}

// Scanning
// --------

bool PM_BMWav::CheckBMTracks(GameInfo* Target)
{
	FXushort Tracks;
	FX::FXFile In(Target->BGMFile, FXIO::Reading);

	In.readBlock(&Tracks, 2);
	In.close();

	return Tracks == Target->TrackCount;
}
static bool Wrap_CheckBMTracks(GameInfo* GI)	{return PM_BMWav::Inst().CheckBMTracks(GI);}

GameInfo* PM_BMWav::Scan(const FXString& Path)
{
	return PF_Scan_BGMFile(Path, Wrap_CheckBMTracks);
}

GameInfo* PM_BMOgg::Scan(const FXString& Path)
{
	return PF_Scan_BGMFile(Path);
}
// --------
