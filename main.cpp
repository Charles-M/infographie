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
std::vector<Vec3f > sommets, sommets_init ;
std::vector<Vec3f > textures ;
std::vector<Vec3f > normaux, normaux_init  ;
std::vector<std::vector<Vec3i> > facets;
Vec3f P1, P2, P3, T1, T2, T3 ;
Vec3f lumiere = Vec3f(1,1,1) ;
Vec3f cam = Vec3f(0,0,5) ;
Vec3f center = Vec3f(0,0,0) ;
Vec3f u = Vec3f(0,1,0) ;
Matrix view, projection, changementBase, M, M_inv, M_shad, trans;
float ** zbuffer = NULL, ** ombre_buff = NULL ; int maxi = 0, mini = 0 ;
const int w = 1000 ;
TGAImage nm, diffuse, specular, tangent ;
bool ombre = false ;

// PROTOTYPE
void readFile(string path) ;
void viewport(float taille) ;
void project(float coeff) ;
void lookat(Vec3f eye, Vec3f up, Vec3f centre) ;
void transform() ;
void createImage() ;
void line(TGAImage &image, int x1, int y1, int z1, Vec3f n1, TGAColor color1, int x2, int y2, int z2, Vec3f n2, TGAColor color2) ;
void triangle(TGAImage &image, Vec3i P1, Vec3f tex1, Vec3f n1, Vec3i P2, Vec3f tex2, Vec3f n2, Vec3i P3, Vec3f tex3, Vec3f n3) ;

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
				if(buff.substr(0,2) == "v ") sommets_init.push_back(coords) ;
                else if(flag.substr(1,1) == "n")	normaux_init.push_back(coords) ;
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
    } else cout << "OUPS" << endl ;
	sommets = std::vector<Vec3f >(sommets_init.size()) ;
	normaux = std::vector<Vec3f >(normaux_init.size()) ;
}

void viewport(int x, int y, int w, int h) {
	view = Matrix::identity();
	view[0][3] = x+w/2.f;
	view[1][3] = y+h/2.f;
	view[2][3] = 0;
	view[0][0] = w/2.f;
	view[1][1] = h/2.f;
	view[2][2] = (w+h)/2.f;
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

void line(TGAImage &image, int x1, int y1, int z1, Vec3f n1, Vec3f tex1, int x2, int y2, int z2, Vec3f n2, Vec3f tex2){
	float t, y, z, s, spe, diff, shad ;
    int x_tex, y_tex ;
	Vec3f n ;
	bool pentu = false ;
	TGAColor color ;
	
	if((pentu = abs(y2-y1)>=abs(x2-x1))){
		swap(x1,y1) ;
		swap(x2,y2) ;
	}
	if(x2<x1) {
		swap(x1,x2) ;
		swap(y1,y2) ;
		swap(z1,z2) ;
		swap(tex2,tex1) ;
		swap(n1,n2) ;
	}
	for(int x=x1; x<=x2; x++){
		t = (x2==x1) ? 1 : (x-x1)/(float)(x2-x1) ;
		y = (1-t)*y1 + t*y2 ;
		z = (1-t)*z1 + t*z2 ;
		n = (n1*(1-t) + n2*t).normalize() ;
		
		if(!ombre){
			mat<3,3,float> baseTan1 ;
			baseTan1[0] = P1-P3 ;
			baseTan1[1] = P2-P3 ;
			baseTan1[2] = n ;
			baseTan1 = baseTan1.invert();
			Vec3f tx = baseTan1*Vec3f(T1.x-T3.x, T2.x-T3.x, 0) ;
			Vec3f ty = baseTan1*Vec3f(T1.y-T3.y, T2.y-T3.y, 0) ;
			mat<3,3,float> baseTan2 ;
			baseTan2.set_col(0, tx.normalize()) ;
			baseTan2.set_col(1, ty.normalize()) ;
			baseTan2.set_col(2, n) ;
			x_tex = ((1-t)*tex1[0] + t*tex2[0])*nm.get_width() ;
			y_tex = ((1-t)*tex1[1] + t*tex2[1])*nm.get_height() ;
			TGAColor colorNm = nm.get(x_tex, y_tex) ;
			TGAColor colorNmTan = tangent.get(x_tex, y_tex) ;
			TGAColor colorDiffuse = diffuse.get(x_tex, y_tex) ;
			TGAColor colorSpec = specular.get(x_tex, y_tex) ;
			Vec3f l = lumiere.normalize() ;
			Vec3f n_tan = Vec3f(colorNmTan.r/255.f*2.f-1.f,colorNmTan.g/255.f*2.f-1.f,colorNmTan.b/255.f*2.f-1.f) ;
			//n = (baseTan2*n_tan).normalize() ;
			n = proj<3>(M_inv*embed<4>(Vec3f(colorNm.r/255.f*2.f-1.f,colorNm.g/255.f*2.f-1.f,colorNm.b/255.f*2.f-1.f), 0.f)).normalize() ;
			Vec3f r = ((n*2.*(n*l))-l).normalize() ;
			s = std::max(0.f,r*cam.normalize()) ;
			diff = std::max(0.f,n*l) ;
			spe = std::sqrt(colorSpec.r*colorSpec.r+colorSpec.g*colorSpec.g+colorSpec.b*colorSpec.b) ;
			color = colorDiffuse*(1.2*diff+.6*pow(s, spe+5));
			Vec4f p_shad = trans*embed<4>(Vec3f((float)y,x,z)) ;
			p_shad = p_shad/p_shad[3] ;
			shad = 1 ;
			if(p_shad[0] < 1000 && p_shad[0] >= 0 && p_shad[1] < 1000 && p_shad[1] >= 0)
				shad = .3+.7*(ombre_buff[(int)p_shad[0]][(int)p_shad[1]]<p_shad[2]+43.34) ;
		/*if(x == 500 && y == 570){
			line(image, x+1, y, 1000, Vec3f(1,1,1), Vec3f(480/1024,800/1024,0), x+(P1-P3).x*10, y+(P1-P3).y*10, 1000, Vec3f(1,1,1), Vec3f(512/1024,800/1024,0) ) ;
			line(image, x+1, y, 1000, Vec3f(1,1,1), Vec3f(480/1024,800/1024,0), x+(P2-P3).x*10, y+(P2-P3).y*10, 1000, Vec3f(1,1,1), Vec3f(512/1024,800/1024,0) ) ;
			line(image, x+1, y, 1000, n, Vec3f(480/1024,800/1024,0), x+cross(n,(P1-P3)).x*10, y+cross(n,(P1-P3)).y*10, 1000, n, Vec3f(512/1024,800/1024,0) ) ;
		}*/
		}
		if(pentu){
			if(ombre && x>0 && y>0 && x<(w-1) && y<(w-1) && ombre_buff[(int)y][x] < z){
				ombre_buff[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				if(mini > z) mini = z ;
			}
			if(!ombre && x>0 && y>0 && x<(w-1) && y<(w-1) && zbuffer[(int)y][x] < z){
				zbuffer[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				if(mini > z) mini = z ;
				image.set(y,x,color*shad) ;
			}
		}
	}
}

void triangle(TGAImage &image, Vec3i P1, Vec3f tex1, Vec3f n1, Vec3i P2, Vec3f tex2, Vec3f n2, Vec3i P3, Vec3f tex3, Vec3f n3){
	float y_h, y_b,z_h, z_b,  t1, t2 ;
	Vec3f n_h, n_b ;
	if (P2[0]<P1[0]){
		swap(P1,P2) ;
		swap(tex2,tex1) ;
		swap(n1,n2) ;
	}
	if (P3[0]<P1[0]){
		swap(P1, P3) ;
		swap(tex3,tex1) ;
		swap(n1,n3) ;
	}
	if (P3[0]<P2[0]){
		swap(P2, P3) ;
		swap(tex3,tex2) ;
		swap(n3,n2) ;
	}
	for(int x = P1[0]; x<P2[0]; x++){
		t1 = (P2[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P2[0]-P1[0]) ;
		t2 = (P3[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P3[0]-P1[0]) ;
		y_h = (1.-t1)*P1[1] + t1*P2[1] ;
		y_b = (1.-t2)*P1[1] + t2*P3[1] ;
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		z_h = (1.-t1)*P1[2] + t1*P2[2] ;
		z_b = (1.-t2)*P1[2] + t2*P3[2] ;
		z_h = (z_h-int(z_h) >= 0.5) ? ceil(z_h) : floor(z_h) ;
		z_b = (z_b-int(z_b) >= 0.5) ? ceil(z_b) : floor(z_b) ;
		n_h = n1*(1.-t1) + n2*t1 ;
		n_b = n1*(1.-t2) + n3*t2 ;

		line(image, x,y_h,z_h,n_h, tex1*(1-t1)+tex2*t1,x,y_b,z_b,n_b, tex1*(1-t2)+tex3*t2) ;
	}
	for(int x = P2[0]; x<=P3[0]; x++){
		t1 = (P3[0]==P2[0]) ? 1 : (x-P2[0])/(float)(P3[0]-P2[0]) ;
		t2 = (P3[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P3[0]-P1[0]) ;
		y_h = (1.-t1)*P2[1] + t1*P3[1] ;
		y_b = (1.-t2)*P1[1] + t2*P3[1] ;
		y_h = (y_h-int(y_h) >= 0.5) ? ceil(y_h) : floor(y_h) ;
		y_b = (y_b-int(y_b) >= 0.5) ? ceil(y_b) : floor(y_b) ;
		z_h = (1.-t1)*P2[2] + t1*P3[2] ;
		z_b = (1.-t2)*P1[2] + t2*P3[2] ;
		z_h = (z_h-int(z_h) >= 0.5) ? ceil(z_h) : floor(z_h) ;
		z_b = (z_b-int(z_b) >= 0.5) ? ceil(z_b) : floor(z_b) ;
		n_h = n2*(1.-t1) + n3*t1 ;
		n_b = n1*(1.-t2) + n3*t2 ;

		line(image, x,y_h,z_h,n_h, tex2*(1-t1)+tex3*t1, x,y_b,z_b,n_b, tex1*(1-t2)+tex3*t2) ;
	}
}

void transform() {
	Vec4f tmp ;
	for(int i = 0; i<(int)sommets_init.size(); i++){
		tmp = M*embed<4>(sommets_init[i]) ;
		sommets[i] = proj<3>(tmp)/tmp[3] ;
	}
	for(int i = 0; i<(int)normaux.size(); i++)
		normaux[i] = proj<3>(M_inv*embed<4>(normaux_init[i], 0.f)).normalize() ;
}

void createImage(){
	TGAImage im = TGAImage(w,w,3) ;
	for(int i = 0; i<w; i++){
		zbuffer[i] = new float[w] ;
		for(int j =0; j<w; j++) zbuffer[i][j] = -1000000 ;
	}
    Vec3f n1,n2,n3 ;
	for(int i = 0; i<facets.size();i++){
        P1 = Vec3f(sommets[facets[i][0][0]-1]) ;
		P2 = Vec3f(sommets[facets[i][1][0]-1]) ;
		P3 = Vec3f(sommets[facets[i][2][0]-1]) ;
		T1 = Vec3f(textures[facets[i][0][1]-1]) ;
        T2 = Vec3f(textures[facets[i][1][1]-1]) ;
        T3 = Vec3f(textures[facets[i][2][1]-1]) ;
        n1 = Vec3f(normaux[facets[i][0][2]-1]) ;
        n2 = Vec3f(normaux[facets[i][1][2]-1]) ;
        n3 = Vec3f(normaux[facets[i][2][2]-1]) ;
		triangle(im, P1, T1, n1, P2, T2, n2, P3, T3, n3) ;
	}
	im.flip_vertically() ;
	im.write_tga_file("shiny.tga") ;
}

int main(int argc, char** argv){
	string path = "obj/african_head.obj" ;
	readFile(path.c_str()) ;
	nm.read_tga_file((path.substr(0,path.length()-4)+"_nm.tga").c_str()) ;
	nm.flip_vertically() ;
	diffuse.read_tga_file((path.substr(0,path.length()-4)+"_diffuse.tga").c_str()) ;
	diffuse.flip_vertically() ;
	specular.read_tga_file((path.substr(0,path.length()-4)+"_spec.tga").c_str()) ;
	specular.flip_vertically() ;
	tangent.read_tga_file((path.substr(0,path.length()-4)+"_nm_tangent.tga").c_str()) ;
	tangent.flip_vertically() ;
	zbuffer = new float*[w] ;
	ombre_buff = new float*[w] ;
	for(int i = 0; i<w; i++){
		ombre_buff[i] = new float[w] ;
		for(int j =0; j<w; j++) ombre_buff[i][j] = -1000000 ;
	}
	viewport(0,0,w,w) ;
	lookat(lumiere, u, center) ;
	project(0) ;
	M_shad = M = view*projection*changementBase ;
	ombre = true ;
	transform() ;
	createImage() ;
	TGAImage buff(w, w, 1) ;
	for(int i = 0; i<w; i++){
		for(int j =0; j<w; j++){
			int couleur = (ombre_buff[i][j]+abs(mini))*255/(maxi+abs(mini)) ;
			buff.set(i,j,TGAColor(max(0,couleur),1)) ;
		}
	}
	buff.flip_vertically() ;
	buff.write_tga_file("buff.tga") ;
	ombre = false ;
	lookat(cam, u, center) ;
	project(-1.f/(cam-center).norm()) ;
	M = view*projection*changementBase ;
	trans = M_shad*M.invert() ;
	M_inv = (projection*changementBase).invert_transpose() ;
	lumiere = proj<3>(projection*changementBase*embed<4>(lumiere, 0.f)).normalize() ;
	transform() ;
	createImage() ;
	system("shiny.tga") ;
	return 0 ;
}