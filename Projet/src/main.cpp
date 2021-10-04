#include "ImageBase.h"
#include <stdio.h>
#include <math.h>
#include <string>

#define AREARADIUS 16 //16
#define SEARCHRADIUS 32 //32
#define SEARCHSTEP 2 //4
#define STEPSIZE 20 //40

using namespace std;

typedef struct {
	int x = 0;
	int y = 0;
	int deviation = 999999999;
} Area;

int max (int a, int b) {
	if (a > b) return a;
	return b;
}

int min (int a, int b) {
	if (a < b) return a;
	return b;
}

int clampChar (int v) {
	return max(0, min(255, v));
}

int mix (int a, int b, float t) {
	return (b * t) + (a * (1 - t));
}

Area findClosestArea(ImageBase& Prev, ImageBase& Next, int px, int py, int r, int range) {
	Area ans;
	for (int x = px - range; x <= px + range; x+= SEARCHSTEP) {
		for (int y = py - range; y <= py + range; y+= SEARCHSTEP) {
			int d = 0;
			for(int i = -r; i <= r; i++) {
				for(int j = -r; j <= r; j++) {
					int xx = x + i;
					int yy = y + j;
					int pxx = px + i;
					int pyy = py + j;

					if (xx >= 0 
					&& xx < Next.getWidth() 
					&& yy >= 0 
					&& yy < Next.getHeight()
					&& pxx >= 0 
					&& pxx < Prev.getWidth()
					&& pyy >= 0 
					&& pyy < Prev.getHeight()) {
						d += abs(Next[yy * 3][xx * 3 + 0] - Prev[pyy * 3][pxx * 3 + 0]);
						d += abs(Next[yy * 3][xx * 3 + 1] - Prev[pyy * 3][pxx * 3 + 1]);
						d += abs(Next[yy * 3][xx * 3 + 2] - Prev[pyy * 3][pxx * 3 + 2]);
					} else {
						d += 50;
					}
				}
			}
			if (d < ans.deviation) {
				ans.deviation = d;
				ans.x = x;
				ans.y = y;
			}
		}
	}
	return ans;
}

void writeResult(ImageBase& outFrame, Area prevArea, Area nextArea) {
	int Rvalue = clampChar(nextArea.x - prevArea.x + 128); //0 -> -128 px, 255 -> 128px
	int Gvalue = clampChar(nextArea.y - prevArea.y + 128);
	int x = prevArea.x;
	int y = prevArea.y;
	outFrame[y*3][x*3+0] = Rvalue;
	outFrame[y*3][x*3+1] = Gvalue;
}

void interpolateResult(ImageBase&outFrame, int stepSize) {

	for (int x = stepSize; x < outFrame.getWidth(); x+=stepSize) {
		for (int y = stepSize; y < outFrame.getHeight(); y+=stepSize) {
			int Ar = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 0];
			int Br = outFrame[(y - stepSize) * 3]	[x* 3 					+ 0];
			int Cr = outFrame[y * 3]				[(x - stepSize) * 3 	+ 0];
			int Dr = outFrame[y * 3]				[x* 3 					+ 0];
			
			int Ag = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 1];
			int Bg = outFrame[(y - stepSize) * 3]	[x * 3 					+ 1];
			int Cg = outFrame[y * 3]				[(x - stepSize) * 3 	+ 1];
			int Dg = outFrame[y * 3]				[x * 3 					+ 1];
			for (int i = -stepSize; i <= 0; i++) {
				for (int j = -stepSize; j <= 0; j++) {
					int xx = x + i;
					int yy = y + j;

					int R1r = mix(Br, Ar, ((float)-i / (float)stepSize));
					int R2r = mix(Dr, Cr, ((float)-i / (float)stepSize));
					int Pr = mix(R2r, R1r, ((float)-j / (float)stepSize));

					outFrame[yy * 3][xx * 3 + 0] = Pr;

					int R1g = mix(Bg, Ag, ((float)-i / (float)stepSize));
					int R2g = mix(Dg, Cg, ((float)-i / (float)stepSize));
					int Pg = mix(R2g, R1g, ((float)-j / (float)stepSize));

					outFrame[yy * 3][xx * 3 + 1] = Pg;
				}
			}
		}
	}

	for (int x = stepSize; x < outFrame.getWidth(); x+=stepSize) {
		int y = outFrame.getHeight() - 1;
		int Ar = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 0];
		int Br = outFrame[(y - stepSize) * 3]	[x* 3 					+ 0];
		int Cr = outFrame[y * 3]				[(x - stepSize) * 3 	+ 0];
		int Dr = outFrame[y * 3]				[x* 3 					+ 0];
		
		int Ag = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 1];
		int Bg = outFrame[(y - stepSize) * 3]	[x * 3 					+ 1];
		int Cg = outFrame[y * 3]				[(x - stepSize) * 3 	+ 1];
		int Dg = outFrame[y * 3]				[x * 3 					+ 1];
		for (int i = -stepSize; i <= 0; i++) {
			for (int j = -stepSize; j <= 0; j++) {
				int xx = x + i;
				int yy = y + j;

				int R1r = mix(Br, Ar, ((float)-i / (float)stepSize));
				int R2r = mix(Dr, Cr, ((float)-i / (float)stepSize));
				int Pr = mix(R2r, R1r, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 0] = Pr;

				int R1g = mix(Bg, Ag, ((float)-i / (float)stepSize));
				int R2g = mix(Dg, Cg, ((float)-i / (float)stepSize));
				int Pg = mix(R2g, R1g, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 1] = Pg;
			}
		}
	}

	for (int y = stepSize; y < outFrame.getHeight(); y+=stepSize) {
		int x = outFrame.getWidth() - 1;
		int Ar = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 0];
		int Br = outFrame[(y - stepSize) * 3]	[x* 3 					+ 0];
		int Cr = outFrame[y * 3]				[(x - stepSize) * 3 	+ 0];
		int Dr = outFrame[y * 3]				[x* 3 					+ 0];
		
		int Ag = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 1];
		int Bg = outFrame[(y - stepSize) * 3]	[x * 3 					+ 1];
		int Cg = outFrame[y * 3]				[(x - stepSize) * 3 	+ 1];
		int Dg = outFrame[y * 3]				[x * 3 					+ 1];
		for (int i = -stepSize; i <= 0; i++) {
			for (int j = -stepSize; j <= 0; j++) {
				int xx = x + i;
				int yy = y + j;

				int R1r = mix(Br, Ar, ((float)-i / (float)stepSize));
				int R2r = mix(Dr, Cr, ((float)-i / (float)stepSize));
				int Pr = mix(R2r, R1r, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 0] = Pr;

				int R1g = mix(Bg, Ag, ((float)-i / (float)stepSize));
				int R2g = mix(Dg, Cg, ((float)-i / (float)stepSize));
				int Pg = mix(R2g, R1g, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 1] = Pg;
			}
		}
	}

	{
		int x = outFrame.getWidth() - 1;
		int y = outFrame.getHeight() - 1;
		int Ar = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 0];
		int Br = outFrame[(y - stepSize) * 3]	[x* 3 					+ 0];
		int Cr = outFrame[y * 3]				[(x - stepSize) * 3 	+ 0];
		int Dr = outFrame[y * 3]				[x* 3 					+ 0];
		
		int Ag = outFrame[(y - stepSize) * 3]	[(x - stepSize) * 3 	+ 1];
		int Bg = outFrame[(y - stepSize) * 3]	[x * 3 					+ 1];
		int Cg = outFrame[y * 3]				[(x - stepSize) * 3 	+ 1];
		int Dg = outFrame[y * 3]				[x * 3 					+ 1];
		for (int i = -stepSize; i <= 0; i++) {
			for (int j = -stepSize; j <= 0; j++) {
				int xx = x + i;
				int yy = y + j;

				int R1r = mix(Br, Ar, ((float)-i / (float)stepSize));
				int R2r = mix(Dr, Cr, ((float)-i / (float)stepSize));
				int Pr = mix(R2r, R1r, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 0] = Pr;

				int R1g = mix(Bg, Ag, ((float)-i / (float)stepSize));
				int R2g = mix(Dg, Cg, ((float)-i / (float)stepSize));
				int Pg = mix(R2g, R1g, ((float)-j / (float)stepSize));

				outFrame[yy * 3][xx * 3 + 1] = Pg;
			}
		}
	}
	

	int wGap = outFrame.getWidth() % stepSize;
	int hGap = outFrame.getHeight() % stepSize;

	for (int y = 0; y < outFrame.getWidth(); y+=stepSize) {
		int x = outFrame.getWidth() - 1 - wGap;


	}

}

ImageBase movementMap(ImageBase& prevFrame, ImageBase& nextFrame, int stepSize) {

	ImageBase* out = new ImageBase(prevFrame.getWidth(), prevFrame.getHeight(), true);
	ImageBase& outFrame = *out;

	//middle part
	for (int x = stepSize; x < outFrame.getWidth(); x+=stepSize) {
		for (int y = stepSize; y < outFrame.getHeight(); y+=stepSize) {
			Area prevArea, nextArea;
			prevArea.x = x;
			prevArea.y = y;

			// printf("nextArea\n", x, y);
			nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);

			// printf("writeResult\n", x, y);
			writeResult(outFrame, prevArea, nextArea);
		}
	}

	for (int x = 0; x < outFrame.getWidth(); x += stepSize) {
		int y = 0;

		Area prevArea, nextArea;
		prevArea.x = x;
		prevArea.y = y;
		nextArea.x = x;
		nextArea.y = y;

		//nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);

		// printf("writeResult\n", x, y);
		writeResult(outFrame, prevArea, nextArea);
	}

	for (int x = 0; x < outFrame.getWidth(); x += stepSize) {
		int y = outFrame.getHeight() - 1;

		Area prevArea, nextArea;
		prevArea.x = x;
		prevArea.y = y;
		nextArea.x = x;
		nextArea.y = y;

		//nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);

		// printf("writeResult\n", x, y);
		writeResult(outFrame, prevArea, nextArea);
	}

	for (int y = 0; y < outFrame.getHeight(); y += stepSize) {
		int x = 0;

		Area prevArea, nextArea;
		prevArea.x = x;
		prevArea.y = y;
		nextArea.x = x;
		nextArea.y = y;

		//nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);
		// printf("writeResult\n", x, y);
		writeResult(outFrame, prevArea, nextArea);
	}

	for (int y = 0; y < outFrame.getHeight(); y += stepSize) {
		int x = outFrame.getWidth() - 1;

		Area prevArea, nextArea;
		prevArea.x = x;
		prevArea.y = y;
		nextArea.x = x;
		nextArea.y = y;

		//nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);
		// printf("writeResult\n", x, y);
		writeResult(outFrame, prevArea, nextArea);
	}

	{
		int y = outFrame.getHeight() - 1;
		int x = outFrame.getWidth() - 1;

		Area prevArea, nextArea;
		prevArea.x = x;
		prevArea.y = y;
		nextArea.x = x;
		nextArea.y = y;

		//nextArea = findClosestArea(prevFrame, nextFrame, x, y, AREARADIUS, SEARCHRADIUS);
		// printf("writeResult\n", x, y);
		writeResult(outFrame, prevArea, nextArea);
	}

	interpolateResult(outFrame, stepSize);

	return outFrame;
}

ImageBase generateFrameAndBack (ImageBase& prevFrame, ImageBase& nextFrame, ImageBase& movementMapForward, ImageBase& movementMapBackward, float t) {
	ImageBase* out = new ImageBase(prevFrame.getWidth(), prevFrame.getHeight(), prevFrame.getColor());
	ImageBase& outFrame = *out;

	for (int x = 0; x < prevFrame.getWidth(); x++) {
		for (int y = 0; y < prevFrame.getHeight(); y++) {
			int pxMovement = (movementMapBackward[y * 3][x * 3 + 0] - 128) * (t);
			int pyMovement = (movementMapBackward[y * 3][x * 3 + 1] - 128) * (t);

			if (x - pxMovement >= 0 
			 && x - pxMovement < prevFrame.getWidth()
			 && y - pyMovement >= 0 
			 && y - pyMovement < prevFrame.getHeight()) {
				outFrame[y * 3][x * 3 + 0] += nextFrame[(y - pyMovement) * 3][(x - pxMovement) * 3 + 0] * (1 - t);
				outFrame[y * 3][x * 3 + 1] += nextFrame[(y - pyMovement) * 3][(x - pxMovement) * 3 + 1] * (1 - t);
				outFrame[y * 3][x * 3 + 2] += nextFrame[(y - pyMovement) * 3][(x - pxMovement) * 3 + 2] * (1 - t);
			}

			int nxMovement = (movementMapForward[y * 3][x * 3 + 0] - 128) * (1 - t);
			int nyMovement = (movementMapForward[y * 3][x * 3 + 1] - 128) * (1 - t);

			if (x - nxMovement >= 0 
			 && x - nxMovement < nextFrame.getWidth()
			 && y - nyMovement >= 0 
			 && y - nyMovement < nextFrame.getHeight()) {
				outFrame[y * 3][x * 3 + 0] += prevFrame[(y - nyMovement) * 3][(x - nxMovement) * 3 + 0] * (t);
				outFrame[y * 3][x * 3 + 1] += prevFrame[(y - nyMovement) * 3][(x - nxMovement) * 3 + 1] * (t);
				outFrame[y * 3][x * 3 + 2] += prevFrame[(y - nyMovement) * 3][(x - nxMovement) * 3 + 2] * (t);
			}
		}
	}

	return outFrame;
}


ImageBase generateFrame (ImageBase& nextFrame, ImageBase& movementMapForward, float t) {
	ImageBase* out = new ImageBase(nextFrame.getWidth(), nextFrame.getHeight(), nextFrame.getColor());
	ImageBase& outFrame = *out;

	for (int x = 0; x < nextFrame.getWidth(); x++) {
		for (int y = 0; y < nextFrame.getHeight(); y++) {

			int nxMovement = (movementMapForward[y * 3][x * 3 + 0] - 128) * (t);
			int nyMovement = (movementMapForward[y * 3][x * 3 + 1] - 128) * (t);

			if (x + nxMovement >= 0 
			 && x + nxMovement < nextFrame.getWidth()
			 && y + nyMovement >= 0 
			 && y + nyMovement < nextFrame.getHeight()) {
				outFrame[y * 3][x * 3 + 0] += nextFrame[(y + nxMovement) * 3][(x + nyMovement) * 3 + 0];
				outFrame[y * 3][x * 3 + 1] += nextFrame[(y + nxMovement) * 3][(x + nyMovement) * 3 + 1];
				outFrame[y * 3][x * 3 + 2] += nextFrame[(y + nxMovement) * 3][(x + nyMovement) * 3 + 2];			}
		}
	}

	return outFrame;
}

int main(int argc, char **argv)
{
	///////////////////////////////////////// Exemple d'un seuillage d'image
	char PrevFrameName[250], NextFrameName[250], InterpolatedFrameName[250], interpolatedNb[250], ctype[250];

	bool preprocessed = false;
	if (argc == 5) {
		sscanf (argv[1],"%s",PrevFrameName);
		sscanf (argv[2],"%s",NextFrameName);
		sscanf (argv[3],"%s",InterpolatedFrameName);
		sscanf (argv[4],"%s",interpolatedNb);
	} else if (argc == 6) {
		sscanf (argv[1],"%s",PrevFrameName);
		sscanf (argv[2],"%s",NextFrameName);
		sscanf (argv[3],"%s",InterpolatedFrameName);
		sscanf (argv[4],"%s",interpolatedNb);
		preprocessed = true;
	} else {
		return -1;
	}
	
	ImageBase prevFrame;
	prevFrame.load(PrevFrameName);
	//prevFrame.save("0.ppm");
	ImageBase nextFrame;
	nextFrame.load(NextFrameName);

	//nextFrame.save("50.ppm");
	ImageBase mapfwd = movementMap(prevFrame, nextFrame, STEPSIZE);
	mapfwd.save("mapfwd.ppm");
	ImageBase mapbwd = movementMap(nextFrame, prevFrame, STEPSIZE);
	mapbwd.save("mapbwd.ppm");

	int nb = atoi(interpolatedNb);
	for (int i = 0; i < nb; i++) {
		string name = to_string(i + 1) + "_" + InterpolatedFrameName;
		float t = (float)(i + 1) / (float)(nb + 1);
		ImageBase interpolatedFrame1 = generateFrameAndBack(prevFrame, nextFrame, mapfwd, mapbwd, 1 - t);	
		interpolatedFrame1.save((char *)name.c_str());
	}
	printf(".\n");

	return 0;
}
