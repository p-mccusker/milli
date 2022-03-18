#ifndef EDITOR_H
#define EDITOR_H

#include <bits/types/FILE.h>
#include <cstddef>
#include <fstream>
#include <vector>
#include <memory>
#include <array>
#include <string>
#include <functional>
#include <locale.h>
#include <locale>
#include <cwchar>
#include <ncursesw/ncurses.h>
#include <ncursesw/panel.h>

#define BASE_PANE 0
#define FILE_PANE 1
#define EDIT_PANE 2
#define HELP_PANE 3
#define TOTAL_PANES 4

#define SPACES_BETWEEN_PANE 2

#define COLOR_FG COLOR_RED
#define COLOR_BG COLOR_BLACK

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
	void paneLeft();
	void paneRight();
	void paneUp();
	void paneDown();
	void toggleMenuActive();
	void setActivePane(const unsigned int& iIndex);
	void closeActivePane();
	void callSelectedOption();
private:
	std::array<Pane, TOTAL_PANES> _pPanes;
	std::vector<std::wstring> _vBuffer;
	std::wstring _sFileName,
				 _sVersion = L"Version 0.1 Alpha",
				 _sDir = L"~";
	std::ifstream _fActiveFile;
	Coord _cCurPos = {1, 1};
	Pane* _pActivePane = NULL;
	bool _bMenuIsActive = false,
		 _bisRunning = false;


};
#endif // EDITOR_H
