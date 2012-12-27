#include "shortcutsmanager.h"
#include "settings/settings.h"

#include <QDebug>


ShortcutsManager *ShortcutsManager::m_instance = 0;

ShortcutsManager::ShortcutsManager(QObject *parent) :
    QObject(parent)
{
    if(m_instance)
            qFatal("Only one instance of ShortcutsManager object is allowed");

    m_instance = this;


    m_gscPlayPause = new QxtGlobalShortcut(this);
    m_gscNext = new QxtGlobalShortcut(this);
    m_gscPrev = new QxtGlobalShortcut(this);
    m_gscShuffle = new QxtGlobalShortcut(this);
    m_gscRepeat = new QxtGlobalShortcut(this);
    m_gscShowWindow = new QxtGlobalShortcut(this);
    m_gscCurrent2library = new QxtGlobalShortcut(this);
    m_gscCurrentDownload = new QxtGlobalShortcut(this);
    connect(m_gscPlayPause, SIGNAL(activated()), this, SIGNAL(playPause()));
    connect(m_gscNext, SIGNAL(activated()), this, SIGNAL(next()));
    connect(m_gscPrev, SIGNAL(activated()), this, SIGNAL(prev()));
    connect(m_gscShuffle, SIGNAL(activated()), this, SIGNAL(shuffle()));
    connect(m_gscRepeat, SIGNAL(activated()), this, SIGNAL(repeat()));
    connect(m_gscShowWindow, SIGNAL(activated()), this, SIGNAL(showWindow()));
    connect(m_gscCurrent2library, SIGNAL(activated()), this, SIGNAL(current2Library()));
    connect(m_gscCurrentDownload, SIGNAL(activated()), this, SIGNAL(currentDownload()));

    QWidget *pWidget = static_cast<QWidget *>(parent);
    m_lscPlayPause = new QShortcut(pWidget);
    m_lscNext = new QShortcut(pWidget);
    m_lscPrev = new QShortcut(pWidget);
    m_lscShuffle = new QShortcut(pWidget);
    m_lscRepeat = new QShortcut(pWidget);
    m_lscCurrent2library = new QShortcut(pWidget);
    m_lscCurrentDownload = new QShortcut(pWidget);
    connect(m_lscPlayPause, SIGNAL(activated()), SIGNAL(playPause()));
    connect(m_lscNext, SIGNAL(activated()), SIGNAL(next()));
    connect(m_lscPrev, SIGNAL(activated()), SIGNAL(prev()));
    connect(m_lscShuffle, SIGNAL(activated()), SIGNAL(shuffle()));
    connect(m_lscRepeat, SIGNAL(activated()), SIGNAL(repeat()));
    connect(m_lscCurrent2library, SIGNAL(activated()), SIGNAL(current2Library()));
    connect(m_lscCurrentDownload, SIGNAL(activated()), SIGNAL(currentDownload()));

    connect(Settings::instance(), SIGNAL(shortcutsChanged()), SLOT(registerShortcuts()));

    registerShortcuts();
}

ShortcutsManager *ShortcutsManager::instance() {
    return m_instance;
}


void ShortcutsManager::registerShortcuts()
{
    Settings *settings = Settings::instance();

    m_gscPlayPause->setShortcut(QKeySequence(settings->getValue("g_shortcut/playpause").toString()));
    m_gscNext->setShortcut(QKeySequence(settings->getValue("g_shortcut/next").toString()));
    m_gscPrev->setShortcut(QKeySequence(settings->getValue("g_shortcut/prev").toString()));
    m_gscShuffle->setShortcut(QKeySequence(settings->getValue("g_shortcut/shuffle").toString()));
    m_gscRepeat->setShortcut(QKeySequence(settings->getValue("g_shortcut/repeat").toString()));
    m_gscShowWindow->setShortcut(QKeySequence(settings->getValue("g_shortcut/showwindow").toString()));
    m_gscCurrent2library->setShortcut(QKeySequence(settings->getValue("g_shortcut/current2library").toString()));
    m_gscCurrentDownload->setShortcut(QKeySequence(settings->getValue("g_shortcut/current_download").toString()));

    m_lscPlayPause->setKey(QKeySequence(settings->getValue("l_shortcut/playpause").toString()));
    m_lscNext->setKey(QKeySequence(settings->getValue("l_shortcut/next").toString()));
    m_lscPrev->setKey(QKeySequence(settings->getValue("l_shortcut/prev").toString()));
    m_lscShuffle->setKey(QKeySequence(settings->getValue("l_shortcut/shuffle").toString()));
    m_lscRepeat->setKey(QKeySequence(settings->getValue("l_shortcut/repeat").toString()));
    m_lscCurrent2library->setKey(QKeySequence(settings->getValue("l_shortcut/current2library").toString()));
    m_lscCurrentDownload->setKey(QKeySequence(settings->getValue("l_shortcut/current_download").toString()));

}
