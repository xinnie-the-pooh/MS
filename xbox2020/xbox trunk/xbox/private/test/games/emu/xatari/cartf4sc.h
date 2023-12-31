#ifndef CARTRIDGEF4SC_HXX
#define CARTRIDGEF4SC_HXX

class CartridgeF4SC;

#include "bspf.h"
#include "Cart.h"

/**
  Cartridge class used for Atari's 32K bankswitched games with
  128 bytes of RAM.  There are eight 4K banks.

  @author  Bradford W. Mott
  @version $Id: CartF4SC.hxx,v 1.2 1998/07/15 20:30:46 bwmott Exp $
*/
class CartridgeF4SC : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image Pointer to the ROM image
    */
    CartridgeF4SC(const uInt8* image);
 
    /**
      Destructor
    */
    virtual ~CartridgeF4SC();

  public:
    /**
      Get a null terminated string which is the device's name (i.e. "M6532")

      @return The name of the device
    */
    virtual const char* name() const;

    /**
      Reset device to its power-on state
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
    /**
      Install pages for the specified bank in the system

      @param bank The bank that should be installed in the system
    */
    void bank(uInt16 bank);

  private:
    // Indicates which bank is currently active
    uInt16 myCurrentBank;

    // The 16K ROM image of the cartridge
    uInt8 myImage[32768];

    // The 128 bytes of RAM
    uInt8 myRAM[128];
};
#endif

