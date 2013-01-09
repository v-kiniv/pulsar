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

#include "auth.h"
#include <QDebug> //FIXME
#include <QRegExp>
#include <QTimer>

Auth *Auth::m_instance = 0;

Auth::Auth(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of Auth object is allowed");

    m_instance = this;

    m_nCookie = new CookieJar(this);

    m_Settings = Settings::instance();

    m_sVkId = m_Settings->getValue("general/vkid").toString();

    if(m_Settings->getValue("general/account_use").toBool())
        setVkAccount(m_Settings->getValue("general/account_login").toString(), m_Settings->getValue("general/account_pass").toString());
    else
        setVkNoAccount();
}

Auth* Auth::instance()
{
    return m_instance;
}

CookieJar *Auth::cookiejar() { return m_nCookie; }
QString Auth::vkId() { return m_sVkId; }
QString Auth::vkLogin() { return m_sVkLogin; }
QString Auth::vkHash() { return m_sVkHash; }

void Auth::cancelAuth()
{
    m_nReply->abort();
}

void Auth::doAuth()
{
    m_sVkId = "0";

    m_nCookie->clearCookies();
    m_nCookie->setVkLogin(m_sVkLogin);

    QString sRequest = "act=login&q=1&al_frame=1&expire=quick_expire_input&captcha_sid=quick_captcha_sid&captcha_key=quick_captcha_key&from_host=vk.com&email=" + m_sVkLogin + "&pass=" + m_sVkPassword;
    m_nRequest.setUrl(QUrl("http://login.vk.com/?"+sRequest));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(authRFirst(QNetworkReply*)));
    m_nReply = m_nManager->get(m_nRequest);
}

void Auth::checkAuth()
{
    // Set network manager, slot and cookiejar
    m_nManager = new QNetworkAccessManager(this);
    m_nManager->setCookieJar(m_nCookie);

    // Checking for diff's accounts in cookies file and current account
    if(m_sVkLogin == m_nCookie->getVkLogin()) {
        m_nRequest.setUrl(QUrl("http://vk.com/invite"));
        m_nManager->disconnect();
        connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(checkAuthReply(QNetworkReply*)));
        m_nReply = m_nManager->get(m_nRequest);
    } else {
        doAuth();
    }
}

void Auth::checkAuthReply(QNetworkReply *nReply)
{
    if(nReply->error() == QNetworkReply::NoError) {
        QString sData = nReply->readAll();

        // Parse js block with id variable
        QRegExp rx("var vk = \\{\(.*)\\}");
        rx.setMinimal(true);
        rx.indexIn(sData);
        QString spData1 = rx.capturedTexts()[1];

        // Parse variable "id" from block
        rx.setPattern("id: (\\d+),");
        rx.indexIn(spData1);
        QString spData2 = rx.capturedTexts()[1];

        int npId = spData2.toInt();

        m_sVkId = QString::number(npId);
        m_Settings->setValue("general/vkid", m_sVkId);

        /* Checking if id equal 0
          if equal that mean not athorized
        */
        if(npId == 0) {
            qDebug() << "Need auth(id:" << npId << ")";
            doAuth();
        } else {
            qDebug() << "Authorized with cookies(id:" << npId << ")";
            m_bAuth = true;
            Q_EMIT authComplete();
        }
    } else {
        errorHandle(Auth::NETWORK);
    }
}

void Auth::authRFirst(QNetworkReply *nReply)
{
    if(nReply->hasRawHeader("Set-Cookie")) {
        QString sCookieHeader = (QString)nReply->rawHeader("Set-Cookie");


        // Parse vkID from cookie header
        QRegExp rx("l=(\\d+);");
        rx.indexIn(sCookieHeader);
        QString sId = rx.capturedTexts().at(1);

        // Checking for contains id number
        if(!sId.isEmpty()) {
            m_sVkId = sId;

            // Init second authorization step
            if(nReply->hasRawHeader("Location")) {
                m_nRequest.setUrl(QUrl(nReply->rawHeader("Location")));

                m_nManager->disconnect();
                connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(authRSecond(QNetworkReply*)));
                m_nManager->get(m_nRequest);
            }
        } else {
            errorHandle(Auth::BADLOGIN);
        }
    } else {
            errorHandle(Auth::NETWORK);
    }
}

void Auth::authRSecond(QNetworkReply *nReply)
{
    Q_UNUSED(nReply);
    /*TODO
      maybe check for correct data received
      Q_EMIT signal to save cookies to file
    */
    m_nCookie->saveCookies();
    m_bAuth = true;
    Q_EMIT authComplete();
}

void Auth::checkComplete()
{
    if(m_bAuth)
        Q_EMIT authComplete();
    else
        QTimer::singleShot(500, this, SLOT(checkComplete()));

}

void Auth::errorHandle(Auth::Errors error)
{
    m_Error = error;
    qDebug() << "ERROR" << error;
    Q_EMIT nError(error);
}

void Auth::setVkAccount(QString sLogin, QString sPassword)
{
    // Set own user VKontakte accoubnt
    m_bVkOwn = true;
    m_sVkLogin = sLogin;
    m_sVkPassword = sPassword;

    // Init auth
    checkAuth();
}

void Auth::setVkNoAccount()
{
    // Set default app VKontakte account
    m_bVkOwn = false;
    m_sVkLogin = "380638126226";
    m_sVkPassword = "pulsar112";

    // Init auth
    checkAuth();
}
