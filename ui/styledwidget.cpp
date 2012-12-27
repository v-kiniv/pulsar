#include "styledwidget.h"
#include "ui/pulsar.h"

#include <QStyleOption>
#include <QPainter>


StyledWidget::StyledWidget(QWidget *parent) :
    QWidget(parent)
{
    Pulsar *pulsar = Pulsar::instance();
    connect(pulsar, SIGNAL(resized(QRect)), SLOT(parentResized(QRect)));

    setObjectName("StyledWidget");

    setStyleSheet("QLineEdit { border-radius: 2px; background-color: rgba(55, 55, 55, 210); color: #fff; } QLabel { color: #fff; } QWidget#StyledWidget {padding-right: 90px; border-radius: 5px; background-color: rgba(55, 55, 55, 210); color: #fff; font-size: 15px;}");

    setMaximumSize(QSize(420, 320));
    setMinimumSize(200,100);

    m_Layout = new QVBoxLayout(this);

    m_wlTitle = new QLabel("Widget title");
    m_wlTitle->setAlignment(Qt::AlignTop|Qt::AlignHCenter);
    m_wlTitle->setStyleSheet("background-color: rgba(30, 30, 30, 120); color: #DDDDDD; border-radius: 2px; padding-top: 1px; padding-bottom: 1px;");
    m_Layout->addWidget(m_wlTitle, 0);
    m_wlTitle->hide();
    //m_Layout->setAlignment(m_wlTitle,  Qt::AlignTop|Qt::AlignHCenter);
}

void StyledWidget::setLayout(QLayout *l)
{
    m_Layout->addLayout(l, 1);
    //->setAlignment(l, Qt::AlignTop);
}

void StyledWidget::setTitle(const QString &str)
{
    m_wlTitle->setText(str);
    m_wlTitle->show();
}

void StyledWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void StyledWidget::show()
{
    setParent(0);
    setParent(Pulsar::instance());
    QWidget::show();
}

void StyledWidget::parentResized(QRect prect)
{
    QPoint center = prect.center() - rect().center();
    center.setY(120);
    move(center);
}

