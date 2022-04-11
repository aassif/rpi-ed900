#include <numeric>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "app.h"
#include "cricket.h"
#include "ed900.h"

#define WIDTH 128
#define HEIGHT 64
#define SCALE 8

using namespace std;

namespace ed900
{
  App::App () :
    window {nullptr},
    renderer {nullptr},
    fonts {},
    bluetooth {nullptr},
    player {nullptr},
    state {State::BLUETOOTH},
    selection {0},
    players {0},
    game {nullptr}
  {
    SDL_Init (SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow ("ED 900", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH * SCALE, HEIGHT * SCALE, 0);
    //renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);
    renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_SOFTWARE);
    SDL_RenderSetLogicalSize (renderer, WIDTH, HEIGHT);

    IMG_Init (IMG_INIT_PNG);
    //fonts.emplace_back (FontPico8 {renderer});
    fonts.emplace_back (FontTomThumb {renderer});
    //fonts.emplace_back (FontRazor1911 {renderer});
    //fonts.emplace_back (FontAmstrad {renderer});
    fonts.emplace_back (FontTopaz {renderer});
    fonts.emplace_back (FontLarge {renderer});
    fonts.emplace_back (FontCricket {renderer});

    bluetooth = IMG_LoadTexture (renderer, "images/bluetooth.png");
    player    = IMG_LoadTexture (renderer, "images/player.png");

    //game = new Cricket ({"Aassif"});
    //game = new Cricket ({"Sylvia", "Aassif"});
    game = new Cricket ({"Jess", "Simon", "Sylvia", "Aassif"}, Cricket::CUTTHROAT);
  }

  App::~App ()
  {
    delete game;
    fonts.clear ();
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    SDL_Quit ();
  }

  void App::render_bluetooth ()
  {
    static const SDL_Rect R = {48, 3, 31, 57};
    SDL_RenderCopy (renderer, bluetooth, NULL, &R);
    SDL_SetRenderDrawBlendMode (renderer, SDL_BLENDMODE_MOD);
    float t = SDL_GetTicks () / 500.0;
    uint8_t c = static_cast<uint8_t> (127.5 + 127.5 * sin (t));
    SDL_SetRenderDrawColor (renderer, c, c, 255, 255);
    SDL_RenderFillRect (renderer, &R);
  }

  void App::render_menu (const vector<string> & items)
  {
    size_t n = min<size_t> (4, items.size ());

    auto f = [] (size_t m, const string & item) {return max (m, item.size ());};
    size_t m = accumulate (items.begin (), items.begin () + n, 0, f);

    for (uint8_t i = 0; i < n; ++i)
    {
      size_t w = items[i].size ();
      SDL_Rect R = {64 - (2 + 4*w), 2 + 16*i, 4 + 8*w, 4 + 8};

      if (i+1 == selection)
        draw (R, Color (i), SDL_BLENDMODE_NONE);

      draw (items[i], R.x + 3, R.y + 2, 1);

      if (i+1 != selection)
        draw (R, Color (i), SDL_BLENDMODE_MOD);
    }
  }

  void App::render_menu_players ()
  {
/*
    static const vector<string> ITEMS = {"1", "2", "3", "4"};
    render_menu (ITEMS);
*/
    for (uint8_t y = 0; y < 2; ++y)
      for (uint8_t x = 0; x < 2; ++x)
      {
        uint8_t n = 2*y + x + 1;

        SDL_Rect R = {64*x, 32*y, 64, 32};

        if (n == selection)
          draw (R, Color (n-1), SDL_BLENDMODE_NONE);

        for (uint8_t i = 0; i < n; ++i)
        {
          SDL_Rect r = {R.x + 32 - 5*n + 10*i, R.y + 8, 9, 16};
          SDL_RenderCopy (renderer, player, NULL, &r);
        }

        if (n != selection)
          draw (R, Color (n-1), SDL_BLENDMODE_MOD);
      }
  }

  void App::render_menu_game ()
  {
    static const vector<string> ITEMS = {"X01", "Cricket", "TicTacToe", "..."};
    render_menu (ITEMS);
  }

  void App::render_menu_x01 ()
  {
    static const vector<string> ITEMS = {"301", "501", "701", "901"};
    render_menu (ITEMS);
  }

  void App::render_menu_cricket ()
  {
    static const vector<string> ITEMS = {"Standard", "CutThroat", "Random", "Hidden"};
    render_menu (ITEMS);
  }

  int App::run ()
  {
    ED900 ed900;
    ed900.start ("/org/bluez/hci0");

    bool stop = false;
    while (! stop)
    {
      SDL_Event e;
      while (SDL_PollEvent (&e))
      {
        switch (e.type)
        {
          case SDL_QUIT:
            stop = true;
            break;

          case SDL_KEYUP:
            switch (e.key.keysym.sym)
            {
              case SDLK_ESCAPE:
                stop = true;
                break;

              case SDLK_1:
                selection = 1;
                break;

              case SDLK_2:
                selection = 2;
                break;

              case SDLK_3:
                selection = 3;
                break;

              case SDLK_4:
                selection = 4;
                break;

              case SDLK_SPACE:
                switch (state)
                {
                  case State::MENU_PLAYERS:
                    switch (selection)
                    {
                      case 1: selection = 0; players = 1; state = State::MENU_GAME; break;
                      case 2: selection = 0; players = 2; state = State::MENU_GAME; break;
                      case 3: selection = 0; players = 3; state = State::MENU_GAME; break;
                      case 4: selection = 0; players = 4; state = State::MENU_GAME; break;
                    }
                    break;

                  case State::MENU_GAME:
                    switch (selection)
                    {
                      case 1: selection = 0; state = State::MENU_X01; break;
                      case 2: selection = 0; state = State::MENU_CRICKET; break;
                      case 3: break;
                      case 4: break;
                    }
                    break;

                  case State::MENU_CRICKET:
                    switch (selection)
                    {
                      case 1: selection = 0; state = State::GAME; break;
                      case 2: selection = 0; state = State::GAME; break;
                      case 3: break;
                      case 4: break;
                    }
                    break;
                }
                break;
            }
            break;

          default:
            if (ED900::IsEvent (e.type))
            {
              switch (e.user.code)
              {
                case ED900::CONNECTED:
                  state = State::MENU_PLAYERS;
                  selection = 0;
                  break;

                case ED900::DART:
                {
                  DartEvent * d = static_cast<DartEvent *> (e.user.data1);
                  if (game) game->dart (*d);
                  delete d;
                  break;
                }

                case ED900::BUTTON:
                {
                  ButtonEvent * b = static_cast<ButtonEvent *> (e.user.data1);
                  if (game) game->button (*b);
                  delete b;
                  break;
                }
              }
            }
        }
      }
      SDL_SetRenderDrawColor (renderer, 0, 0, 0, 0);
      SDL_RenderClear (renderer);
      switch (state)
      {
        case State::BLUETOOTH :
          render_bluetooth ();
          break;

        case State::MENU_PLAYERS :
          render_menu_players ();
          break;

        case State::MENU_GAME :
          render_menu_game ();
          break;

        case State::MENU_X01 :
          render_menu_x01 ();
          break;

        case State::MENU_CRICKET :
          render_menu_cricket ();
          break;

/*
        case State::MENU_PLAYERS :
          render_menu_players ();
          break;
*/

        case State::GAME:
          game->render (this);
          break;
      }
      SDL_RenderPresent (renderer);
    }

    ed900.stop ();
    return 0;
  }

  const uint8_t App::COLORS [4][3] =
  {
    {0xFF, 0x00, 0x00},
    {0x00, 0x00, 0xFF},
    {0xFF, 0xFF, 0x00},
    {0x00, 0xFF, 0x00}
  };

  SDL_Color App::Color (int k, float alpha)
  {
    const auto & c = COLORS [k];
    return SDL_Color {c[0], c[1], c[2], static_cast<uint8_t> (round (255 * alpha))};
  }

  void App::draw (const SDL_Rect & r, const SDL_Color & c, SDL_BlendMode mode)
  {
    SDL_SetRenderDrawBlendMode (renderer, mode);
    SDL_SetRenderDrawColor (renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect (renderer, &r);
  }

  void App::draw (const std::string & text, int x, int y, int font)
  {
    if (font >= 0 && font < fonts.size ())
      fonts [font].draw (text, x, y);
  }

}

