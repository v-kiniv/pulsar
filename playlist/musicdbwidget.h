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

#ifndef MUSICDBWIDGET_H
#define MUSICDBWIDGET_H

#include "ui/styledbutton.h"
#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QLabel>
#include <QMovie>

class MusicDbWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MusicDbWidget(QWidget *parent = 0);

    void setModel(QSortFilterProxyModel *model);
    void clear();

private:
    QListView *m_wList;
    QLineEdit *m_weSearch;
    StyledButton *m_wbClose;

    // Label show animated loading gif
    QLabel *m_wlLoading;

    // Animate gif loading
    QMovie *m_LoadingMov;

    bool eventFilter(QObject *target, QEvent *event);
    void listResize(QSize size);

protected:
    void paintEvent(QPaintEvent *);

Q_SIGNALS:
    void searchChanged(QString);
    void itemActivated(QModelIndex);

public Q_SLOTS:
    void showLoading();
    void hideLoading();

private Q_SLOTS:

};

#endif // MUSICDBWIDGET_H
