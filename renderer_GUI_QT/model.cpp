#include "model.h"

using namespace std ;

Model::Model(){}

Model::Model(string p) : path(p), zbuffer(NULL), maxi(0), w(1000) {
	readFile(path);
	diffuse.read_tga_file((path.substr(0,path.length()-4)+"_diffuse.tga").c_str()) ;
	diffuse.flip_vertically() ;
	nm.read_tga_file((path.substr(0,path.length()-4)+"_nm.tga").c_str()) ;
	nm.flip_vertically() ;
	spec.read_tga_file((path.substr(0,path.length()-4)+"_spec.tga").c_str()) ;
	spec.flip_vertically() ;
	image = TGAImage(w,w,3) ;
	tampon = TGAImage(w,w,1) ;
	wire = TGAImage(w,w,1) ;
	zbuffer = new float*[w] ;
	for(int i = 0; i<w; i++){
		zbuffer[i] = new float[w] ;
		for(int j =0; j<w; j++) zbuffer[i][j] = 0.f ;
	}
	ombreBuff = new float*[w] ;
	for(int i = 0; i<w; i++){
		ombreBuff[i] = new float[w] ;
		for(int j =0; j<w; j++) ombreBuff[i][j] = 0.f ;
	}
	lumiere = Vec3f(0,1.f,1.f) ;
	cam = Vec3f(0.f,0.f,1.f) ;
	center = Vec3f(0.f,0.f,0.f) ;
	viewport(w) ;
	lookat(lumiere, Vec3f(0.f,1.f,0.f), center) ;
	project(0.f) ;
	M = view*projection*changementBase ;
	M_inv = (projection*changementBase).invert_transpose() ;
	ombre = true ;
	transform() ;
	loadImage() ;
	ombre = false ;
	lookat(cam, Vec3f(0.f,1.f,0.f), center) ;
	MShad = M*(view*projection*changementBase).invert() ;
	M = view*projection*changementBase ;
	M_inv = (projection*changementBase).invert_transpose() ;
	transform() ;
	loadImage() ; // I love you <3
}

void Model::viewport(float taille) {
	view = Matrix::identity() ;
	view[0][3] = view[1][3] = view[2][3] = taille/2.f ;
	view[0][0] = view[1][1] = view[2][2] = taille/3.f ;
}

void Model::project(float coeff){
	projection = Matrix::identity() ;
	projection[3][2] = coeff ;
}

void Model::lookat(Vec3f eye, Vec3f up, Vec3f centre) {
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

void Model::transform() {
	for(int i = 0; i<(int)sommets.size(); i++)
		sommets[i] = proj<3>(M*embed<4>(sommets_init[i])) ;
	for(int i = 0; i<(int)normaux.size(); i++)
		normaux[i] = proj<3>(M_inv*embed<4>(normaux_init[i], 0.f)).normalize() ;
}

void Model::loadImage(){
	image.clear();
	tampon.clear();
	wire.clear();
    maxi = 0 ;
    for(int i = 0; i<w; i++)
        for(int j =0; j<w; j++) zbuffer[i][j] = 0 ;
    Vec3f P1,P2,P3,T1,T2,T3 ;
    for(int i = 0; i<(int)facets.size();i++){
        P1 = Vec3f(sommets[facets[i][0][0]-1]) ;
		P2 = Vec3f(sommets[facets[i][1][0]-1]) ;
		P3 = Vec3f(sommets[facets[i][2][0]-1]) ;
		T1 = Vec3f(textures[facets[i][0][1]-1]) ;
        T2 = Vec3f(textures[facets[i][1][1]-1]) ;
        T3 = Vec3f(textures[facets[i][2][1]-1]) ;
		// Dessin du fil de fer
		line(wire, P1.x,P1.y, P2.x,P2.y, TGAColor(255,1));
		line(wire, P2.x,P2.y, P3.x,P3.y, TGAColor(255,1));
		line(wire, P1.x,P1.y, P3.x,P3.y, TGAColor(255,1));
		// Dessin de l'image reelle
		triangleTexture(image, P1, T1, P2, T2, P3, T3) ;
    }
	// Dessin du tampon
	/*for(int i = 0; i<w; i++)
		for(int j =0; j<w; j++)
			tampon.set(i,j,TGAColor(zbuffer[i][j]*255/maxi,1)) ;*/
	// Retournement
	tampon.flip_vertically() ;
	image.flip_vertically() ;
	image.write_tga_file("debug.tga") ;
	wire.flip_vertically() ;
}

void Model::line(TGAImage &image, int x1, int y1, int x2, int y2, TGAColor color){
	float t, y;
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
		t = (x2==x1) ? 1 : (x-x1)/(float)(x2-x1) ;
		y = (1.-t)*y1 + t*y2 ;

		if(pentu)
			image.set(y,x,color) ;
		else
			image.set(x,y,color) ;
	}
}

void Model::lineDegrad(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2){
	float t, y ;
    int z ;
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
			if(x>0 && y>0 && x<(w-1) && y<(w-1) && zbuffer[(int)y][x] < z){
				zbuffer[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				image.set(y,x,color) ;
			}
		}else{
			if(x>0 && y>0 && x<(w-1) && y<(w-1) && zbuffer[x][(int)y] < z){
				zbuffer[x][(int)y] = z ;
				if(maxi < z) maxi = z ;
				image.set(x,y,color) ;
			}
		}
	}
}

void Model::lineTexture(TGAImage &image, int x1, int y1, int z1, Vec3f tex1, int x2, int y2, int z2, Vec3f tex2){
	float t, y, z ;
    int x_tex, y_tex ;
	bool pentu = false ;
	if((pentu = abs(y2-y1)>abs(x2-x1))){
		swap(x1,y1) ;
		swap(x2,y2) ;
	}
	if(x2<x1) {
		swap(x1,x2) ;
		swap(y1,y2) ;
		swap(z1,z2) ;
		swap(tex2,tex1) ;
	}
	for(int x=x1; x<=x2; x++){
		t = (x2==x1) ? 1 : (x-x1)/(float)(x2-x1) ;
		y = (1-t)*y1 + t*y2 ;
		z = (1-t)*z1 + t*z2 ;
		Vec3f p_shad = proj<3>(MShad*embed<4>(Vec3f((float)x,y,z))) ;
		x_tex = ((1-t)*tex1[0] + t*tex2[0])*diffuse.get_width() ;
		y_tex = ((1-t)*tex1[1] + t*tex2[1])*diffuse.get_height() ;
		TGAColor colorDiffuse = diffuse.get(x_tex, y_tex) ;
		TGAColor colorNm = nm.get(x_tex, y_tex) ;
		TGAColor colorSpec = spec.get(x_tex, y_tex) ;
		Vec3f n = proj<3>(M_inv*embed<4>(Vec3f(colorNm.r/255.f*2.f-1.f,colorNm.g/255.f*2.f-1.f,colorNm.b/255.f*2.f-1.f), 0.f)).normalize() ;
		Vec3f r = ((n*2.*(n*lumiere))-lumiere).normalize() ;
		Vec3f l = lumiere.normalize() ;
		float spe = std::max(0.f,r*cam.normalize()) ;
		float diff = std::max(0.f,n*l) ;
		float gna = std::sqrt(colorSpec.r*colorSpec.r+colorSpec.g*colorSpec.g+colorSpec.b*colorSpec.b) ;
		color = colorDiffuse*(1.2*diff+.6*pow(spe, gna+5));
		if(pentu){
			if(ombre && x>0 && y>0 && x<(w-1) && y<(w-1) && ombreBuff[(int)y][x] < z)
				ombreBuff[(int)y][x] = z ;
			if(x>0 && y>0 && x<(w-1) && y<(w-1) && zbuffer[(int)y][x] < z){
				zbuffer[(int)y][x] = z ;
				if(maxi < z) maxi = z ;
				//cout << "buff = " << ombreBuff[(int)p_shad[1]][(int)p_shad[0]] << "z_shad = " << p_shad[2] << endl ;
				float shad = .3+.7*(ombreBuff[(int)p_shad[1]][(int)p_shad[0]]<p_shad[2]+42) ;
				image.set(y,x,color*shad) ;
			}
		}else{
			if(ombre && x>0 && y>0 && x<(w-1) && y<(w-1) && ombreBuff[x][(int)y] < z)
				ombreBuff[x][(int)y] = z ;
			if(x>0 && y>0 && x<(w-1) && y<(w-1) && zbuffer[x][(int)y] < z){
				zbuffer[x][(int)y] = z ;
				if(maxi < z) maxi = z ;
				float shad = .3+.7*(ombreBuff[(int)p_shad[0]][(int)p_shad[1]]<p_shad[2]+42) ;
				image.set(x,y,color*shad) ;
			}
		}
	}
}

void Model::triangleDegrad(TGAImage &image, int x1, int y1, int z1, TGAColor color1, int x2, int y2, int z2, TGAColor color2, int x3, int y3, int z3, TGAColor color3){
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

		lineDegrad(image, x,y_h,z_h, TGAColor((1-t1)*color1.val+t1*color2.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
	for(int x = x2; x<=x3; x++){
		t1 = (x3==x2) ? 1 : (x-x2)/(float)(x3-x2) ;
		t2 = (x3==x1) ? 1 : (x-x1)/(float)(x3-x1) ;
		y_h = (1.-t1)*y2 + t1*y3 ;
		y_b = (1.-t2)*y1 + t2*y3 ;
		z_h = (1.-t1)*z2 + t1*z3 ;
		z_b = (1.-t2)*z1 + t2*z3 ;

		lineDegrad(image, x,y_h,z_h, TGAColor((1-t1)*color2.val+t1*color3.val, 1),x,y_b,z_b, TGAColor((1-t2)*color1.val+t2*color3.val, 1)) ;
	}
}

void Model::triangleTexture(TGAImage &image, Vec3i P1, Vec3f tex1, Vec3i P2, Vec3f tex2, Vec3i P3, Vec3f tex3){
	float y_h, y_b,z_h, z_b,  t1, t2 ;
	if (P2[0]<P1[0]){
		swap(P1,P2) ;
		swap(tex2,tex1) ;
	}
	if (P3[0]<P1[0]){
		swap(P1, P3) ;
		swap(tex3,tex1) ;
	}
	if (P3[0]<P2[0]){
		swap(P2, P3) ;
		swap(tex3,tex2) ;
	}
	for(int x = P1[0]; x<P2[0]; x++){
		t1 = (P2[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P2[0]-P1[0]) ;
		t2 = (P3[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P3[0]-P1[0]) ;
		y_h = (1.-t1)*P1[1] + t1*P2[1] ;
		y_b = (1.-t2)*P1[1] + t2*P3[1] ;
		z_h = (1.-t1)*P1[2] + t1*P2[2] ;
		z_b = (1.-t2)*P1[2] + t2*P3[2] ;

		lineTexture(image, x,y_h,z_h, tex1*(1-t1)+tex2*t1,x,y_b,z_b, tex1*(1-t2)+tex3*t2) ;
	}
	for(int x = P2[0]; x<=P3[0]; x++){
		t1 = (P3[0]==P2[0]) ? 1 : (x-P2[0])/(float)(P3[0]-P2[0]) ;
		t2 = (P3[0]==P1[0]) ? 1 : (x-P1[0])/(float)(P3[0]-P1[0]) ;
		y_h = (1.-t1)*P2[1] + t1*P3[1] ;
		y_b = (1.-t2)*P1[1] + t2*P3[1] ;
		z_h = (1.-t1)*P2[2] + t1*P3[2] ;
		z_b = (1.-t2)*P1[2] + t2*P3[2] ;

		lineTexture(image, x,y_h,z_h, tex2*(1-t1)+tex3*t1,x,y_b,z_b, tex1*(1-t2)+tex3*t2) ;
	}
}

TGAImage &Model::getImage() {
	return image ;
}

TGAImage &Model::getTampon() {
	return tampon ;
}

TGAImage &Model::getWire() {
	return wire ;
}

void Model::readFile(string path) {
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
