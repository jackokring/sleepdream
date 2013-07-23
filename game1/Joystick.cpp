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
#include "Joystick.h"

Joystick::Joystick () : QObject()
{
  m_init = false,
  m_axes = 0,
  m_buttons = 0,
  m_run = true;
}

Joystick::~Joystick ()
{
  this->close ();
}

bool Joystick::open (QString device)
{
  m_fd = ::open (device.toStdString().c_str(), O_RDONLY);
  if (m_fd == -1)
  {
    //cerr << "Error opening joystick device!" << endl;
    
    return false;
  }
  else
  {
    char buttons;
    char axes;
    char name[128];
    
    // get number of buttons
    ioctl (m_fd, JSIOCGBUTTONS, &buttons);
    m_buttons = buttons;
    
    // get number of axes
    ioctl (m_fd, JSIOCGAXES, &axes);
    m_axes = axes;
    
    // get device name
    if (ioctl(m_fd, JSIOCGNAME (sizeof(name)), name) < 0)
    {
      m_name = "Unknown";
    }
    else
    {
      m_name = name;
    }
    
    /* TODO: support those if needed
     * #define JSIOCGVERSION   // get driver version
     * #define JSIOCSCORR      // set correction values
     * #define JSIOCGCORR      // get correction values
     */
    
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(loop()));
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
  m_init = false;
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

    read (m_fd, &joy_event, sizeof(struct js_event));

    eventJoy.time = joy_event.time;
    eventJoy.value = joy_event.value;

    switch (joy_event.type)
    {
    case JS_EVENT_BUTTON:
      if (!m_init) m_init = true;
      eventJoy.number = joy_event.number;
      eventJoy.synthetic = false;
      emit signalButton(eventJoy);
      break;

    case JS_EVENT_AXIS:
      if (!m_init) m_init = true;
      eventJoy.number = joy_event.number;
      eventJoy.synthetic = false;
      emit signalAxis(eventJoy);
      break;

    case JS_EVENT_BUTTON | JS_EVENT_INIT:
      if (m_init) // skip the synthetic events on driver start
      {
        eventJoy.number = joy_event.number & ~JS_EVENT_INIT;
        eventJoy.synthetic = true;
        emit signalButton(eventJoy);
      }
      break;

    case JS_EVENT_AXIS | JS_EVENT_INIT:
      if (m_init) // skip the synthetic events on driver start
      {
        eventJoy.number = joy_event.number & ~JS_EVENT_INIT;
        eventJoy.synthetic = true;
        emit signalAxis(eventJoy);
      }
      break;

    default: // we should never reach this point
        ;
      //printf ("unknown event: %x\n", joy_event.type);
    }
  }
}

int Joystick::getNumberOfButtons ()
{
  return m_buttons;
}

int Joystick::getNumberOfAxes ()
{
  return m_axes;
}

const QString Joystick::getIdentifier ()
{
  return m_name;
}
