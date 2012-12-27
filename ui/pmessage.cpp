/* This file is part of Pulsar.
   Copyright 2011, Vasily Kiniv <yuberion@gmail.com>

   Pulsar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   Pulsar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with Pulsar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pmessage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>

PMessage::PMessage(QWidget *parent) :
    QWidget(parent)
{
    installEventFilter(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    //QHBoxLayout *topLayout = new QHBoxLayout();

    setObjectName("PMessage");

    setStyleSheet("QLabel{ color: rgba(0, 0, 0, 200); } QWidget#PMessage {border-radius: 5px; border-bottom-left-radius: 0px;  border-top-left-radius: 0px; background-color: rgba(55, 55, 55, 0); background-image: url(:/images/bkg_light);}");

    setMinimumSize(200, 70);
    //setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    m_wlTitle = new QLabel("Track added to library");
    m_wlTitle->setStyleSheet("border-radius: 3px; background-color: rgba(0, 0, 0, 30); font-size: 15px; ");
    m_wlTitle->setAlignment(Qt::AlignCenter);
    m_wlTitle->setMaximumHeight(20);

    m_wlMsg = new QLabel("Update library list to see new added tracks");
    m_wlMsg->setWordWrap(true);
    m_wlMsg->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_wlTitle);
    layout->addWidget(m_wlMsg);
    setMouseTracking(true);

    adjustSize();

    // Setup animation
    m_Animation = new QPropertyAnimation(this, "pos");
    hide();

    m_Timer = new QTimer(this);
    m_Timer->setInterval(5000);
    connect(m_Timer, SIGNAL(timeout()), SLOT(hideMe()));
}

bool PMessage::eventFilter(QObject *target, QEvent *event)
{
    if(event->type() == QEvent::Paint) {
        QWidget *widget = (QWidget *)target;
        QStyleOption opt;
        opt.initFrom(widget);
        QPainter p(widget);
        p.setOpacity(0.9);
        widget->style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, widget);
    }
    if(event->type() == QEvent::MouseMove) {
            hideMe();
    }
    return false;
}

void PMessage::hideMe()
{
    m_Animation->setDuration(200);
    m_Animation->setStartValue(m_VPos);
    m_Animation->setEndValue(m_HPos);
    m_Animation->start();

    connect(m_Animation, SIGNAL(finished()), SLOT(hide()));
}

void PMessage::showMe()
{
    m_Animation->disconnect();
    m_Animation->setDuration(200);
    m_Animation->setStartValue(m_HPos);
    m_Animation->setEndValue(m_VPos);
    m_Animation->start();

    show();
    m_Timer->start();
}

void PMessage::message(QString title, QString msg)
{
    m_Timer->stop();

    m_wlTitle->setText(title);
    m_wlMsg->setText(msg);

    adjustSize();

    showMe();
}

void PMessage::parentResized(QRect pRect)
{
    m_VPos = QPoint(pRect.left(), pRect.center().y() - rect().center().y() / 2);
    m_HPos.setY(m_VPos.y());
    m_HPos.setX(m_VPos.x() - width());

    move(m_VPos);
}
