// Music Room BGM Library
// ----------------------
// bgmlib.cpp - BGM Library main class
// ----------------------
// "©" Nmlgc, 2011

#include "platform.h"
#include "list.h"
#include "config.h"
#include "infostruct.h"
#include "bgmlib.h"
#include "ui.h"
#include "packmethod.h"
#include "utils.h"

#include <FXPath.h>
#include <FXIO.h>
#include <FXDir.h>
#include <FXSystem.h>

ushort Lang;	// Current language

namespace BGMLib
{
	// String constants
	// ----------------
	const FXString Trial[LANG_COUNT] = {L" 体験版", " (Trial)"};
	const FXString GNDelim[2] = {L"「",  L"」"};
	const FXString WriteError = "Couldn't get write access to ";
	// ----------------

	// Values read from a config file
	// -----
	// [bgmlib]
	FXString InfoPath;	// BGM info file directory
	// -----

	List<PackMethod*>	PM;	// Supported pack methods
	List<GameInfo>	Game;	// Supported games
}

bool BGMLib::Init(ConfigFile *Cfg, FXString CfgPath, bool DefaultPM)
{
	ConfigParser* Lib;
	
	if(DefaultPM)
	{
		// Invoke PM_None
		PM_None& Invoke = PM_None::Inst();
	}

	SetupLang();

	if(!Cfg)	return false;

	if(Lib = Cfg->FindSection("bgmlib"))
	{
		Lib->LinkValue("lang", TYPE_USHORT, &Lang);
		Lib->GetValue("infopath", TYPE_STRING, &InfoPath);
	}
	InfoPath = absolutePath(CfgPath, InfoPath);
	
	return true;
}

bool BGMLib::LoadBGMInfo()
{
	FXString* BGM;
	FXint	BGMCount;
	FXString PrevPath;
	GameInfo* New;

	if(InfoPath.empty())	return false;

	BGMCount = FXDir::listFiles(BGM, InfoPath, "*.bgm");

	if(BGMCount == 0)
	{
		UI_Error("No BGM info files found in " + InfoPath + "!\nGet some and restart this application.\n");
		return false;
	}

	Game.Clear();

	PrevPath = FXSystem::getCurrentDirectory();
	FXSystem::setCurrentDirectory(InfoPath);

	UI_Stat("---------------------------\nSupported:\n");

	for(FXint c = 0; c < BGMCount; c++)
	{
		New = &(Game.Add()->Data);
		if(!New->ParseGameData(BGM[c]))	Game.PopLast();
		BGM[c].clear();
	}
	SAFE_DELETE_ARRAY(BGM);

	UI_Stat("---------------------------\n");

	FXSystem::setCurrentDirectory(PrevPath);

	return true;
}

GameInfo* BGMLib::ScanGame(const FXString& Path)
{
	GameInfo* NewGame = NULL;
	FXString Str;
	FXString PrevPath;

	PrevPath = FXSystem::getCurrentDirectory();

	FXSystem::setCurrentDirectory(Path);

	UI_Stat("Scanning " + Path + "...\n");
	UI_Stat("------------------------\n");

	// Scan the pack methods in reverse order.
	// The latter ones tend to be more advanced.
	ListEntry<PackMethod*>* CurPM = PM.Last();

	while(CurPM)
	{
		if(NewGame = CurPM->Data->Scan(Path))	break;
		CurPM = CurPM->Prev();
	}

	if(NewGame != NULL)
	{
		Str.format("Identified %s\n", NewGame->DelimName(Lang));
		UI_Stat(Str);
	}
	else
	{
		BGMLib::UI_Stat("No compatible game found!\n");
	}

	BGMLib::UI_Stat("------------------------\n");

	FXSystem::setCurrentDirectory(PrevPath);

	return NewGame;
}

void BGMLib::Clear()
{
	for(ushort c = 0; c < LANG_COUNT; c++)
	{
		LI[c].Clear();
	}
	PM.Clear();
	Game.Clear();
}
