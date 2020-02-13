#ifndef PLAYERWINDOW_H
#define PLAYERWINDOW_H

#include <QtWidgets/QOpenGLWidget>
#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

class mpvWidget Q_DECL_FINAL: public QOpenGLWidget
{
    Q_OBJECT
public:
    mpvWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~mpvWidget();
    void command(const QVariant& params);
    void setProperty(const QString& name, const QVariant& value);
    QVariant getProperty(const QString& name) const;
    QSize sizeHint() const { return QSize(480, 270);}
    void loadFile(QString);
    void setLoop(bool looping);

    void setCrop();
    void loadFilePaused(QString videoFile);
    unsigned int getId() const;
    void setId(unsigned int value);
public slots:
    void pause(void);
    void stopAndHide(void);
    void stop();
    void play();

signals:
    void durationChanged(int value);
    void positionChanged(int value);
    void videoOver();
    void clicked(QPoint pos);
protected:
    void initializeGL() Q_DECL_OVERRIDE;
    void paintGL() Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event);
private Q_SLOTS:
    void swapped();
    void on_mpv_events();
    void maybeUpdate();
private:
    void handle_mpv_event(mpv_event *event);
    static void on_update(void *ctx);
    unsigned int id;

    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
};



#endif // PLAYERWINDOW_H
