#include "qclickablegraphicsview.h"

#include <QMouseEvent>

QClickableGraphicsView::QClickableGraphicsView(QWidget* parent) :
    QGraphicsView{parent}
{
}

void QClickableGraphicsView::mousePressEvent(QMouseEvent *event)
{
    mousePressed(event->x(), event->y());
    event->accept();
}

void QClickableGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    mouseReleased(event->x(), event->y());
    event->accept();
}
