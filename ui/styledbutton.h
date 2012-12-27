#ifndef STYLEDBUTTON_H
#define STYLEDBUTTON_H

#include <QPushButton>

class StyledButton : public QPushButton
{
    Q_OBJECT
public:
    //explicit StyledButton(QWidget *parent = 0);
    explicit StyledButton(QWidget *parent=0);
    explicit StyledButton(const QString &text, QWidget *parent=0);
    StyledButton(const QIcon& icon, const QString &text, QWidget *parent=0);

    void setTransparent(const bool &s);

private:
    void setupUi();

Q_SIGNALS:

public Q_SLOTS:

};

#endif // STYLEDBUTTON_H
