#include "IPlayer.h"
#include "Player.h"

IPlayer::IPlayer()
{
}

IPlayer *IPlayer::createObj(PlayerLinstener *listener)
{
    return new Player(listener);
}

