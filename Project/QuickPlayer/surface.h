#ifndef SURFACE_H
#define SURFACE_H

#include <QWidget>
#include <QImage>
class Surface : public QWidget
{
    Q_OBJECT
public:
    explicit Surface(QWidget *parent = nullptr);
    QImage image;
    void paintEvent(QPaintEvent *);

signals:

public slots:
    void slotNewImage(QImage image);
};

#endif // SURFACE_H
