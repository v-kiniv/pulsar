#include "authwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QDebug>


AuthWindow::AuthWindow(QWidget *parent) :
    StyledWidget(parent)
{
    setupUi();
    setTitle(tr("Authorization required"));
    hide();
    //requireAuth("LastFM", "http://google.com");
}

void AuthWindow::setupUi()
{
    setFixedSize(350, 250);

    QVBoxLayout *mLayout = new QVBoxLayout();
    setLayout(mLayout);
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    mLayout->setMargin(10);

    m_wlText = new QLabel("text", this);
    m_wlText->setAlignment(Qt::AlignTop);
    m_wlText->setWordWrap(true);
    mLayout->addWidget(m_wlText);

    // Setup loading label
    m_wlLoading = new QLabel("LOADING");
    m_wlLoading->setMinimumWidth(160);

    // Set loading qmovie
    m_LoadingMov = new QMovie(":/icons/loading_light");
    m_wlLoading->setMovie(m_LoadingMov);

    mLayout->addWidget(m_wlLoading);
    mLayout->setAlignment(m_wlLoading, Qt::AlignHCenter);


    m_wbCancel = new StyledButton(tr("Cancel"));
    m_wbNext = new StyledButton(tr("Next"));
    buttonsLayout->addWidget(m_wbCancel);
    buttonsLayout->addWidget(m_wbNext);

    connect(m_wbCancel, SIGNAL(clicked()), SLOT(hide()));
    connect(m_wbCancel, SIGNAL(clicked()), SIGNAL(authCanceled()));
    connect(m_wbNext, SIGNAL(clicked()), SLOT(doAuth()));

    mLayout->addLayout(buttonsLayout);
    mLayout->setAlignment(buttonsLayout, Qt::AlignBottom);
}

void AuthWindow::openBrowser(const QUrl &url)
{
    QDesktopServices::openUrl(url);
    m_wbNext->setText(tr("Next"));
    m_bBrowserOpened = true;
}

void AuthWindow::setLoadig(const bool &s)
{
    if(s) {
        m_wlLoading->show();
        m_LoadingMov->start();
    } else {
        m_wlLoading->hide();
        m_LoadingMov->stop();
    }
}

void AuthWindow::requireAuth(const QString &service, const QString &url)
{
    qDebug() << "aw: auth required";
    m_bBrowserOpened = false;
    m_bAuthRequested = false;

    m_url = QUrl(url);
    m_sService = service;

    setLoadig(false);

    m_wlText->setText(tr("To integrate with the service <b>%0</b>, click button 'login', then authorize on website and allow application <b>Pulsar</b> to use your account on <b>%0</b>.<br><br>After you allow application to use your account, close browser and click <b>Next</b> button.").arg(service));
    m_wbNext->setText(tr("Login"));

    show();
}

void AuthWindow::onSuccess(bool s)
{
    m_bSuccess = s;
    setLoadig(false);
    if(s) {
        setLoadig(false);
        m_wlText->setText(tr("Your %0 account successfully connected to Pulsar.").arg(m_sService));
        m_wbNext->setText(tr("Close"));
    } else {
        m_wlText->setText(tr("<b>Pulsar</b> can't access to your <b>%0</b> account, something goes wrong or you're does not grant access for application on <b>%0</b> website.<br><br>Click <b>Retry</b> button and try again or <b>Cancel</b> for closing this window.").arg(m_sService));
        m_wbNext->setText(tr("Retry"));
    }
}

void AuthWindow::doAuth()
{
    if(m_bBrowserOpened) {
        if(!m_bAuthRequested) {
            Q_EMIT authCompleted();
            setLoadig(true);
            m_bAuthRequested = true;
        } else {
            if(m_bSuccess)
                hide();
            else
                requireAuth(m_sService, m_url.toString());
        }
    } else {
        openBrowser(m_url);
    }
}
