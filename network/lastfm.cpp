#include "lastfm.h"
#include "settings/settings.h"

#include <QDebug>
#include <QCryptographicHash>
#include <QNetworkReply>


Lastfm::Lastfm(QObject *parent) :
    QObject(parent)
{
    m_sBaseUrl = "http://ws.audioscrobbler.com/2.0/";
    m_sApiKey = "53b57f30c6f5c16cc9e5ea36d3935db6";
    m_sApiSecret = "72148f04716a9d75a6d9b7599c0d0bd7";
    m_sApiToken = "";
    m_sApiSk = Settings::instance()->getValue("lastfm/sk").toString();
    m_sUserName = Settings::instance()->getValue("lastfm/username").toString();

    m_bScrobbling = Settings::instance()->getValue("lastfm/scrobbling").toBool();
    connect(Settings::instance(), SIGNAL(changed()), SLOT(settingsUpdated()));

    m_ScrobbleTimer = new QTimer(this);
    m_ScrobbleTimer->setInterval(5000);
    connect(m_ScrobbleTimer, SIGNAL(timeout()), SLOT(updateNowPlaying()));

    m_request.setRawHeader("User-Agent", "Pulsar");
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    // Setup NetwokAccessManager
    m_nmanager = new QNetworkAccessManager(this);
}

QString Lastfm::createSig(QMap<QString, QString> &params)
{
    params.insert("api_key", m_sApiKey);
    if(!m_sApiSk.isEmpty())
        params.insert("sk", m_sApiSk);

    QByteArray sig;
    QMap<QString, QString>::iterator i;
     for (i = params.begin(); i != params.end(); ++i)
         sig.append(i.key() + i.value());

    sig.append(m_sApiSecret);
    sig = QUrl::fromPercentEncoding(sig).toUtf8();

    return QCryptographicHash::hash(sig, QCryptographicHash::Md5).toHex();
}

QUrl Lastfm::createUrl(QMap<QString, QString> &params, QByteArray *outParams)
{
    bool isPost = false;
    if(outParams)
        isPost = true;


    QString sig = createSig(params);
    params.insert("api_sig", sig);

    QUrl url = QUrl(m_sBaseUrl);

    QMap<QString, QString>::iterator i;
    for (i = params.begin(); i != params.end(); ++i)
        if(isPost)
            outParams->append(i.key() + "=" + i.value() +"&");//QUrl::toPercentEncoding(i.value())
        else
            url.addQueryItem(i.key(), i.value());

    return url;
}

void Lastfm::handleError(int e, const QString &str)
{
    qDebug() << QString("Lastfm error: %0 (%1)").arg(str).arg(e);

    switch(e) {
    case 4:
        getToken();
        break;
    case 9:
        getToken();
        break;
    }

}

void Lastfm::getToken()
{
    QMap<QString, QString> params;
    params["method"] = "auth.getToken";

    m_request.setUrl(createUrl(params));

    QNetworkReply *reply = m_nmanager->get(m_request);
    connect(reply, SIGNAL(finished()), SLOT(replyToken()));
}

void Lastfm::getSession()
{
    QMap<QString, QString> params;
    params["token"] = m_sApiToken;
    params["method"] = "auth.getSession";

    m_request.setUrl(createUrl(params));

    QNetworkReply *reply = m_nmanager->get(m_request);
    connect(reply, SIGNAL(finished()), SLOT(replySession()));
}

void Lastfm::requestAuth()
{
    QString url = QString("http://www.last.fm/api/auth/?api_key=%0&token=%1").arg(m_sApiKey).arg(m_sApiToken);
    Q_EMIT needAuth("LastFM", url);
}

QDomElement Lastfm::prepareXml(QString &data)
{
    QDomDocument domDocument;
    domDocument.setContent(data);

      QDomNodeList nodeList = domDocument.childNodes();
      for (int i = 0; i<nodeList.count(); ++i) {

        if (nodeList.at(i).isProcessingInstruction())
          domDocument.removeChild(nodeList.at(i));
      }

      return domDocument.documentElement();
}

void Lastfm::scrobble()
{
    QMap<QString, QString> params;
    params["method"] = "track.scrobble";
    params["timestamp"] = QString::number(m_uStartStamp);
    params["artist"] = QUrl::toPercentEncoding(m_currentTrack->artist());
    params["track"] = QUrl::toPercentEncoding(m_currentTrack->title());

    QByteArray postParams;
    m_request.setUrl(createUrl(params, &postParams));

    QNetworkReply *reply = m_nmanager->post(m_request, postParams);
    connect(reply, SIGNAL(finished()), SLOT(replyScrobble()));
}

void Lastfm::onAuthCompleted()
{
    getSession();
}

void Lastfm::onAuthCanceled()
{
    m_sApiSk.clear();
    Settings::instance()->setValue("lastfm/sk", m_sApiSk);

    setScrobbling(false);
}

void Lastfm::onTrackUpdated(Track *t)
{
    m_currentTrack = t;
    if(t->lenght() > 30) {
        if(t->lenght() > 480)
            m_iScrobblePos = 240;
        else
            m_iScrobblePos = t->lenght() / 2;
    } else {
        m_bScrobbled = true;
    }
    m_uStartStamp = QDateTime::currentDateTimeUtc().toTime_t();

    if(m_bScrobbling) {
        updateNowPlaying();
        m_bScrobbled = false;
    }
}

void Lastfm::onTick(int t)
{
    if(m_bScrobbling && !m_bScrobbled) {
        if(t > m_iScrobblePos) {
            scrobble();
            m_bScrobbled = true;
        }
    }
}

void Lastfm::setScrobbling(bool s)
{
    m_bScrobbling = s;
    Settings::instance()->setValue("lastfm/scrobbling", s);

    if(m_bScrobbling && m_sApiSk.isEmpty())
        getToken();
}

void Lastfm::updateNowPlaying()
{
    QMap<QString, QString> params;
    params["method"] = "track.updateNowPlaying";
    params["artist"] = QUrl::toPercentEncoding(m_currentTrack->artist());
    params["track"] = QUrl::toPercentEncoding(m_currentTrack->title());

    QByteArray postParams;
    m_request.setUrl(createUrl(params, &postParams));

    QNetworkReply *reply = m_nmanager->post(m_request, postParams);
    connect(reply, SIGNAL(finished()), SLOT(replyNowPlaying()));
}

void Lastfm::replyToken()
{
    QNetworkReply *reply = (QNetworkReply *)sender();
    QString data = (QString)reply->readAll();

    QDomElement root = prepareXml(data);
    if(root.attribute("status") == "ok") {
        m_sApiToken = root.elementsByTagName("token").at(0).toElement().text();
    }

    requestAuth();
}

void Lastfm::replySession()
{
    QNetworkReply *reply = (QNetworkReply *)sender();
    QString data = (QString)reply->readAll();

    QDomElement root = prepareXml(data);
    if(root.attribute("status") == "ok") {
        m_sApiSk = root.elementsByTagName("key").at(0).toElement().text();
        m_sUserName = root.elementsByTagName("name").at(0).toElement().text();

        Settings::instance()->setValue("lastfm/sk", m_sApiSk);
        Settings::instance()->setValue("lastfm/username", m_sUserName);

        Q_EMIT authSuccess(true);
    } else {
        Q_EMIT authSuccess(false);
    }
}

void Lastfm::replyNowPlaying()
{
    QNetworkReply *reply = (QNetworkReply *)sender();
    QString data = (QString)reply->readAll();

    QDomElement root = prepareXml(data);
    QString status = root.attribute("status");

    if(status == "failed") {
        int eCode = root.elementsByTagName("error").at(0).toElement().attribute("code").toInt();
        QString eStr = root.elementsByTagName("error").at(0).toElement().text();
        handleError(eCode, eStr);

    }
}

void Lastfm::replyScrobble()
{
    QNetworkReply *reply = (QNetworkReply *)sender();
    QString data = (QString)reply->readAll();

    QDomElement root = prepareXml(data);
    QString status = root.attribute("status");

    if(status == "failed") {
        int eCode = root.elementsByTagName("error").at(0).toElement().attribute("code").toInt();
        QString eStr = root.elementsByTagName("error").at(0).toElement().text();
        handleError(eCode, eStr);

    }
}

void Lastfm::settingsUpdated()
{
    setScrobbling(Settings::instance()->getValue("lastfm/scrobbling").toBool());
}
