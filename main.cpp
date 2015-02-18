#include <stdlib.h>
#include "tgaimage.cpp"

using namespace std;

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

int main(int argc, char** argv){
	int w = 100 ;
	TGAImage im = TGAImage(w,w,1) ;
	TGAColor blanc = TGAColor(255, 255, 255, 3) ;
	triangle(im, 10, 10, 10, w-10, w-10, w/2, blanc) ;
	triangle(im, 20, 10, w-10, 10, w-10, w/2-10, blanc) ;
	triangle(im, 20, w-10, w-10, w-10, w-10, w/2+10, blanc) ;
	im.flip_vertically() ;
	im.write_tga_file("im.tga") ;
	return 0 ;
}