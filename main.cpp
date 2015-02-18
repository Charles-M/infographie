#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "tgaimage.cpp"
#include "geometrie.cpp"

using namespace std;

std::vector<Vec3f > sommets;
std::vector<Vec3f > textures;
std::vector<Vec3f > normaux;
std::vector<std::vector<Vec3i> > facets;
int ** zbuffer = NULL ; int maxi = 0 ;
int taille = 0 ;

void readFile(string path) {
	ifstream fichier(path.c_str()) ;
	if(fichier) {
        string buff, flag;
        Vec3f coords;
		vector<Vec3i> triplets(3) ;
        char c ;
        while(!fichier.eof()){
            getline(fichier, buff) ;
            if(buff.length()<2) continue ;
            istringstream s(buff.c_str()) ;
            s >> flag ;
            if(flag.substr(0,1) == "v"){
                s >> coords[0] >> coords[1] >> coords[2] ;
				if(buff.substr(0,2) == "v ") sommets.push_back(coords) ;
                else if(flag.substr(1,1) == "n")	normaux.push_back(coords) ;
                else if(flag.substr(1,1) == "t")	textures.push_back(coords) ;
            } else if(flag.substr(0,1) == "f") {
				for(int i = 0; i<3; i++){
                    s >> buff ;
                    istringstream s2(buff.c_str()) ;
                    s2 >> triplets[i][0] >> c >> triplets[i][1] >> c >> triplets[i][2] ;
                }
                facets.push_back(triplets) ;
            }
		}
    } else cout << "Erreur lors de la lecture du fichier obj" << endl ;
}

void line(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2){
	float t, y ;
	int z ;
	TGAColor color(0,1) ;
	bool pentu = false ;
	if((pentu = abs(y2-y1)>abs(x2-x1))){
		swap(x1,y1) ;
		swap(x2,y2) ;
	}
	if(x2<x1) {
		swap(x1,x2) ;
		swap(y1,y2) ;
		swap(z1,z2) ;
		swap(color2,color1) ;
	}

	for(int x=x1; x<=x2; x++){
		t = (x2==x1) ? 1 : (x-x1)/(float)(x2-x1) ;
		y = (1-t)*y1 + t*y2 ;
		z = (1-t)*z1 + t*z2 ;

		color.val = (1-t)*color1.val + t*color2.val ;
		if(pentu){
			if(x>0 && y>0 && x<taille && y<taille && zbuffer[(int)y][x] < z){
				zbuffer[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				image.set(y,x,color) ;
			}
		}else{
			if(x>0 && y>0 && x<taille && y<taille && zbuffer[x][(int)y] < z){
				zbuffer[x][(int)y] = z ;
				if(maxi < z) maxi = z ;
				image.set(x,y,color) ;
			}
		}
	}
}

void triangle(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2, int x3, int y3, int z3, TGAColor color3){
	float y_h, y_b,z_h, z_b,  t1, t2 ;
	if (x2<x1){
		swap(x2,x1) ;
		swap(y2,y1) ;
		swap(z2,z1) ;
		swap(color2,color1) ;
	}
	if (x3<x1){
		swap(x3,x1) ;
		swap(y3,y1) ;
		swap(z3,z1) ;
		swap(color3,color1) ;
	}
	if (x3<x2){
		swap(x3,x2) ;
		swap(y3,y2) ;
		swap(z3,z2) ;
		swap(color3,color2) ;
	}
	for(int x = x1; x<x2; x++){
		t1 = (x2==x1) ? 1 : (x-x1)/(float)(x2-x1) ;
		t2 = (x3==x1) ? 1 : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y1 + t1*y2 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		z_h = (1.-t1)*z1 + t1*z2 ;
		z_b = (1.-t2)*z1 + t2*z3 ;

		line(image, x,y_h,z_h, TGAColor((1-t1)*color1.val+t1*color2.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
	for(int x = x2; x<=x3; x++){
		t1 = (x3==x2) ? 1 : (x-x2)/(float)(x3-x2) ;
		t2 = (x3==x1) ? 1 : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y2 + t1*y3 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		z_h = (1.-t1)*z2 + t1*z3 ;
		z_b = (1.-t2)*z1 + t2*z3 ;

		line(image, x,y_h,z_h, TGAColor((1-t1)*color2.val+t1*color3.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
}

int main(int argc, char** argv){
	readFile("obj/african_head.obj") ;
	const int w = 1000 ;
	zbuffer = new int*[w] ;
	for(int i = 0; i<w; i++){
		zbuffer[i] = new int[w] ;
		for(int j =0; j<w; j++) zbuffer[i][j] = 0 ;
	}
	taille = w-1 ;
	TGAImage im = TGAImage(w,w,1) ;
	TGAColor blanc = TGAColor(255, 255, 255, 3) ;
	Vec3f lum(0,0,1) ;
	float x1,x2,x3,y1,y2,y3,z1,z2,z3, scalaire1,scalaire2,scalaire3,moyenne, zoom = w/2 ;
	for(int i = 0; i<facets.size();i++){
		// Zoom + Translation
		x1 = zoom*(sommets[facets[i][0][0]-1][0]) + w/2 ;
		y1 = zoom*(sommets[facets[i][0][0]-1][1]) + w/2 ;
		z1 = zoom*(sommets[facets[i][0][0]-1][2]) + w/2 ;
		x2 = zoom*(sommets[facets[i][1][0]-1][0]) + w/2 ;
		y2 = zoom*(sommets[facets[i][1][0]-1][1]) + w/2 ;
		z2 = zoom*(sommets[facets[i][1][0]-1][2]) + w/2 ;
		x3 = zoom*(sommets[facets[i][2][0]-1][0]) + w/2 ;
		y3 = zoom*(sommets[facets[i][2][0]-1][1]) + w/2 ;
		z3 = zoom*(sommets[facets[i][2][0]-1][2]) + w/2 ;
		Vec3f n1(normaux[facets[i][0][2]-1]) ;
		Vec3f n2(normaux[facets[i][1][2]-1]) ;
		Vec3f n3(normaux[facets[i][2][2]-1]) ;
		n1 = n1.normalize() ;
		n2 = n2.normalize() ;
		n3 = n3.normalize() ;
		//	Coloration
		scalaire1 = min(max(lum * n1, 0.f), 1.f);
		scalaire2 = min(max(lum * n2, 0.f), 1.f);
		scalaire3 = min(max(lum * n3, 0.f), 1.f);
		moyenne = (scalaire1+scalaire2+scalaire3)/3 ;
		if(moyenne > 0)
			 triangle(im, x1,y1,z1, TGAColor(scalaire1*255,1),x2,y2,z2, TGAColor(scalaire2*255,1),x3,y3,z3, TGAColor(scalaire3*255,1)) ;
	}
	im.flip_vertically() ;
	im.write_tga_file("ombrage_lisse.tga") ;
	return 0 ;
}