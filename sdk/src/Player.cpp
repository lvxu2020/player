#include "Player.h"
#include "../inc/Debug.h"

#define FOR_SDL_SAMPLES 512
#define AUDIO_MAX_PKT_SIZE 600
#define VIDEO_MAX_PKT_SIZE 300

Player::Player(PlayerLinstener *listener)
{
    m_listener = listener;
    //初始化SDL Audio系统
    int ret = -1;
    ret = SDL_Init(SDL_INIT_AUDIO);
    if (ret != 0) {
        DEBUG_E("init error %s", SDL_GetError());
        return;
    }
}

Player::~Player()
{
    m_state = State::Stopped;
    free();
    SDL_Quit();
}

void Player::stop()
{
    // 上层UI手动进行shop，read线程结束也会stop。加锁保证此函数线程安全
    std::lock_guard<std::mutex> mtx(m_stopMtx);
    // 内部状态抢先更新，资源释放结束之后返送停止信号
    m_state = State::Stopped;
    // 释放资源
    free();
    m_listener->stateChanged(this);
}

void Player::play()
{
    setState(State::Playing);
}

void Player::pause()
{
    setState(State::Paused);
}

void Player::playFile(std::string str)
{
    if (m_state != State::Stopped) {
        return;
    }

    m_file = str;
    auto fu = std::async(std::launch::async, &Player::readFile, this);
    addFutureList(fu);
}

int Player::getDuration()
{
    AVRational temp = {1, AV_TIME_BASE};

    if (m_fmtCtx) {
        auto it1 = m_fmtCtx->duration * av_q2d(temp);
        auto it = round(it1);
        return it; // 时间转化为秒
    }

    return 0;
}

int Player::getPlayTime()
{
    return round(m_aTime);
}

void Player::setPlayTime(int time)
{
    m_seekTime = time;
}

int Player::getVolume()
{
    return m_valume;
}

void Player::setVolume(int valume)
{
    m_valume = valume;
}

void Player::setMute(bool mute)
{
    m_mute = mute;
}

bool Player::isMute()
{
    return m_mute;
}

void Player::setPlaySpeed(int index)
{
    m_speedIndex = index;
}

void Player::setState(State state)
{
    if (state != m_state) {
        m_state = state;
        m_listener->stateChanged(this);
    }
}

int Player::initDecoder(AVCodecContext **decodeCtx, AVStream **stream, AVMediaType type)
{
    // 根据type寻找最合适的流信息 返回流索引
    int ret = av_find_best_stream(m_fmtCtx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    // 检验流
    int streamIndex = ret;
    *stream = m_fmtCtx->streams[streamIndex];
    if (!*stream) {
        DEBUG_E("stream is empty");
        return -1;
    }

    // 为当前流找到合适的解码器
    const AVCodec *decoder = avcodec_find_decoder((*stream)->codecpar->codec_id);
    if (!decoder) {
        DEBUG_E("decoder not found: %d", (*stream)->codecpar->codec_id);
        return -1;
    }

    // 初始化解码上下文
    *decodeCtx = avcodec_alloc_context3(decoder);
    if (!decodeCtx) {
        DEBUG_E("avcodec_alloc_context3 error");
        return -1;
    }

    // 从流中拷贝参数到解码上下文中
    ret = avcodec_parameters_to_context(*decodeCtx, (*stream)->codecpar);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    // 打开解码器
    ret = avcodec_open2(*decodeCtx, decoder, nullptr);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    return 0;
}

void Player::readFile()
{
    int ret = 0;
    //创建解封装上下文、打开文件
    ret = avformat_open_input(&m_fmtCtx, m_file.c_str(), nullptr, nullptr);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return;
    }

    //检索流信息
    ret = avformat_find_stream_info(m_fmtCtx, nullptr);
    if (ret < 0) {
        if (m_fmtCtx) {
            avformat_close_input(&m_fmtCtx);
            m_fmtCtx = nullptr;
        }
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return;
    }

    //初始化音频信息
    m_hasAudio = initAudioInfo() >= 0;
    // 目前只支持有音频的多媒体文件，同步方式以音频为主
    if (!m_hasAudio) {
        if (m_fmtCtx) {
            avformat_close_input(&m_fmtCtx);
            m_fmtCtx = nullptr;
        }
        freeAudio();
        playError("没有音频无法播放");
        return;
    }
    //初始化视频信息
    m_hasVideo = initVideoInfo() >= 0;
    if (!m_hasAudio && !m_hasVideo) {
        DEBUG_E("!m_hasAudio && !m_hasVideo");
        playError("没有视频和音频 ");
        return;
    }

    //初始化倍数过滤器
    ret = initFilter(&m_graph_1, &m_srcFilterCtx_1, &m_sinkFilterCtx_1, &m_formatFilterCtx_1, &m_tempoFilterCtx_1, "0.5");
    if (ret < 0) {
        DEBUG_E("initFilter fail");
        return;
    }
    ret = initFilter(&m_graph_2, &m_srcFilterCtx_2, &m_sinkFilterCtx_2, &m_formatFilterCtx_2, &m_tempoFilterCtx_2, "1.0");
    if (ret < 0) {
        DEBUG_E("initFilter fail");
        return;
    }
    ret = initFilter(&m_graph_3, &m_srcFilterCtx_3, &m_sinkFilterCtx_3, &m_formatFilterCtx_3, &m_tempoFilterCtx_3, "1.5");
    if (ret < 0) {
        DEBUG_E("initFilter fail");
        return;
    }
    ret = initFilter(&m_graph_4, &m_srcFilterCtx_4, &m_sinkFilterCtx_4, &m_formatFilterCtx_4, &m_tempoFilterCtx_4, "2.0");
    if (ret < 0) {
        DEBUG_E("initFilter fail");
        return;
    }

    m_listener->initFinished(this);

    //音频解码
    SDL_PauseAudio(0);

    //视频解码
    auto fu = std::async(std::launch::async, &Player::decodeVideo, this);
    addFutureList(fu);

    setState(State::Playing);

    // 解封装
    //从输入文件中读取数据
    AVPacket pkt;
    while (m_state != State::Stopped) {
        // 处理seek操作
        if (m_seekTime >= 0) {
            seek();
        }

        int vSize = m_vPktList.size();
        int aSize = m_aPktList.size();

        if (vSize >= VIDEO_MAX_PKT_SIZE ||
            aSize >= AUDIO_MAX_PKT_SIZE) {
            continue;
        }

        ret = av_read_frame(m_fmtCtx, &pkt);
        if (ret == 0) {
            if (m_hasAudio && pkt.stream_index == m_aStream->index) {
                addAudioPkt(pkt);
            } else if (m_hasVideo && pkt.stream_index == m_vStream->index) {
                addVideoPkt(pkt);
            } else { //非视频、音频包
                av_packet_unref(&pkt);
            }
        } else if (ret == AVERROR_EOF) { // 读到了文件的尾部
            // 文件读完后 队列也播放完了就退出循环，发信号
            if (m_aPktList.empty() && m_aPktList.empty())
                break;
        } else {
            DEBUG_E("%s", getErrorStr(ret).c_str());
            continue;
        }
    }

    std::thread([this](){
        stop();
    }).detach();
}

void Player::free()
{
    // 等待线程结束
    {
        std::lock_guard<std::mutex> mtx(m_futureMtx);
        while (!m_threadFutures.empty()) {
            if (m_threadFutures.front().valid()) {
                m_threadFutures.front().get();
            } else {
                DEBUG_I("future invalid!");
            }
            m_threadFutures.pop_front();
        }
    }

    if (m_fmtCtx) {
        avformat_close_input(&m_fmtCtx);
        m_fmtCtx = nullptr;
    }

    m_seekTime = -1;
    freeAudio();
    freeVideo();
    freeFilter();
}

void Player::freeAudio()
{
    // 先释放sdl音频系统，防止声音回调函数没执行完
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    clearAudioPktList();
    m_aTime = 0;
    m_aSwrOutIndex = 0;
    m_aSwrOutSize = 0;
    m_aStream = nullptr;
    m_aSeekTime = -1;
    avcodec_free_context(&m_aDecodeCtx);
    m_aDecodeCtx = nullptr;
    swr_free(&m_aSwrCtx);
    m_aSwrCtx = nullptr;
    av_frame_free(&m_aSwrInFrame);
    m_aSwrInFrame = nullptr;
    if (m_aSwrOutFrame) {
        av_freep(&m_aSwrOutFrame->data[0]);
        av_frame_free(&m_aSwrOutFrame);
        m_aSwrOutFrame = nullptr;
    }
}

void Player::freeVideo()
{
    clearVideoPktList();
    avcodec_free_context(&m_vDecodeCtx);
    m_vTime = 0;
    m_vCanFree = false;
    m_vSeekTime = -1;
    m_vDecodeCtx = nullptr;
    av_frame_free(&m_vSwsInFrame);
    m_vSwsInFrame = nullptr;
    if (m_vSwsOutFrame) {
        av_freep(&m_vSwsOutFrame->data[0]);
        av_frame_free(&m_vSwsOutFrame);
        m_vSwsOutFrame = nullptr;
    }
    sws_freeContext(m_vSwsCtx);
    m_vSwsCtx = nullptr;
    m_vStream = nullptr;

}

void Player::playError(std::string err)
{
    m_listener->playFailed(this, err);
}

void Player::addFutureList(std::future<void> &fu)
{
    std::lock_guard<std::mutex> mtx(m_futureMtx);
    m_threadFutures.emplace_back(std::move(fu));
}

std::future<void> Player::getFrontFuture()
{
    std::lock_guard<std::mutex> mtx(m_futureMtx);
    std::future<void> res = std::move(m_threadFutures.front());
    m_threadFutures.pop_front();
    return res;
}

std::string Player::getErrorStr(int ret)
{
    char errbuf[1024];
    av_strerror(ret, errbuf, sizeof (errbuf));;
    return std::string(errbuf);
}

int Player::seek()
{
    AVPacket keypkt;
    int streamIdx, ret;
    // 选择使用流作为时间基准 seek
    if (m_hasAudio) {
        streamIdx = m_aStream->index;
    }else{
        streamIdx = m_vStream->index;
    }

    // 定位到seekTime前
    AVRational timeBase = m_fmtCtx->streams[streamIdx]->time_base;
    int64_t time;
    time = m_seekTime / av_q2d(timeBase);

    ret = av_seek_frame(m_fmtCtx, streamIdx, time, AVSEEK_FLAG_BACKWARD);
    if (ret < 0) { // seek失败
        DEBUG_I("seek失败 m_seekTime: %d, time: %lld", m_seekTime, time);
        m_seekTime = -1;
        return ret;
    } else {
        // 清空数据包
        clearAudioPktList();
        clearVideoPktList();
        // 恢复时钟
        m_aTime = 0;
        m_vTime = 0;
        // 设置音视频的seek
        {
            m_aSeekTime = m_seekTime;
            m_vSeekTime = m_seekTime;
            m_seekTime = -1;
        }

        // 有视频的话 以视频的关键字为准
        if (m_hasVideo) {
            while (av_read_frame(m_fmtCtx, &keypkt) >= 0) {
                if(keypkt.flags & AV_PKT_FLAG_KEY){
                    addVideoPkt(keypkt);
                    break;
                } else {
                    av_packet_unref(&keypkt);
                }
            }
        }
    }
    return 0;
}

int Player::initAudioInfo()
{
    // 初始化解码器
    int ret = initDecoder(&m_aDecodeCtx, &m_aStream, AVMEDIA_TYPE_AUDIO);
    if (ret < 0) {
        DEBUG_E("initDecoder falid");
        return ret;
    }

    // 初始化音频重采样
    ret = initSwr();
    if (ret < 0) {
        DEBUG_E("initSwr falid");
        return ret;
    }

    // 初始化SDL
    ret = initSDL();
    if (ret < 0) {
        DEBUG_E("initSDL falid");
        return ret;
    }
    return 0;
}

int Player::initSwr()
{
    // 重采样输入参数
    m_swrInParam.sampleFmt = m_aDecodeCtx->sample_fmt;
    m_swrInParam.sampleRate = m_aDecodeCtx->sample_rate;
    m_swrInParam.channelLayout = m_aDecodeCtx->channel_layout;
    m_swrInParam.channels = m_aDecodeCtx->channels;

    // 重采样输出参数
    m_swrOutParam.sampleFmt = AV_SAMPLE_FMT_S16;
    m_swrOutParam.sampleRate = 44100;
    m_swrOutParam.channelLayout = AV_CH_LAYOUT_STEREO; // 双声道立体声
    m_swrOutParam.channels = av_get_channel_layout_nb_channels(m_swrOutParam.channelLayout);
    m_swrOutParam.onceSampleSize = m_swrOutParam.channels *
                                       av_get_bytes_per_sample(m_swrOutParam.sampleFmt);

    // 创建重采样上下文
    m_aSwrCtx = swr_alloc_set_opts(nullptr,
                                  m_swrOutParam.channelLayout,m_swrOutParam.sampleFmt,m_swrOutParam.sampleRate,
                                  m_swrInParam.channelLayout,m_swrInParam.sampleFmt,m_swrInParam.sampleRate,
                                  0, nullptr);
    if (!m_aSwrCtx) {
        DEBUG_E("swr_alloc_set_opts error");
        return -1;
    }

    // 初始化重采样上下文
    int ret = swr_init(m_aSwrCtx);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    // 初始化重采样的输入frame
    m_aSwrInFrame = av_frame_alloc();
    if (!m_aSwrInFrame) {
        DEBUG_E("%s", "av_frame_alloc error");
        return -1;
    }

    // 初始化重采样的输出frame
    m_aSwrOutFrame = av_frame_alloc();
    if (!m_aSwrOutFrame) {
        DEBUG_E("%s", "av_frame_alloc error");
        return -1;
    }

    // 初始化输出frame data空间
    ret = av_samples_alloc(m_aSwrOutFrame->data, m_aSwrOutFrame->linesize,
                           m_swrOutParam.channels, (8 * FOR_SDL_SAMPLES),
                           m_swrOutParam.sampleFmt, 1/*使用默认对齐*/);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    return 0;
}

int Player::initSDL()
{
    SDL_AudioSpec spec;
    spec.freq = m_swrOutParam.sampleRate;
    spec.format = AUDIO_S16LSB; /**< 对应ffmpeg的 AV_SAMPLE_FMT_S16 */
    spec.channels = m_swrOutParam.channels;
    spec.samples = FOR_SDL_SAMPLES; //音频缓冲区的样本数量
    spec.callback = sdlAudioCallbackFunc;
    spec.userdata = this;

    // 打开音频设备
    if (SDL_OpenAudio(&spec, nullptr)) {
        DEBUG_E("poen error:%s", SDL_GetError());
        return -1;
    }

    return 0;
}

void Player::addAudioPkt(AVPacket &pkt)
{
    std::lock_guard<std::mutex> mtx(m_aMutex);
    m_aPktList.push_back(pkt);
}

void Player::clearAudioPktList()
{
    std::lock_guard<std::mutex> mtx(m_aMutex);
    for (auto &pkt : m_aPktList) av_packet_unref(&pkt);
    m_aPktList.clear();
}

void Player::sdlAudioCallbackFunc(void *userdata, Uint8 *stream, int len)
{
    Player *player = nullptr;
    player = static_cast<Player*>(userdata);
    if (player) {
        player->sdlAudioCallback(stream, len);
    } else {
        DEBUG_E("player null");
    }
}

void Player::sdlAudioCallback(Uint8 *stream, int len)
{
    // 清零
    SDL_memset(stream, 0, len);
//    DEBUG_I("%d++++%d", len, m_swrOutParam.onceSampleSize); // len = FOR_SDL_SAMPLES * 声道数 * 2
    while (len > 0) {
        if (m_state != State::Playing) break;

        //解码pkt，获取新的PCM数据
        if (m_aSwrOutIndex >= m_aSwrOutSize) {
            //获取新的包解码出来的PCM
            m_aSwrOutSize = decodeAudio();
            m_aSwrOutIndex = 0;
            //没有解码数据 播放静音数据
            if (m_aSwrOutSize <= 0) {
                m_aSwrOutSize = 1024;
                memset(m_aSwrOutFrame->data[0], 0, m_aSwrOutSize);
            }
        }

        // 本次需要填充到stream中的PCM数据大小
        int fillLen = m_aSwrOutSize - m_aSwrOutIndex;
        fillLen = std::min(fillLen, len);

        // 获取当前音量
        int volume = m_mute ? 0 :((m_valume * 1.0 / (double)Volume::Max) * SDL_MIX_MAXVOLUME);
        // 填充SDL缓冲区
        SDL_MixAudio(stream,
                     m_aSwrOutFrame->data[0] + m_aSwrOutIndex,
                     fillLen, volume);

        // 移动偏移量
        len -= fillLen;
        stream += fillLen;
        m_aSwrOutIndex += fillLen;
    }
}

int Player::decodeAudio()
{
    m_aMutex.lock();
    if (m_aPktList.empty()) {
        m_aMutex.unlock();
        return 0;
    }

    //取出数据包
    AVPacket pkt = m_aPktList.front();
    m_aPktList.pop_front();
    m_aMutex.unlock();

    // 保存音频时钟
    if (pkt.pts != AV_NOPTS_VALUE) {//音频时钟无效判断
        m_aTime = av_q2d(m_aStream->time_base) * pkt.pts;//获取包时间
        // 通知外界：播放时间点发生了改变
        m_listener->timeChanged(this);
    }

    if (m_aSeekTime >= 0) {
        if (m_aTime < m_aSeekTime) {
            // 释放pkt
            av_packet_unref(&pkt);
            return 0;
        } else {
            m_aSeekTime = -1;
        }
    }

    // 发送压缩数据到解码器
    int ret = avcodec_send_packet(m_aDecodeCtx, &pkt);
    av_packet_unref(&pkt);
    if (ret != 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return 0;
    }

    // 获取解码后的数据 音频只接收一次丢几帧数据无所谓
    ret = avcodec_receive_frame(m_aDecodeCtx, m_aSwrInFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    //    DEBUG_I("%s", getErrorStr(ret).c_str());
        return 0;
    }

    //解码后数据送入过滤器

    switch(m_speedIndex){
    case 1:
        av_buffersrc_add_frame(m_srcFilterCtx_1, m_aSwrInFrame);
        av_buffersink_get_frame(m_sinkFilterCtx_1, m_aSwrInFrame);
        break;
    case 2:
        av_buffersrc_add_frame(m_srcFilterCtx_2, m_aSwrInFrame);
        av_buffersink_get_frame(m_sinkFilterCtx_2, m_aSwrInFrame);
        break;
    case 3:
        av_buffersrc_add_frame(m_srcFilterCtx_3, m_aSwrInFrame);
        av_buffersink_get_frame(m_sinkFilterCtx_3, m_aSwrInFrame);
        break;
    case 4:
        av_buffersrc_add_frame(m_srcFilterCtx_4, m_aSwrInFrame);
        av_buffersink_get_frame(m_sinkFilterCtx_4, m_aSwrInFrame);
        break;
    default:
        av_buffersrc_add_frame(m_srcFilterCtx_2, m_aSwrInFrame);
        av_buffersink_get_frame(m_sinkFilterCtx_2, m_aSwrInFrame);
    };


    // 重采样输出的样本数
    //a*b/c 向上取整
    int outSamples = av_rescale_rnd(m_swrOutParam.sampleRate,
                                    m_aSwrInFrame->nb_samples,
                                    m_swrInParam.sampleRate, AV_ROUND_UP);

    // 重采样 获取采样数
    //return number of samples output per channel, negative value on error
    ret = swr_convert(m_aSwrCtx,
                      m_aSwrOutFrame->data,
                      outSamples,
                      (const uint8_t **)m_aSwrInFrame->data,
                      m_aSwrInFrame->nb_samples);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return 0;
    }

    return ret * m_swrOutParam.onceSampleSize;
}

int Player::initVideoInfo()
{
    // 初始化解码器
    int ret = initDecoder(&m_vDecodeCtx, &m_vStream, AVMEDIA_TYPE_VIDEO);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return 0;
    }

    //视频帧率
    m_frameRate = m_vStream->avg_frame_rate.num / m_vStream->avg_frame_rate.den;

    // 初始化像素格式转换
    ret = initSws();
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return 0;
    }

    return 0;
}

int Player::initSws()
{
    // 输出frame的参数
    m_vSwsOutParam.width = m_vDecodeCtx->width >> 4 << 4;
    m_vSwsOutParam.height = m_vDecodeCtx->height >> 4 << 4;
    m_vSwsOutParam.pixFmt = AV_PIX_FMT_RGB24;
    m_vSwsOutParam.size = av_image_get_buffer_size(
        m_vSwsOutParam.pixFmt,
        m_vSwsOutParam.width,
        m_vSwsOutParam.height, 1);

    // 初始化像素格式转换的上下文
    m_vSwsCtx = sws_getContext(m_vDecodeCtx->width, m_vDecodeCtx->height, m_vDecodeCtx->pix_fmt,
                              m_vSwsOutParam.width,m_vSwsOutParam.height,m_vSwsOutParam.pixFmt,
                              SWS_BILINEAR, nullptr, nullptr, nullptr); // SWS_BILINEAR 插值算法缩放的时候保证图片质量
    if (!m_vSwsCtx) {
        DEBUG_E("sws_getContext error");
        return -1;
    }

    // 初始化像素格式转换的输入frame
    m_vSwsInFrame = av_frame_alloc();
    if (!m_vSwsInFrame) {
        DEBUG_E("av_frame_alloc error");
        return -1;
    }

    // 初始化像素格式转换的输出frame
    m_vSwsOutFrame = av_frame_alloc();
    if (!m_vSwsOutFrame) {
        DEBUG_E("av_frame_alloc error");
        return -1;
    }

    // 分配m_vSwsOutFrame中data控件
    int ret = av_image_alloc(m_vSwsOutFrame->data,
                             m_vSwsOutFrame->linesize,
                             m_vSwsOutParam.width,
                             m_vSwsOutParam.height,
                             m_vSwsOutParam.pixFmt,
                             1);
    if (ret < 0) {
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return ret;
    }

    return 0;
}

void Player::addVideoPkt(AVPacket &pkt)
{
    std::lock_guard<std::mutex> mtx(m_vMutex);
    m_vPktList.push_back(pkt);
}

void Player::clearVideoPktList()
{
    std::lock_guard<std::mutex> mtx(m_vMutex);
    for (auto &pkt : m_vPktList) av_packet_unref(&pkt);
    m_vPktList.clear();
}

void Player::decodeVideo()
{
    while (true) {
        // 如果是暂停，并且没有Seek操作
        if (m_state == State::Paused && m_vSeekTime == -1) {
            std::this_thread::yield();
            continue;
        }
        if (m_state == State::Stopped) {
            break;
        }

        m_vMutex.lock();

        if (m_vPktList.empty()) {
            m_vMutex.unlock();
            continue;
        }

        // 取出头部的视频包
        AVPacket pkt = m_vPktList.front();
        m_vPktList.pop_front();
        m_vMutex.unlock();

        // 视频时钟
        if (pkt.dts != AV_NOPTS_VALUE) {
            m_vTime = av_q2d(m_vStream->time_base) * pkt.dts;

        }

        // 发送压缩数据到解码器
        int ret = avcodec_send_packet(m_vDecodeCtx, &pkt);
        av_packet_unref(&pkt);
        if (ret < 0) {
            DEBUG_I("%s", getErrorStr(ret).c_str());
            continue;
        }

        while (true) {
            // 获取解码后的数据
            ret = avcodec_receive_frame(m_vDecodeCtx, m_vSwsInFrame);
            if (ret != 0) {
              //  DEBUG_I("%s", getErrorStr(ret).c_str());
                break;
            }

            // 发现视频的时间是早于seekTime的，直接丢弃
            if (m_vSeekTime >= 0) {
                if (m_vTime < m_vSeekTime) {
                    continue;
                } else {
                    m_vSeekTime = -1;
                }
            }

            // 像素格式的转换
            sws_scale(m_vSwsCtx,
                      m_vSwsInFrame->data, m_vSwsInFrame->linesize,
                      0, m_vDecodeCtx->height,
                      m_vSwsOutFrame->data, m_vSwsOutFrame->linesize);

            if (m_hasAudio) {
                // 等待音频包同步
                while (m_vTime >= m_aTime && m_state == State::Playing) {
                    std::this_thread::yield();
                }
            } else {
                // 暂时不实现
            }

            // 把像素格式转换后的图片数据，拷贝一份出来
            uint8_t *data = new uint8_t[m_vSwsOutParam.size]();
            memcpy(data, m_vSwsOutFrame->data[0], m_vSwsOutParam.size);
            m_listener->videoFrameDecoded(this, data, m_vSwsOutParam.size, m_vSwsOutParam.width,
                                          m_vSwsOutParam.height);
        }
    }
}

int Player::initFilter(AVFilterGraph **graph, AVFilterContext **srcFilterCtx,
                       AVFilterContext **sinkFilterCtx, AVFilterContext **formatFilterCtx,
                       AVFilterContext **tempoFilterCtx, const char *value)
{
    *graph = avfilter_graph_alloc();

    // 源过滤器和格式转换过滤器参数
    std::string s1 = "sample_rate=" +
                     std::to_string(m_aDecodeCtx->sample_rate) +
                     ":sample_fmt=" +
                     av_get_sample_fmt_name(m_aDecodeCtx->sample_fmt) +
                     ":channel_layout=" +
                     std::to_string(m_aDecodeCtx->channel_layout);
    std::string s2 = "sample_rates=" +
                     std::to_string(m_aDecodeCtx->sample_rate) +
                     ":sample_fmts=" +
                     av_get_sample_fmt_name(m_aDecodeCtx->sample_fmt) +
                     ":channel_layouts=" +
                     std::to_string(m_aDecodeCtx->channel_layout);

    // 创建源过滤器
    const AVFilter *srcFilter = avfilter_get_by_name("abuffer");
    *srcFilterCtx = avfilter_graph_alloc_filter(*graph, srcFilter, "src");
    if (avfilter_init_str(*srcFilterCtx, s1.c_str()) < 0) {
        DEBUG_E("init filter fail");
        return -1;
    }


    //创建变速过滤器
    const AVFilter *atempoFilter = avfilter_get_by_name("atempo");
    *tempoFilterCtx = avfilter_graph_alloc_filter(*graph, atempoFilter, "atempo");
    AVDictionary *args = nullptr;
    av_dict_set(&args, "tempo", value, 0);//根据value的值调节速度
    if (avfilter_init_dict(*tempoFilterCtx, &args) < 0) {
        DEBUG_E("init filter fail");
        return -1;
    }


    //创建格式转化过滤器
    const AVFilter *aformatFilter = avfilter_get_by_name("aformat");
    *formatFilterCtx = avfilter_graph_alloc_filter(*graph, aformatFilter, "aformat");
    if (avfilter_init_str(*formatFilterCtx, s2.c_str()) < 0) {
        DEBUG_E("init filter fail");
        return -1;
    }


    //创建接收过滤器
    const AVFilter *sinkFilter = avfilter_get_by_name("abuffersink");
    *sinkFilterCtx = avfilter_graph_alloc_filter(*graph, sinkFilter, "sink");
    if (avfilter_init_dict(*sinkFilterCtx, nullptr) < 0) {
        DEBUG_E("init filter fail");
        return -1;
    }

    //链接过滤器
    int ret = avfilter_link(*srcFilterCtx, 0, *tempoFilterCtx, 0);
    if(ret != 0){
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return -1;
    }
    ret = avfilter_link(*tempoFilterCtx, 0, *formatFilterCtx, 0);
    if(ret != 0){
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return -1;
    }
    ret = avfilter_link(*formatFilterCtx, 0, *sinkFilterCtx, 0);
    if(ret != 0){
        DEBUG_E("%s", getErrorStr(ret).c_str());
        return -1;
    }


    //配置图
    if (avfilter_graph_config(*graph, nullptr) < 0) {
        DEBUG_E("link filter fail");
        return -1;
    }

    return 0;
}

void Player::freeFilter()
{
    if (m_srcFilterCtx_1) {
        avfilter_free(m_srcFilterCtx_1);
        m_srcFilterCtx_1 = nullptr;
    }
    if (m_sinkFilterCtx_1) {
        avfilter_free(m_sinkFilterCtx_1);
        m_sinkFilterCtx_1 = nullptr;
    }
    if (m_formatFilterCtx_1) {
        avfilter_free(m_formatFilterCtx_1);
        m_formatFilterCtx_1 = nullptr;
    }
    if (m_tempoFilterCtx_1) {
        avfilter_free(m_tempoFilterCtx_1);
        m_tempoFilterCtx_1 = nullptr;
    }
    if (m_graph_1) {
        avfilter_graph_free(&m_graph_1);
        m_graph_1 = nullptr;
    }

    if (m_srcFilterCtx_2) {
        avfilter_free(m_srcFilterCtx_2);
        m_srcFilterCtx_2 = nullptr;
    }
    if (m_sinkFilterCtx_2) {
        avfilter_free(m_sinkFilterCtx_2);
        m_sinkFilterCtx_2 = nullptr;
    }
    if (m_formatFilterCtx_2) {
        avfilter_free(m_formatFilterCtx_2);
        m_formatFilterCtx_2 = nullptr;
    }
    if (m_tempoFilterCtx_2) {
        avfilter_free(m_tempoFilterCtx_2);
        m_tempoFilterCtx_2 = nullptr;
    }
    if (m_graph_2) {
        avfilter_graph_free(&m_graph_2);
        m_graph_2 = nullptr;
    }

    if (m_srcFilterCtx_3) {
        avfilter_free(m_srcFilterCtx_3);
        m_srcFilterCtx_3 = nullptr;
    }
    if (m_sinkFilterCtx_3) {
        avfilter_free(m_sinkFilterCtx_3);
        m_sinkFilterCtx_3 = nullptr;
    }
    if (m_formatFilterCtx_3) {
        avfilter_free(m_formatFilterCtx_3);
        m_formatFilterCtx_3 = nullptr;
    }
    if (m_tempoFilterCtx_3) {
        avfilter_free(m_tempoFilterCtx_3);
        m_tempoFilterCtx_3 = nullptr;
    }
    if (m_graph_3) {
        avfilter_graph_free(&m_graph_3);
        m_graph_3 = nullptr;
    }

    if (m_srcFilterCtx_4) {
        avfilter_free(m_srcFilterCtx_4);
        m_srcFilterCtx_4 = nullptr;
    }
    if (m_sinkFilterCtx_4) {
        avfilter_free(m_sinkFilterCtx_4);
        m_sinkFilterCtx_4 = nullptr;
    }
    if (m_formatFilterCtx_4) {
        avfilter_free(m_formatFilterCtx_4);
        m_formatFilterCtx_4 = nullptr;
    }
    if (m_tempoFilterCtx_4) {
        avfilter_free(m_tempoFilterCtx_4);
        m_tempoFilterCtx_4 = nullptr;
    }
    if (m_graph_4) {
        avfilter_graph_free(&m_graph_4);
        m_graph_4 = nullptr;
    }
}

State Player::getState()
{
    return m_state;
}
