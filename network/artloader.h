#ifndef ARTLOADER_H
#define ARTLOADER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMap>
class ArtLoader : public QObject
{
    Q_OBJECT
public:
    explicit ArtLoader(QObject *parent = 0);

    // Return pointer to the ArtLoader instance
    static ArtLoader* instance();

    void getArt(QString query);
    QString artLocalUri();

private:
    // Instance pointer
    static ArtLoader *m_instance;

    QNetworkAccessManager *m_nManager;
    QNetworkRequest m_nRequest;

    QString m_TmpPath;
    QString m_queryHash;
    QString m_imageExt;
    bool m_bArtLoaded;

    QString localUri();
    bool isExists();
    void downloadImage(QString url);
    
Q_SIGNALS:
    void artLoaded(bool);
    
public Q_SLOTS:
    void clearArt();
    void loadArt(QString);

private Q_SLOTS:
    void respReply(QNetworkReply*);
    void imgReply(QNetworkReply*);
};

#endif // ARTLOADER_H
