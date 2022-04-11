#ifndef __ED900_GAME__
#define __ED900_GAME__

#include "ed900.h"

namespace ed900
{
  class App;

  class Game
  {
    private:
      class State
      {
        public:
          uint8_t players;
          uint8_t round;
          uint8_t player;
          uint8_t dart;

        public:
          State (uint8_t players);
          State & operator++ ();
          bool is_waiting () const;
          void next ();
      };

    protected:
      State state;

    public:
      Game (uint8_t players);
      virtual ~Game ();
      virtual void dart (const DartEvent &) = 0;
      virtual void button (const ButtonEvent &) = 0;
      virtual bool finished () const = 0;
      virtual void render (App *) const = 0;
  };
}

#endif // __ED900_GAME__

