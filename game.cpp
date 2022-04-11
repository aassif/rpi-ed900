#include <iostream>

#include "game.h"

using namespace std;

namespace ed900
{
  Game::State::State (uint8_t n) :
    players {n},
    round {0},
    player {0},
    dart {0}
  {
  }
    
  Game::State & Game::State::operator++ ()
  {
    ++dart;
    return *this;
  }

  bool Game::State::is_waiting () const
  {
    return dart >= 3;
  }

  void Game::State::next ()
  {
    dart = 0;
    ++player;
    if (player == players)
    {
      ++round;
      player = 0;
    }
  }

  Game::Game (uint8_t players) :
    state {players}
  {
  }

  Game::~Game ()
  {
  }
}

