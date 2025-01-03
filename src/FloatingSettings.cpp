#include "FloatingSettings.h"

#include <QMainWindow>

extern "C" {
#include "settings.h"
}


extern QMainWindow* tool2;

#include <QMap>

class SettingsPages {
public:
    void addPage(qint64 id, QWidget *data) {
        values[id] = data;
    }

    QWidget * getPage(qint64 id) {
        if (values.contains(id)) {
            return values[id];
        } else {
            return NULL;
        }
    }

private:
    QMap<qint64, QWidget*> values;
};

SettingsPages settingsPages;

FloatingSettings::FloatingSettings(QWidget *parent) : QWidget(parent) {
    layout = new QVBoxLayout(this);
    setLayout(layout);
    QString style = QString(
    "QWidget {"
        "border-radius:13px;"
        "background-color: #cc939393;"
    "}");
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    setStyleSheet(style);
    cur_height = 0;
    cur_width = 0;
}

void FloatingSettings::addPage(QWidget *widget) {
    settingsPages.addPage(num_of_item, widget);
    layout->addWidget(widget);
    widget->hide();
    num_of_item++;
}

void FloatingSettings::reload(){
    if(num_of_item <= current_page){
        return;
    }
    settingsPages.getPage(current_page)->show();
    cur_width = settingsPages.getPage(current_page)->size().width();
    cur_height = settingsPages.getPage(current_page)->size().height();
    setFixedSize(cur_width, cur_height);
    if(tool2 != nullptr) {
        tool2->setFixedSize(cur_width, cur_height);
    }

}

void FloatingSettings::setHide(){
    if(tool2 != nullptr) {
        tool2->hide();
    }
    hide();
}

void FloatingSettings::setPage(int num){
    if(num_of_item < num){
        return;
    }
    current_page = num;
    if(settingsPages.getPage(current_page)->isVisible()){
        settingsPages.getPage(current_page)->hide();
        setHide();
        return;
    }
    for(int i=0;i<num_of_item;i++){
        settingsPages.getPage(i)->hide();
    }
    reload();
    if(tool2 != nullptr) {
        tool2->show();
    }
    show();
}
