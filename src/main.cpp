#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QMainWindow>
#include <QColorDialog>
#include <QProcess>

#include <stdlib.h>
#include <locale.h>
#include <libintl.h>
#include <signal.h>

#include "DrawingWidget.h"
#include "FloatingWidget.h"
#include "FloatingSettings.h"
#include "WhiteBoard.h"
#include "Button.h"

#define _(String) gettext(String)

extern "C" {
#include "settings.h"
extern void* load_archive(void*args);
}


DrawingWidget *drawing;
FloatingWidget *floatingWidget;
FloatingSettings *floatingSettings;
WhiteBoard *board;
QMainWindow* mainWindow;
QMainWindow* tool;
QMainWindow* tool2;

QSlider *scrollHSlider;
QSlider *scrollVSlider;

QWidget *mainWidget;

#ifdef screenshot
#include "ScreenShot.h"
QPushButton *ssButton;
#endif


float scale = 1.0;

extern void setupWidgets();


extern QString archive_target;

int pagestatus;

int eraser_status;

extern int new_x;
extern int new_y;

void sighandler(int signum) {
    (void)signum;
    return;
}

class MainWindow : public QMainWindow {

public:
    MainWindow(){

    }

#define SCROLLSIZE 22*scale

protected:
     void closeEvent(QCloseEvent *event){
        puts("Close event");
        sighandler(SIGTERM);
        event->ignore();
     }
     void resizeEvent(QResizeEvent *event) override {
        QScreen *screen = QGuiApplication::primaryScreen();
        mainWidget->setFixedSize(screen->size().width(), screen->size().height());
        drawing->setFixedSize(screen->size().width(), screen->size().height());
        board->setFixedSize(screen->size().width(), screen->size().height());

        scrollHSlider->setFixedSize(event->size().width() - SCROLLSIZE*2, SCROLLSIZE);
        scrollHSlider->move(SCROLLSIZE, event->size().height() - SCROLLSIZE);
        scrollHSlider->setRange(0, screen->size().width() - event->size().width() );
        scrollHSlider->setVisible(screen->size().width() > event->size().width());

        scrollVSlider->setFixedSize(SCROLLSIZE, event->size().height() - SCROLLSIZE*2);
        scrollVSlider->move(event->size().width() - SCROLLSIZE, SCROLLSIZE);
        scrollVSlider->setRange(0, screen->size().height() - event->size().height() );
        scrollVSlider->setVisible(screen->size().height() > event->size().height());

        printf("%d %d\n",event->size().width(), event->size().height());
        new_x = get_int((char*)"cur-x");
        new_y = get_int((char*)"cur-y");
        // tool is not set under wayland
        if(tool != nullptr){
            tool->resize(floatingWidget->geometry().width(), floatingWidget->geometry().height());
        }
        if(tool2 != nullptr){
            tool2->resize(floatingSettings->geometry().width(), floatingSettings->geometry().height());
        }
        floatingWidget->moveAction();
        // Call the base class implementation
        QWidget::resizeEvent(event);
        drawing->update();
    }
    void changeEvent(QEvent *event) override {
        // Call the base class implementation
        QMainWindow::changeEvent(event);
        if(tool != nullptr){
            if (event->type() == QEvent::WindowStateChange) {
                if (isMinimized()) {
                    tool->hide();
                } else {
                    tool->show();
                }
            }
        }
        if(tool2 != nullptr){
            if (event->type() == QEvent::WindowStateChange) {
                if (isMinimized()) {
                    tool2->hide();
                } else {
                    tool2->show();
                }
            }
        }
        drawing->update();
    }
};

bool fuarMode;

int main(int argc, char *argv[]) {

#ifdef ETAP19
    QStringList args1;
    QProcess p1;
    args1 << "-e" << "eta-disable-gestures@pardus.org.tr";
    p1.execute("gnome-shell-extension-tool", args1);

    QStringList args2;
    QProcess p2;
    args2 << "set" << "org.gnome.mutter" << "overlay-key" << "''";
    p2.execute("gsettings", args2);
#endif

    // Force use X11 or Xwayland
    if(get_bool((char*)"xwayland")){
        setenv("QT_QPA_PLATFORM", "xcb;wayland",1);
    }
    //Force ignore system dpi
    setenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0", 1);
    setenv("QT_SCALE_FACTOR", "1", 1);

    settings_init();

    // translation part
    const char *systemLanguage = getenv("LANG");
    if (systemLanguage != nullptr) {
        bindtextdomain("pardus-pen", "/usr/share/locale");
        setlocale(LC_ALL, systemLanguage);
        textdomain("pardus-pen");
    } else {
        fprintf(stderr, "WARNING: LANG environment variable not set.\n");
    }

    // Fuar mode
    fuarMode = false;
    if (argc > 1) {
        if (strcmp(argv[1], "--fuar") == 0) {
            fuarMode = true;
        }
    }


    #ifdef QT5
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    #endif

    QCoreApplication::setApplicationName(_("Pardus Pen"));
    QCoreApplication::setOrganizationName("Pardus");

    QApplication app(argc, argv);

    mainWindow = new MainWindow();
    scale = QGuiApplication::primaryScreen()->geometry().height() / 1080.0;

    mainWidget = new QWidget(mainWindow);
    mainWidget->setAttribute(Qt::WA_AcceptTouchEvents, true);

    board = new WhiteBoard(mainWidget);
    board->setType(get_int((char*)"page"));
    board->setOverlayType(get_int((char*)"page-overlay"));

    drawing = new DrawingWidget(mainWidget);

    mainWindow->setWindowIcon(QIcon(":tr.org.pardus.pen.svg"));
    mainWindow->setWindowTitle(QString(_("Pardus Pen")));


#ifndef ETAP19
    // detect x11
    if(!getenv("WAYLAND_DISPLAY")){
        tool = new QMainWindow();
        tool2 = new QMainWindow();
        floatingSettings = new FloatingSettings(tool2);
        floatingWidget = new FloatingWidget(tool);
    } else {
#endif
        tool = nullptr;
        floatingSettings = new FloatingSettings(mainWindow);
        floatingWidget = new FloatingWidget(mainWindow);
#ifndef ETAP19
    }
#endif
    floatingWidget->setMainWindow(mainWindow);
    floatingWidget->setSettings(floatingSettings);
    floatingSettings->setHide();

    setupWidgets();
    if (tool != nullptr) {
        tool->setWindowFlags(Qt::WindowStaysOnTopHint
                                  | Qt::Tool
                                  | Qt::WindowSystemMenuHint
                                  | Qt::FramelessWindowHint);
        tool->setAttribute(Qt::WA_TranslucentBackground, true);
        tool->setAttribute(Qt::WA_NoSystemBackground, true);
        tool->setStyleSheet(
            "background: none;"
            "color: black;"
            "font-size: "+QString::number(18*scale)+"px;"
        );

        tool->show();
    }
     if (tool2 != nullptr) {
        tool2->setWindowFlags(Qt::WindowStaysOnTopHint
                                  | Qt::Tool
                                  | Qt::WindowSystemMenuHint
                                  | Qt::FramelessWindowHint);
        tool2->setAttribute(Qt::WA_TranslucentBackground, true);
        tool2->setAttribute(Qt::WA_NoSystemBackground, true);
        tool2->setStyleSheet(
            "background: none;"
            "color: black;"
            "font-size: "+QString::number(18*scale)+"px;"
        );
        tool2->hide();
    }

    //mainWindow->setWindowFlags(Qt::WindowSystemMenuHint
    //                      | Qt::FramelessWindowHint);

    mainWindow->setAttribute(Qt::WA_TranslucentBackground, true);
    mainWindow->setAttribute(Qt::WA_NoSystemBackground, true);
    mainWindow->setAttribute(Qt::WA_AcceptTouchEvents, true);
    mainWindow->setStyleSheet(
        "background: none;"
        "color: black;"
        "font-size: "+QString::number(18*scale)+"px;"
    );

    scrollHSlider = new QSlider(Qt::Horizontal, mainWindow);
    scrollVSlider = new QSlider(Qt::Vertical, mainWindow);

    QObject::connect(scrollHSlider, &QSlider::valueChanged, [=](int value) {
        mainWidget->move(-1*value, mainWidget->y());
    });

    QObject::connect(scrollVSlider, &QSlider::valueChanged, [=](int value) {
        mainWidget->move(mainWidget->x(), value - scrollVSlider->maximum());
    });

    scrollVSlider->setValue(0);
    scrollHSlider->setValue(0);
    QScreen *screen = QGuiApplication::primaryScreen();
    mainWindow->resize(screen->size().width(), screen->size().height());
    mainWidget->setFixedSize(screen->size().width(), screen->size().height());
    mainWindow->showFullScreen();
    mainWindow->move(0,0);


#ifdef LIBARCHIVE
    if (argc > 1) {
        pthread_t ptid;
        archive_target = QString(argv[1]);
        pthread_create(&ptid, NULL, &load_archive, NULL);
    }
#endif
    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGQUIT, sighandler);
    return app.exec();
}
