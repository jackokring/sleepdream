HEADERS       = glwidget.h \
                window.h
SOURCES       = glwidget.cpp \
                main.cpp \
                window.cpp \
                alsa.c
RESOURCES     = game1.qrc
QT           += opengl
DEFINES += HAS_ALSA
LIBS_PRIVATE += -lasound
