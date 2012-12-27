#ifndef SONGINFO_H
#define SONGINFO_H

#include <QWidget>
#include <QLabel>

class SongInfo : public QWidget
{
    Q_OBJECT
public:
    explicit SongInfo(QWidget *parent = 0);

private:
    static const QSize COVER_SIZE;

    QString m_TmpPath;

    QLabel *m_wlArt;
    QLabel *m_wlArtist;
    QLabel *m_wlTitle;
    QLabel *m_wlAlbum;
    QLabel *m_wlBrate;

    void setupUi();
    void setupImage(QPixmap imageSrc);
    
Q_SIGNALS:
    
public Q_SLOTS:
    void onArtLoaded(bool);
    void clearCover();
    
};

#endif // SONGINFO_H
