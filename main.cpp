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

int main(int argc, char** argv){
	int w = 100 ;
	TGAImage im = TGAImage(w,w,1) ;
	line(im, 10, 10, 10, w-10, TGAColor(255,1)) ;
	line(im, w-10, w-10, w-10, 10, TGAColor(255,1)) ;
	line(im, 10, 10, w-10, 10, TGAColor(255,1)) ;
	line(im, 10, w-10, w-10, w-10, TGAColor(255,1)) ;
	line(im, 10, 10, w/2, w-10, TGAColor(255,1)) ;
	line(im, 10, 10, w-10, w/2, TGAColor(255,1)) ;
	line(im, w-10, 10, 10, w-10, TGAColor(255,1)) ;
	line(im, w-10, w-10, 10, 10, TGAColor(255,1)) ;
	im.flip_vertically() ;
	im.write_tga_file("im.tga") ;
	return 0 ;
}