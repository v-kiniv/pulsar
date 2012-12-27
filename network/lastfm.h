#ifndef LASTFM_H
#define LASTFM_H

#include "playlist/track.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QMapIterator>
#include <QtXml>

class Lastfm : public QObject
{
    Q_OBJECT
public:
    explicit Lastfm(QObject *parent = 0);

private:
    QNetworkAccessManager *m_nmanager;
    QNetworkRequest m_request;

    QString m_sBaseUrl;
    QString m_sApiKey;
    QString m_sApiSecret;
    QString m_sApiToken;
    QString m_sApiSk;
    QString m_sUserName;
    bool m_bScrobbling;
    QTimer *m_ScrobbleTimer;
    Track *m_currentTrack;
    int m_iScrobblePos;
    bool m_bScrobbled;
    uint m_uStartStamp;

    QString createSig(QMap<QString, QString> &params);
    QUrl createUrl(QMap<QString, QString> &params, QByteArray *outParams = 0);
    void handleError(int e, const QString &str);
    void getToken();
    void getSession();
    void requestAuth();
    QDomElement prepareXml(QString &data);
    void scrobble();
    
Q_SIGNALS:
    void needAuth(QString, QString);
    void authSuccess(bool);
    
public Q_SLOTS:
    void onAuthCompleted();
    void onAuthCanceled();
    void onTrackUpdated(Track *);
    void onTick(int);
    void setScrobbling(bool);


private Q_SLOTS:
    void updateNowPlaying();
    void replyToken();
    void replySession();
    void replyNowPlaying();
    void replyScrobble();
    void settingsUpdated();
    
};

#endif // LASTFM_H
