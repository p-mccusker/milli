TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -march=native -pipe -std=c++17

HEADERS += \
	src/editor.h

SOURCES += \
        src/editor.cpp \
        src/main.cpp

unix|win32: LIBS += -lncursesw

unix|win32: LIBS += -lpanelw

unix|win32: LIBS += -ltinfow
