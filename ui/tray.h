#ifndef TRAY_H
#define TRAY_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QTimer>

class SysTrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit SysTrayIcon(QObject *parent = 0) : QSystemTrayIcon(parent) {

    }

protected:
    virtual bool event(QEvent *e) {
        Q_EMIT sEvent(e);
        return true;
    }

Q_SIGNALS:
    void sEvent(QEvent *);
};

class Tray : public QObject
{
    Q_OBJECT
public:
    explicit Tray(QObject *parent = 0);

private:
    SysTrayIcon *trayIcon;
    QMenu *trayMenu;

    QAction *showWindow;
    QAction *pauseAction;
    QAction *nextAction;
    QAction *prevAction;
    QAction *quitAction;

    QTimer *m_blinkTimer;

    void createActions();
    void createIcon();


Q_SIGNALS:
    void showW();
    void click();
    void pause();
    void next();
    void prev();
    void quit();

    void wheelUp();
    void wheelDown();

private Q_SLOTS:
    void trayClicked(QSystemTrayIcon::ActivationReason);
    void catchEvent(QEvent *);
    void blink();

public Q_SLOTS:
    void setPauseIcon();
    void setPlayIcon();
    void setTitles(QString, QString);

};

#endif // TRAY_H
