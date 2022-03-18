#include "editor.h"
#include <algorithm>
#include <bits/types/FILE.h>
#include <bits/types/mbstate_t.h>
#include <cstddef>
#include <ncursesw/curses.h>
#include <ncursesw/panel.h>

Editor::Editor()
{

}

Editor::Editor(const bool& bColors)
{
	initscr();
	cbreak();
	noecho();
	curs_set(1);

	if (bColors) {
		start_color();
		init_pair(1, COLOR_FG, COLOR_BG);
		init_pair(2, COLOR_BG, COLOR_FG);
	}

	clear();
	setlocale(LC_ALL, "");
	initPanes();

	wmove(panel_window(_pPanes[BASE_PANE].p), 1, 1);
	update_panels();
	doupdate();
}

Editor::~Editor() {
	for (auto& pane : _pPanes) {
		del_panel(pane.p);
		pane.p = NULL;
	}

	endwin();
}

void Editor::initPanes() {
	//Initialize Base Pane--------------------------------------------------------------------------------------
	int scrWidth, scrHeight;
	getmaxyx(stdscr, scrHeight, scrWidth);
	_pPanes[BASE_PANE].cPos.x = 0;
	_pPanes[BASE_PANE].cPos.y = 0;
	_pPanes[BASE_PANE].dDim.x = scrWidth;
	_pPanes[BASE_PANE].dDim.y = scrHeight;
	_pPanes[BASE_PANE].w = newwin(_pPanes[BASE_PANE].dDim.y, _pPanes[BASE_PANE].dDim.x,
								  _pPanes[BASE_PANE].cPos.y, _pPanes[BASE_PANE].cPos.x);
	_pPanes[BASE_PANE].p = new_panel(_pPanes[BASE_PANE].w);

	keypad(panel_window(_pPanes[BASE_PANE].p), true);
	wbkgd(panel_window(_pPanes[BASE_PANE].p), COLOR_PAIR(1));
	wattrset(panel_window(_pPanes[BASE_PANE].p), COLOR_PAIR(1));
	box(panel_window(_pPanes[BASE_PANE].p), 0, 0);

	show_panel(_pPanes[BASE_PANE].p);

	//Initialize Option Panes-----------------------------------------------------------------------------------
	Coord currPaneStart = {SPACES_BETWEEN_PANE, 1};
	unsigned long longestOptionLen = 0;
	std::vector<std::wstring> paneNames = {L"File", L"Edit", L"Help"};

	_pPanes[FILE_PANE].vItems = std::vector<Item>({	{ OPEN,    0 },
													{ SAVE,    1 },
													{ SAVE_AS, 2 },
													{ QUIT,    3 }});

	_pPanes[EDIT_PANE].vItems = std::vector<Item>({ { COPY,    0 },
													{ PASTE,   1 },
													{ FIND,    2 },
													{ REPLACE, 3 },
													{ UNDO,    4 },
													{ REDO,    5 }});

	_pPanes[HELP_PANE].vItems = std::vector<Item>({	{ ABOUT,   0 },
													{ WEBSITE, 1 }});

	for (int i = 1; i < TOTAL_PANES; i++) {
		_pPanes[i].sHeader = paneNames[i-1];
		mvwaddwstr(panel_window(_pPanes[BASE_PANE].p), 0, currPaneStart.x + 1, _pPanes[i].sHeader.c_str());
		_pPanes[i].cPos.x = currPaneStart.x;
		_pPanes[i].cPos.y = currPaneStart.y;

		for (size_t j = 1; j < _pPanes[i].vItems.size(); j++)
			 longestOptionLen = std::max(longestOptionLen, _pPanes[i].vItems[j].sName.length());

		_pPanes[i].lLongestOptLen = longestOptionLen;
		_pPanes[i].dDim.x = _pPanes[i].lLongestOptLen + 2; //Add 2 to width for boxes on each side
		_pPanes[i].dDim.y = _pPanes[i].vItems.size() + 2; //Add 2 to height for the box on the top/bottom

		_pPanes[i].w = newwin(_pPanes[i].dDim.y, _pPanes[i].dDim.x,
									  _pPanes[i].cPos.y, _pPanes[i].cPos.x);
		_pPanes[i].p = new_panel(_pPanes[i].w);

		_pPanes[i].pPrev = (i != 1)				  ? &_pPanes[i-1] : _pPanes.end() - 1;
		_pPanes[i].pNext = (i != TOTAL_PANES - 1) ? &_pPanes[i+1] : &_pPanes[1];

		wattrset(panel_window(_pPanes[i].p), COLOR_PAIR(2));
		box(panel_window(_pPanes[i].p), 0, 0 ); // Remove top char from border

		//Populate the pane with the options
		for (size_t row = 0; row < _pPanes[i].vItems.size(); row++) {
			mvwchgat(panel_window(_pPanes[i].p), _pPanes[i].cPos.y + row, 1, //Color options in reverse color scheme
					 _pPanes[i].lLongestOptLen, A_NORMAL, 2, NULL);
			waddwstr(panel_window(_pPanes[i].p), _pPanes[i].vItems[row].sName.c_str());
			//wprintw(panel_window(_pPanes[i].p), "%ls\n", _pPanes[i].vItems[row].sName.c_str());

		}
		hide_panel(_pPanes[i].p);

		longestOptionLen = 0;
		currPaneStart.x += _pPanes[i].dDim.x + SPACES_BETWEEN_PANE;
	}
	//-----------------------------------------------------------------------------------------------------------

	//Update Panels
	update_panels();
	doupdate();
}

void Editor::Run() {
	wint_t in;
	_bisRunning = true;

	while(_bisRunning) {
		wget_wch(panel_window(_pPanes[BASE_PANE].p), &in);

		switch(in) {
			case L'\t':

				toggleMenuActive();
				break;
			case KEY_LEFT:
				if (_bMenuIsActive)
					paneLeft();
				break;
			case KEY_RIGHT:
				if (_bMenuIsActive)
					paneRight();
				break;
			case KEY_UP:
				if (_bMenuIsActive)
					paneUp();
				break;
			case KEY_DOWN:
				if (_bMenuIsActive)
					paneDown();
				break;
			case L'Q':
				_bisRunning = false;
			case L'\n':
				if (_bMenuIsActive) {
					callSelectedOption();
				}
			default:
				break;
		}
		update_panels();
		doupdate();

	}
	clear();
}

void Editor::Open()
{
	wclear(panel_window(_pPanes[BASE_PANE].p));


}

void Editor::Save(const std::wstring& sFilename)
{
	if (sFilename == L"") {

	}
	//Save
}

void Editor::Copy()
{

}

void Editor::Paste()
{

}

void Editor::Find()
{

}

void Editor::Replace()
{

}

void Editor::Undo()
{

}

void Editor::Redo()
{

}

void Editor::About()
{

}

void Editor::Website()
{

}



//Horribly unelegant, eventually want to shift to storing callback functions
//in the menu options themselves
void Editor::callSelectedOption() {
	if (!_pActivePane)
		return;

	std::wstring option = _pActivePane->vItems[_pActivePane->iActiveItemIndex].sName;
	toggleMenuActive();

	//File Options
	if (option == OPEN) {
		Open();
	}
	else if (option == SAVE) {
		Save(_sFileName);
	}
	else if (option == SAVE_AS) {
		Save();
	}
	else if (option == QUIT) {
		_bisRunning = false;
		return;
	}
	//Edit Options
	else if (option == COPY) {
		Copy();
	}
	else if (option == PASTE) {
		Paste();
	}
	else if (option == FIND) {
		Find();
	}
	else if (option == REPLACE) {
		Replace();
	}
	else if (option == UNDO) {
		Undo();
	}
	else if (option == REDO) {
		Redo();
	}

	//Help
	else if (option == ABOUT) {
		About();
	}
	else if (option == WEBSITE) {
		Website();
	}
}

void Editor::closeActivePane() {
	if (_pActivePane) {

		//Unhighlight option header
		mvwchgat(panel_window(_pPanes[BASE_PANE].p), _pActivePane->cPos.y - 1, _pActivePane->cPos.x + 1,
				 _pActivePane->sHeader.length(), A_NORMAL, 1, NULL);
		//Unhighlight active option
		mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
				 _pActivePane->lLongestOptLen, A_NORMAL, 2, NULL);

		doupdate();
		update_panels();
		_pActivePane->iActiveItemIndex = 0;
		hide_panel(_pActivePane->p);
	}
}

void Editor::setActivePane(const unsigned int& index) {
	if (index >= TOTAL_PANES)
		return;
	if (_pActivePane)
		closeActivePane();

	_pActivePane = &_pPanes[index];
	show_panel(_pActivePane->p);
	//Highlight option header
	mvwchgat(panel_window(_pPanes[BASE_PANE].p), 0, _pActivePane->cPos.x + 1,
			 _pActivePane->sHeader.length(), A_NORMAL, 2, NULL);

	//wmove(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1);
	mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
			 _pActivePane->lLongestOptLen, A_NORMAL, 1, NULL);
}

void Editor::toggleMenuActive(){
	if (!_bMenuIsActive) {
		curs_set(0);
		setActivePane(BASE_PANE + 1);
	}
	else {
		closeActivePane();
		curs_set(1);
		wmove(panel_window(_pPanes[BASE_PANE].p), _cCurPos.y, _cCurPos.x);
	}
	_bMenuIsActive = !_bMenuIsActive;
}

void Editor::paneLeft() {
	closeActivePane();
	_pActivePane = _pActivePane->pPrev;
	show_panel(_pActivePane->p);
	//Highlight Header
	mvwchgat(panel_window(_pPanes[BASE_PANE].p), 0, _pActivePane->cPos.x + 1,
			 _pActivePane->sHeader.length(), A_NORMAL, 2, NULL);
	//Highlight active option
	mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
			 _pActivePane->lLongestOptLen, A_NORMAL, 1, NULL);

	update_panels();
	doupdate();
}

void Editor::paneRight() {
	closeActivePane();
	_pActivePane = _pActivePane->pNext;
	show_panel(_pActivePane->p);
	//Highlist Header
	mvwchgat(panel_window(_pPanes[BASE_PANE].p), 0, _pActivePane->cPos.x + 1,
			 _pActivePane->sHeader.length(), A_NORMAL, 2, NULL);
	//Highlight active option
	mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
			 _pActivePane->lLongestOptLen, A_NORMAL, 1, NULL);

	update_panels();
	doupdate();
}

void Editor::paneUp() {
	if (!panel_hidden(_pActivePane->p)) {
		if (_pActivePane->iActiveItemIndex > 0)
			_pActivePane->iActiveItemIndex--;

		//Highlight active option
		mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
							  _pActivePane->lLongestOptLen, A_NORMAL, 1, NULL);
		//UnHighlight previous option
		mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 2, 1,
				 _pActivePane->lLongestOptLen, A_NORMAL, 2, NULL);
	}
}

void Editor::paneDown() {
	if (!panel_hidden(_pActivePane->p)) {
		if (_pActivePane->iActiveItemIndex < _pActivePane->vItems.size() - 1)
			_pActivePane->iActiveItemIndex++;

		//Highlight active option
		mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex + 1, 1,
							  _pActivePane->lLongestOptLen, A_NORMAL, 1, NULL);
		//UnHighlight previous option
		mvwchgat(panel_window(_pActivePane->p), _pActivePane->iActiveItemIndex, 1,
				 _pActivePane->lLongestOptLen, A_NORMAL, 2, NULL);
	}
}
