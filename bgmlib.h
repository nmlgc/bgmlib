// Music Room BGM Library
// ----------------------
// bgmlib.h - BGM Library main class
// ----------------------
// "©" Nmlgc, 2011

// And no, I'm not calling it "Voile", just because it's a library ololol. Fuck you, EoSD fanboys.

/** The using program has to take care of the following:

	- Provide implementations for the UI_* functions.
	- Call Init() and LoadBGMInfo(), and Clear() before exiting the program.
	- Provide a config file (not mandatory, but highly recommended. <InfoPath> will else default to the application directory.)
**/

#pragma once

#ifndef BGMLIB_H
#define BGMLIB_H

// Forward declarations
struct GameInfo;
class ConfigFile;
class PackMethod;

extern ushort Lang;	// Current language

namespace BGMLib
{
	// This will all be changed yet again once people want more i18n... -.-
	extern LangInfo LI[LANG_COUNT];

	// String constants
	// ----------------
	extern const FXString Trial[LANG_COUNT];
	extern const FXString WriteError;
	extern const FXString GNDelim[2];
	// ----------------

	// Values read from a config file
	// -----
	// [bgmlib]
	extern FXString InfoPath;	// BGM info file directory
	// -----

	extern List<PackMethod*>	PM;	// Supported pack methods
	extern List<GameInfo>	Game;	// Supported games

	// Fills <LI> with language information
	void SetupLang();

	// Loads BGMLib config data from [Cfg] ([bgmlib] and [update] sections)
	// [DefaultPM] specifies whether a fallback pack method should be registered if something is not supported
	bool Init(ConfigFile* Cfg, FXString CfgPath = "", bool DefaultPM = true);
	
	// Fills <Game> with the *.bgm files in [InfoPath]
	bool LoadBGMInfo();

	// Convenience function to search for [PackMethod] in <PM>.
	// PM_None is returned if no fitting pack method is found!
	PackMethod* FindPM(const short& PackMethod);

	// Scans [Path] for a supported game and returns it.
	GameInfo* ScanGame(const FXString& Path);

	void Clear();
};

#endif /* BGMLIB_H */
