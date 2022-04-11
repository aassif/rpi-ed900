#ifndef __ED900__
#define __ED900__

#include <string>
#include <vector>
#include <mutex>
#include <thread>

#include <systemd/sd-bus.h>
#include <SDL2/SDL.h>

namespace ed900
{
  class DartEvent
  {
    public:
      uint8_t value;
      uint8_t multiplier;

    public:
      DartEvent (uint8_t value, uint8_t multiplier = 1);
      std::string text (bool abbr = false) const;
      uint8_t total () const;
  };

  enum class Button : uint16_t
  {
    BUTTON_REPEATED = 0xCCCC,
    CANCEL_PRESSED  = 0xDDDD,
    NEXT_RELEASED   = 0xEEEE,
    NEXT_PRESSED    = 0xFFFF
  };
    
  class ButtonEvent
  {
    public:
      Button button;

    public:
      ButtonEvent (uint16_t);
      std::string text () const;
  };

  class ED900
  {
    public:
      enum {CONNECTED = 0, DART = 1, BUTTON = 2} EventCode;

    private:
      sd_bus * bus;
      sd_event * event;
      std::string adapter;
      std::string device;
      std::string characteristic;
      std::mutex mutex;
      std::thread thread;

    private:
      // SD-bus root objects.
      std::vector<std::string> objects ();
      // BlueZ properties.
      std::string property (const std::string & path,
                            const std::string & interface,
                            const std::string & property);
      // BlueZ GATT characterisitc.
      std::vector<uint8_t> data ();
      // BlueZ discovery.
      bool find (const std::string & adapter);
      // BlueZ connection.
      bool connect (const std::string & device);
      // SD-bus event loop.
      bool loop (const std::string & characteristic);
      // Debug.
      void debug ();

    public:
      ED900 ();
      ~ED900 ();
      void start (const std::string & adapter);
      void stop ();

    private:
      // SDL event type.
      static Uint32 EventType;
      static void RegisterEvents ();
      // SDL pushed events.
      static void PushConnectionEvent ();
      static void PushDartEvent (uint8_t value, uint8_t multiplier);
      static void PushButtonEvent (uint16_t button);
      // SD-bus event loop.
      static int Notification (sd_bus_message *, void *, sd_bus_error *);
      // Debug.
      static std::string Hex (const std::vector<uint8_t> &);
      static void Dump (const std::vector<uint8_t> &);

    public:
      // Check SDL event type.
      static bool IsEvent (Uint32);
  };
}

#endif // __ED900__

