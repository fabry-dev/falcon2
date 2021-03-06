#ifndef MAINSCREEN_H
#define MAINSCREEN_H


#include <QWidget>
#include <qobject.h>
#include "qlabel.h"
#include "mpvwidget.h"
#include "qtimer.h"
#include "qpropertyanimation.h"

class mainScreen : public QLabel
{
    Q_OBJECT
public:
    explicit mainScreen(QLabel *parent = nullptr, QString PATH="", bool DEBUG=false);

private:
    QString PATH;
    std::vector<mpvWidget*> vps;
    std::vector<QPropertyAnimation*> anims;
    mpvWidget *mainVp;
    bool isPlaying;
    int totalWidth;
    void moveVideo(int dx);
    void loadContent(QStringList content);
    QStringList contentList;
    bool DEBUG;
    std::vector<mpvWidget*>sbVps;
    QTimer *timeoutTimer;
    void clearMovingPlayers(void);
    void setVideoFocus(mpvWidget *vp,bool focus);
private slots:
    void startVideos();
    void getUID(QString uid);

    void goBackward(void);
    void goForward(void);
    void standbyScreen(void);
    void preloadStandbyScreen(void);

protected:
     void keyPressEvent(QKeyEvent *ev);
     void mousePressEvent(QMouseEvent *event);

signals:
    void forceStop(void);

};

#endif // MAINSCREEN_H
