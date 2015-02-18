#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include <vector>
#include "tgaimage.cpp"

using namespace std;

vector<vector<float> > sommets ;
vector<vector<float> > textures ;
vector<vector<float> > normaux  ;
vector<vector<vector<int> > > facets  ;

void readFile(string path) {
	ifstream fichier(path.c_str()) ;
	if(fichier) {
		string buff, flag;
		vector<float> coords;
		vector<vector<int> > triplets ;
		vector<int> triplet ;
		char * p ;
		while(getline(fichier, buff)){
			if(buff.length()<2) continue ;
			flag = buff.substr(0,2) ;
			buff = buff.substr(2);
			if(flag.substr(0,1) == "v"){
				coords.push_back(strtod(buff.c_str(), &p)) ;
				coords.push_back(strtod(p, &p)) ;
				coords.push_back(strtod(p, NULL)) ;
				if(flag == "v ")	sommets.push_back(coords) ;
				else if(flag == "vn")	normaux.push_back(coords) ;
				else if(flag == "vt")	textures.push_back(coords) ;
				for(int i = 0; i<3; i++) coords.pop_back() ;
			} else if(flag == "f ") {
				p = (char * )buff.c_str() ;
				for(int i = 0; i<3; i++){
					triplet.push_back(strtol(p, &p, 10)) ;
					if(sizeof(p)>1) p = &p[1] ;
					triplet.push_back(strtol(p, &p, 10)) ;
					if(sizeof(p)>1) p = &p[1] ;
					triplet.push_back(strtol(p, &p, 10)) ;
					if(sizeof(p)>1) p = &p[1] ;
					triplets.push_back(triplet) ;
					for(int j = 0; j<3; j++) triplet.pop_back() ;
				}
				facets.push_back(triplets) ;
				for(int j = 0; j<3; j++) triplets.pop_back() ;
			}
		}
	} else cout << "OUPS" << endl ;
}

void line(TGAImage &image, int x1, int y1, int x2, int y2, TGAColor color){
	float t, y ;
	bool pentu = false ;
	if((pentu = abs(y2-y1)>abs(x2-x1))){
		swap(x1,y1) ;
		swap(x2,y2) ;
	}
	if(x2<x1) {
		swap(x1,x2) ;
		swap(y1,y2) ;
	}
	for(int x=x1; x<=x2; x++){
		t = (x2==x1) ? 1. : (x-x1)/(float)(x2-x1) ;
		y = (1.-t)*y1 + t*y2 ;
		y = (y-int(y) >= 0.5) ? ceil(y) : floor(y) ;
		
		if(pentu)
			image.set(y,x,color) ;
		else
			image.set(x,y,color) ;
	}
}

void triangle(TGAImage &image, int x1, int y1, int x2, int y2, int x3, int y3, TGAColor color){
	float y_h, y_b, t1, t2 ;
	if (x2<x1){
		swap(x2,x1) ;
		swap(y2,y1) ;
	}
	if (x3<x1){
		swap(x3,x1) ;
		swap(y3,y1) ;
	}
	if (x3<x2){
		swap(x3,x2) ;
		swap(y3,y2) ;
	}
	for(int x = x1; x<=x2; x++){
		t1 = (x2==x1) ? 1. : (x-x1)/(float)(x2-x1) ;
		t2 = (x3==x1) ? 1. : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y1 + t1*y2 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		
		line(image, x,y_h,x,y_b, color) ;
	}
	for(int x = x2; x<=x3; x++){
		t1 = (x3==x2) ? 1. : (x-x2)/(float)(x3-x2) ;
		t2 = (x3==x1) ? 1. : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y2 + t1*y3 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		
		line(image, x,y_h,x,y_b, color) ;
	}
}

struct Vec3f {
  union {
    struct {float x, y, z;};
    float raw[3];
  };
  Vec3f() : x(0), y(0), z(0) {}
  Vec3f(float _x, float _y, float _z) : x(_x),y(_y),z(_z) {}
  Vec3f(vector<float> tab) : x(tab[0]),y(tab[1]),z(tab[2]) {}
  Vec3f operator ^(const Vec3f &v) const { return Vec3f(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
  Vec3f operator +(const Vec3f &v) const { return Vec3f(x+v.x, y+v.y, z+v.z); }
  Vec3f operator -(const Vec3f &v) const { return Vec3f(x-v.x, y-v.y, z-v.z); }
  Vec3f operator *(float f) const { return Vec3f(x*f, y*f, z*f); }
  float operator *(const Vec3f &v) const { return x*v.x + y*v.y + z*v.z; }
  float norm () const { return std::sqrt(x*x+y*y+z*z); }
  Vec3f & normalize(float l=1) { *this = (*this)*(l/norm()); return *this; }
};

int main(int argc, char** argv){
	readFile("obj/african_head.obj") ;
	int w = 1000 ;
	TGAImage im = TGAImage(w,w,1) ;
	TGAColor blanc = TGAColor(255, 255, 255, 3) ;
	Vec3f lum(0,0,1) ;
	float x1,x2,x3,y1,y2,y3,z1,z2,z3, scalaire, zoom = w/2 ;
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
		Vec3f a(x1,y1,z1) ;
		Vec3f b(x2,y2,z2) ;
		Vec3f c(x3,y3,z3) ;
		Vec3f n = (a-b)^(a-c);
		n = n.normalize() ;
		scalaire = n*lum ;
		// Affichage
		if(scalaire > 0)
			triangle(im, x1,y1,x2,y2,x3,y3, TGAColor(scalaire*255, 1)) ;
	}
	im.flip_vertically() ;
	im.write_tga_file("ombrage_plat.tga") ;
	return 0 ;
}