#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include "ui/styledwidget.h"
#include "ui/styledbutton.h"

#include <QLabel>
#include <QMovie>
#include <QUrl>

class AuthWindow : public StyledWidget
{
    Q_OBJECT
public:
    explicit AuthWindow(QWidget *parent = 0);

private:
    void setupUi();
    void openBrowser(const QUrl &url);
    void setLoadig(const bool &s);

    QLabel *m_wlText;
    StyledButton *m_wbCancel;
    StyledButton *m_wbNext;

    // Label show animated loading gif
    QLabel *m_wlLoading;

    // Animate gif loading
    QMovie *m_LoadingMov;

    bool m_bBrowserOpened;
    bool m_bAuthRequested;
    bool m_bSuccess;
    QUrl m_url;
    QString m_sService;
    
signals:
    void authCompleted();
    void authCanceled();
    
public slots:
    void requireAuth(const QString &service, const QString &url);
    void onSuccess(bool);

private slots:
    void doAuth();
    
};

#endif // AUTHWINDOW_H
