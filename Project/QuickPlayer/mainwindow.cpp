#include "mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QKeyEvent>
#include <QStatusBar>
#include <QSlider>
Mainwindow::Mainwindow(QWidget *parent) : QMainWindow(parent)
{
    surface = new Surface;
    setCentralWidget(surface);

    player = NULL;

    QMenuBar * bar = menuBar();
    QMenu * menu = bar->addMenu("File");
    menu->addAction("Open",this,SLOT(slotOpen()));
    menu->addAction("Pause",this,SLOT(slotPause()));
    menu->addAction("Resume",this,SLOT(slotResume()));
    menu->addAction("Exit",this,SLOT(close()));

    QStatusBar * status = statusBar();
    status->addWidget(slider = new QSlider(Qt::Horizontal));
    status->addWidget(speed = new QLineEdit);

    connect(slider,SIGNAL(sliderReleased()),this,SLOT(slotSliderChanged()));
    connect(speed,SIGNAL(textChanged(QString)),this,SLOT(slotSpeedChanged()));
}

void Mainwindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Space)
    {
        if(player)
            player->triggler();
    }
}

void Mainwindow::open(QString filename)
{
    if(player) delete player;
    player = new Player();
    MyDebug() << "open success";
    bool b = player->open(filename);
    if(b)
    {
        slider->setMinimum(0);
        slider->setMaximum(player->getNBFrames());
    }

    connect(player,SIGNAL(sigNewImage(QImage)),
            surface,SLOT(slotNewImage(QImage)));
    connect(player,SIGNAL(sigNewImage(QImage)),
            this,SLOT(slotNewImage(QImage)));
}

void Mainwindow::slotOpen()
{
    QString filename = QFileDialog::getOpenFileName();
    if(filename.length()== 0)
    {
        return;
    }
    open(filename);
}

void Mainwindow::slotPause()
{
    if(player)
        player->pause();
}

void Mainwindow::slotResume()
{
    if(player)
        player->resume();
}

void Mainwindow::slotNewImage(QImage)
{
    this->slider->setValue(this->slider->value() + 1);
}

void Mainwindow::slotSliderChanged()
{
    int v = slider->value();
    if(player)
        player->seek(v);

}

void Mainwindow::slotSpeedChanged()
{
    int s = speed->text().toInt();
    if(s > 200 || s < 50)
    {
        return;
    }

    if(player)
    {
        player->setSpeed(s);
    }
}
