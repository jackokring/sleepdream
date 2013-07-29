/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtWebKitWidgets/QWebView>
#include <QShortcut>
#include <QDebug>

#include "glwidget.h"
#include "window.h"

const char * Window::names[16] = {
    "AstroDefender",
    "Zap",
    "Carnage",
    "Grid",
    "TrainYard",
    "StuntBike",
    "GhostChess",
    "CascadeRipple",
    "Across",
    "Magnetris",
    "Simplatronics",
    "MakeWords",
    "MusicSynth",
    "SoundEvolver",
    "Calculator",
    "Extras"
};

Window::Window()
{
    glWidgets[0][0] = new GLWidget(0, 0, 0);
    glWidgets[0][1] = new GLWidget(0, 1, 0);
    glWidgets[0][2] = new GLWidget(0, 2, 0);
    glWidgets[0][3] = new GLWidget(0, 3, 0);

    glWidgets[1][0] = new GLWidget(0, 4, 0);
    glWidgets[1][1] = new GLWidget(0, 5, 0);
    glWidgets[1][2] = new GLWidget(0, 6, 0);
    glWidgets[1][3] = new GLWidget(0, 7, 0);

    glWidgets[2][0] = new GLWidget(0, 8, 0);
    glWidgets[2][1] = new GLWidget(0, 9, 0);
    glWidgets[2][2] = new GLWidget(0, 10, 0);
    glWidgets[2][3] = new GLWidget(0, 11, 0);

    glWidgets[3][0] = new GLWidget(0, 12, 0);
    glWidgets[3][1] = new GLWidget(0, 13, 0);
    glWidgets[3][2] = new GLWidget(0, 14, 0);
    glWidgets[3][3] = new GLWidget(0, 15, 0);

    mainLayout = new QGridLayout;
    mouseOn = mouse[0] = mouse[1] = false;

    for (int i = 0; i < NumRows; ++i) {
        for (int j = 0; j < NumColumns; ++j) {
            mainLayout->addWidget(glWidgets[i][j], i, j);

            connect(glWidgets[i][j], SIGNAL(clicked()),
                    this, SLOT(setCurrentGlWidget()));
        }
    }
    setLayout(mainLayout);

    currentGlWidget = glWidgets[0][0];

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
    timer->start(20);

    setWindowTitle(tr("Sleep Dream Games"));

    shortcut(QKeySequence(Qt::Key_Escape), SLOT(keyHandleWM()));
    shortcut(QKeySequence(Qt::Key_Tab), SLOT(keyHandleWP()));

    shortcut(QKeySequence(Qt::Key_Space), SLOT(keyHandleXM()));
    shortcut(QKeySequence(Qt::Key_Enter), SLOT(keyHandleXP()));

    shortcut(QKeySequence(Qt::Key_Up), SLOT(keyHandleYM()));
    shortcut(QKeySequence(Qt::Key_Down), SLOT(keyHandleYP()));

    shortcut(QKeySequence(Qt::Key_Left), SLOT(keyHandleZM()));
    shortcut(QKeySequence(Qt::Key_Right), SLOT(keyHandleZP()));

    shortcut(QKeySequence(Qt::Key_Alt), SLOT(mouseFlip()));

    j = new Joystick();

    connect(j,SIGNAL(keyHandleWM()), this , SLOT(keyHandleWM()));
    connect(j,SIGNAL(keyHandleWP()), this , SLOT(keyHandleWP()));
    connect(j,SIGNAL(keyHandleXM()), this , SLOT(keyHandleXM()));
    connect(j,SIGNAL(keyHandleXP()), this , SLOT(keyHandleXP()));

    connect(j,SIGNAL(keyHandleYM()), this , SLOT(keyHandleYM()));
    connect(j,SIGNAL(keyHandleYP()), this , SLOT(keyHandleYP()));
    connect(j,SIGNAL(keyHandleZM()), this , SLOT(keyHandleZM()));
    connect(j,SIGNAL(keyHandleZP()), this , SLOT(keyHandleZP()));

    QTimer *hot = new QTimer(this);
    connect(hot, SIGNAL(timeout()), this, SLOT(hotPlug()));
    hot->start(1000);
}

bool Window::eventFilter(QObject *object, QEvent *event)
 {
    if(!mouseOn || object != this) return false;
     if (event->type() == QEvent::MouseButtonPress) {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
         if (mouseEvent->buttons() & Qt::LeftButton) {
             mouse[0] = true;
             return true;
         }
         if (mouseEvent->buttons() & Qt::RightButton) {
             mouse[1] = true;
             return true;
         }
     }
     if (event->type() == QEvent::MouseButtonRelease) {
         QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
         if (mouseEvent->buttons() & Qt::LeftButton) {
             mouse[0] = false;
             return true;
         }
         if (mouseEvent->buttons() & Qt::RightButton) {
             mouse[1] = false;
             return true;
         }
     }
     return false;
 }

void Window::mouseFlip() {
    mouseOn = !mouseOn;
    mouse[0] = mouse[1] = 0;
}

void Window::hotPlug() {
    if(j->m_run) return;
    j->close();
    j->open(new QString("/dev/input/js0"), this);
}

void Window::shortcut(QKeySequence key, const char * execThis)
{
    QShortcut *shortcut = new QShortcut(key, this);
    shortcut->setContext(Qt::ApplicationShortcut);
    connect(shortcut, SIGNAL(activated()), this, execThis);
    wxyz[0] = wxyz[1] = wxyz[2] = wxyz[3] = 0;
}

void Window::keyHandleWM()
{
    wv = new QWebView(this);
    keyHandle(0, -1);
    if(wxyz[0] == -1) {
        mainLayout->addWidget(wv, 0, 0, 4, 4);
        wv->setUrl(QUrl("qrc:/docs.html"));
    }
    if(wxyz[0] < -1) close();//exit on escape!
    if(wxyz[0] == 0) {
        for (int i = 0; i < NumRows; ++i) {
            for (int k = 0; k < NumColumns; ++k) {
                mainLayout->addWidget(glWidgets[i][k], i, k);
                glWidgets[i][k]->show();
            }
        }
        currentGlWidget->play(false);
    }
}

void Window::keyHandleWP()
{
    keyHandle(0, 1);
    if(wxyz[0] == 0) delete wv;
    if(wxyz[0] == 1) {
        GLWidget *idx = currentGlWidget;
        for (int i = 0; i < NumRows; ++i) {
            for (int k = 0; k < NumColumns; ++k) {
                if(idx != glWidgets[i][k]) glWidgets[i][k]->hide();
            }
        }
        mainLayout->addWidget(idx, 0, 0, 4, 4);
        this->repaint();//1st draw
    }
    if(wxyz[0] == 2) {
        currentGlWidget->play(true);
        wxyz[0]--;
    }
}

void Window::keyHandleXM()
{
    keyHandle(1, -1);
}

void Window::keyHandleXP()
{
    keyHandle(1, 1);
}

void Window::keyHandleYM()
{
    keyHandle(2, -1);
}

void Window::keyHandleYP()
{
    keyHandle(2, 1);
}

void Window::keyHandleZM()
{
    keyHandle(3, -1);
}

void Window::keyHandleZP()
{
    keyHandle(3, 1);
}

void Window::keyHandle(int idx, int inc)
{
    wxyz[idx] += inc;
    if( (!currentGlWidget->playDo) && wxyz[0] != 0) {//reset events
        wxyz[1] = wxyz[2] = wxyz[3] = 0;
    }
    //force an emit of click on one selection
    if(wxyz[0] == 0) glWidgets[(unsigned int)(wxyz[2])%NumRows]
            [(unsigned int)(wxyz[3])%NumColumns]->clickProxy();
}

void Window::setCurrentGlWidget()
{
    currentGlWidget = qobject_cast<GLWidget *>(sender());
}

void Window::rotateOneStep()
{
    if (currentGlWidget)
        currentGlWidget->rotateBy(+2 * 16, +2 * 16, -1 * 16);
}
