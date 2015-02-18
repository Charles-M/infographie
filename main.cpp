#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include "tgaimage.cpp"
#include "geometrie.cpp"

using namespace std;

// CHAMPS
std::vector<Vec3f > sommets;
std::vector<Vec3f > textures;
std::vector<Vec3f > normaux;
std::vector<std::vector<Vec3i> > facets;
Vec3f lumiere = Vec3f(1,1,1) ;
Vec3f cam = Vec3f(0,0,1) ;
Vec3f center = Vec3f(0,0,0) ;
Vec3f u = Vec3f(0,1,0) ;
Matrix view, projection, changementBase, M, M_inv;
int ** zbuffer = NULL ; int maxi = 0 ;
const int w = 1000 ;

// PROTOTYPE
void readFile(string path) ;
void viewport(float taille) ;
void project(float coeff) ;
void lookat(Vec3f eye, Vec3f up, Vec3f centre) ;
void transform() ;
void createImage() ;
void line(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2) ;
void triangle(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2, int x3, int y3, int z3, TGAColor color3) ;

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

void viewport(int x, int y, int w, int h) {
	view = Matrix::identity();
	view[0][3] = x+w/2.f;
	view[1][3] = y+h/2.f;
	view[2][3] = 1.f;
	view[0][0] = w/2.f;
	view[1][1] = h/2.f;
	view[2][2] = 0;
}

void project(float coeff){
	projection = Matrix::identity() ;
	projection[3][2] = coeff ;
}

void lookat(Vec3f eye, Vec3f up, Vec3f centre) {
	changementBase = Matrix::identity() ;
	Vec3f iPrime, jPrime, kPrime ;
	kPrime = (eye-centre).normalize();
	iPrime = cross(up,kPrime).normalize();
	jPrime = cross(kPrime,iPrime).normalize();
	for (int i=0; i<3; i++) {
		changementBase[0][i] = iPrime[i];
		changementBase[1][i] = jPrime[i];
		changementBase[2][i] = kPrime[i];
		changementBase[i][3] = -centre[i];
	}
}

void transform() {
	for(int i = 0; i<(int)sommets.size(); i++)
		sommets[i] = proj<3>(M*embed<4>(sommets[i])) ;
	for(int i = 0; i<(int)normaux.size(); i++)
		normaux[i] = proj<3>(M_inv*embed<4>(normaux[i], 0.f)).normalize() ;
}

void createImage(){
	TGAImage im = TGAImage(w,w,1) ;
	for(int i = 0; i<w; i++){
		zbuffer[i] = new int[w] ;
		for(int j =0; j<w; j++) zbuffer[i][j] = -1000000 ;
	}
	float x1,x2,x3,y1,y2,y3,z1,z2,z3, scalaire1,scalaire2,scalaire3,moyenne ;
	for(int i = 0; i<facets.size();i++){
		// Zoom + Translation
		x1 = sommets[facets[i][0][0]-1][0] ;
		y1 = sommets[facets[i][0][0]-1][1];
		z1 = sommets[facets[i][0][0]-1][2];
		x2 = sommets[facets[i][1][0]-1][0];
		y2 = sommets[facets[i][1][0]-1][1];
		z2 = sommets[facets[i][1][0]-1][2];
		x3 = sommets[facets[i][2][0]-1][0];
		y3 = sommets[facets[i][2][0]-1][1];
		z3 = sommets[facets[i][2][0]-1][2];
		Vec3f n1(normaux[facets[i][0][2]-1]) ;
		Vec3f n2(normaux[facets[i][1][2]-1]) ;
		Vec3f n3(normaux[facets[i][2][2]-1]) ;
		n1 = n1.normalize() ;
		n2 = n2.normalize() ;
		n3 = n3.normalize() ;
		lumiere = lumiere.normalize() ;
		//	Coloration
		scalaire1 = max(lumiere * n1, 0.f);
		scalaire2 = max(lumiere * n2, 0.f);
		scalaire3 = max(lumiere * n3, 0.f);
		moyenne = (scalaire1+scalaire2+scalaire3)/3 ;
		if(moyenne > 0)
			 triangle(im, x1,y1,z1, TGAColor(scalaire1*255,1),x2,y2,z2, TGAColor(scalaire2*255,1),x3,y3,z3, TGAColor(scalaire3*255,1)) ;
	}
	im.flip_vertically() ;
	im.write_tga_file("ombrage_lisse.tga") ;
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
		y = (y-int(y) >= 0.5) ? ceil(y) : floor(y) ;
		z = (z-int(z) >= 0.5) ? ceil(z) : floor(z) ;

		color.val = (1-t)*color1.val + t*color2.val ;
		if(pentu){
			if(x>0 && y>0 && x<w-1 && y<w-1 && zbuffer[(int)y][x] < z){
				zbuffer[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				image.set(y,x,color) ;
			}
		}else{
			if(x>0 && y>0 && x<w-1 && y<w-1 && zbuffer[x][(int)y] < z){
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
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		z_h = (1.-t1)*z1 + t1*z2 ;
		z_b = (1.-t2)*z1 + t2*z3 ;
		z_h = (z_h-int(z_h) >= 0.5) ? ceil(z_h) : floor(z_h) ;
		z_b = (z_b-int(z_b) >= 0.5) ? ceil(z_b) : floor(z_b) ;

		line(image, x,y_h,z_h, TGAColor((1-t1)*color1.val+t1*color2.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
	for(int x = x2; x<=x3; x++){
		t1 = (x3==x2) ? 1 : (x-x2)/(float)(x3-x2) ;
		t2 = (x3==x1) ? 1 : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y2 + t1*y3 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		z_h = (1.-t1)*z2 + t1*z3 ;
		z_b = (1.-t2)*z1 + t2*z3 ;
		z_h = (z_h-int(z_h) >= 0.5) ? ceil(z_h) : floor(z_h) ;
		z_b = (z_b-int(z_b) >= 0.5) ? ceil(z_b) : floor(z_b) ;

		line(image, x,y_h,z_h, TGAColor((1-t1)*color2.val+t1*color3.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
}

int main(int argc, char** argv){
	readFile("obj/african_head.obj") ;
	zbuffer = new int*[w] ;
	viewport(0,0,w,w) ;
	lookat(cam, u, center) ;
	project(0) ;
	M = view*projection*changementBase ;
	M_inv = (projection*changementBase).invert_transpose() ;
	
	transform() ;
	createImage() ;
	return 0 ;
}