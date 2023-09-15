#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_playerListener = new MyListener();
    m_player = IPlayer::createObj(m_playerListener);
    //音量设置
    ui->volumeSlider->setRange(0, 100);
    ui->volumeSlider->setValue(ui->volumeSlider->maximum() >> 1);
    // 倍速播放
    ui->mutipleSpeed->setCurrentIndex(2);
    // UI空间操作 start
    connect(ui->timeSlider, &VideoSlider::clicked,
            this, &MainWindow::onTimeSliderClicked);
    connect(ui->volumeSlider, &VideoSlider::clicked,
            this, &MainWindow::onVolumeSliderClicked);
    // UI空间操作 end
    // 引擎上报 start
    connect(m_playerListener, &MyListener::sigStateChanged,
            this, &MainWindow::onPlayerStateChanged);
    connect(m_playerListener, &MyListener::sigTimeChanged,
            this, &MainWindow::onPlayerTimeChanged);
    connect(m_playerListener, &MyListener::sigInitFinished,
            this, &MainWindow::onInitFinished);
    connect(m_playerListener, &MyListener::sigPlayFailed,
            this, &MainWindow::onPlayFailed);
    connect(m_playerListener, &MyListener::sigVideoFrameDecoded,
            ui->videoWidget, &VideoWidget::onVideoFrameDecoded);
    connect(m_playerListener, &MyListener::sigStateChanged,
            ui->videoWidget, &VideoWidget::onPlayerStateChanged);
    // 引擎上报 end
    playComponentForbid();

}

MainWindow::~MainWindow()
{
    disconnectSig();
    m_player->stop();
    delete ui;
    delete m_playerListener;
    m_playerListener = nullptr;
    delete m_player;
}


void MainWindow::on_openFileBtn_clicked()
{
    QString qstrFileName = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("file (*.mp4 *.mp3 *.mov)"));
    if (!qstrFileName.isEmpty()) {
        m_player->playFile(qstrFileName.toStdString());
    }
}


void MainWindow::on_volumeSlider_valueChanged(int value)
{
    ui->volumeLabel->setText(QString("%1").arg(value));
    //调整音量时解除静音
    if(m_player->isMute())
    {
        ui->muteBtn->setIcon(QIcon(":/new/prefix/aloud.png"));
        m_player->setMute(false);
    }

    m_player->setVolume(value);
}

void MainWindow::onPlayerStateChanged(IPlayer * player)
{
    auto state = player->getState();
    if (state == State::Playing) {
        ui->playBtn->setIcon(QIcon(":/new/prefix/pause.png"));
    } else {
        ui->playBtn->setIcon(QIcon(":/new/prefix/play.png"));
    }

    if (state == State::Stopped) {
        //禁止按钮
        playComponentForbid();

    } else {
        ui->playBtn->setEnabled(true);
        ui->stopBtn->setEnabled(true);
        ui->timeSlider->setEnabled(true);
        ui->volumeSlider->setEnabled(true);
        ui->mutipleSpeed->setEnabled(true);
        ui->muteBtn->setEnabled(true);
        ui->openFileBtn->setHidden(true);       
    }
}

void MainWindow::onPlayerTimeChanged(IPlayer * player)
{
    int time = player->getPlayTime();
    ui->timeLabel->setText(getTimeText(time));
    ui->timeSlider->setValue(time);
}

void MainWindow::onInitFinished(IPlayer * player)
{
    int duration = player->getDuration();
    ui->timeSlider->setRange(0, duration);
    ui->durationLabel->setText(getTimeText(duration));
}

void MainWindow::onPlayFailed(IPlayer * player, QString err)
{
    QMessageBox::critical(nullptr, "提示", err);
}

void MainWindow::onTimeSliderClicked(VideoSlider *slider)
{
    if (m_player) {
        m_player->setPlayTime(slider->value());
    }
}

void MainWindow::onVolumeSliderClicked(VideoSlider *slider)
{
    if (m_player) {
        m_player->setVolume(slider->value());
    }
}

QString MainWindow::getTimeText(int value)
{
    //获取XX：XX：XX格式的时间文本
    QLatin1Char fill = QLatin1Char('0');
    return QString("%1:%2:%3")
        .arg(value / 3600, 2, 10, fill)
        .arg((value / 60) % 60, 2, 10, fill)
        .arg(value % 60, 2, 10, fill);
}

void MainWindow::disconnectSig()
{
    // UI空间操作 start
    disconnect(ui->timeSlider, &VideoSlider::clicked,
            this, &MainWindow::onTimeSliderClicked);
    disconnect(ui->volumeSlider, &VideoSlider::clicked,
            this, &MainWindow::onVolumeSliderClicked);
    // UI空间操作 end
    // 引擎上报 start
    disconnect(m_playerListener, &MyListener::sigStateChanged,
            this, &MainWindow::onPlayerStateChanged);
    disconnect(m_playerListener, &MyListener::sigTimeChanged,
            this, &MainWindow::onPlayerTimeChanged);
    disconnect(m_playerListener, &MyListener::sigInitFinished,
            this, &MainWindow::onInitFinished);
    disconnect(m_playerListener, &MyListener::sigPlayFailed,
            this, &MainWindow::onPlayFailed);
    disconnect(m_playerListener, &MyListener::sigVideoFrameDecoded,
            ui->videoWidget, &VideoWidget::onVideoFrameDecoded);
    disconnect(m_playerListener, &MyListener::sigStateChanged,
            ui->videoWidget, &VideoWidget::onPlayerStateChanged);
    // 引擎上报 end
}

void MainWindow::playComponentForbid()
{
    //禁止按钮
    ui->playBtn->setEnabled(false);
    ui->stopBtn->setEnabled(false);
    ui->timeSlider->setEnabled(false);
    ui->volumeSlider->setEnabled(false);
    ui->mutipleSpeed->setEnabled(false);
    ui->muteBtn->setEnabled(false);
    ui->durationLabel->setText(getTimeText(0));
    ui->timeSlider->setValue(0);
    ui->openFileBtn->setHidden(false);
    ui->timeLabel->setText(getTimeText(0));

}


void MainWindow::on_muteBtn_clicked()
{
    if (m_player->isMute()) {
        m_player->setMute(false);
        ui->muteBtn->setIcon(QIcon(":/new/prefix/aloud.png"));
    } else {
        m_player->setMute(true);
        ui->muteBtn->setIcon(QIcon(":/new/prefix/mute.png"));
    }
}


void MainWindow::on_playBtn_clicked()
{
    if (m_player->getState() == State::Playing) {
        m_player->pause();
    } else {
        m_player->play();
    }
}


void MainWindow::on_stopBtn_clicked()
{
    m_player->stop();
}


void MainWindow::on_mutipleSpeed_currentIndexChanged(int index)
{
    if (m_player) {
        m_player->setPlaySpeed(4 - index);
    }
}

