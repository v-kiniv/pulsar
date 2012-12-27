#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include "playlist/track.h"
#include "settings/settings.h"

#include <QObject>
#include <QQueue>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QTimer>

class DownloadManager : public QObject
{
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = 0);

    static DownloadManager *instance();

    void add(Track* track);
    
private:
    QQueue <Track*> m_queue;

    // Network manager
    QNetworkAccessManager *m_nManager;
    QNetworkRequest m_nRequest;
    QNetworkReply *m_nReply;

    static DownloadManager *m_instance;

    QTimer *m_timeoutTimer;
    Track* m_current;
    QFile m_file;
    bool m_bBusy;

    QNetworkReply::NetworkError m_NetError;

    void download(Track* track);

Q_SIGNALS:
    void started();
    void queueChanged(int);
    void currentChanged(Track*);
    void progressChanged(QString, QString, int);

    void allComplete();
    
public Q_SLOTS:
    void onCancelAll();

private Q_SLOTS:
    void readyRead();
    void downloadProgress(qint64, qint64);
    void finished();
    void onNetworkError(QNetworkReply::NetworkError);
    void onUrlUpdated(bool);
    void onTimeout();
    
};

#endif // DOWNLOADMANAGER_H
