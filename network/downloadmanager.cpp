#include "downloadmanager.h"

#include <QDebug>
#include <QDir>

DownloadManager *DownloadManager::m_instance = 0;

DownloadManager::DownloadManager(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of DownloadManager object is allowed");

    m_instance = this;
    m_bBusy = false;
    m_nManager = new QNetworkAccessManager(this);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setInterval(5000);
    connect(m_timeoutTimer, SIGNAL(timeout()), SLOT(onTimeout()));


}

DownloadManager* DownloadManager::instance()
{
    return m_instance;
}

void DownloadManager::add(Track *track)
{
    if(m_queue.isEmpty() && !m_bBusy) {
        download(track);
        Q_EMIT started();
    } else {
        m_queue.enqueue(track);
        Q_EMIT queueChanged(m_queue.size());
    }
}

void DownloadManager::download(Track *track)
{
    Q_EMIT progressChanged("0", "0", 0);

    m_timeoutTimer->start();

    m_current = track;
    m_bBusy = true;
    m_NetError = QNetworkReply::NoError;

    Q_EMIT currentChanged(track);
    Q_EMIT queueChanged(m_queue.size());

    m_nRequest.setUrl(track->url());

    m_nReply = m_nManager->get(m_nRequest);

    connect(m_nReply, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(m_nReply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
    connect(m_nReply, SIGNAL(finished()), this, SLOT(finished()));
    connect(m_nReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onNetworkError(QNetworkReply::NetworkError)));

    // Create&open file
    QString fileName = track->artist().trimmed() + " - " + track->title().trimmed() + ".mp3";
    fileName.replace(QRegExp("[?*/\"<>]"), "_");
    QDir::setCurrent(Settings::instance()->getValue("general/music_path").toString());
    m_file.setFileName(fileName);
    m_file.open(QIODevice::WriteOnly);



}

void DownloadManager::onCancelAll()
{
    if(m_bBusy) {
        m_queue.clear();
        m_nReply->abort();
        m_file.close();
        m_file.remove();

        Q_EMIT queueChanged(m_queue.size());
        m_bBusy = false;
    }
}

void DownloadManager::readyRead()
{
    m_file.write(m_nReply->readAll());
}

void DownloadManager::downloadProgress(qint64 received, qint64 total)
{
    int percent;
    if(total > 0)
        percent = (received * 100 ) / total;
    else
        percent = 0;

    qreal r = received / 1024.0 / 1024.0;
    qreal t = total / 1024.0 / 1024.0;

    Q_EMIT progressChanged(QString::number(r, 'f', 2), QString::number(t, 'f', 2), percent);

    m_timeoutTimer->start();
}

void DownloadManager::finished()
{
    m_file.close();

    if(m_NetError == QNetworkReply::NoError) {
        m_bBusy = false;

        if(!m_queue.isEmpty()) {
            download(m_queue.dequeue());
        } else {
            Q_EMIT allComplete();
        }
        m_timeoutTimer->stop();
    } else if(m_NetError == QNetworkReply::ContentNotFoundError || m_NetError == QNetworkReply::ProtocolUnknownError) {
        m_current->updateUrl();
        connect(m_current, SIGNAL(updated(bool)), this, SLOT(onUrlUpdated(bool)));
    }



}

void DownloadManager::onNetworkError(QNetworkReply::NetworkError error)
{
    m_NetError = error;

    qDebug() << m_NetError;
}

void DownloadManager::onUrlUpdated(bool s)
{
    if(s) {
        download(m_current);
        m_NetError = QNetworkReply::NoError;
    } else {
        if(!m_queue.isEmpty()) {
            download(m_queue.dequeue());
        } else {
            Q_EMIT allComplete();
        }
    }
}

void DownloadManager::onTimeout()
{
    m_timeoutTimer->stop();

    m_current->updateUrl();
    connect(m_current, SIGNAL(updated(bool)), this, SLOT(onUrlUpdated(bool)));
}
