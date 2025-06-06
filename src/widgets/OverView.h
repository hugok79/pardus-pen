#ifndef _overview_h
#define _overview_h
#include <QPoint>
#include <QPainter>
#include <QWidget>


class OverView : public QWidget {
public:
    OverView(QWidget *parent = nullptr);
    void updateImage();
protected:
    void paintEvent(QPaintEvent *event) override;
    QPainter painter;
};
#endif
