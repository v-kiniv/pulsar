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


#ifndef AUTH_H
#define AUTH_H

#include <QObject>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>

#include "network/cookiejar.h"
#include "settings/settings.h"


class Auth : public QObject
{
    Q_OBJECT
public:
    explicit Auth(QObject *parent = 0);

    enum Error {
             BADLOGIN = 0x0,
             NETWORK = 0x1,
             UNKNOWN = 0x2
    };
    Q_DECLARE_FLAGS(Errors, Error)
    Auth::Errors m_Error;

    static Auth *instance();

    // Return cookiejar object
    CookieJar *cookiejar();

    QString vkId();
    QString vkLogin();
    QString vkHash();

    bool isAuth() { return m_bAuth; /*checkComplete();*/}
    void cancelAuth();

private:
    static Auth *m_instance;

    // Vkonkte account fiels
    QString m_sVkLogin;
    QString m_sVkPassword;
    QString m_sVkId;
    QString m_sVkHash;
    bool m_bVkOwn;

    // Network manager
    QNetworkAccessManager *m_nManager;
    QNetworkRequest m_nRequest;
    QNetworkReply *m_nReply;
    CookieJar *m_nCookie;
    int m_nReqType;

    bool m_bAuth;

    Settings *m_Settings;

    // Init authorization
    void doAuth();

    // Check if already authorized(cookies loaded end correct)
    void checkAuth();

    // Function for handle errors
    void errorHandle(Auth::Errors error);

protected:
    QString m_appPath;


Q_SIGNALS:
    // Emmitted when authorization done. True if authorized, false else.
    void authStateChanged(bool);
    void authComplete();

    // Emmitted if auth or network error occured
    void nError(Auth::Errors);

public Q_SLOTS:
    // Receive and set VKontakte account
    // Also init auth function
    void setVkAccount(QString sLogin, QString sPassword);

    // Set default VKontakte account
    void setVkNoAccount();

private Q_SLOTS:
    // Slot for check auth function network reply
    void checkAuthReply(QNetworkReply *nReply);

    // Two step's of authorization
    void authRFirst(QNetworkReply *nReply);
    void authRSecond(QNetworkReply *nReply);

    void checkComplete();

};

#endif // AUTH_H
