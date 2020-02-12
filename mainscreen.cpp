#include "mainscreen.h"
#include "math.h"
#include "qdebug.h"
#include "qpainter.h"

#define ANIM_MAG 1920/40
#define ANIM_DURATION 100


mainScreen::mainScreen(QLabel *parent, QString PATH,bool DEBUG) : QLabel(parent),PATH(PATH),DEBUG(DEBUG)
{

    resize(1920,1080);
    setStyleSheet("QLabel { background-color : black; }");

    totalWidth = 0;
    isPlaying = false;


    getUID("9723E0DF9000");


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

        for(auto anim:anims)
        {
            if(anim->state()==QAbstractAnimation::Running)//do not move if one animation is still going.
                return;
        }


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

            /*QPropertyAnimation *appearance1 = new QPropertyAnimation(vp, "pos");
            appearance1->setDuration(200);*/

            if(vp->pos().x()>=totalWidth-1920)
                vp->move(-vp->width(),0);

            else if(vp->pos().x()<-1920)
                vp->move(totalWidth-1920,0);



            anims[vp->getId()]->setStartValue(vp->pos());
            anims[vp->getId()]->setEndValue(QPoint(vp->x()+dx,vp->y()));
            anims[vp->getId()]->setEasingCurve(QEasingCurve::InCurve);
            anims[vp->getId()]->start();



            //vp->move(vp->pos().x()+dx,vp->pos().y());



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

void mainScreen::checkVideoPosition(void)
{
    return;

    QPropertyAnimation *anim = (QPropertyAnimation*)QObject::sender() ;
    mpvWidget *vp = (mpvWidget*)anim->targetObject();
    if(vp->pos().x()<-vp->width())
    {
        vp->move(totalWidth-vp->width(),0);
    }
    else if(vp->pos().x()>=totalWidth)
    {
        vp->move(-vp->width(),0);
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


    for(QPropertyAnimation *anim:anims)
        anim->deleteLater();

    anims.clear();


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
        vp->setId(i);
        totalWidth+=vp->width();
        if(vp->x()==1920)
        {
            isPlaying = true;
            mainVp = vp;
        }


        QPropertyAnimation *anim = new QPropertyAnimation(vp, "pos");
        anim->setDuration(ANIM_DURATION);
        anim->setEasingCurve(QEasingCurve::Linear);
        // connect(anim,SIGNAL(finished()),this,SLOT(checkVideoPosition()));
        anims.push_back(anim);
    }
    QTimer::singleShot(100, this, SLOT(startVideos()));


}



void mainScreen::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == 16777236)//forward
    {

        moveVideo(ANIM_MAG);

    }
    if(ev->key() == 16777234)//backward
    {
        moveVideo(-ANIM_MAG);

    }
}


void mainScreen::goBackward(void)
{
    moveVideo(ANIM_MAG);

}

void mainScreen::goForward(void)
{
    moveVideo(-ANIM_MAG);

}

void mainScreen::mousePressEvent(QMouseEvent *event)
{
    if(DEBUG)
        emit forceStop();

}
