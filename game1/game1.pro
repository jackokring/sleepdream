HEADERS       = glwidget.h \
                window.h \
                names.h \
    Joystick.h
SOURCES       = glwidget.cpp \
                main.cpp \
                window.cpp \
                alsa.c \
    Joystick.cpp
RESOURCES     = game1.qrc
QT           += opengl core gui webkitwidgets
DEFINES += HAS_ALSA
LIBS_PRIVATE += -lasound -lc

OTHER_FILES += \
    docs.html \
    images/MusicSynth.png \
    images/MakeWords.png \
    images/Magnetris.png \
    images/Grid.png \
    images/GhostChess.png \
    images/Extras.png \
    images/CascadeRipple.png \
    images/Carnage.png \
    images/Calculator.png \
    images/AstroDefender.png \
    images/Across.png \
    images/Zap.png \
    images/TrainYard.png \
    images/StuntBike.png \
    images/SoundEvolver.png \
    images/Simplatronics.png
