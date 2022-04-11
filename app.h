#ifndef __ED900_APP__
#define __ED900_APP__

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "font.h"
#include "game.h"

namespace ed900
{
  class App
  {
    public:
      //enum {FONT_SMALL = 0, FONT_LARGE = 1, FONT_CRICKET = 2};
      //enum {RECT_DRAW = 0, RECT_FILL = 1};
      enum class State : uint8_t
      {
        BLUETOOTH = 0,
        MENU_PLAYERS = 1,
        MENU_GAME = 2,
        MENU_X01 = 3,
        MENU_CRICKET = 4,
        GAME = 5
      };

    private:
      SDL_Window * window;
      SDL_Renderer * renderer;
      std::vector<Font> fonts;
      SDL_Texture * bluetooth;
      SDL_Texture * player;
      State state;
      uint8_t selection;
      uint8_t players;
      Game * game;

    private:
      void render_bluetooth ();
      void render_menu (const std::vector<std::string> &);
      void render_menu_players ();
      void render_menu_game ();
      void render_menu_x01 ();
      void render_menu_cricket ();

    public:
      App ();
      ~App ();
      void draw (const std::string &, int x, int y, int font);
      void draw (const SDL_Rect &, const SDL_Color &, SDL_BlendMode);
      int run ();

    private:
      static const uint8_t COLORS [][3];
    public:
      static SDL_Color Color (int k, float alpha = 1.0);
  };
}

#endif // __ED900_APP__

