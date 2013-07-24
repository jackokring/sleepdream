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
#include "Joystick.h"

Joystick::Joystick () : QObject()
{
  m_axes = 0,
  m_buttons = 0,
  m_run = true;
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
}

Joystick::~Joystick ()
{
  this->close ();
}

bool Joystick::open (QString *device)
{
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
    
    timer->start(100);
    m_run = true;
  }

  return true;
}

bool Joystick::close ()
{
  // end thread
  m_run = false;
  
  // reset some values
  m_axes = 0; 
  m_buttons = 0;
  
  return !::close (m_fd);
}

void Joystick::loop ()
{
  // wait for all synthetic event until the first real event comes
  // then we've all available axis and buttons.

  while (m_run)
  {
    EventJoystick eventJoy;

    while(read (m_fd, &joy_event, sizeof(struct js_event)) > 0) {

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
        eventJoy.value += qrand() % 16384;
        switch(eventJoy.number) {
        case 1: if(eventJoy.value > 16385) { emit keyHandleYP(); break; }
            else { emit keyHandleYM(); break; }
        case 2: if(eventJoy.value < -16385) { emit keyHandleZP(); break; }
            else emit keyHandleZM(); break;
        default : break;
        }
      break;

    default: // we should never reach this point
        break;
      //printf ("unknown event: %x\n", joy_event.type);
    }
  } }
}

int Joystick::getNumberOfButtons ()
{
  return m_buttons;
}

int Joystick::getNumberOfAxes ()
{
  return m_axes;
}