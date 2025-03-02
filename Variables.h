#pragma once
#include <string>
#include <vector>
#include "imgui.h"
#include "Styling.h"

#define MAX_ITEMS 32

enum DialogReturn : int32_t
{
	RETURN_NONE = 0,
	RETURN_OK,
	RETURN_NO
};

enum MenuTabs : int32_t
{
	MENU_MAIN = 0,

#ifdef ENABLE_CONFIG
	MENU_CONFIG
#endif
};

enum DurTypes : int32_t
{
	DUR_LENGTH = 0,
	DUR_TIMECODE
};

static const char* DurTypeNames[] =
{
	"Length",
	"Timecode"
};

class FontVariable
{
public:
	FontVariable(const std::pair<std::string, std::string>& t1, int32_t fs, int32_t dms) :
		PrettyName(t1.first), FilePath(t1.second), FontSize(fs), DurationMS(dms)
	{	}

	std::string PrettyName;
	std::string FilePath;
	int32_t FontSize = 64;
	int32_t DurationMS = 250;
};

class TextVariable
{
public:
	TextVariable(const char* t1, int32_t dur, int32_t tc, ImVec4 col) :
		Text(t1), DurationMS(dur), TimecodeMS(tc), Color(col)
	{	}

	std::string Text;
	int32_t DurationMS = 1000;
	int32_t TimecodeMS = 0;
	ImVec4 Color = col_text;
};

//custom variables for the menu go here
//remember to add each variable to CConfig::Setup()
//	if you want to save those to a config file
struct Variables
{
	Variables()
	{
#ifdef ENABLE_CONFIG
		Menu = Menu_v();
#endif
		LyricVars = LyricVars_v();
		LyricVars.SelectedFonts.reserve(MAX_ITEMS);
		LyricVars.SelectedTexts.reserve(MAX_ITEMS);
	}

	//example of custom variables
	struct LyricVars_v
	{
		bool UpdateFonts = true;
		bool ShouldUpdateFonts = false;
		bool ResetDrawing = true;
		int32_t DefaultFontSize = 64;
		int32_t DefaultFontDuration = 250;
		int32_t DefaultTextDuration = 1000;
		int32_t DurType = DUR_LENGTH;

		uint32_t FontSelected = 0;
		uint32_t FontAvailSelected = 0;
		uint32_t TextSelected = 0;

		uint32_t FontToUse = 0;
		uint32_t TextToUse = 0;

		std::vector<FontVariable> SelectedFonts{};
		std::vector<std::pair<std::string, std::string>> AvailableFonts{};
		std::vector<ImFont*> LoadedFonts{};
		std::vector<TextVariable> SelectedTexts{};
		char CustomText[128];

	} LyricVars{};

	//config-specific system variables
	struct Menu_v
	{
		MenuTabs MenuTab = MENU_MAIN;
		bool MenuEnabled = true;

#ifdef ENABLE_CONFIG
		int32_t		ConfigFile = 0;
		std::string AssignedConfig = "";
		char	ConfigName[128] = "";

		bool DeleteBox = false;
		bool LoadBox = false;
		bool SaveBox = false;
		bool NewConfigBox = false;

		bool ChangedWindowSize = false;
		bool BorderlessWindowed = false;
#endif
	} Menu{};

};

extern Variables g_Options;
