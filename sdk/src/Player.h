#ifndef PLAYER_H
#define PLAYER_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersrc.h>
#include <libavfilter/buffersink.h>
#include <libavutil/imgutils.h>
#include <SDL/SDL.h>
}

#include "../inc/IPlayer.h"


#include <list>
#include <mutex>
#include <future>
#include <string>

class Player : public IPlayer
{
public:
    explicit Player(PlayerLinstener* listener);
    ~Player();
    State getState() override;
    void stop() override;
    void play() override;
    void pause() override;
    void playFile(std::string str) override;
    // 文件时长
    int getDuration() override;
    int getPlayTime() override;
    void setPlayTime(int time) override;
    // 声音 0-100
    int getVolume() override;
    void setVolume(int valume) override;
    // 静音
    void setMute(bool mute) override;
    bool isMute() override;
    // 倍数播放
    void setPlaySpeed(int index) override;
private:
    // 改变状态
    void setState(State state);
    // 初始化解码器和解码上下文
    int initDecoder(AVCodecContext **decodeCtx,AVStream **stream,AVMediaType type);
    // 读取文件数据
    void readFile();
    // 释放音视频所有资源资源
    void free();
    void freeAudio();
    void freeVideo();
    // 错误处理
    void playError(std::string err = "");
    void addFutureList(std::future<void> &fu);
    std::future<void> getFrontFuture();
    // 获取ffmpeg错误信息
    std::string getErrorStr(int ret);
    int seek();
private:
    // 解封装上下文
    AVFormatContext *m_fmtCtx = nullptr;
    std::string m_file;
    // 用于等待线程结束
    std::list<std::future<void>> m_threadFutures;
    std::mutex m_futureMtx;
    std::mutex m_stopMtx;

    // for UI start *****
    PlayerLinstener* m_listener;
    State m_state = State::Stopped;
    int m_valume = 0;
    int m_playTime = 0;
    bool m_mute = false;
    int m_speedIndex = 2;
    // ui设置的当前播放时刻
    int m_seekTime = -1;
    // for UI end *****

    // 音频 start *****
    // 初始化音频信息
    int initAudioInfo();
    // 初始化音频重采样
    int initSwr();
    // 初始化SDL
    int initSDL();
    // 添加数据包到音频包列表中
    void addAudioPkt(AVPacket &pkt);
    // 清空音频包列表
    void clearAudioPktList();
    // SDL填充缓冲区的回调函数
    static void sdlAudioCallbackFunc(void *userdata, Uint8 *stream, int len);
    // SDL填充缓冲区的回调函数
    void sdlAudioCallback(Uint8 *stream, int len);
    // 音频解码
    int decodeAudio();

    typedef struct {
        AVSampleFormat sampleFmt; // 采样格式
        int sampleRate; // 采样率 每秒采集的样本数量
        uint64_t  channelLayout; // 声道类型
        int channels; // 声道数量
        int onceSampleSize; // 重采样每一次采样数据大小（字节）
    } ResampeParam;
    //解码器上下文
    AVCodecContext *m_aDecodeCtx = nullptr;
    //音频流
    AVStream *m_aStream = nullptr;
    //音频包队列
    std::list<AVPacket> m_aPktList;
    //    CondMutex _aMutex;
    //音频队列锁
    std::mutex m_aMutex;
    //音频重采样上下文
    SwrContext *m_aSwrCtx = nullptr;
    //音频重采样输入、输出
    ResampeParam m_swrInParam, m_swrOutParam;
    //重采样frame输入、输出
    AVFrame *m_aSwrInFrame = nullptr, *m_aSwrOutFrame = nullptr;
    //音频重采样输出PCM的索引
    int m_aSwrOutIndex = 0;
    //音频重采样输出PCM的大小(字节)
    int m_aSwrOutSize = 0;
    //音频时钟，当前音频包对应的时间值
    double m_aTime = 0;
    //外面设置的当前播放时刻
    int m_aSeekTime = -1;
    // 是否有音频流
    bool m_hasAudio = false;
    // 音频 end *****

    // 视频 start *****
    // 初始化视频信息
    int initVideoInfo();
    // 初始化视频像素格式转换
    int initSws();
    // 添加数据包到视频包列表中
    void addVideoPkt(AVPacket &pkt);
    // 清空视频包列表
    void clearVideoPktList();
    // 解码视频
    void decodeVideo();

    typedef struct {
        AVPixelFormat pixFmt; // 图片格式
        int width;
        int height;
        int size; // 重采样后图片大小
    } VideoSwsParam;
    // 解码视频上下文
    AVCodecContext *m_vDecodeCtx = nullptr;
    // 视频流
    AVStream *m_vStream = nullptr;
    // 像素格式转换的输入、输出frame
    AVFrame *m_vSwsInFrame = nullptr, *m_vSwsOutFrame = nullptr;
    // 像素格式转换的上下文
    SwsContext *m_vSwsCtx = nullptr;
    //像素格式转换的输出frame的参数
    VideoSwsParam m_vSwsOutParam;
    // 视频包的列表
    std::list<AVPacket> m_vPktList;
//    CondMutex _vMutex;//视频包列表锁
    // 视频包列表锁
    std::mutex m_vMutex;
    // 视频时钟，当前视频包对应的时间值
    double m_vTime = 0;
    // 视频资源是否可以释放
    bool m_vCanFree = false;
    // 外面设置的当前播放时刻
    int m_vSeekTime = -1;
    // 是否有视频流
    bool m_hasVideo = false;
    // 视频帧率
    int m_frameRate = 25;
    // 视频 end *****

    // 过滤器 start *****
    // 倍数播放的过滤器 直接保存四个过滤器（0.5\1.0\1.5\2.0)
    // 过滤器链接图
    AVFilterGraph *m_graph_1 = nullptr;
    // 源过滤器上下文
    AVFilterContext *m_srcFilterCtx_1 = nullptr;
    // 接收过滤器上下文
    AVFilterContext *m_sinkFilterCtx_1 = nullptr;
    // 格式过滤器上下文
    AVFilterContext *m_formatFilterCtx_1 = nullptr;
    // 时间倍数控制过滤器上下文
    AVFilterContext *m_tempoFilterCtx_1 = nullptr;


    AVFilterGraph *m_graph_2 = nullptr;
    AVFilterContext *m_srcFilterCtx_2 = nullptr;
    AVFilterContext *m_sinkFilterCtx_2 = nullptr;
    AVFilterContext *m_formatFilterCtx_2 = nullptr;
    AVFilterContext *m_tempoFilterCtx_2 = nullptr;

    AVFilterGraph *m_graph_3 = nullptr;
    AVFilterContext *m_srcFilterCtx_3 = nullptr;
    AVFilterContext *m_sinkFilterCtx_3 = nullptr;
    AVFilterContext *m_formatFilterCtx_3 = nullptr;
    AVFilterContext *m_tempoFilterCtx_3 = nullptr;

    AVFilterGraph *m_graph_4 = nullptr;
    AVFilterContext *m_srcFilterCtx_4 = nullptr;
    AVFilterContext *m_sinkFilterCtx_4 = nullptr;
    AVFilterContext *m_formatFilterCtx_4 = nullptr;
    AVFilterContext *m_tempoFilterCtx_4 = nullptr;

    // 初始化过滤器
    int initFilter(AVFilterGraph **graph, AVFilterContext **srcFilterCtx,
                   AVFilterContext **sinkFilterCtx, AVFilterContext **formatFilterCtx,
                   AVFilterContext **tempoFilterCtx, const char *value);
    // 释放过滤器
    void freeFilter();
    // 过滤器 end *****
};

#endif // PLAYER_H
