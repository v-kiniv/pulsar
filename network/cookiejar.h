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

#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QNetworkCookieJar>

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QObject *parent = 0);

    // Return's current login in cookies file
    QString getVkLogin();

    // Save cookies to file
    void saveCookies();

    // Set login for check diff acc later
    void setVkLogin(QString sLogin);

    // Clear all cookie for new authorization
    void clearCookies();


private:
    QString m_sVkLogin;
    QString m_appPath;

    // Loading cookies from file
    void loadCookies();
Q_SIGNALS:

public Q_SLOTS:

};

#endif // COOKIEJAR_H
