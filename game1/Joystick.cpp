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
  m_axes = 0,
  m_buttons = 0,
  count = 0,
  m_run = false;

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
  timer->start(100);
}

Joystick::~Joystick ()
{
  this->close ();
}

int rdr(int m_fd, void *jev, unsigned int size) {
    if(Joystick::count == Joystick::mouseCount) return 0;
    js_event *je = (js_event *)jev;
    je->type = JS_EVENT_AXIS;
    int count = qrand()&3;
    je->number = count&1;
    switch(count) {
    case 0: je->value = QCursor::pos().x() * 65536 / (Joystick::mouseState->frameGeometry().width())
                - 32768; break;
    case 1: je->value = QCursor::pos().y() * 65536 / (Joystick::mouseState->frameGeometry().height())
                - 32768; break;

    default: je->type = JS_EVENT_BUTTON;
        je->number = count;
        je->value = Joystick::mouseState->mouse[3-(count&3)];
        break;
    }
    Joystick::mouseCount = Joystick::count;
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
    char buttons;
    char axes;
    
    // get number of buttons
    ioctl (m_fd, JSIOCGBUTTONS, &buttons);
    m_buttons = buttons;
    
    // get number of axes
    ioctl (m_fd, JSIOCGAXES, &axes);
    m_axes = axes;
    
    m_run = true;
  }

  return true;
}

bool Joystick::close ()
{
  m_run = false;
  lastCount[0] = lastCount[1] = lastJoy[0] = lastJoy[1] = 0;
  // end thread
  
  // reset some values
  m_axes = 0; 
  m_buttons = 0;
  
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
              case 1: if(eventJoy.value) emit keyHandleWP(); break;
              case 2: if(eventJoy.value) emit keyHandleXP(); break;
              case 3: if(eventJoy.value) emit keyHandleXM(); break;
              case 4: if(eventJoy.value) emit keyHandleWM(); break;
              default : break;
              }
          break;
        case JS_EVENT_AXIS:
            if(eventJoy.number > 2) break;
            inc = Joystick::count - lastCount[eventJoy.number];
            lastCount[eventJoy.number] = Joystick::count;
            lastJoy[eventJoy.number] = eventJoy.value;
            if(inc == 0) break;//flood handler!
            x = eventJoy.value + qrand() % 16384;
            switch(eventJoy.number) {
            case 1: if(x > 16385) { emit keyHandleYP(); break; }
                else { emit keyHandleYM(); break; }
            case 2: if(x < -16385) { emit keyHandleZP(); break; }
                else emit keyHandleZM(); break;
            default : break;
            }
          break;

        default: // we should never reach this point
            break;
          //printf ("unknown event: %x\n", joy_event.type);
        }
  }
  for(int i = 0; i < 2; i++) { //event regulator
      if(lastCount[i] != Joystick::count) {
          x = lastJoy[i] + qrand() % 16384;
          switch(i) {
          case 1: if(x > 16385) { emit keyHandleYP(); break; }
              else { emit keyHandleYM(); break; }
          case 2: if(x < -16385) { emit keyHandleZP(); break; }
              else emit keyHandleZM(); break;
          default : break;
          }
      }
  }
  Joystick::count ++;
}

int Joystick::getNumberOfButtons ()
{
  return m_buttons;
}

int Joystick::getNumberOfAxes ()
{
  return m_axes;
}
