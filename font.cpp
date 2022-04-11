#include <iostream>

#include <SDL2/SDL_image.h>

#include "font.h"

using namespace std;

namespace ed900
{

////////////////////////////////////////////////////////////////////////////////

  Font::Font (SDL_Renderer * r,
              const string & path,
              int w, int h) :
    width {w},
    height {h},
    renderer {r},
    texture {nullptr},
    index {}
  {
    texture = IMG_LoadTexture (r, path.c_str ());
  }
    
  Font::Font (Font && f) :
    width {f.width},
    height {f.height},
    renderer {f.renderer},
    texture {std::exchange (f.texture, nullptr)},
    index {f.index}
  {
  }

  Font::~Font ()
  {
    SDL_DestroyTexture (texture);
  }

  void Font::draw (char c, int x, int y)
  {
    //cout << "draw '" << (int) c << "' " << x << ' ' << y << endl;

    auto f = index.find (c);
    if (f != index.end ())
    {
      const SDL_Rect & src = f->second;
      SDL_Rect dst = {x, y, width, height};
      SDL_RenderCopy (renderer, texture, &src, &dst);
    }
  }

  void Font::draw (const string & text, int x, int y)
  {
    for (auto c : text)
    {
      draw (c, x, y);
      x += width;
    }
  }

////////////////////////////////////////////////////////////////////////////////

  FontPico8::FontPico8 (SDL_Renderer * r) :
    Font (r, "images/pico-8.png", 4, 6)
  {
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 16; ++x)
      {
        char c = 32 + 16*y + x;
        SDL_Rect rect = {x * width, y * height, width, height};
        index.emplace (c, rect);
      }
  }

////////////////////////////////////////////////////////////////////////////////

  FontTomThumb::FontTomThumb (SDL_Renderer * r) :
    Font (r, "images/tom-thumb-new.png", 4, 6)
  {
    for (int y = 0; y < 4; ++y)
      for (int x = 0; x < 32; ++x)
      {
        char c = 32*y + x;
        SDL_Rect rect = {x * width, y * height, width, height};
        index.emplace (c, rect);
      }
  }

////////////////////////////////////////////////////////////////////////////////

  FontAmstrad::FontAmstrad (SDL_Renderer * r) :
    Font (r, "images/amstrad.png", 8, 8)
  {
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 16; ++x)
      {
        char c = 32 + 16*y + x;
        SDL_Rect rect = {x * width, y * height, width, height};
        index.emplace (c, rect);
      }
  }

////////////////////////////////////////////////////////////////////////////////

  FontTopaz::FontTopaz (SDL_Renderer * r) :
    Font (r, "images/topaz.png", 8, 8)
  {
    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 16; ++x)
      {
        char c = 32 + 16*y + x;
        SDL_Rect rect = {x * width, y * height, width, height};
        index.emplace (c, rect);
      }
  }

////////////////////////////////////////////////////////////////////////////////

  FontRazor1911::FontRazor1911 (SDL_Renderer * r) :
    Font (r, "images/rzrnfnt.png", 8, 8)
  {
    static const char _Q_ = '\'';
    static const char _B_ = '\\';
    static const char _C_ = 0xA9;

    static const uint8_t INDEX [] =
    {
      ' ', '!', '"', '#', '$', '%', '&', '(', ')', '*', '+', ',', '-', '.', _Q_, '/',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', '@',
      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
      'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
      '[', _B_, ']', '^', _C_, '_', '{', '}'
    };

    for (uint8_t c = 0; c < sizeof (INDEX); ++c)
    {
      SDL_Rect rect = {c * width, 0, width, height};
      index.emplace (INDEX [c], rect);
    }
  }

////////////////////////////////////////////////////////////////////////////////

  FontLarge::FontLarge (SDL_Renderer * r) :
    Font (r, "images/font254r.png", 32, 32)
  {
    static const char ___ = 0;
    static const char _Q_ = '\'';

    static const char INDEX [][10] =
    {
      {' ', '!', ___, ___, ___, ___, ___, _Q_, '(', ')'},
      {___, ___, '.', '-', ',', ___, '0', '1', '2', '3'},
      {'4', '5', '6', '7', '8', '9', ___, ___, ___, ___},
      {___, '?', ___, 'A', 'B', 'C', 'D', 'E', 'F', 'G'},
      {'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q'},
      {'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ___}
    };

    for (int y = 0; y < 6; ++y)
      for (int x = 0; x < 10; ++x)
      {
        char c = INDEX[y][x];
        if (c != 0)
        {
          SDL_Rect rect = {x * width, y * width, width, height};
          index.emplace (c, rect);
        }
      }
  }

////////////////////////////////////////////////////////////////////////////////

  FontCricket::FontCricket (SDL_Renderer * r) :
    Font (r, "images/cricket.png", 8, 8)
  {
    static const char INDEX [] = {'0', '1', '2', '3', '(', ')'};
    for (int k = 0; k < 6; ++k)
    {
      SDL_Rect rect = {k * width, 0, width, height};
      index.emplace (INDEX[k], rect);
    }
  }

}

