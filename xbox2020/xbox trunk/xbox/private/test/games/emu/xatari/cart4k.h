#ifndef CARTRIDGE4K_HXX
#define CARTRIDGE4K_HXX

class Cartridge4K;
class System;

#include "bspf.h"
#include "Cart.h"

/**
  This is the standard Atari 4K cartridge.  These cartridges are 
  not bankswitched.

  @author  Bradford W. Mott
  @version $Id: Cart4K.hxx,v 1.2 1998/07/15 20:51:00 bwmott Exp $
*/
class Cartridge4K : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    Cartridge4K(const uInt8* image);
 
    /**
      Destructor
    */
    virtual ~Cartridge4K();

  public:
    /**
      Get a null terminated string which is the device's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const;

    /**
      Reset cartridge to its power-on state
    */
    virtual void reset();

    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

      @param system The system the device should install itself in
    */
    virtual void install(System& system);

  public:
    /**
      Get the byte at the specified address.

      @return The byte at the specified address
    */
    virtual uInt8 peek(uInt16 address);

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
    */
    virtual void poke(uInt16 address, uInt8 value);

  private:
    // The 4K ROM image for the cartridge
    uInt8 myImage[4096];
};
#endif

