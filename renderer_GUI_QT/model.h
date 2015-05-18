#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <sstream>
#include "tgaimage.h"
#include "geometrie.h"
#include <string.h>
#include <vector>
#include <cmath>
#include <stdlib.h>

class Model
{
	public:
		Model() ;
		Model(std::string path);
		void readFile(std::string path) ;
		void line(TGAImage &image, int x1, int y1, int x2, int y2, TGAColor color) ;
		void lineDegrad(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2) ;
		void lineTexture(TGAImage &image, int x1, int y1, int z1, Vec3f tex1, int x2, int y2, int z2, Vec3f tex2) ;
		void triangleDegrad(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2, int x3, int y3, int z3, TGAColor color3) ;
		void triangleTexture(TGAImage &image, Vec3i P1, Vec3f tex1, Vec3i P2, Vec3f tex2, Vec3i P3, Vec3f tex3) ;
		void loadImage() ;
		void translation(int x0, int x1, int x2) ;
		void rotation2x(int x0, int x1, float angle) ;
		void rotation2y(int x0, int x1, float angle) ;
		void rotation2z(int x0, int x1, float angle) ;
		void zoom(int zoom) ;
		void  transform() ;
		void viewport(float taille);
		void project(float coeff);
		void lookat(Vec3f eye, Vec3f up, Vec3f centre) ;
		TGAImage & getImage() ;
		TGAImage & getTampon() ;
		TGAImage & getWire() ;


	private:
		std::string path ;
		TGAColor color ;
		TGAImage image, tampon, wire, diffuse, nm, spec ;
		float ** zbuffer, ** ombreBuff;
		int maxi, w ;
		bool ombre ;
		Vec3f lumiere, cam, center ;
		std::vector<Vec3f > sommets, sommets_init ;
		std::vector<Vec3f > textures ;
		std::vector<Vec3f > normaux, normaux_init  ;
		std::vector<std::vector<Vec3i> > facets  ;
		Matrix view, projection, changementBase, M, M_inv, MShad ;
};

/*struct Vec3f {
  union {
    struct {float x, y, z;};
    float raw[3];
  };
  Vec3f() : x(0), y(0), z(0) {}
  Vec3f(float _x, float _y, float _z) : x(_x),y(_y),z(_z) {}
  Vec3f(std::vector<float> tab) : x(tab[0]),y(tab[1]),z(tab[2]) {}
  Vec3f operator ^(const Vec3f &v) const { return Vec3f(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
  Vec3f operator +(const Vec3f &v) const { return Vec3f(x+v.x, y+v.y, z+v.z); }
  Vec3f operator -(const Vec3f &v) const { return Vec3f(x-v.x, y-v.y, z-v.z); }
  Vec3f operator *(float f) const { return Vec3f(x*f, y*f, z*f); }
  float operator *(const Vec3f &v) const { return x*v.x + y*v.y + z*v.z; }
  float norm () const { return std::sqrt(x*x+y*y+z*z); }
  Vec3f & normalize(float l=1) { *this = (*this)*(l/norm()); return *this; }
};*/

#endif // MODEL_H
