#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <linux/joystick.h>
#include <QString>
#include <QObject>
#include <QTimer>
#include "window.h"

class Window;

// for more info about the Linux Joystick API read
// /usr/src/linux/Documentation/input/joystick-api.txt

struct EventJoystick
{
  int32_t time;
  int32_t value;
  int8_t number;
};

// TODO: configurable joystick device; best a manager for autodetect...
class Joystick : public QObject //: public sigc::trackable
{
    Q_OBJECT

public:
  Joystick ();
  ~Joystick ();

  /* Open a joystick device.
   * @param device A device (e.g. /dev/input/jsX).
   */
  bool open (QString *device, Window *widget);

  /* Close the joystick device.
   */
  bool close ();

  /*
   * @return Number of available buttons.
   * @return -1 Initializing not finished.
   */
  int getNumberOfButtons ();

  /*
   * @return Number of available axis.
   * @return -1 Initializing not finished.
   */
  int getNumberOfAxes ();

signals:
  void keyHandleWM();
  void keyHandleWP();
  void keyHandleXM();
  void keyHandleXP();
  void keyHandleYM();
  void keyHandleYP();
  void keyHandleZM();
  void keyHandleZP();

private:
  struct js_event joy_event;
  int m_fd;
  int m_axes;
  int m_buttons;
  QTimer * timer;
  long count;
  EventJoystick eventJoy;
  long lastCount[2];
  long lastJoy[2];
  int inc;
  long x;
  //int rdr(int m_fd, void *jev, unsigned int size);
  int (*rdrf)(int m_fd, void *jev, unsigned int size);

public:
  bool m_run;
  static Window *mouseState;

public slots:
  void loop ();
};

#endif // JOYSTICK_H
