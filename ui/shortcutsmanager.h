#ifndef SHORTCUTSMANAGER_H
#define SHORTCUTSMANAGER_H

#include <QObject>
#include <QxtGlobalShortcut>
#include <QShortcut>

class ShortcutsManager : public QObject
{
    Q_OBJECT
public:
    explicit ShortcutsManager(QObject *parent = 0);

    static ShortcutsManager *instance();

private:
    static ShortcutsManager *m_instance;

    QxtGlobalShortcut *m_gscPlayPause;
    QShortcut *m_lscPlayPause;
    QxtGlobalShortcut *m_gscNext;
    QShortcut *m_lscNext;
    QxtGlobalShortcut *m_gscPrev;
    QShortcut *m_lscPrev;
    QxtGlobalShortcut *m_gscShuffle;
    QShortcut *m_lscShuffle;
    QxtGlobalShortcut *m_gscRepeat;
    QShortcut *m_lscRepeat;
    QxtGlobalShortcut *m_gscShowWindow;
    QxtGlobalShortcut *m_gscCurrent2library;
    QShortcut *m_lscCurrent2library;
    QxtGlobalShortcut *m_gscCurrentDownload;
    QShortcut *m_lscCurrentDownload;



Q_SIGNALS:
    void playPause();
    void next();
    void prev();
    void shuffle();
    void repeat();
    void showWindow();
    void current2Library();
    void currentDownload();
    
public Q_SLOTS:
    void registerShortcuts();
    
};

#endif // SHORTCUTSMANAGER_H
