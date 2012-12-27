#include "dmwidget.h"

#include <QTimer>

DmWidget::DmWidget(QWidget *parent) :
    QWidget(parent)
{
    //installEventFilter(this);

    setObjectName("dmwidget");

    setStyleSheet("QWidget#dmwidget {padding-right: 90px; border-radius: 7px; background-color: rgba(55, 55, 55, 210); color: #fff; font-size: 15px;} QLineEdit { border-radius: 2px; background-color: rgba(55, 55, 55, 210); color: #fff; } QLabel { color: #fff;  } ");

    setMaximumSize(QSize(420, 320));

    setFixedSize(QSize(250, 80));


    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *topLayout = new QHBoxLayout();
    QHBoxLayout *middleLayout = new QHBoxLayout();
    layout->addLayout(topLayout);
    layout->addLayout(middleLayout);

    m_wlTitle = new QLabel(this);
    m_wlTitle->setText("Ac/Dc - Back In Black");
    m_wlTitle->setStyleSheet("font-size: 12px;");
    m_wlTitle->setAlignment(Qt::AlignCenter);
    topLayout->addWidget(m_wlTitle);

    //middleLayout
    m_wlMb = new QLabel(this);
    m_wlMb->setStyleSheet("font-size: 10px;");
    m_wlMb->setText("5/25 Mb");
    middleLayout->addWidget(m_wlMb);

    m_wlQueue = new QLabel(this);
    m_wlQueue->setStyleSheet("font-size: 10px;");
    m_wlQueue->setText("In queue: 5");
    m_wlQueue->setAlignment(Qt::AlignRight);
    middleLayout->addWidget(m_wlQueue);

    m_wProgress = new QProgressBar(this);
    m_wProgress->setStyleSheet(":horizontal {border-radius: 0px; background: #BDBDBD;} ::chunk:horizontal { background: #000;}");
    m_wProgress->setFixedHeight(3);
    m_wProgress->setRange(0, 100);
    m_wProgress->setValue(25);
    layout->addWidget(m_wProgress);

    m_wbCancel = new StyledButton(tr("Cancel"), this);
    m_wbCancel->setFixedSize(50, 20);
    layout->addWidget(m_wbCancel);

    connect(m_wbCancel, SIGNAL(clicked()), this, SIGNAL(cancelAll()));
    connect(m_wbCancel, SIGNAL(clicked()), this, SLOT(onCancelAll()));



    //layout->addWidget(m_wList);

    hide();
    repaint();

    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(0.0);

    setGraphicsEffect(m_effect);

    m_animation = new QPropertyAnimation(m_effect, "opacity");
    m_animation->setDuration(550);
}

void DmWidget::paintEvent(QPaintEvent *)
 {
     QStyleOption opt;
     opt.init(this);
     QPainter p(this);
     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
 }

void DmWidget::parentResized(QRect pRect)
{
    m_VPos = QPoint(pRect.center().x() - width() / 2, pRect.bottom() - height() - 100);
    m_HPos.setY(m_VPos.y());
    m_HPos.setX(m_VPos.x() - width());

    move(m_VPos);
}

void DmWidget::onStarted()
{
    show();

    m_animation->disconnect();

    m_animation->setStartValue(0.0);
    m_animation->setEndValue(0.9);

    m_animation->start();
}

void DmWidget::onCurrentChanged(Track *track)
{
    QString title = track->artist() + " - " + track->title();
    if(title.size() > 35)
        title = title.left(35)+"...";

    m_wlTitle->setText(title);
}

void DmWidget::onProgressChanged(QString received, QString total, int percent)
{
    m_wlMb->setText(tr("%1/%2 Mb").arg(received).arg(total));

    m_wProgress->setValue(percent);
}

void DmWidget::onQueueChanged(int q)
{
    m_wlQueue->setText(tr("In queue: %1").arg(q));
}

void DmWidget::onAllComplete()
{
    m_wlTitle->setText(tr("All downloads completed"));

    QTimer::singleShot(1500, this, SLOT(hideMe()));
}

void DmWidget::onCancelAll()
{
    m_wlTitle->setText(tr("All downloads canceled"));
    m_wProgress->setValue(0);
    hideMe();

}

void DmWidget::hideMe()
{
    m_animation->setStartValue(0.9);
    m_animation->setEndValue(0.0);

    m_animation->start();

    m_animation->disconnect();
    connect(m_animation, SIGNAL(finished()), this, SLOT(hide()));

}
