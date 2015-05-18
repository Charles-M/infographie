#include <QApplication>
#include <QImage>
#include "fenetre.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Fenetre fen ;
    /*TGAImage image(1000,1000,1) ;
    image.read_tga_file("/home/ubuntu/Desktop/build/cube_orig.tga") ;
    QImage *im = new QImage(image.buffer(), 1000,1000,1000,QImage::Format_Indexed8);
    QScrollArea sc(&fen) ;
    QLabel lab ;
    lab.setMinimumSize(im->size());
    lab.setPixmap(QPixmap::fromImage(*im));
    sc.setWidget(&lab);*/
	fen.showMaximized();

    return app.exec();
}
