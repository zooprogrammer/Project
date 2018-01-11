#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "player.h"
#include "surface.h"
#include <QSlider>
#include <QLineEdit>

class Mainwindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit Mainwindow(QWidget *parent = nullptr);

    Surface * surface;
    Player * player;
    QSlider * slider;
    QLineEdit * speed;

    void keyPressEvent(QKeyEvent *ev);

    void open(QString filename);
signals:

public slots:
    void slotOpen();
    void slotPause();
    void slotResume();
    void slotNewImage(QImage);
    void slotSliderChanged();
    void slotSpeedChanged();


};

#endif // MAINWINDOW_H
