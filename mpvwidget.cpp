#include "mpvwidget.h"
#include <stdexcept>
#include <QtGui/QOpenGLContext>
#include <QtCore/QMetaObject>
#include "qdebug.h"
static void wakeup(void *ctx)
{
    QMetaObject::invokeMethod((mpvWidget*)ctx, "on_mpv_events", Qt::QueuedConnection);
}

static void *get_proc_address(void *ctx, const char *name) {
    Q_UNUSED(ctx);
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void *)glctx->getProcAddress(QByteArray(name));
}

mpvWidget::mpvWidget(QWidget *parent, Qt::WindowFlags f)
    : QOpenGLWidget(parent, f)
{

    setlocale(LC_NUMERIC, "C");

    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    //  mpv_set_option_string(mpv, "terminal", "yes");
    //  mpv_set_option_string(mpv, "msg-level", "all=v");
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Make use of the MPV_SUB_API_OPENGL_CB API.
    mpv::qt::set_option_variant(mpv, "vo", "opengl-cb");


    // Request hw decoding, just for testing.
    mpv::qt::set_option_variant(mpv, "hwdec", "auto");

    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(mpv_gl, mpvWidget::on_update, (void *)this);
    connect(this, SIGNAL(frameSwapped()), SLOT(swapped()));

    mpv_observe_property(mpv, 0, "duration", MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "time-pos", MPV_FORMAT_DOUBLE);
    mpv_set_wakeup_callback(mpv, wakeup, this);


}

mpvWidget::~mpvWidget()
{
    makeCurrent();
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);
    // Until this call is done, we need to make sure the player remains
    // alive. This is done implicitly with the mpv::qt::Handle instance
    // in this class.
    mpv_opengl_cb_uninit_gl(mpv_gl);
}

void mpvWidget::command(const QVariant& params)
{
    mpv::qt::command_variant(mpv, params);
}

void mpvWidget::loadFile(QString videoFile)
{
    QByteArray ba = videoFile.toLatin1();
    //command(QStringList() << "loadfile" << videoFile<<"append-play");
    command(QStringList() << "loadfile" << videoFile);
    setProperty("pause",false);
}

void mpvWidget::loadFilePaused(QString videoFile)
{
    QByteArray ba = videoFile.toLatin1();
    //command(QStringList() << "loadfile" << videoFile<<"append-play");
    setProperty("pause",true);
    command(QStringList() << "loadfile" << videoFile);

}


void mpvWidget::stop()
{

    const char *cmd[] = {"stop",NULL, NULL};

    mpv_command(mpv, cmd);

}


void mpvWidget::stopAndHide(void)
{


  setProperty("pause", true);

     command(QStringList()<< "seek"<<"0"<<"absolute"<<"exact");

    lower();
    hide();
}

void mpvWidget::pause(void)
{

    const char *cmd[] = {"pause",NULL, NULL};

    mpv_command(mpv, cmd);


}





void mpvWidget::play()
{
    show();
    setProperty("pause",false);
    const char *cmd[] = {"play",NULL, NULL};

    mpv_command(mpv, cmd);

    raise();

}


void mpvWidget::setLoop(bool looping)
{
    if(looping)
        mpv::qt::set_option_variant( mpv, "loop", "inf");
    else
        mpv::qt::set_option_variant( mpv, "loop", "0");
}



void mpvWidget::setCrop()
{

    mpv::qt::set_option_variant(mpv, "vf", QStringList()<<"crop"<<"1920:1080:0:0");
}



void mpvWidget::setProperty(const QString& name, const QVariant& value)
{
    mpv::qt::set_property_variant(mpv, name, value);
}

QVariant mpvWidget::getProperty(const QString &name) const
{
    return mpv::qt::get_property_variant(mpv, name);
}

void mpvWidget::initializeGL()
{
    int r = mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
        throw std::runtime_error("could not initialize OpenGL");
}

void mpvWidget::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void mpvWidget::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void mpvWidget::on_mpv_events()
{
    // Process all events, until the event queue is empty.
    while (mpv) {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event->event_id == MPV_EVENT_NONE) {
            break;
        }
        handle_mpv_event(event);
    }
}

void mpvWidget::handle_mpv_event(mpv_event *event)
{
    switch (event->event_id) {
    case MPV_EVENT_PAUSE:
    {
        // qDebug()<<"ennnnnd";
        emit videoOver();
        break;
    }
    case MPV_EVENT_IDLE:
    {

        emit videoOver();
        break;
    }
    case MPV_EVENT_PROPERTY_CHANGE: {
        mpv_event_property *prop = (mpv_event_property *)event->data;
        if (strcmp(prop->name, "time-pos") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT positionChanged(time);
            }
        } else if (strcmp(prop->name, "duration") == 0) {
            if (prop->format == MPV_FORMAT_DOUBLE) {
                double time = *(double *)prop->data;
                Q_EMIT durationChanged(time);
            }
        }
        break;
    }
    default: ;
        // Ignore uninteresting or unknown events.
    }
}

// Make Qt invoke mpv_opengl_cb_draw() to draw a new/updated video frame.
void mpvWidget::maybeUpdate()
{
    // If the Qt window is not visible, Qt's update() will just skip rendering.
    // This confuses mpv's opengl-cb API, and may lead to small occasional
    // freezes due to video rendering timing out.
    // Handle this by manually redrawing.
    // Note: Qt doesn't seem to provide a way to query whether update() will
    //       be skipped, and the following code still fails when e.g. switching
    //       to a different workspace with a reparenting window manager.
    if (window()->isMinimized()) {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    } else {
        update();
    }
}

void mpvWidget::on_update(void *ctx)
{
    QMetaObject::invokeMethod((mpvWidget*)ctx, "maybeUpdate");
}

unsigned int mpvWidget::getId() const
{
    return id;
}

void mpvWidget::setId(unsigned int value)
{
    id = value;
}


void mpvWidget::mousePressEvent(QMouseEvent *event)
{
    emit clicked(event->pos());
}
