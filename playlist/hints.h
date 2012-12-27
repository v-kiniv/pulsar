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

#ifndef HINTS_H
#define HINTS_H

#include <network/auth.h>
#include <QWidget>
#include <QPainter>
#include <QStyleOption>
#include <QListWidget>
#include <QStandardItemModel>


class Hints : public QWidget
{
    Q_OBJECT
public:
    explicit Hints(QWidget *parent = 0);

private:
    Auth *m_Auth;
    QNetworkAccessManager *m_nManager;

    QListWidget *m_wList;
    QStringList m_HintsList;
    QStandardItemModel *m_Model;

    QString m_sLastHint;

protected:
    void paintEvent(QPaintEvent *);

Q_SIGNALS:
    void selected(QString);

public Q_SLOTS:
    void parentResized(QRect);
    void getFocus();
    void textChanged(QString);

private Q_SLOTS:
    void netReply(QNetworkReply *reply);

    void itemActivated(QModelIndex);

};

#endif // HINTS_H
