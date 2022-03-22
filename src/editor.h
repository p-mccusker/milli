#ifndef EDITOR_H
#define EDITOR_H

#include <cstddef>
#include <fstream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <functional>
#include <locale.h>
#include <locale>
#include <codecvt>
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>

#define BORDER_PANE 0
#define OPEN_PANE 1
#define SAVE_PANE 2
#define BASE_PANE 3
#define FILE_PANE 4
#define EDIT_PANE 5
#define HELP_PANE 6
#define TOTAL_PANES 7

#define SPACES_BETWEEN_PANE 2

#define COLOR_FG COLOR_CYAN
#define COLOR_BG COLOR_BLACK

#define COLOR_PAIR_NORMAL 1
#define COLOR_PAIR_REVERSE 2
#define COLOR_PAIR_DIRECTORY 3
#define COLOR_PAIR_FILE 4

//File Options
#define OPEN	L"Open"
#define SAVE	L"Save"
#define SAVE_AS L"Save As..."
#define QUIT	L"Quit"

//Edit Options
#define COPY	L"Copy"
#define PASTE	L"Paste"
#define FIND	L"Find"
#define REPLACE	L"Replace"
#define UNDO	L"Undo"
#define REDO	L"Redo"

//Help Options
#define ABOUT	L"About"
#define WEBSITE L"Website"

//setup converter
using convert_type = std::codecvt_utf8<wchar_t>;
static std::wstring_convert<convert_type, wchar_t> converter;

inline std::string toString(const std::wstring& str) {
	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes( str );
}

inline std::wstring toWString(const std::string& str) {
	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.from_bytes( str );
}


#if __cplusplus >= 201703L
#include <filesystem>

inline std::vector<std::filesystem::directory_entry> fileList(const std::wstring& directory) {
	std::vector<std::filesystem::directory_entry> files;
	for (const auto & entry : std::filesystem::directory_iterator(directory))
			files.push_back(entry);
	return files;
}
#else
#include <dirent.h>
#include <cstdio>

inline std::vector<std::wstring> fileList(const std::wstring& directory) {
	DIR *d;
	struct dirent *dir;
	std::vector<std::wstring> files;
	auto str = toString(directory);

	d = opendir(str.c_str());
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			files.push_back(toWString(dir->d_name));
	}
	closedir(d);

	}
	return files;
}

#endif



template <typename T>
struct Vec2D {
	Vec2D() { }
	Vec2D(T xC, T yC) :x(xC), y(yC) { }
	T x, y;
};

using Coord = Vec2D<unsigned int>;
using Dimension = Vec2D<unsigned int>;

class Editor;

struct Item {
	std::wstring sName = L"";
	unsigned int iIndex = 0;
};

struct Pane {
	Pane					  *pNext = NULL,
							  *pPrev = NULL;
	WINDOW*					  w = NULL;
	PANEL*					  p = NULL;
	std::wstring			  sHeader = L"";
	Coord					  cPos = {0, 0};
	Dimension				  dDim = {0 , 0};
	unsigned int			  iActiveItemIndex = 0;
	unsigned long			  lLongestOptLen = 0;
	std::vector<Item> vItems;
};

using fileEntry = std::filesystem::directory_entry;

class Editor
{
public:
	Editor();
	Editor(const bool& bColors=true);
	~Editor();

	void Run();

	void Open();
	void Save(const std::wstring& sFilename=L"");

	void Copy();
	void Paste();
	void Find();
	void Replace();
	void Undo();
	void Redo();

	void About();
	void Website();

private:

	void initPanes();
	void initScreens(const unsigned int& scrWidth, const unsigned int& scrHeight);
	void paneLeft();
	void paneRight();
	void paneUp();
	void paneDown();
	void toggleMenuActive();
	void setActivePane(const unsigned int& iIndex);
	void closeActivePane();
	void callSelectedOption();

	void switchScreens(const int&  scr);
	void listFiles(const std::vector<fileEntry>& files, const int& scr);

	void ReDraw();
private:
	std::array<Pane, TOTAL_PANES> _pPanes;
	std::vector<std::wstring> _vBuffer;
	std::wstring _sFileName,
				 _sVersion = L"Version 0.1 Alpha",
				 _sDir = L"/home/peter/";
	std::ifstream _fActiveFile;
	Coord _cCurPos = {0, 0};
	Pane* _pActivePane = NULL;
	bool _bMenuIsActive = false,
		 _bisRunning = false;


};
#endif // EDITOR_H
