#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <linux/joystick.h>
#include <QString>
#include <QObject>

// for more info about the Linux Joystick API read
// /usr/src/linux/Documentation/input/joystick-api.txt

struct EventJoystick
{
  int32_t time;
  int16_t value;
  int8_t number;
  bool synthetic;
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
  bool open (QString device);

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
  
  /*
   * @return Identifier string of the Joystick
   */
  const QString getIdentifier ();

signals:
    void signalAxis(EventJoystick e);
    void signalButton(EventJoystick e);

private:
  struct js_event joy_event;
  int m_fd;
  bool m_init;
  int m_axes;
  int m_buttons;
  QString m_name;
  bool m_run;

public slots:
  void loop ();
};

#endif // JOYSTICK_H
