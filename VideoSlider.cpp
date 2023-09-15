#include "VideoSlider.h"

#include <QMouseEvent>
#include <QStyle>

VideoSlider::VideoSlider(QWidget *parent)
{
    //监听鼠标轨迹
    setMouseTracking(true);
}

void VideoSlider::mouseReleaseEvent(QMouseEvent *ev)
{
    int value = QStyle::sliderValueFromPosition(minimum(),
                                                maximum(),
                                                ev->pos().x(),
                                                width());
    setValue(value);
    QSlider::mouseReleaseEvent(ev);
    emit clicked(this);
}


