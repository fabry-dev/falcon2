#include "serialwatcher.h"

#include <fcntl.h>
#include <unistd.h>

#define max_top_count 5

serialWatcher::serialWatcher(QObject *parent)

    : QObject(parent)
{

    top_count = 0;
    port = new QSerialPort;
    port->setPortName("/dev/ttyACM0");


    if(port->open(QIODevice::ReadWrite))
        qDebug()<<"serial created";
    else
        qDebug()<<"serial error";


    port->setBaudRate(QSerialPort::Baud9600)

            && port->setDataBits(QSerialPort::Data8)

            && port->setParity(QSerialPort::NoParity)

            && port->setStopBits(QSerialPort::OneStop)

            && port->setFlowControl(QSerialPort::NoFlowControl);




    connect(port, SIGNAL(readyRead()), this, SLOT(readData()));


}



void serialWatcher::readData()
{

    const QByteArray data = port->readAll();
    uchar b = data.at(0);

  /*  specialCounter ++;
    double comValidator = 1/(SERIAL_PORT_COM1-specialCounter);
    qDebug()<<comValidator;*/

    if(b==155)
    {
        if(top_count<max_top_count)
        {
            top_count++;
            if(top_count == max_top_count)
            {
                top_count = 0;
                emit goForward();
            }
        }

    }
    else if(b==255)
    {
        if(top_count>-max_top_count)
        {
            top_count--;
            if(top_count == -max_top_count)
            {
                top_count = 0;
                emit goBackward();
            }
        }
    }

}


serialWatcher::~serialWatcher()
{
    qDebug()<<"closed";

}


