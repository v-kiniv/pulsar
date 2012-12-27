#include "tray.h"
#include "settings/settings.h"

#include <QDebug>
#include <QEvent>
#include <QWheelEvent>

Tray::Tray(QObject *parent) :
    QObject(parent)
{
    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(1000);

    createActions();
    createIcon();

    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), SLOT(trayClicked(QSystemTrayIcon::ActivationReason)));
    connect(m_blinkTimer, SIGNAL(timeout()), SLOT(blink()));

    if(!Settings::instance()->getValue("window/hide_tray").toBool())
        trayIcon->show();

}

void Tray::createActions()
{
    showWindow = new QAction(QIcon::fromTheme("view-restore"), tr("Show"), this);
    connect(showWindow, SIGNAL(triggered()), this, SIGNAL(showW()));

    pauseAction = new QAction(QIcon::fromTheme("media-playback-start"), tr("Play"), this);
    connect(pauseAction, SIGNAL(triggered()), this, SIGNAL(pause()));

    nextAction = new QAction(QIcon::fromTheme("media-skip-forward"), tr("Next"), this);
    connect(nextAction, SIGNAL(triggered()), this, SIGNAL(next()));

    prevAction = new QAction(QIcon::fromTheme("media-skip-backward"), tr("Prev"), this);
    connect(prevAction, SIGNAL(triggered()), this, SIGNAL(prev()));

    quitAction = new QAction(QIcon::fromTheme("application-exit"), tr("Quit"), this);
    connect(quitAction, SIGNAL(triggered()), this, SIGNAL(quit()));
}

void Tray::createIcon()
{
    // Create context menu
    trayMenu = new QMenu();
    trayMenu->addAction(showWindow);
    trayMenu->addSeparator();
    trayMenu->addAction(pauseAction);
    trayMenu->addAction(prevAction);
    trayMenu->addAction(nextAction);
    trayMenu->addSeparator();
    trayMenu->addAction(quitAction);

    // Create tray icon object
    trayIcon = new SysTrayIcon(this);
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, SIGNAL(sEvent(QEvent*)), SLOT(catchEvent(QEvent*)));
    //setPauseIcon();
    trayIcon->setIcon(QIcon(QPixmap(":/icons/tray_default")));
    trayIcon->setToolTip("Pulsar");
}

/* SLOTS */
void Tray::trayClicked(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
        Q_EMIT showW();

    if(reason == QSystemTrayIcon::MiddleClick)
        Q_EMIT pause();
}

void Tray::catchEvent(QEvent *e)
{
    if(e->type() == QEvent::Wheel) {
        if(((QWheelEvent*)e)->delta() > 0)
            Q_EMIT wheelUp();
        else
            Q_EMIT wheelDown();
    } else {
        e->accept();
    }
}

void Tray::blink()
{
    static bool pauseIcon = true;

    if(pauseIcon) {
        trayIcon->setIcon(QIcon(QPixmap(":/icons/tray_default")));
        pauseIcon = false;
    } else {
        trayIcon->setIcon(QIcon(QPixmap(":/icons/tray_paused")));
        pauseIcon = true;
    }
}

void Tray::setPlayIcon()
{
    trayIcon->setIcon(QIcon(QPixmap(":/icons/tray_playing")));
    pauseAction->setText(tr("Pause"));
    pauseAction->setIcon(QIcon::fromTheme("media-playback-pause"));
    m_blinkTimer->stop();
}

void Tray::setPauseIcon()
{
    trayIcon->setIcon(QIcon(QPixmap(":/icons/tray_paused")));
    pauseAction->setText(tr("Play"));
    pauseAction->setIcon(QIcon::fromTheme("media-playback-start"));
    m_blinkTimer->start();
}

void Tray::setTitles(QString artist, QString title)
{
    trayIcon->setToolTip(artist + " - " + title);
}
