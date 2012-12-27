#ifndef STYLEDWIDGET_H
#define STYLEDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class StyledWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StyledWidget(QWidget *parent = 0);
    void setLayout(QLayout *l);
    void setTitle(const QString &str);

protected:
    void paintEvent(QPaintEvent *);

    QVBoxLayout *m_Layout;
    QLabel *m_wlTitle;
    
signals:
    
public slots:
    void show();

private slots:
    void parentResized(QRect);

protected slots:

};

#endif // STYLEDWIDGET_H
