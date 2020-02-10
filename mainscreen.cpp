#include "mainscreen.h"
#include "math.h"
#include "qdebug.h"
#include "qpainter.h"




mainScreen::mainScreen(QLabel *parent, QString PATH) : QLabel(parent),PATH(PATH)
{

    resize(1920,1080);
    setStyleSheet("QLabel { background-color : black; }");

    totalWidth = 0;
    isPlaying = false;





}

void mainScreen::startVideos()
{

    qDebug()<<contentList;
    qDebug()<<vps.size();

    if(contentList.size()!=vps.size())
    {
        qDebug()<<"content size and player mismatch";
        return;
    }

    for(int i = 0;i<vps.size();i++)
    {
        mpvWidget *vp = vps.at(i);
        vp->show();
        vp->setProperty("pause", true);
        vp->loadFilePaused(PATH+contentList[i]);
      // vp->raise();
    }
}


void mainScreen::moveVideo(int dx)
{

    if(!isPlaying)
    {
        for(auto vp:vps)
        {
            if((vp->pos().x()>1920) && (vp->pos().x()+dx<=1920)&&(mainVp!=vp))
            {

                mainVp = vp;
                isPlaying = true;

                //entrée par la droite
            }
            else  if((vp->pos().x()<1920) && (vp->pos().x()+dx>=1920)&&(mainVp!=vp))
            {

                mainVp = vp;
                isPlaying = true;

                //entrée par la gauche
            }
            else
                vp->show();



            vp->move(vp->pos().x()+dx,vp->pos().y());

            if(vp->pos().x()<-vp->width())
            {
                vp->move(totalWidth-vp->width(),0);
            }
            else if(vp->pos().x()+vp->width()>totalWidth)
            {

                vp->move(-vp->width(),0);
            }
        }

    }
    else
    {
        if(mainVp==NULL)
        {
            isPlaying = false;
            return;
        }


        if(dx<0)
        {
            mainVp-> command(QStringList()<< "frame-step");

            if((mainVp->getProperty("eof-reached")).toBool())//end of file
            {
                isPlaying = false;
                mainVp = NULL;
            }

        }
        else if(dx>0)
        {

            double percent = mainVp->getProperty("percent-pos").toDouble();

            percent-=5;


            mainVp-> command(QStringList()<< "seek"<<QString::number(percent)<<"absolute-percent"<<"exact");

          mainVp->setProperty("pause", true);

            if(mainVp->getProperty("time-pos").toDouble()<=0)
            {
                isPlaying = false;
                mainVp = NULL;
            }

        }


    }


}





void mainScreen::getUID(QString uid)
{
    QStringList content;
    QFile file(PATH+"content/"+uid+".cfg");
    if(!file.open(QIODevice::ReadOnly)) {
        qDebug()<<"no content file";

    }
    else
    {
        QTextStream in(&file);
        QString  line;
        while(!in.atEnd())
        {
            line = in.readLine();
            content.append(line);
        }
        file.close();
    }

    if(content.size()>0)
        loadContent(content);
}



void mainScreen::loadContent(QStringList content)
{

    for(auto vp:vps)
    {
        vp->close();
        vp->deleteLater();
    }
    mainVp = NULL;
    vps.clear();
    totalWidth = 0;
    isPlaying = false;
    contentList.clear();
    for(int i = 0;i<content.size();i++)
    {
        contentList.append(content.at(i));
        mpvWidget *vp = new mpvWidget(this);
        vp->resize(1920,1080);
        vp->move(totalWidth,0);
        vp->setLoop(false);
        vp->setProperty("keep-open",true);
        vp->setProperty("mute",true);
        vps.push_back(vp);
        vp->setAttribute( Qt::WA_TransparentForMouseEvents );
        totalWidth+=vp->width();
        if(vp->x()==1920)
        {
            isPlaying = true;
            mainVp = vp;
        }


    }
   QTimer::singleShot(500, this, SLOT(startVideos()));


}



void mainScreen::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == 16777236)//forward
    {

        moveVideo(10);

    }
    if(ev->key() == 16777234)//backward
    {
        moveVideo(-10);

    }
}




void mainScreen::mousePressEvent(QMouseEvent *event)
{

emit forceStop();

}
