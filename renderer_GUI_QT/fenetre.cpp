#include "fenetre.h"
#include <QFileDialog>
#include <QLayout>
#include <QTimer>
#include <QEventLoop>
#include <QShortcut>

Fenetre::Fenetre(QWidget *parent) : QWidget(parent), lab(), enB(false), enW(false)
{
	lab.setMinimumSize(1000,1000);
	sc = new QScrollArea(this) ;
	sc->setWidget(&lab);

	QGridLayout *laybutton = new QGridLayout()  ;
	droite = new QPushButton("droite") ;
	gauche = new QPushButton("gauche") ;
	haut = new QPushButton("haut") ;
	bas = new QPushButton("bas") ;
	open = new QPushButton("OPEN") ;
	laybutton->addWidget(new QLabel("Rotation 1 : "), 0,0, Qt::AlignCenter);
	laybutton->addWidget(gauche,0,1);
	connect(gauche, SIGNAL(clicked()), this, SLOT(rotGauche())) ;
	laybutton->addWidget(droite,0,2);
	connect(droite, SIGNAL(clicked()), this, SLOT(rotDroite())) ;
	laybutton->addWidget(new QLabel("Rotation 2 : "),1,0, Qt::AlignCenter);
	laybutton->addWidget(haut,1,1);
	connect(haut, SIGNAL(clicked()), this, SLOT(rotHaut())) ;
	laybutton->addWidget(bas,1,2);
	connect(bas, SIGNAL(clicked()), this, SLOT(rotBas())) ;
	laybutton->addWidget(open,0,3,2,1);
	connect(open, SIGNAL(clicked()), this, SLOT(ouvrirPopup())) ;

	// Raccourci clavier
	QShortcut *space = new QShortcut(Qt::Key_Space, this) ;
	connect(space, SIGNAL(activated()), this, SLOT(enableBuff()));
	QShortcut *w = new QShortcut(Qt::Key_W, this) ;
	connect(w, SIGNAL(activated()), this, SLOT(enableWire()));
	QShortcut *o = new QShortcut(Qt::Key_O, this) ;
	connect(o, SIGNAL(activated()), this, SLOT(ouvrirPopup()));
	QShortcut *f_r = new QShortcut(Qt::Key_Right, this) ;
	connect(f_r, SIGNAL(activated()), this, SLOT(rotDroite()));
	QShortcut *f_l = new QShortcut(Qt::Key_Left, this) ;
	connect(f_l, SIGNAL(activated()), this, SLOT(rotGauche()));
	QShortcut *f_u = new QShortcut(Qt::Key_Up, this) ;
	connect(f_u, SIGNAL(activated()), this, SLOT(rotHaut()));
	QShortcut *f_d = new QShortcut(Qt::Key_Down, this) ;
	connect(f_d, SIGNAL(activated()), this, SLOT(rotBas()));
	QShortcut *esc = new QShortcut(Qt::Key_Escape, this) ;
	connect(esc, SIGNAL(activated()), qApp, SLOT(quit()));
	QShortcut *q = new QShortcut(Qt::Key_Q, this) ;
	connect(q, SIGNAL(activated()), qApp, SLOT(quit()));

	QVBoxLayout *lay = new QVBoxLayout();
	lay->addWidget(sc);
	lay->addLayout(laybutton);

	setLayout(lay);
	m = Model("african_head.obj") ;
	choixIm();
}

void Fenetre::ouvrirPopup() {
    QString rep = QFileDialog::getOpenFileName(this,"Choix du .obj") ;
	m = Model(rep.toStdString()) ;
	choixIm();
}

void Fenetre::rotDroite(){
    m.loadImage();
	choixIm();
}

void Fenetre::rotGauche(){
	m.loadImage();
	choixIm();
}

void Fenetre::rotHaut(){
	m.loadImage();
	choixIm();
}

void Fenetre::rotBas(){
	m.loadImage();
	choixIm();
}

void Fenetre::enableBuff() {
	enW = false ;
	enB = !enB ;
	choixIm();
}

void Fenetre::enableWire() {
	enB = false ;
	enW = !enW ;
	choixIm();
}

void Fenetre::choixIm(){
	if(enB)
		im = new QImage(m.getTampon().buffer(), 1000, 1000, 1000, QImage::Format_Indexed8) ;
	else if(enW)
		im = new QImage(m.getWire().buffer(), 1000, 1000, 1000, QImage::Format_Indexed8) ;
	else
		im = new QImage(m.getImage().buffer(), 1000, 1000, 3000, QImage::Format_RGB888) ;
	lab.setPixmap(QPixmap::fromImage(*im));
}
