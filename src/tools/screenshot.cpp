#include "../tools.h"
QPushButton *ssButton;

void setupScreenShot(){
    #ifdef screenshot
    ssButton = create_button(":images/screenshot.svg", [=](){
        floatingWidget->hide();
        floatingSettings->setHide();
        takeScreenshot();
        floatingWidget->show();
    });
    #endif
}
