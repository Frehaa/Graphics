#include "tgaimage.h"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);

int main(void) {
	TGAImage image(100, 100, TGAImage::RGB);
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 50; j++) {
			image.set(i, j, red);
		}
	}
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 50; j++) {
			image.set(i, 50+j, white);
		}
	}

	image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
	image.write_tga_file("output.tga");
	return 0;
}

