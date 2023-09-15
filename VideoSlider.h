#ifndef VIDEOSLIDER_H
#define VIDEOSLIDER_H

#include <QSlider>
#include <QObject>
#include <QWidget>


class VideoSlider : public QSlider
{
    Q_OBJECT
public:
    explicit VideoSlider(QWidget *parent = nullptr);

signals:

    void clicked(VideoSlider *slider);

private:
    void mouseReleaseEvent(QMouseEvent *ev) override;
};

#endif // VIDEOSLIDER_H
