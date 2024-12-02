#if screenshot
#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include <iostream>

#include "ScreenShot.h"

#define _(String) gettext(String)

extern "C" {
#include "which.h"
}

void takeScreenshot(){
    QString pics = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QDateTime time = QDateTime::currentDateTime();
    QString imgname = pics + "/" + time.toString("yyyy-MM-dd_hh-mm-ss") + ".png";
    int status = 1;
    // detect X11
    if (!getenv("WAYLAND_DISPLAY")){
        QScreen *screen = QGuiApplication::primaryScreen();
        QPixmap pixmap = screen->grabWindow(0);
        QFile file(imgname);
        file.open(QIODevice::WriteOnly);
        pixmap.save(&file, "PNG");
        status = 0;
    } else {
        std::string spectacle(which((char*)"spectacle"));
        std::string grim(which((char*)"grim"));
        if(strlen(spectacle.c_str()) != 0){
            status = system(("QT_QPA_PLATFORM='wayland' "+spectacle+" -fbnmo "+imgname.toStdString()).c_str());
        } else if(strlen(grim.c_str()) != 0){
            status = system((grim+" -t png "+imgname.toStdString()).c_str());
        }
    }
    // show message
    QMessageBox messageBox;
    Qt::WindowFlags flags =  Qt::Dialog | Qt::X11BypassWindowManagerHint;
    messageBox.setWindowFlags(flags);
    messageBox.setText(_("Info"));
    std::string msg;
    if (status == 0){
        msg = _("Screenshot saved:") + imgname.toStdString() + "\n";
    } else {
        msg = _("Failed To save:") + imgname.toStdString() + "\n";
    }
    messageBox.setInformativeText(msg.c_str());
    messageBox.setIcon(QMessageBox::Information);
    messageBox.exec();

}

#endif
