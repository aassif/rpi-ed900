#include <iostream>
#include <iomanip>
#include <regex>

#include "ed900.h"

#define BLUEZ "org.bluez"
#define BLUEZ_ADAPTER "org.bluez.Adapter1"
#define BLUEZ_DEVICE "org.bluez.Device1"
#define BLUEZ_GATT_SERVICE "org.bluez.GattService1"
#define BLUEZ_GATT_CHARACTERISTIC "org.bluez.GattCharacteristic1"

using namespace std;
using namespace literals::chrono_literals;

namespace ed900
{
  DartEvent::DartEvent (uint8_t v, uint8_t m) :
    value {v},
    multiplier {m}
  {
  }

  string prefix (uint8_t m, bool abbr)
  {
    switch (m)
    {
      case 1: return "";
      case 2: return abbr ? "D" : "Double ";
      case 3: return abbr ? "T" : "Triple ";
      default: return "?";
    }
  }

  string DartEvent::text (bool abbr) const
  {
    return prefix (multiplier, abbr) + to_string (value);
  }
    
  uint8_t DartEvent::total () const
  {
    return multiplier * value;
  }
    
  ButtonEvent::ButtonEvent (uint16_t b) :
    button {b}
  {
  }

  string ButtonEvent::text () const
  {
    switch (button)
    {
      case Button::BUTTON_REPEATED: return "BUTTON_REPEATED";
      case Button::CANCEL_PRESSED:  return "CANCEL_PRESSED";
      case Button::NEXT_PRESSED:    return "NEXT_PRESSED";
      case Button::NEXT_RELEASED:   return "NEXT_RELEASED";
      default:                      return "?";
    }
  }

  Uint32 ED900::EventType = 0xFFFFFFFF;

  bool ED900::IsEvent (Uint32 t)
  {
    return t == EventType;
  }

  ED900::ED900 () :
    bus {nullptr},
    event {nullptr},
    adapter {},
    device {},
    characteristic {},
    thread {}
  {
    sd_bus_open_system (&bus);

    if (EventType == 0xFFFFFFFF)
      EventType = SDL_RegisterEvents (1);
  }

  ED900::~ED900 ()
  {
    sd_bus_unref (bus);
  }

  vector<string> ED900::objects ()
  {
    sd_bus_error e = SD_BUS_ERROR_NULL;
    sd_bus_message * m = nullptr;

    sd_bus_call_method (bus, BLUEZ, "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects", &e, &m, NULL);

    sd_bus_message_enter_container (m, 'a', "{oa{sa{sv}}}");

    vector<string> o;
    auto f = [m] () {return sd_bus_message_enter_container (m, 'e', "oa{sa{sv}}");};
    for (int r = f (); r > 0; r = f ())
    {
      char * path;
      sd_bus_message_read_basic (m, 'o', &path);
      o.push_back (path);

      sd_bus_message_skip (m, "a{sa{sv}}");
      sd_bus_message_exit_container (m);
    }

    sd_bus_message_exit_container (m);

    sd_bus_error_free (&e);
    sd_bus_message_unref (m);

    return o;
  }

  string ED900::property (const string & path, const string & interface, const string & property)
  {
    sd_bus_error e = SD_BUS_ERROR_NULL;
    char * p = nullptr;
    int r = sd_bus_get_property_string (bus, BLUEZ, path.c_str (), interface.c_str (), property.c_str (), &e, &p);
    sd_bus_error_free (&e);

    if (r < 0)
      return string {};

    string s {p};
    free (p);
    return s;
  }

  vector<uint8_t> ED900::data ()
  {
    sd_bus_message * m = nullptr;
    sd_bus_error e = SD_BUS_ERROR_NULL;
    int r = sd_bus_get_property (bus, BLUEZ, characteristic.c_str (), BLUEZ_GATT_CHARACTERISTIC, "Value", &e, &m, "ay");
    sd_bus_error_free (&e);

    if (r < 0)
      return vector<uint8_t> {};

    const uint8_t * d = nullptr;
    size_t n = 0;
    sd_bus_message_read_array (m, 'y', (const void **) &d, &n);
    vector<uint8_t> v {d, d + n};
    sd_bus_message_unref (m);
    return v;
  }

  void ED900::PushConnectionEvent ()
  {
    SDL_Event e;
    //SDL_memset (&e, 0, sizeof (e));
    e.type = EventType;
    e.user.code = CONNECTED;
    e.user.data1 = nullptr;
    e.user.data2 = nullptr;
    SDL_PushEvent (&e);
  }

  bool ED900::find (const std::string & a)
  {
    // Set adapter.
    adapter = a;

    sd_bus_error e = SD_BUS_ERROR_NULL;
    sd_bus_call_method (bus, BLUEZ, adapter.c_str (), BLUEZ_ADAPTER, "StartDiscovery", &e, NULL, NULL);

    regex r ('^' + adapter + "/dev(_([[:xdigit:]]{2})){6}$");

    while (1)
    {
      this_thread::sleep_for (1s);
      for (auto o : objects ())
      {
        if (regex_match (o, r))
        {
          string address = property (o, BLUEZ_DEVICE, "Address");
          string name    = property (o, BLUEZ_DEVICE, "Name");
          cout << o << " : " << address << " : " << name << endl;

          if (name == "SDB-BT")
          {
            sd_bus_call_method (bus, BLUEZ, adapter.c_str (), BLUEZ_ADAPTER, "StopDiscovery", &e, NULL, NULL);
            sd_bus_error_free (&e);
            return connect (o);
          }
        }
      }
    }

    sd_bus_call_method (bus, BLUEZ, adapter.c_str (), BLUEZ_ADAPTER, "StopDiscovery", &e, NULL, NULL);
    sd_bus_error_free (&e);
    return false;
  }

  bool ED900::connect (const std::string & d)
  {
    // Set device.
    device = d;

    sd_bus_error e = SD_BUS_ERROR_NULL;
    sd_bus_call_method (bus, BLUEZ, device.c_str (), BLUEZ_DEVICE, "Connect", &e, NULL, NULL);
    sd_bus_error_free (&e);

    // Expected GATT characteristic.
    string c = device + "/service000b/char000c";

    while (1)
    {
      this_thread::sleep_for (1s);
      for (auto o : objects ())
        if (o == c)
        {
          PushConnectionEvent ();
          return loop (c);
        }
        else
          cout << o << endl;
    }

    return false;
  }

  void ED900::start (const string & adapter)
  {
    thread = std::thread {&ED900::find, this, adapter};
  }

  string ED900::Hex (const vector<uint8_t> & data)
  {
    ostringstream oss;

    for (auto d : data)
      oss << hex << setw (2) << setfill ('0') << (int) d;

    return oss.str ();
  }

  void ED900::Dump (const vector<uint8_t> & data)
  {
    cout << Hex (data) << endl;
  }

  void ED900::PushButtonEvent (uint16_t button)
  {
    SDL_Event e;
    //SDL_memset (&e, 0, sizeof (e));
    e.type = EventType;
    e.user.code = BUTTON;
    e.user.data1 = new ButtonEvent {button};
    e.user.data2 = nullptr;
    SDL_PushEvent (&e);
  }

  void ED900::PushDartEvent (uint8_t value, uint8_t multiplier)
  {
    SDL_Event e;
    //SDL_memset (&e, 0, sizeof (e));
    e.type = EventType;
    e.user.code = DART;
    e.user.data1 = new DartEvent {value, multiplier};
    e.user.data2 = nullptr;
    SDL_PushEvent (&e);
  }

  int ED900::Notification (sd_bus_message *, void * p, sd_bus_error *)
  {
    ED900 * ed900 = static_cast<ED900 *> (p);
    std::vector<uint8_t> d = ed900->data ();

    if (d.size () == 10)
    {
      //ED900::Dump (d);
      uint32_t id = d[0] | (d[1] << 8) | (d[2] << 16) | (d[3] << 24);
      uint16_t reserved1 = d[4] | (d[5] << 8);
      uint8_t multiplier = d[6];
      if (multiplier != 0)
      {
        uint8_t units = d[7];
        uint8_t tens = d[8];
        uint8_t reserved2 = d[9];

        switch (tens)
        {
          case 0x0E:
            PushDartEvent (25, 1);
            break;

          case 0x0F:
            PushDartEvent (25, 2);
            break;

          default:
          {
            int value = 10 * tens + units;
            int m = 1;
            switch (multiplier)
            {
              case 0x0B : m = 3; break;
              case 0x0D : m = 2; break;
            }
            PushDartEvent (value, m);
            break;
          }
        }
      }
      else
      {
        uint8_t reserved2 = d[7];
        uint16_t button = d[8] | (d[9] << 8);
        PushButtonEvent (button);
      }
    }
    else
    {
      ED900::Dump (d);
    }

    return 0;
  }

  bool ED900::loop (const std::string & c)
  {
    characteristic = c;

    sd_bus_error e = SD_BUS_ERROR_NULL;
    sd_bus_call_method (bus, BLUEZ, characteristic.c_str (), BLUEZ_GATT_CHARACTERISTIC, "StartNotify", &e, NULL, NULL);
    sd_bus_error_free (&e);

    sd_event_default (&event);
    string match = "path=" + characteristic;
    sd_bus_add_match (bus, NULL, match.c_str (), ED900::Notification, (void *) this);
    sd_bus_attach_event (bus, event, SD_EVENT_PRIORITY_NORMAL);
    sd_event_loop (event);

    return true;
  }

  void ED900::stop ()
  {
    sd_event_exit (event, 0);

    sd_bus_error e = SD_BUS_ERROR_NULL;
    sd_bus_call_method (bus, BLUEZ, device.c_str (), BLUEZ_DEVICE, "Disconnect", &e, NULL, NULL);
    sd_bus_error_free (&e);

    thread.join ();
  }

  void ED900::debug ()
  {
    for (auto o : objects ())
      cout << o << endl;
  }
}

