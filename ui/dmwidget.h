#ifndef DMWIDGET_H
#define DMWIDGET_H

#include "ui/styledbutton.h"
#include "playlist/track.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QEvent>
#include <QPropertyAnimation>
#include <QProgressBar>
#include <QGraphicsOpacityEffect>

class DmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DmWidget(QWidget *parent = 0);
    
private:
    QLabel *m_wlTitle;
    QLabel *m_wlMb;
    QLabel *m_wlQueue;
    StyledButton *m_wbCancel;
    QProgressBar *m_wProgress;

    QPoint m_VPos;
    QPoint m_HPos;

    QGraphicsOpacityEffect *m_effect;

    QPropertyAnimation *m_animation;

protected:
    void paintEvent(QPaintEvent *);

Q_SIGNALS:
    void cancelAll();
    
public Q_SLOTS:
    void parentResized(QRect);

    void onStarted();
    void onCurrentChanged(Track*);
    void onProgressChanged(QString, QString, int);
    void onQueueChanged(int);
    void onAllComplete();
    
private Q_SLOTS:
    void onCancelAll();
    void hideMe();

};

#endif // DMWIDGET_H
