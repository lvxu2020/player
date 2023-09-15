#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include "IPlayer.h"
#include "VideoSlider.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// 使用信号和槽的话 基类必须先继承 QObject
class MyListener :public QObject , public PlayerLinstener {
    Q_OBJECT
public:
    MyListener(){};
    void stateChanged(IPlayer * player) {emit sigStateChanged(player);};
    void timeChanged(IPlayer * player) {emit sigTimeChanged(player);};
    void initFinished(IPlayer * player) {emit sigInitFinished(player);};
    void playFailed(IPlayer * player, std::string err) {emit sigPlayFailed(player, QString::fromStdString(err));};
    void videoFrameDecoded(IPlayer * player, uint8_t *data, int size, int width, int height) {emit sigVideoFrameDecoded(player, data, size, width, height);};
signals:
    void sigStateChanged(IPlayer * player);
    void sigTimeChanged(IPlayer * player);
    void sigInitFinished(IPlayer * player);
    void sigPlayFailed(IPlayer * player, QString err);
    void sigVideoFrameDecoded(IPlayer * player, uint8_t *data, int size, int width, int height);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //播放器状态改变
    void onPlayerStateChanged(IPlayer * player);
    //进度条
    void onPlayerTimeChanged(IPlayer * player);
    // 播放文件所需解码器等ffmpeg资源准备完毕
    void onInitFinished(IPlayer *player);
    // 播放失败通知
    void onPlayFailed(IPlayer *player, QString err);

    void onTimeSliderClicked(VideoSlider *slider);
    void onVolumeSliderClicked(VideoSlider *slider);
    void on_openFileBtn_clicked();

    void on_volumeSlider_valueChanged(int value);

    void on_muteBtn_clicked();

    void on_playBtn_clicked();

    void on_stopBtn_clicked();

    void on_mutipleSpeed_currentIndexChanged(int index);

private:
    QString getTimeText(int value);
    void disconnectSig();
    void playComponentForbid();
private:
    Ui::MainWindow *ui;
    IPlayer* m_player = nullptr;
    MyListener* m_playerListener = nullptr;
};
#endif // MAINWINDOW_H
