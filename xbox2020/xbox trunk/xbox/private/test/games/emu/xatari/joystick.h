#ifndef JOYSTICK_HXX
#define JOYSTICK_HXX

#include "bspf.h"
#include "Control.h"

/**
  The standard Atari 2600 joystick controller.

  @author  Bradford W. Mott
  @version $Id: Joystick.hxx,v 1.2 1998/07/15 20:51:14 bwmott Exp $
*/
class Joystick : public Controller
{
  public:
    /**
      Create a new joystick controller plugged into the specified jack

      @param jack The jack the controller is plugged into
      @param event The event object to use for events
    */
    Joystick(Jack jack, const Event& event);

    /**
      Destructor
    */
    virtual ~Joystick();

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    virtual bool read(DigitalPin pin);

    /**
      Read the resistance at the specified analog pin for this controller.
      The returned value is the resistance measured in ohms.

      @param pin The pin of the controller jack to read
      @return The resistance at the specified pin
    */
    virtual Int32 read(AnalogPin pin);

    /**
      Write the given value to the specified digital pin for this
      controller.  Writing is only allowed to the pins associated
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    virtual void write(DigitalPin pin, bool value);
};
#endif

