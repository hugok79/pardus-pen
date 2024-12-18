#include "tools.h"

QPushButton *save;
QPushButton *open;

#ifdef LIBARCHIVE
extern "C" {
    QString archive_target;
    void *save_all(void* arg) {
        (void)arg;
        drawing->saveAll(archive_target);
        return NULL;
    }
    void *load_archive(void* arg) {
        (void)arg;
        drawing->loadArchive(archive_target);
        return NULL;
    }
}
#endif

void setupSaveLoad(){
#ifdef LIBARCHIVE
    save = create_button(":images/save.svg", [=](){
        QString filter = _("Pen Files (*.pen);;");
        #ifdef QPRINTER
        filter += _("PDF Files (*.pdf);;");
        #endif
        filter += _("All Files (*.*)");
        QString file = QFileDialog::getSaveFileName(drawing, _("Save File"), QDir::homePath(), filter);
        //drawing->saveAll(file);
        pthread_t ptid;
        // Creating a new thread
        archive_target = file;
        pthread_create(&ptid, NULL, &save_all, NULL);
    });
    
    open = create_button(":images/open.svg", [=](){
        QString filter = _("Pen Files (*.pen);;");
        filter += _("All Files (*.*)");
        QString filename = QFileDialog::getOpenFileName(drawing, _("Open File"), QDir::homePath(), filter);
        if(!filename.isEmpty()){
            pthread_t ptid;
            archive_target = filename;
            pthread_create(&ptid, NULL, &load_archive, NULL);
        }
    });
#endif
}