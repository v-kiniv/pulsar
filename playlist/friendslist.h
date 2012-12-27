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

#ifndef FRIENDSLIST_H
#define FRIENDSLIST_H

#include <QObject>
#include <QStandardItemModel>
#include <QStandardItem>
#include "playlist/friendslistwidget.h"
#include <QSortFilterProxyModel>
#include <network/auth.h>
#include <QNetworkAccessManager>
#include "settings/settings.h"

class FriendsList : public QObject
{
    Q_OBJECT
public:
    explicit FriendsList(QObject *parent = 0);

    FriendsListWidget *widget() { return m_Widget; }

private:
    Auth *m_Auth;
    QString m_AppPath;
    QNetworkAccessManager *m_nManager;

    FriendsListWidget *m_Widget;
    QList<QStringList> m_List;
    QList<QStandardItem *> m_ItemsList;
    QStringList m_AvatarsUrls;
    QStandardItemModel *m_Model;
    int m_AvatarI;
    QSortFilterProxyModel *m_ProxyModel;

    Settings *m_Settings;

    void getList();
    void getAvatars();

    void getUserId(QString userLogin);

Q_SIGNALS:
    void friendSelected(QString, QString);

public Q_SLOTS:

private Q_SLOTS:
    void load();
    void save();
    void parseList(QNetworkReply *);
    void parseId(QNetworkReply *);
    void downloadAvatar(QNetworkReply *);
    void add(QStringList);
    void search(QString);
    void listItemAcivated(QModelIndex);
    void setId(QString);
    void refreshList();


};

#endif // FRIENDSLIST_H
