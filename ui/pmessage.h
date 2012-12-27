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

#ifndef PMESSAGE_H
#define PMESSAGE_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QEvent>
#include <QPropertyAnimation>
#include <QTimer>

class PMessage : public QWidget
{
    Q_OBJECT
public:
    explicit PMessage(QWidget *parent = 0);

private:
    QLabel *m_wlTitle;
    QLabel *m_wlMsg;

    QPropertyAnimation *m_Animation;

    QPoint m_VPos;
    QPoint m_HPos;

    QTimer *m_Timer;

    bool eventFilter(QObject *target, QEvent *event);


protected:
    //setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical);

Q_SIGNALS:

public Q_SLOTS:
    void message(QString title, QString msg);
    void parentResized(QRect);

private Q_SLOTS:
    void hideMe();
    void showMe();

};

#endif // PMESSAGE_H
