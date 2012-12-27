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

#ifndef VKACTIONS_H
#define VKACTIONS_H

#include <QObject>
#include <network/auth.h>
#include <QNetworkAccessManager>
#include <playlist/track.h>

class VkActions : public QObject
{
    Q_OBJECT
public:
    explicit VkActions(QObject *parent = 0);

    static VkActions *instance();

    void addToLibrary(Track *track);
    void removeFromLibrary(Track *track);
    void setStatus(Track *track);

private:
    static VkActions *m_instance;

    Auth *m_Auth;
    QNetworkAccessManager *m_nManager;

    Track *m_lastTrack;
    QString m_sLastTitle;

Q_SIGNALS:
    void message(QString, QString);

public Q_SLOTS:

private Q_SLOTS:
    void toLibReply(QNetworkReply*);
    void fromLibReply(QNetworkReply*);
    void trackUpdated(bool);

};

#endif // VKACTIONS_H
