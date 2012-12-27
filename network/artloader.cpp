#include "artloader.h"
#include "settings/settings.h"

#include <QDebug>
#include <QRegExp>
#include <QFile>
#include <QDir>
#include <QCryptographicHash>

ArtLoader *ArtLoader::m_instance = 0;

ArtLoader::ArtLoader(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of ArtLoader object is allowed");
    m_instance = this;

    m_TmpPath = Settings::instance()->appPath()+"tmp/";

    m_nManager = new QNetworkAccessManager();
    m_nRequest.setRawHeader("User-Agent", "LinuxPulsar/0.9 +http://forum.ubuntu.ru/index.php?topic=168217");

}

ArtLoader *ArtLoader::instance()
{
    return m_instance;
}

void ArtLoader::getArt(QString query)
{
    m_queryHash = QString(QCryptographicHash::hash((query.toAscii()),QCryptographicHash::Md5).toHex());
    if(isExists()) {
        m_bArtLoaded = true;
        Q_EMIT artLoaded(true);
    } else {
        m_nRequest.setUrl(QUrl(QString("http://ajax.googleapis.com/ajax/services/search/images?v=1.0&q=%0&imgsz=medium").arg(query)));

        m_nManager->disconnect();
        connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(respReply(QNetworkReply*)));
        m_nManager->get(m_nRequest);
    }
}

QString ArtLoader::artLocalUri()
{
    if(m_bArtLoaded)
        return localUri();
    else
        return "none";
        //return m_TmpPath+"no_cover.svg";
}

QString ArtLoader::localUri()
{
    return m_TmpPath+m_queryHash+m_imageExt;
}

bool ArtLoader::isExists()
{
    QString path = m_TmpPath+m_queryHash;

        QStringList extenstions;
        extenstions << ".jpg"
                    << ".jpeg"
                    << ".gif"
                    << ".png";

        QListIterator<QString> itr (extenstions);
        while (itr.hasNext()) {
            QString ext = itr.next();
            if(QFile(path+ext).exists()) {
                m_imageExt = ext;
                return true;
            }
        }
    return false;
}

void ArtLoader::downloadImage(QString url)
{
    m_nRequest.setUrl(QUrl(url));

    m_nManager->disconnect();
    connect(m_nManager, SIGNAL(finished(QNetworkReply*)), SLOT(imgReply(QNetworkReply*)));
    m_nManager->get(m_nRequest);
}

void ArtLoader::clearArt()
{
    m_bArtLoaded = false;
}

void ArtLoader::loadArt(QString str)
{
    getArt(str);
    //qDebug() << "load art: " << str;
}

void ArtLoader::respReply(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();

    QString url;
    QRegExp rx("\"unescapedUrl\":\"(http[s]*://[^:,]+jpg|png|gif)\"");
    rx.setMinimal(true);
    rx.indexIn(data);
    url = rx.capturedTexts()[1];

    if(url != "") {
        m_imageExt = url;
        m_imageExt.remove(0, url.lastIndexOf("."));
        downloadImage(url);
    } else {
        m_bArtLoaded = false;
        Q_EMIT artLoaded(false);
    }

    //qDebug() << url;
}

void ArtLoader::imgReply(QNetworkReply *reply)
{
    QFile file(localUri());
    //QDir::setCurrent(m_TmpPath);
    file.open(QIODevice::WriteOnly);
    file.write(reply->readAll());
    file.close();

    m_bArtLoaded = true;
    Q_EMIT artLoaded(true);

}
