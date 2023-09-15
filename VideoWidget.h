#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QImage>
#include "IPlayer.h"

class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
public slots:
    // 解码视频帧完成
    void onVideoFrameDecoded(IPlayer * player, uint8_t *data, int size, int width, int height);
    void onPlayerStateChanged(IPlayer * player);
private:
    void paintEvent(QPaintEvent *event) override;
    void releaseImage();
private:
    QImage *m_image = nullptr;
    QRect m_showRect;
};

#endif // VIDEOWIDGET_H
