#ifndef FENETRE_H
#define FENETRE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <QApplication>
#include "model.h"

class Fenetre : public QWidget
{
		Q_OBJECT
	public:
		Fenetre(QWidget *parent = 0);
		void choixIm() ;

	public slots:
		void ouvrirPopup() ;
		void rotDroite() ;
		void rotGauche() ;
		void rotHaut() ;
		void rotBas() ;
		void enableBuff() ;
		void enableWire() ;

	signals:

	private:
		Model m ;
		QPushButton *droite, *gauche, *haut, *bas ;
		QPushButton *open ;
		QLabel lab ;
        QImage *im;
		QScrollArea *sc ;
		bool enB, enW ;

};

#endif // FENETRE_H
