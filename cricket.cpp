#include <iostream>
#include <algorithm>
#include <random>

#include <SDL2/SDL.h>

#include "app.h"
#include "cricket.h"

using namespace std;

namespace ed900
{
  array<uint8_t, 7> Cricket::Targets (Mode m)
  {
    switch (m)
    {
      case STANDARD:
      case CUTTHROAT:
        return {20, 19, 18, 17, 16, 15, 25};

      case RANDOM:
      case HIDDEN:
      {
        array<uint8_t, 20> t20;
        iota (t20.begin (), t20.end (), 1);

        random_device rd;
        mt19937 g {rd ()};
        shuffle (t20.begin (), t20.end (), g);

        array<uint8_t, 7> t7;
        copy_n (t20.begin (), 6, t7.begin ());
        t7[6] = 25;
        return t7;
      }
    }

    return array<uint8_t, 7> {};
  }

  Cricket::Player::Player (const string & n) :
    name {n},
    score {0},
    targets {}
  {
  }

  Cricket::Cricket (const vector<string> & names, Mode m) :
    Game {names.size ()},
    players {},
    mode {m},
    targets {Targets (m)}
  {
    for (auto & n : names)
      players.push_back (Player {n});
  }

  void Cricket::dart_score (uint8_t target, uint8_t multiplier)
  {
    switch (mode)
    {
      case STANDARD:
      case RANDOM:
      case HIDDEN:
        for (auto & p: players)
        {
          if (p.targets[target] >= 3u)
            p.score += targets[target] * multiplier;
        }
        break;

      case CUTTHROAT:
        for (auto & p: players)
        {
          if (p.targets[target] < 3u)
            p.score += targets[target] * multiplier;
        }
        break;
    }
  }

  void Cricket::dart (const DartEvent & e)
  {
    cout << e.text (true) << endl;

    auto f = find (targets.begin (), targets.end (), e.value);
    if (f != targets.end ())
    {
      uint8_t p = state.player;
      uint8_t t = distance (targets.begin (), f);
      uint8_t m = e.multiplier;
      if (players[p].targets[t] < 3u)
      {
        uint8_t d = min<uint8_t> (3 - players[p].targets[t], m);
        players[p].targets[t] += d;
        dart_score (t, m - d);
      }
      else
        dart_score (t, m);
    }

    ++state;
  }

  void Cricket::button (const ButtonEvent & e)
  {
    cout << e.text () << endl;

    if (e.button == Button::NEXT_PRESSED)
      state.next ();
  }

  bool Cricket::finished () const
  {
    return false;
  }

  void Cricket::render_scores (App * app) const
  {
    static const int W = 6*4 + 1; // 3*8 + 1
    static const int H = 6 + 8 + 1;

    static const SDL_Rect R [] =
    {
      {0,     0,    W, H},
      {128-W, 0,    W, H},
      {0,     64-H, W, H},
      {128-W, 64-H, W, H}
    };

    for (uint8_t i = 0; i < 4; ++i)
    {
      const Player & p = players [i];
      const SDL_Rect & r = R [i];

      if (i == state.player)
        app->draw (r, App::Color (i), SDL_BLENDMODE_NONE);

      app->draw (p.name, r.x + 1, r.y + 1, 0);
      string score = to_string (p.score);
      size_t n = score.size ();
      if (n <= 3)
        app->draw (score, r.x + 1 + (3 - n) * 8, r.y + 7, 1);
      else
        app->draw (score, r.x + 1 + (6 - n) * 4, r.y + 9, 0);

      if (i != state.player)
        app->draw (r, App::Color (i), SDL_BLENDMODE_MOD);
    }
  }

  void Cricket::render_columns (App * app) const
  {
    for (int i = 0; i < 6; ++i)
    {
      string item = to_string (targets [i]);
      app->draw (item, 64 - (targets[i] < 10 ? 4 : 8), 4 + i*8, 1);
    }

    app->draw ("BULL", 64-8, 4 + 6*8 + 1, 0);

    static const SDL_Rect R [] =
    {
      {30,       3, 9, 57},
      {42,       3, 9, 57},
      {128-42-9, 3, 9, 57},
      {128-30-9, 3, 9, 57}
    };

    for (uint8_t i = 0; i < 4; ++i)
    {
      const SDL_Rect & r = R [i];
      app->draw (r, App::Color (i), SDL_BLENDMODE_NONE);
      for (uint8_t j = 0; j < 7; ++j)
      {
        uint8_t n = min<uint8_t> (players[i].targets[j], 3);
        app->draw (to_string (n), r.x + 1, r.y + 8*j + 1, 3);
      }
    }
  }

  void Cricket::render_button (App * app) const
  {
    if (state.is_waiting ())
    {
      static const SDL_Rect R = {0, 0, 128, 64};
      static const SDL_Color C = {0, 0, 0, 192};
      app->draw (R, C, SDL_BLENDMODE_BLEND);
      app->draw ("NEXT", 0, 16, 2);
    }
  }

  void Cricket::render (App * app) const
  {
    render_scores (app);
    render_columns (app);
    render_button (app);
  }
}

