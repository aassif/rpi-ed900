#ifndef __ED900_CRICKET__
#define __ED900_CRICKET__

#include <string>
#include <array>
#include <vector>

#include "game.h"

namespace ed900
{
  class Cricket : public Game
  {
    public:
      enum Mode
      {
        STANDARD = 0,
        CUTTHROAT = 1,
        RANDOM = 2,
        HIDDEN = 3,
      };

    private:
      class Player
      {
        public:
          std::string name;
          uint16_t score;
          std::array<uint8_t, 7> targets;

        public:
          Player (const std::string &);
      };

    private:
      std::vector<Player> players;
      Mode mode;
      std::array<uint8_t, 7> targets;

    private:
      void dart_score (uint8_t target, uint8_t multiplier);
      void render_scores (App *) const;
      void render_columns (App *) const;
      void render_button (App *) const;

    public:
      Cricket (const std::vector<std::string> & players, Mode);
      void dart (const DartEvent &);
      void button (const ButtonEvent &);
      bool finished () const;
      void render (App *) const;

    private:
      static std::array<uint8_t, 7> Targets (Mode);
  };
}

#endif // __ED900_CRICKET__

