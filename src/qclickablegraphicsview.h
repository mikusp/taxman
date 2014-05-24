#ifndef QCLICKABLEGRAPHICSVIEW_H
#define QCLICKABLEGRAPHICSVIEW_H

#include <QGraphicsView>

class QClickableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit QClickableGraphicsView(QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

signals:
    void mousePressed(int x, int y);
    void mouseReleased(int x, int y);

};

#endif // QCLICKABLEGRAPHICSVIEW_H
