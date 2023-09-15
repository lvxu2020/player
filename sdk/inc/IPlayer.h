#ifndef IPLAYER_H
#define IPLAYER_H

#include <memory>
#include <iostream>
#include <string>


// 播放状态
enum class State {
    Stopped = 0,
    Playing,
    Paused
};

// 音量
enum class Volume{
    Min = 0,
    Max = 100
} ;

class IPlayer;
class PlayerLinstener {
public:
    virtual ~PlayerLinstener(){};
    virtual void stateChanged(IPlayer * player) {};
    virtual void timeChanged(IPlayer * player) {};
    virtual void initFinished(IPlayer * player) {};
    virtual void playFailed(IPlayer * player, std::string err) {};
    virtual void videoFrameDecoded(IPlayer * player, uint8_t *data, int size, int width, int height) {};
};

class IPlayer
{    
public:  
    IPlayer();
    virtual ~IPlayer(){};
    static IPlayer *createObj(PlayerLinstener* listener);
    virtual State getState() = 0;
    virtual void stop() = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void playFile(std::string str) = 0;
    // 文件时长
    virtual int getDuration() = 0;
    virtual int getPlayTime() = 0;
    virtual void setPlayTime(int time) = 0;
    // 声音 0-100
    virtual int getVolume() = 0;
    virtual void setVolume(int valume) = 0;
    // 静音
    virtual void setMute(bool mute) = 0;
    virtual bool isMute() = 0;
    // 倍数播放 4:2倍速 3:1.5倍速 2:1倍速 1:0.5倍数
    virtual void setPlaySpeed(int index) = 0;
};

#endif // IPLAYER_H
