#ifndef PARSER_H
#define PARSER_H

#include "playlist/track.h"
#include "network/auth.h"

#include <QObject>
#include <QQueue>

class Parser : public QObject
{
    Q_OBJECT
public:
    explicit Parser(QObject *parent = 0);

    /* Trim trash from tracks title strings

      "Люди" называющие треки "DJ Xz - Обожаю эту песенку!!1"
      при этом добавляющие кучу значков-сердечек и прочего хлама -
      должны быть анально покараны.
      Православная функция для удаления мусора.
    */
    static QString trimXlam(QString str);

    // Start searching&parsing with this string
    void search(QString str);

    // Loads library by given id
    void library(QString id, QString gid = "0");

    // Loads user music suggestions
    void suggestions();

    // Options set
    void setSearchStr(QString strSearch);
    void setOffset(int offset);
    void setMore(bool s);
    void setReqType(int type);

    // Return options
    int offset() { return m_iOffset; }
    bool more() { return m_morePossible; }

private:
    Auth *m_Auth;

    // Network manager
    QNetworkAccessManager *m_nManager;
    QNetworkRequest m_nRequest;
    QNetworkReply *m_nReply;

    QString m_lastSearch;
    int m_iOffset;
    bool m_morePossible;
    int m_ReqType;
    bool m_bBusy;

    // Store track titles for trim duplicates
    QStringList m_TrackStrings;

    Track *m_lastTrack;

    QQueue<Track *> m_UpTrackQueue;
    QQueue<Track *> m_MetaTrackQueue;

Q_SIGNALS:
    // Sends hot parsed track to playlist
    void newTrack(Track *);

    // Let us know, if more pages for parsing available
    void moreResults(bool);

    // Indicate when loading of data started or stoped
    void busy();
    void free();

    // Emit when loading meta/etc. is complete
    void savePlaylist();

private Q_SLOTS:
    void searchReply(QNetworkReply*);
    void libraryReply(QNetworkReply*);
    void suggestionsReply(QNetworkReply*);
    void trackReply(QNetworkReply*);

    void authComplete();

    void updateTrackUrl();

    void onTrackDestroyed();

public Q_SLOTS:
    void loadMoreResults();

    void updateTrack();

};

#endif // PARSER_H
