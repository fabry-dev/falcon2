#ifndef ACR122U_H
#define ACR122U_H


#include <QObject>
#include "qdebug.h"
#include "qtimer.h"
#include "qthread.h"
class ACR122U:public QObject
{
    Q_OBJECT
public:
    explicit ACR122U(QObject *parent = nullptr);
signals:
    void getUID(QString);
};


class ACR122UWorker:public QObject
{
 Q_OBJECT
public:
    explicit ACR122UWorker(QObject *parent = nullptr);
    ~ACR122UWorker();
private slots:
    int readNFC();
    void disableBuzzer(void);
signals:
    void getUID(QString);

};


#endif // ACR122U_H
