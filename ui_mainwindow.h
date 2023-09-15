/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <VideoWidget.h>
#include <videoslider.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    VideoSlider *timeSlider;
    QLabel *timeLabel;
    QLabel *label_2;
    QLabel *durationLabel;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *playBtn;
    QPushButton *stopBtn;
    QSpacerItem *horizontalSpacer;
    QComboBox *mutipleSpeed;
    QPushButton *muteBtn;
    VideoSlider *volumeSlider;
    QLabel *volumeLabel;
    VideoWidget *videoWidget;
    QGridLayout *gridLayout_2;
    QSpacerItem *verticalSpacer;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *openFileBtn;
    QSpacerItem *horizontalSpacer_3;
    QSpacerItem *verticalSpacer_2;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(3);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        timeSlider = new VideoSlider(centralwidget);
        timeSlider->setObjectName(QString::fromUtf8("timeSlider"));
        timeSlider->setEnabled(false);
        timeSlider->setMaximum(99);
        timeSlider->setSingleStep(1);
        timeSlider->setPageStep(10);
        timeSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_2->addWidget(timeSlider);

        timeLabel = new QLabel(centralwidget);
        timeLabel->setObjectName(QString::fromUtf8("timeLabel"));
        timeLabel->setMinimumSize(QSize(0, 0));
        timeLabel->setMaximumSize(QSize(70, 16777215));
        QFont font;
        font.setFamily(QString::fromUtf8("Consolas"));
        timeLabel->setFont(font);
        timeLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(timeLabel);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(label_2);

        durationLabel = new QLabel(centralwidget);
        durationLabel->setObjectName(QString::fromUtf8("durationLabel"));
        durationLabel->setMaximumSize(QSize(70, 16777215));
        durationLabel->setFont(font);
        durationLabel->setAlignment(Qt::AlignCenter);

        horizontalLayout_2->addWidget(durationLabel);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        playBtn = new QPushButton(centralwidget);
        playBtn->setObjectName(QString::fromUtf8("playBtn"));
        playBtn->setEnabled(false);
        playBtn->setMaximumSize(QSize(40, 16777215));
        playBtn->setFocusPolicy(Qt::NoFocus);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/new/prefix/play.png"), QSize(), QIcon::Normal, QIcon::Off);
        playBtn->setIcon(icon);
        playBtn->setFlat(true);

        horizontalLayout_3->addWidget(playBtn);

        stopBtn = new QPushButton(centralwidget);
        stopBtn->setObjectName(QString::fromUtf8("stopBtn"));
        stopBtn->setEnabled(false);
        stopBtn->setMaximumSize(QSize(40, 16777215));
        stopBtn->setFocusPolicy(Qt::NoFocus);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/new/prefix/24gl-stop.png"), QSize(), QIcon::Normal, QIcon::Off);
        stopBtn->setIcon(icon1);
        stopBtn->setFlat(true);

        horizontalLayout_3->addWidget(stopBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        mutipleSpeed = new QComboBox(centralwidget);
        mutipleSpeed->addItem(QString());
        mutipleSpeed->addItem(QString());
        mutipleSpeed->addItem(QString());
        mutipleSpeed->addItem(QString());
        mutipleSpeed->setObjectName(QString::fromUtf8("mutipleSpeed"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(mutipleSpeed->sizePolicy().hasHeightForWidth());
        mutipleSpeed->setSizePolicy(sizePolicy);
        mutipleSpeed->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_3->addWidget(mutipleSpeed);

        muteBtn = new QPushButton(centralwidget);
        muteBtn->setObjectName(QString::fromUtf8("muteBtn"));
        muteBtn->setEnabled(false);
        muteBtn->setMaximumSize(QSize(40, 16777215));
        muteBtn->setFocusPolicy(Qt::NoFocus);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/new/prefix/aloud.png"), QSize(), QIcon::Normal, QIcon::Off);
        muteBtn->setIcon(icon2);
        muteBtn->setFlat(true);

        horizontalLayout_3->addWidget(muteBtn);

        volumeSlider = new VideoSlider(centralwidget);
        volumeSlider->setObjectName(QString::fromUtf8("volumeSlider"));
        volumeSlider->setEnabled(false);
        volumeSlider->setMinimumSize(QSize(100, 0));
        volumeSlider->setMaximumSize(QSize(100, 16777215));
        volumeSlider->setMaximum(100);
        volumeSlider->setValue(100);
        volumeSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_3->addWidget(volumeSlider);

        volumeLabel = new QLabel(centralwidget);
        volumeLabel->setObjectName(QString::fromUtf8("volumeLabel"));
        volumeLabel->setMinimumSize(QSize(30, 0));
        volumeLabel->setMaximumSize(QSize(30, 16777215));
        volumeLabel->setFont(font);
        volumeLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(volumeLabel);


        verticalLayout->addLayout(horizontalLayout_3);


        gridLayout->addLayout(verticalLayout, 2, 0, 1, 1);

        videoWidget = new VideoWidget(centralwidget);
        videoWidget->setObjectName(QString::fromUtf8("videoWidget"));
        videoWidget->setEnabled(true);
        gridLayout_2 = new QGridLayout(videoWidget);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        verticalSpacer = new QSpacerItem(20, 202, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer, 0, 1, 1, 1);

        horizontalSpacer_2 = new QSpacerItem(327, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_2, 1, 0, 1, 1);

        openFileBtn = new QPushButton(videoWidget);
        openFileBtn->setObjectName(QString::fromUtf8("openFileBtn"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/new/prefix/open_file.png"), QSize(), QIcon::Normal, QIcon::Off);
        openFileBtn->setIcon(icon3);
        openFileBtn->setIconSize(QSize(80, 80));

        gridLayout_2->addWidget(openFileBtn, 1, 1, 1, 1);

        horizontalSpacer_3 = new QSpacerItem(327, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout_2->addItem(horizontalSpacer_3, 1, 2, 1, 1);

        verticalSpacer_2 = new QSpacerItem(20, 201, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout_2->addItem(verticalSpacer_2, 2, 1, 1, 1);


        gridLayout->addWidget(videoWidget, 0, 0, 2, 1);

        gridLayout->setRowStretch(0, 1);
        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        mutipleSpeed->setCurrentIndex(2);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        timeLabel->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "/", nullptr));
        durationLabel->setText(QCoreApplication::translate("MainWindow", "00:00:00", nullptr));
        playBtn->setText(QString());
        stopBtn->setText(QString());
        mutipleSpeed->setItemText(0, QCoreApplication::translate("MainWindow", "x2", nullptr));
        mutipleSpeed->setItemText(1, QCoreApplication::translate("MainWindow", "x1.5", nullptr));
        mutipleSpeed->setItemText(2, QCoreApplication::translate("MainWindow", "x1", nullptr));
        mutipleSpeed->setItemText(3, QCoreApplication::translate("MainWindow", "x0.5", nullptr));

        muteBtn->setText(QString());
        volumeLabel->setText(QCoreApplication::translate("MainWindow", "100", nullptr));
        openFileBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
