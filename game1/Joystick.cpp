/* Copyright (C) 2008 Andreas Volz (Adaptation by Sleep Dream Games)

Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the “Software”), to
 deal in the Software without restriction, including without limitation the
 rights to use, copy, *modify*, merge, publish, distribute, sublicense, and/or
 sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
 all copies of the Software and its Copyright notices. In addition publicly
 documented acknowledgment must be given that this software has been used if no
 source code of this software is made available publicly. This includes
 acknowledgments in either Copyright notices, Manuals, Publicity and Marketing
 documents or any documentation provided with any product containing this
 software. This License does not apply to any software that links to the
 libraries provided by this software (statically or dynamically), but only to
 the software provided.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <fcntl.h>
#include <QString>
#include <QTimer>
#include <QtGui>
#include <QtGlobal>
#include <QCursor>
#include <QDebug>
#include "Joystick.h"

Window * Joystick::mouseState = NULL;
long Joystick::count = 0;
long Joystick::mouseCount = 0;

Joystick::Joystick () : QObject()
{
  m_run = false;

  lastJoy[0] = lastJoy[1] = 0;
  joyBut[0] = joyBut[1] = false;

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
  timer->start(200);
}

Joystick::~Joystick ()
{
  this->close ();
}

int rdr(int m_fd, void *jev, unsigned int size) {
    if(Joystick::mouseCount > 3) {
        Joystick::mouseCount = 0;
        return 0;
    }
    js_event *je = (js_event *)jev;
    je->type = JS_EVENT_AXIS;
    je->number = Joystick::mouseCount&1;
    switch(Joystick::mouseCount&3) {
    case 0: je->value = (long)(QCursor::pos().x() * 65536) / (Joystick::mouseState->geometry().width())
                - 32768; break;
    case 1: je->value = (long)(QCursor::pos().y() * 65536) / (Joystick::mouseState->geometry().height())
                - 32768; break;

    default: //if(!Joystick::mouseState->mouse[3-(Joystick::mouseCount&3)]) return 0;
        //else emit //and reset
        je->type = JS_EVENT_BUTTON;
        je->number = Joystick::mouseCount-1;
        je->value = Joystick::mouseState->mouse[3-(Joystick::mouseCount&3)];
        //Joystick::mouseState->mouse[3-(Joystick::mouseCount&3)] = false;
        break;
    }
    Joystick::mouseCount++;
    return size;
}

bool Joystick::open (QString *device, Window *widget)
{
    Joystick::mouseState = widget;
  m_fd = ::open (device->toStdString().c_str(), O_RDONLY | O_NONBLOCK);
  if (m_fd == -1)
  {
    //cerr << "Error opening joystick device!" << endl;
    
    return false;
  }
  else
  {
    m_run = true;
  }

  return true;
}

bool Joystick::close ()
{
  m_run = false;
  lastJoy[0] = lastJoy[1] = 0;
  // end thread
  
  return !::close (m_fd);
}

void Joystick::loop ()
{
  // wait for all synthetic event until the first real event comes
  // then we've all available axis and buttons.
    //qDebug() << "ok";
    if(Joystick::mouseState && Joystick::mouseState->mouseOn) {
        rdrf = rdr;
        //Joystick::mouseState->setCursor(Qt::CrossCursor);
    } else {
        rdrf = read;
        //if(Joystick::mouseState) Joystick::mouseState->setCursor(Qt::ClosedHandCursor);
        if(!m_run) return;
    }
    while((*rdrf) (m_fd, &joy_event, sizeof(struct js_event)) > 0) {

        eventJoy.time = joy_event.time;
        eventJoy.value = joy_event.value;
        eventJoy.number = joy_event.number;

        switch (joy_event.type & ~JS_EVENT_INIT)
        {
        case JS_EVENT_BUTTON:
              switch(eventJoy.number) {
              case 0: if(eventJoy.value) emit keyHandleWP(); break;
              case 1: //if(eventJoy.value) emit keyHandleXP(); break;
                  joyBut[0] = eventJoy.value;
              case 2: //if(eventJoy.value) emit keyHandleXM(); break;
                  joyBut[1] = eventJoy.value;
              case 3: if(eventJoy.value) emit keyHandleWM(); break;
              default : break;
              }
          break;
        case JS_EVENT_AXIS:
            if(eventJoy.number > 1) break;
            lastJoy[eventJoy.number] = eventJoy.value;
        }
  }
  for(int i = 0; i < 2; i++) { //event regulator
      long x = lastJoy[i] + (signed int)(qrand() % 32768) - 16384;
      switch(i) {
      case 0: if(x > 16386) { emit keyHandleZP(); }
          if(x < -16386) { emit keyHandleZM(); }
          break;
      case 1: if(x > 16386) { emit keyHandleYP(); }
          if(x < -16386) { emit keyHandleYM(); }
          break;
      default : break;
      }
  }
  //arcade button latch
  if(joyBut[0]) emit keyHandleXP();
  if(joyBut[1]) emit keyHandleXM();
}
