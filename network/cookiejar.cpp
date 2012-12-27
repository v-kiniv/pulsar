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

#include <QDebug>
#include <QSettings>

#include "cookiejar.h"
#include "network/auth.h"
#include "settings/settings.h"


CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
    m_appPath = Settings::instance()->appPath();
    loadCookies();
}

void CookieJar::loadCookies()
{
    QString fName = m_appPath;
    fName += "cookies";
    QSettings cnFile(fName, QSettings::IniFormat);

    setVkLogin(cnFile.value("login").toString());

    QList<QNetworkCookie> cookies;
    int size = cnFile.beginReadArray("cookies");
    for(int i=0; i<size; i++) {
        QNetworkCookie cook;

        cnFile.setArrayIndex(i);

        cook.setName(cnFile.value("name").toByteArray());
        cook.setValue(cnFile.value("value").toByteArray());
        cook.setDomain(cnFile.value("domain").toByteArray());
        cook.setPath(cnFile.value("path").toByteArray());

        cookies.insert(i, cook);
    }
    cnFile.endArray();

    setAllCookies(cookies);
}

void CookieJar::saveCookies()
{
    QString fName = m_appPath;
    fName += "cookies";
    QSettings cnFile(fName, QSettings::IniFormat);
    cnFile.clear();

    cnFile.setValue("login", m_sVkLogin);

    QList<QNetworkCookie> cookies = allCookies();
    cnFile.beginWriteArray("cookies");
    for(int i=0; i<cookies.size(); i++) {
        QNetworkCookie cook = cookies.at(i);

        cnFile.setArrayIndex(i);

        cnFile.setValue("name", cook.name());
        cnFile.setValue("value", cook.value());
        cnFile.setValue("domain", cook.domain());
        cnFile.setValue("path", cook.path());
    }
    cnFile.endArray();
}

void CookieJar::setVkLogin(QString sLogin)
{
    m_sVkLogin = sLogin;
}

QString CookieJar::getVkLogin()
{
    return m_sVkLogin;
}

void CookieJar::clearCookies()
{
    QList<QNetworkCookie> cookies;
    setAllCookies(cookies);
}
