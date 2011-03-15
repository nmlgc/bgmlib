// Music Room BGM Library
// ----------------------
// ui.h - User interface function definitions
// ----------------------
// "©" Nmlgc, 2011

#ifndef BGMLIB_UI_H
#define BGMLIB_UI_H

namespace BGMLib
{
	// These are _not_ implemented in BGMLib, the calling program has to provide an own implementation.
	// ------------------------
	// Status messages
	void UI_Stat(const FXString& Msg);
	void UI_Stat_Safe(const FXString& Msg);

	// Error messages
	void UI_Error(const FXString& Msg);
	void UI_Error_Safe(const FXString& Msg);

	// BGM file notice (personal appeal)
	void UI_Notice(const FXString& Msg);

	// Wiki update messages. [Msg] contains a caption, [Old] and [New] the respective strings.
	// Should offer selections and return a value according to the UPDATE_* values below
	uint UI_Update(const FXString& Msg, const FXString& Old, const FXString& New);
}

// Selections/return values for BGMLib::UI_Update
enum : uint
{
  UPDATE_YES      = 1,            /// The YES button was clicked
  UPDATE_NO       = 2,            /// The NO button was clicked
  UPDATE_CANCEL   = 4,            /// The CANCEL button was clicked
  UPDATE_YESALL   = 9,            /// The YES ALL button was clicked
  UPDATE_NOALL    = 10            /// The NO ALL button was clicked
};

#endif /* BGMLIB_UI_H */
