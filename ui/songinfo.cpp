#include "songinfo.h"

#include <QHBoxLayout>
#include <QGridLayout>
#include <QDebug>

#include <QPainter>
#include <QLinearGradient>

#include "settings/settings.h"
#include "network/artloader.h"

// define cover size
const QSize SongInfo::COVER_SIZE = QSize(100, 100);

SongInfo::SongInfo(QWidget *parent) :
    QWidget(parent)
{

    m_TmpPath = Settings::instance()->appPath()+"tmp/";

    setupUi();

    //setObjectName("CoverWgt");

    //setStyleSheet("#CoverWgt {background-color: blue; margin-bottom: -20px;}");
}

QImage blurred(const QImage& image, const QRect& rect, int radius, bool alphaOnly = false)
{
    int tab[] = { 14, 10, 8, 6, 5, 5, 4, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2 };
    int alpha = (radius < 1)  ? 16 : (radius > 17) ? 1 : tab[radius-1];

    QImage result = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    int r1 = rect.top();
    int r2 = rect.bottom();
    int c1 = rect.left();
    int c2 = rect.right();

    int bpl = result.bytesPerLine();
    int rgba[4];
    unsigned char* p;

    int i1 = 0;
    int i2 = 3;

    if (alphaOnly)
        i1 = i2 = (QSysInfo::ByteOrder == QSysInfo::BigEndian ? 0 : 3);

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r1) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += bpl;
        for (int j = r1; j < r2; j++, p += bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c1 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p += 4;
        for (int j = c1; j < c2; j++, p += 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int col = c1; col <= c2; col++) {
        p = result.scanLine(r2) + col * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= bpl;
        for (int j = r1; j < r2; j++, p -= bpl)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    for (int row = r1; row <= r2; row++) {
        p = result.scanLine(row) + c2 * 4;
        for (int i = i1; i <= i2; i++)
            rgba[i] = p[i] << 4;

        p -= 4;
        for (int j = c1; j < c2; j++, p -= 4)
            for (int i = i1; i <= i2; i++)
                p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha / 16) >> 4;
    }

    return result;
}


void SongInfo::setupUi()
{
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    m_wlArt = new QLabel("art", this);
    m_wlArt->setFixedSize(110, 121);

    mainLayout->addWidget(m_wlArt);

    // Set last art or clear
    QString lastArt = Settings::instance()->getValue("song/art").toString();

    if(lastArt != "")
        setupImage(QPixmap(lastArt));
    else
        clearCover();


}

void SongInfo::setupImage(QPixmap imageSrc)
{
    if(!imageSrc.isNull()) {
        QImage imgBack = QImage(QPixmap(":/images/cover_back").toImage());

        QImage image = imageSrc.scaledToHeight(COVER_SIZE.height()).toImage();

        image = image.copy( 0, 0, COVER_SIZE.width(), COVER_SIZE.height());

        // shadow
        //QPixmap shadowPix = QPixmap(":/images/cover_back");
        //QPixmap shadowPix = QPixmap(":/images/bkg_light");

//        // Y Gradient
//        QPoint start(0,0);
//        QPoint end(0, image.height());
//        QLinearGradient gradient(start, end);
//        gradient.setColorAt(0.0, Qt::black);
//        gradient.setColorAt(0.1, Qt::transparent);
//        //gradient.setColorAt(0.9, Qt::transparent);
//        //gradient.setColorAt(1.0, Qt::black);
//        gradient.setColorAt(1.0, Qt::transparent);

//        // X Gradient
//        QPoint startX(0,0);
//        QPoint endX(image.width(), 0);
//        QLinearGradient gradientX(startX, endX);
//        gradientX.setColorAt(0.0, Qt::black);
//        gradientX.setColorAt(0.1, Qt::transparent);
//        gradientX.setColorAt(0.9, Qt::transparent);
//        gradientX.setColorAt(1.0, Qt::black);

//        // Create gradient alfa mask
//        QImage mask(image.size(), image.format());
//        QPainter painter(&mask);
//        painter.setBrush(Qt::white);
//        painter.drawRect(-1,-1,image.width(), image.width());
//        //painter.fillRect(image.rect(), gradient);
//        //painter.fillRect(image.rect(), gradientX);
//        painter.end();

//        // Setup gradient alfa mask
//        image.setAlphaChannel(mask);

        //paint shadow
        //QImage image2 = QImage(100, 100, QImage::Format_RGB32);
        //image2 = image.copy(0,5,100,100);

        QPainter sPainter(&imgBack);
        //sPainter.fillRect(0,95,100,5, Qt::black);
        sPainter.drawPixmap(5, 5, 100, 100, QPixmap::fromImage(image));
        sPainter.end();

        // Setup image
        m_wlArt->setPixmap(QPixmap::fromImage(imgBack));
    } else {
        clearCover();
    }
}

void SongInfo::onArtLoaded(bool state)
{
    QString artUrl = ArtLoader::instance()->artLocalUri();


    if(state) {
        setupImage(QPixmap(artUrl));
        Settings::instance()->setValue("song/art", artUrl);
    } else {
        clearCover();
        Settings::instance()->setValue("song/art", "");
    }
}

void SongInfo::clearCover()
{
    //qDebug() << "Cover Cleared";
    Settings::instance()->setValue("song/art", "");
    setupImage(QPixmap(":/images/no_cover"));
}
