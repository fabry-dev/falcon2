#ifndef MAINSCREEN_H
#define MAINSCREEN_H


#include <QWidget>
#include <qobject.h>
#include "qlabel.h"
#include "mpvwidget.h"
#include "qtimer.h"

#define vpCount 7

class mainScreen : public QLabel
{
    Q_OBJECT
public:
    explicit mainScreen(QLabel *parent = nullptr, QString PATH="");

private:
    QString PATH;
    std::vector<mpvWidget*> vps;
    mpvWidget *mainVp;
    bool isPlaying;
    int totalWidth;
    void moveVideo(int dx);
    void loadContent(QStringList content);
    QStringList contentList;
private slots:
    void startVideos();
    void getUID(QString uid);

protected:
     void keyPressEvent(QKeyEvent *ev);
     void mousePressEvent(QMouseEvent *event);

signals:
    void forceStop(void);

};

#endif // MAINSCREEN_H
