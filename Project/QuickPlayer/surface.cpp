#include "surface.h"
#include <QPainter>
Surface::Surface(QWidget *parent) : QWidget(parent)
{

}

void Surface::paintEvent(QPaintEvent *)
{
    if(image.isNull()) return;

    QPainter p(this);
    p.drawImage(0,0,image.scaled(size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation));

}

void Surface::slotNewImage(QImage image)
{
    this->image = image;
    update();
}
