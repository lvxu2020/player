#include "VideoWidget.h"
#include <QPainter>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent) {
    // 设置背景为黑色
    setAttribute(Qt::WA_StyledBackground);
    setStyleSheet("background: black");
}

VideoWidget::~VideoWidget()
{
    releaseImage();
}

void VideoWidget::onVideoFrameDecoded(IPlayer *player, uint8_t *data, int size, int width, int height)
{
    if (player->getState() == State::Stopped || size & width & height == 0) {
        return;
    }
    releaseImage();
    // 创建新的图片
    if (data != nullptr) {
        m_image = new QImage(data, width, height, QImage::Format_RGB888);

        // 组件的尺寸
        int w = VideoWidget::width();
        int h = VideoWidget::height();

        // 为了放入窗口，计算图片如何缩放
        int dw = width;
        int dh = height;
        int dx = 0;
        int dy = 0;
        

        if (dw > w || dh > h) { // 缩放
            // 判断 横纵向哪个方向缩放影响因子更大
            if (dw * h > w * dh) { // 视频的宽高比 > 播放器显示视频部分的宽高比
                dh = w * dh / dw;
                dw = w;
            } else {
                dw = h * dw / dh;
                dh = h;
            }
        }
        // 窗口 与 显示图片部分 差值除以2，为显示部分坐标
        dx = (w - dw) >> 1;
        dy = (h - dh) >> 1;
        m_showRect = QRect(dx, dy, dw, dh);
    }
    update();
}

void VideoWidget::onPlayerStateChanged(IPlayer *player)
{
    if (player->getState() == State::Stopped) {
        releaseImage();
        update();
    }

}

void VideoWidget::paintEvent(QPaintEvent *event)
{
    if (!m_image) {
        return;
    }
    QPainter(this).drawImage(m_showRect, *m_image);
}

void VideoWidget::releaseImage()
{
    if (m_image) {
        delete[] m_image->bits();
        delete m_image;
        m_image = nullptr;
    }
}
