// Music Room BGM Library
// ----------------------
// infostruct.cpp - GameInfo and TrackInfo structures
// ----------------------
// "©" Nmlgc, 2011

#include "platform.h"

#include "list.h"
#include "config.h"
#include <FXHash.h>
#include <FXStream.h>
#include <FXIO.h>
#include <FXFile.h>
#include <FXIcon.h>
#include <FXSystem.h>
#include "infostruct.h"
#include "bgmlib.h"
#include "ui.h"
#include "packmethod.h"

// i18n
// ----
namespace BGMLib
{
	LangInfo LI[LANG_COUNT];

	void SetupLang()
	{
		LI[LANG_JP].Display.assign(L"日本語");
		LI[LANG_JP].GUILang.assign("Japanese");
		LI[LANG_JP].Code2.assign("jp");
		LI[LANG_JP].Code3.assign("jpn");

		LI[LANG_EN].Display.assign("English");
		LI[LANG_EN].GUILang.assign("English");
		LI[LANG_EN].Code2.assign("en");
		LI[LANG_EN].Code3.assign("eng");
	}
}
void LangInfo::Clear()
{
	Display.clear();
	GUILang.clear();
	Code2.clear();
	Code3.clear();
}
// ----

static ulong ParseSubstring(FXString& Split)
{
	return Split.toULong(BaseCheck(&Split));
}

// TrackInfo
// ---------
TrackInfo::TrackInfo()
{
	Clear();
}

// Converts position data from sample to byte format and vice versa
void TrackInfo::PosFmtConvert(const bool& TrgFmt)
{
	if(PosFmt == TrgFmt)	return;
		
	if(TrgFmt == FMT_SAMPLE)
	{	// byte to sample
		Start[0] >>= 2;
		Start[1] >>= 2;
		Loop >>= 2;
		End >>= 2;
	}
	else
	{	// sample to byte
		Start[0] <<= 2;
		Start[1] <<= 2;
		Loop <<= 2;
		End <<= 2;
	}
	PosFmt = TrgFmt;
}

void TrackInfo::GetPos(const bool& Fmt, const bool& SilRem, ulong* S, ulong* L, ulong* E)
{
	if(Fmt == PosFmt)
	{
		if(S)	*S = Start[SilRem];
		if(L)	*L = Loop;
		if(E)	*E = End;
	}
	else if(Fmt == FMT_BYTE)
	{	// sample to byte
		if(S)	*S = Start[SilRem] << 2;
		if(L)	*L = Loop << 2;
		if(E)	*E = End << 2;
	}
	else
	{	// byte to sample
		if(S)	*S = Start[SilRem] >> 2;
		if(L)	*L = Loop >> 2;
		if(E)	*E = End >> 2;
	}
}

ulong TrackInfo::GetStart(const bool& Fmt, const bool& SilRem)
{
	if(Fmt == PosFmt)	return Start[SilRem];
	if(Fmt == FMT_BYTE)	return Start[SilRem] << 2;
	else				return Start[SilRem] >> 2;
}

ulong TrackInfo::GetStart()
{
	return Start[0];
}

ulong TrackInfo::GetByteLength(const bool& SilRem, const ushort& LC, const float& FD)
{
	ulong tl, te, ts, len;
	GetPos(FMT_BYTE, SilRem, &ts, &tl, &te);

	if(FS == 0)	{tl -= ts;	te -= ts;}

	if(tl != 0)
	{
		len = (te-tl) * LC + tl;
		if(FD > 0 && tl != te)	len += (ulong)((FD) * ((float)Freq) * 4.0f);
	}
	else	len = te;

	return len;
}

/*ulong TrackInfo::GetByteLength()
{
	return GetByteLength(LoopCnt, FadeDur);
}*/

FXString TrackInfo::LengthString(const ulong& ByteLen)
{
	FXfloat SecLen;
	FXfloat Rem;
	FXString Str;

	SecLen = (float)(ByteLen / 4) / Freq;

	Rem = fmodf(SecLen, NULL);
	if(Rem < 0.5f)	SecLen = floorf(SecLen);
	else			SecLen = ceilf(SecLen);

	Str.format("%d:%2d", (ulong)(SecLen / 60.0f), ((ulong)SecLen) % 60);
	Str.substitute(' ', '0');

	return Str;
}

FXString TrackInfo::GetComment(const ushort& Lang)
{
	FXString Ret(Comment[Lang]);
	ListEntry<IntString>*	CurCmt = Afterword.First();
	while(CurCmt)
	{
		if(!Ret.empty())	Ret.append("\n\n");
		Ret.append(CurCmt->Data[Lang]);
		CurCmt = CurCmt->Next();
	}
	return Ret;
}

FXString TrackInfo::GetNumber()
{
	FXString Ret;
	Ret.fromInt(Number);
	if(Number < 10)	Ret.prepend('0');
	return Ret;
}

void TrackInfo::Clear()
{
	Start[0] = Start[1] = 0;
	Loop = 0;
	End = 0;
	Freq = 44100.0f;
	FS = 0;
	PosFmt = FMT_BYTE;
}

TrackInfo::~TrackInfo()
{
	Afterword.Clear();
}
// ---------

// GameInfo
// --------
void GameInfo::Clear()
{
	Composer.Clear();
	Track.Clear();
	HaveTrackData = false;
	Scanned = false;
	SAFE_DELETE(Icon);
}

GameInfo::GameInfo()
{
	HaveTrackData = false;
	Scanned = false;
	Vorbis = false;
	HeaderSize = 0;
	CryptKind = 0;
	PM = NULL;
	Icon = NULL;
}

FXString GameInfo::DelimName(const ushort& Lang)
{
	return BGMLib::GNDelim[0] + Name[Lang] + BGMLib::GNDelim[1];
}

FXString GameInfo::FullName(const ushort& Lang)
{
	if(TrackCount == Track.Size())	return Name[Lang];
	else							return Name[Lang] + BGMLib::Trial[Lang];
}

FXString GameInfo::NumName(const ushort& Lang)
{
	FXString Ret = DelimName(Lang);
	if(PM)	PM->DisplayName(Ret, this);
	if( (Track.Size() > 0) && Track.Size() != TrackCount)	Ret.append(" " + BGMLib::Trial[Lang]);
	if(!GameNum.empty())	Ret.prepend(GameNum + " ");
	return Ret;
}

FXString GameInfo::NamePlusInfoFN(const ushort& Lang)
{
	FXString Ret = DelimName(Lang);
	if(PM)	PM->DisplayName(Ret, this);
	Ret += " (" + InfoFile + ")\n";
	return Ret;
}

FXString GameInfo::TrackFN(TrackInfo* TI)
{
	if(PM)	return Path + PATHSEP + PM->TrackFN(this, TI);
	else			return "";
}

bool GameInfo::OpenBGMFile(FXFile& File, TrackInfo* TI)
{
	if(!TI)	return false;

	FXString& FN(TrackFN(TI));

	FXbool Ret = File.open(FN);
	if(!Ret)	BGMLib::UI_Error("Couldn't open " + FN + "!\n");
	return Ret;
}

bool GameInfo::ParseGameData(const FXString& Info)
{
	FXString Str;

	InfoFile = Info;
	short PackMethod;

	// LARGE_INTEGER Time[2], Diff[3], TimeTotal;
	// QueryPerformanceCounter(&Time[0]);
	ConfigFile NewGame(InfoFile);
	// QueryPerformanceCounter(&Diff[0]);
	NewGame.Load();
	// QueryPerformanceCounter(&Diff[1]);

	// Game Info
	// ---------
	ConfigParser* Game = NewGame.FindSection("game");
	Game->GetValue("name", TYPE_STRING, &Name[LANG_JP]);
	Game->GetValue("name_en", TYPE_STRING, &Name[LANG_EN]);
	Game->GetValue("packmethod", TYPE_SHORT, &PackMethod);
	Game->GetValue("encryption", TYPE_UCHAR, &CryptKind);
	Game->GetValue("entrysize", TYPE_USHORT, &EntrySize);
	Game->GetValue("headersize", TYPE_USHORT, &HeaderSize);
	Game->GetValue("gamenum", TYPE_STRING, &GameNum);

	Game->GetValue("tracks", TYPE_USHORT, &TrackCount);

	if(Name[LANG_EN].empty())	Name[LANG_EN] = Name[LANG_JP];

	PM = BGMLib::FindPM(PackMethod);
	
	if(!PM)
	{
		Str.format("Unsupported BGM packing method specified in %s!\n", InfoFile);
		BGMLib::UI_Error_Safe(Str);
		return false;
	}
	else if(!PM->ParseGameInfo(NewGame, this))	return false;
	BGMLib::UI_Stat(NamePlusInfoFN(Lang));

	// QueryPerformanceCounter(&Diff[2]);
	/* QueryPerformanceCounter(&Time[1]);

	Diff[2] = Diff[2] - Diff[1];
	Diff[1] = Diff[1] - Diff[0];
	Diff[0] = Diff[0] - Time[0];

	TimeTotal = Time[1] - Time[0];

	Str.format("   (Timings: %ld-%ld-%ld (%ld total)\n", Diff[0].LowPart, Diff[1].LowPart, Diff[2].LowPart, TimeTotal.LowPart);
	BGMLib::UI_Stat(Str);*/
	return true;
}



bool GameInfo::ParseTrackData()
{
	FXString Str, Key;
	ulong TempVal;
	ConfigParser* Game;

	TrackInfo* NewTrack;
	ushort Tracks = 0;
	ushort c = 1, l = 0;

	ConfigParser* TS;
	IntString* NewCmt;

	Str = BGMLib::InfoPath + InfoFile;

	ConfigFile NewGame(Str);
	NewGame.Load();

	// Game Info
	// ---------
	Game = NewGame.FindSection("game");
	Game->GetValue("circle", TYPE_STRING, &Circle[LANG_JP]);
	Game->GetValue("circle_en", TYPE_STRING, &Circle[LANG_EN]);
	Game->GetValue("artist", TYPE_STRING, &Artist[LANG_JP]);
	Game->GetValue("artist_en", TYPE_STRING, &Artist[LANG_EN]);
	Game->GetValue("year", TYPE_USHORT, &Year);

	if(Artist[LANG_EN].empty())	Artist[LANG_EN] = Artist[LANG_JP];
	if(Circle[LANG_EN].empty())	Circle[LANG_EN] = Circle[LANG_JP];
	// ---------

	// Composers
	// ---------
	ListEntry<IntString>* New = NULL;

	Str.format("cmp%d", c);
	while(NewGame.GetValue("composer", Str, TYPE_STRING, &Str))
	{
		New = Composer.Add();
		New->Data[LANG_JP] = Str;

		Str.format("cmp%d_en", c);
		if(!NewGame.GetValue("composer", Str, TYPE_STRING, &New->Data[LANG_EN]))	New->Data[LANG_EN] = New->Data[LANG_JP];

		Str.format("cmp%d", ++c);
	}

	if(!New)
	{
		New = Composer.Add();
		New->Data[LANG_JP] = Artist[LANG_JP];
		New->Data[LANG_EN] = Artist[LANG_EN];
	}
	// ---------

	// Track Info
	// ---------

	NewGame.GetValue("game", "tracks", TYPE_USHORT, &Tracks);

	for(c = 0; c < Tracks; c++)
	{
		NewTrack = &(Track.Add()->Data);
		NewTrack->Number = c + 1;

		TS = NewGame.FindSection(NewTrack->GetNumber());
		if(!TS)	continue;
		
		for(l = 0; l < LANG_COUNT; l++)
		{
			TS->LinkValue("name_" + BGMLib::LI[l].Code2, TYPE_STRING, &NewTrack->Name[l]);
			TS->LinkValue("comment_" + BGMLib::LI[l].Code2, TYPE_STRING, &NewTrack->Comment[l]);
		}
		if(NewTrack->Name[LANG_EN].empty())	NewTrack->Name[LANG_EN] = NewTrack->Name[LANG_JP];

		if(!TS->GetValue("composer", TYPE_USHORT, &NewTrack->CmpID))	NewTrack->CmpID = 1;
		NewTrack->CmpID--;
				
		ushort Cmt = 2;
		while(1)
		{
			Key.format("comment%d_%s", Cmt, BGMLib::LI[LANG_JP].Code2);
			if(!TS->GetValue(Key.text(), TYPE_STRING, &Str))	break;
		
			NewCmt = &NewTrack->Afterword.Add()->Data;
			NewCmt->s[LANG_JP].assign(Str);
			TS->LinkValue(Key.text(), TYPE_STRING, &NewCmt->s[LANG_JP], false);

			Key.replace(Key.length() - 2, 2, BGMLib::LI[LANG_EN].Code2.text(), 2);
			TS->LinkValue(Key.text(), TYPE_STRING, &NewCmt->s[LANG_EN]);

			Cmt++;
		}

		if(!PM->ParseTrackInfo(NewGame, this, TS, NewTrack))	continue;

		// Read position data
		// ------------------

		TS->GetValue("start", TYPE_ULONG, &NewTrack->Start[0]);

		// Absolute values
		TS->GetValue("abs_loop", TYPE_ULONG, &NewTrack->Loop);
		TS->GetValue("abs_end", TYPE_ULONG, &NewTrack->End);

		// Relative values
		if(TS->GetValue("rel_loop", TYPE_ULONG, &TempVal))	NewTrack->Loop = NewTrack->Start[0] + TempVal;
		if(TS->GetValue("rel_end", TYPE_ULONG, &TempVal))	NewTrack->End = NewTrack->Start[0] + TempVal;

		// Coolier array format
		if(TS->GetValue("position", TYPE_STRING, &Str))
		{
			ulong Val[3] = {0, 0, 0};
			FXint s = 0, t;
			ushort v;	// Substring ID

			for(v = 0; v < 3; v++)
			{
				if( (t = Str.find(',', s)) == -1)	t = Str.length();
				Val[v] = ParseSubstring(Str.mid(s, t-s).trim());
				s = t + 1;
			}

			// Found something?
			if(v > 1)
			{
				NewTrack->Start[0] = NewTrack->Start[1] = Val[0];
				NewTrack->Loop = Val[1] + Val[0];
				NewTrack->End = Val[2] + Val[0];
			}
		}
	}
	// ---------

	ParseTrackDataEx(NewGame);

	NewGame.Clear();
	return HaveTrackData = true;
}

bool GameInfo::Init(const FXString &P)
{
	FXString PrevPath;

	Path.assign(P);
	PrevPath = FXSystem::getCurrentDirectory();
	FXSystem::setCurrentDirectory(Path);

	if(!HaveTrackData)	ParseTrackData();
	if(PM)	PM->TrackData(this);

	// Print track position warning messages now
	TrackInfo* TI;
	ListEntry<TrackInfo>* CurTrack = Track.First();
	FXString Str;
	while(CurTrack)
	{
		TI = &CurTrack->Data;
		if(!TI->Start[0] && !TI->Loop && !TI->End)
		{
			Str.format("WARNING: Couldn't read track position data for #%s %s!\n", TI->GetNumber(), TI->Name[Lang]);
			BGMLib::UI_Stat(Str);
		}

		CurTrack = CurTrack->Next();
	}

	if(PM)
	{
		bool Ret = PM->SeekTest(this);

		if(Ret)
		{
			if(!Scanned)	PM->SilenceScan(this);
			return Ret;
		}
	}
	FXSystem::setCurrentDirectory(PrevPath);
	return true;
}

GameInfo::~GameInfo()
{
	Clear();
}
// --------
