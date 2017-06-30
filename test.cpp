#include<stdio.h>
#include<stdlib.h>
#include<math.h> 
#include<iostream>
#include<string>
extern "C"
{
#include "image.h"
}

using namespace std;

typedef long INT32;
typedef unsigned short int INT16;
typedef unsigned char U_CHAR;
#define PI 3.1415926
#define SIZE 3
//--------------------------------------------------------------
#define	FACTOR	256
#define GAP 10
//--------------------------------------------------------------
#define UCH(x)	((int) (x))
#define GET_2B(array,offset)  ((INT16) UCH(array[offset]) + \
			       (((INT16) UCH(array[offset+1])) << 8))
#define GET_4B(array,offset)  ((INT32) UCH(array[offset]) + \
				   (((INT32) UCH(array[offset+1])) << 8) + \
			       (((INT32) UCH(array[offset+2])) << 16) + \
			       (((INT32) UCH(array[offset+3])) << 24))
#define FREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
int dataArray[SIZE][SIZE];
INT32 biWidth4;


int ReadDataSize(char *name);
//void ReadImageData(char *name, U_CHAR *bmpfileheader, U_CHAR *bmpinfoheader, U_CHAR *color_table, U_CHAR *data);
void SET_4B(U_CHAR *header, int num, int site);
void SET_2B(U_CHAR *header, int num, int site);
void ReadImageFheader(char *name, U_CHAR *bmpfileheader, INT32 *length);
void ReadImageIheader(char *name, U_CHAR *bmpinfoheader, INT32 *length);
void ReadImageBMdata(char *name, U_CHAR *bmpinfoheader, INT32 bfOffBits, int length);
void fisheye(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length);
void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N);
void aver_arg(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length, int time);
void aver(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length, int time, int a, int size_square);
void edge(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length);
void negative(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length);
void relief(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length);
//--------------------------------------------------------------
int absi(int x);
int getVal(int ed, int val, int con);
int effect(ImageData *img, ImageData *outimg, int act, int st, int sm);//鉛筆畫風
int effect_1(ImageData *img, ImageData *outimg, int ol, int gammaint);//沙畫風
//--------------------------------------------------------------
//#define SIZE	3

int main()
{
	errno_t err;
	FILE *output_file = 0;

	U_CHAR bmpfileheader[14] = { 0 };
	U_CHAR *bmpinfoheaderI;
	U_CHAR *data, *data_gray, color_table1[1024];
	U_CHAR color_table[1024];
	U_CHAR bmpinfoheaderO[40] = {
		40,0,0,0,//Info header size
		0,0,0,0,//biWidth    **not set
		0,0,0,0,//biHeight   **not set
		1,0,		//Planes
		8,0,		//Bits per pixel
		0,0,0,0,//Compression
		0,0,0,0,//Bitmap data size
		0,0,0,0,//H-Resolution
		0,0,0,0,//V-Resolution
		0,0,0,0,//Used Colors
		0,0,0,0 //Important Colors
	};

	INT32 InfoHL = 0;
	INT32 bfOffBits = 0;
	INT32 biWidth = 0;
	INT32 biHeight = 0;
	INT16 biCount = 0;

	int i, j, k;
	//--------------------------------------------------------------
	ImageData *img, *outimg;
	int res;
	int x, y, mx, my;
	//--------------------------------------------------------------

	for (int i = 0; i < 256; i++) {
		color_table[i * 4] = i;
		color_table[i * 4 + 1] = i;
		color_table[i * 4 + 2] = i;
		color_table[i * 4 + 3] = 0;
	}
	string filename_temp;
	printf("請輸入要修改的圖片：");
	getline(cin,filename_temp);
	char *filename = new char[filename_temp.length() + 1];
	strcpy(filename, filename_temp.c_str());

	i = ReadDataSize(filename);
	data = (U_CHAR *)malloc(i);
	if (data == NULL) {
		exit(0);
	}

	ReadImageFheader(filename, bmpfileheader, &InfoHL);
	bfOffBits = GET_4B(bmpfileheader, 10);
	bmpinfoheaderI = (U_CHAR *)malloc(InfoHL);
	for (int n = 0;n < InfoHL;n++) {
		bmpinfoheaderI[n] = 0;
	}
	ReadImageIheader(filename, bmpinfoheaderI, &InfoHL);
	ReadImageBMdata(filename, data, bfOffBits, i);

	biWidth = GET_4B(bmpinfoheaderI, 4);
	biHeight = GET_4B(bmpinfoheaderI, 8);
	biCount = GET_2B(bmpinfoheaderI, 14);

	SET_4B(bmpfileheader, biWidth * biHeight + 54, 2);
	SET_4B(bmpfileheader, 1078, 10);
	SET_4B(bmpinfoheaderO, biWidth, 4);
	SET_4B(bmpinfoheaderO, biHeight, 8);
	SET_4B(bmpinfoheaderO, i, 20);

	j = i / (biCount / 8);
	data_gray = (U_CHAR *)malloc(j);
	if (data_gray == NULL) {
		exit(0);
	}

	// Convert to 256-color
	biWidth4 = ((biWidth * 1 + 3) / 4 * 4);
	switch (biCount) {
		case 1:
			for (int n = 0;n < i;n++) {
				data_gray[n * 8 + 7] = data[n] % 2;
				data[n] -= data_gray[n * 8 + 7];
				for (int m = 0;m < 7;m++) {
					int temp = pow(2, 7 - m);
					data_gray[n * 8 + m] = data[n] / temp;
					data[n] -= data_gray[n * 8 + m] * temp;
				}
			}
			break;

		case 4:
			for (int n = 0;n < i;n++) {
				data_gray[n * 2 + 1] = data[n] % 16;
				data[n] -= data_gray[n * 2 + 1];
				data_gray[n * 2] = data[n] / 16;
			}
			break;

		case 8:
			for (int n = 0;n < i;n++) {
				data_gray[n] = data[n];
			}
			break;
		case 24:
			for (int n = 0;n < biHeight;n++) {
				for (int m = 0;m < biWidth;m++) {
					k = n*biWidth4 + m;
					data_gray[k] = (data[k * 3] + data[k * 3 + 1] + data[k * 3 + 2]) / 3;
				}
			}
			break;
	}
	//do function
	int input = 0;
	while (true) {
		printf("選擇修圖效果：\n");
		printf("選擇模糊      輸入1\n");
		printf("選擇邊緣      輸入2\n");
		printf("選擇負片      輸入3\n");
		printf("選擇魚眼      輸入4\n");
		printf("選擇浮雕      輸入5\n");
		printf("選擇鉛筆畫風  輸入6\n");
		printf("選擇插畫畫風  輸入7\n");
		printf("選擇結束      輸入0\n");


		std::cin >> input;

		if (input == 0)
			break;
		else if (input == 1)
			aver_arg(data_gray, biWidth, biHeight, j, 3);
		else if (input == 2)
			edge(data_gray, biWidth, biHeight, j);
		else if (input == 3)
			negative(data_gray, biWidth, biHeight, j);
		else if (input == 4)
			fisheye(data_gray, biWidth, biHeight, j);
		else if (input == 5)
			relief(data_gray, biWidth, biHeight, j);
		//--------------------------------------------------------------
		else if (input == 6)
		{ 
			res = readBMPfile(filename, &img);
			if (res<0) {
				printf("讀取錯誤逼波逼波\n");
				system("PAUSE");
			}
			outimg = createImage(img->width, img->height, 24);

			effect(img, outimg, 5, 1000, 1000);

			writeBMPfile("鉛筆畫風.bmp", outimg);
			disposeImage(img);
			disposeImage(outimg);
		}
		else if (input == 7)
		{
			res = readBMPfile(filename, &img);
			if (res<0) {
				printf("讀取錯誤逼波逼波\n");
				system("PAUSE");
			}

			outimg = createImage(img->width, img->height, 24);

			effect_1(img, outimg, 25, 50);

			writeBMPfile("插畫畫風.bmp", outimg);
			disposeImage(img);
			disposeImage(outimg);
		}
		//--------------------------------------------------------------
		printf("成功\n");

		/* 開啟新檔案 */
		if ((err = fopen_s(&output_file, "result.bmp", "wb")) != 0) {
			fprintf(stderr, "Output file can't open.\n");
			exit(0);
		}

		fwrite(bmpfileheader, sizeof(bmpfileheader), 1, output_file);
		fwrite(bmpinfoheaderO, sizeof(bmpinfoheaderO), 1, output_file);

		fwrite(color_table, 1024, 1, output_file);

		fwrite(data_gray, ((biWidth * 1 + 3) / 4 * 4)*biHeight, 1, output_file);

		fclose(output_file);
	}
	free(data);
	free(data_gray);

	return 0;
}
//--------------------------------------------------------------

int absi(int x)
{
	if (x<0) return -x;
	return x;
}

int getVal(int ed, int val, int con)
{
	int a, b;
	int v2;

	if (ed>255) ed = 255;
	v2 = (255 - 128)*val / 255 + 128;
	a = 255 - v2;
	b = -a*ed / 255 + 255;
	b = (b*con + val*(100 - con)) / 100;
	return b;
}

int effect(ImageData *img, ImageData *outimg, int act, int st, int sm) //鉛筆畫風
{
	int val;
	int x, y;
	int xx, yy;

	int f1, f2;

	int err, egg, ebb;
	int sr_x, sg_x, sb_x;
	int sr_y, sg_y, sb_y;
	int r1, g1, b1, dum1, dum2;
	int rr, gg, bb, ed;
	int ro, go, bo;
	int ff;
	int st2, con;
	int sum;
	int endn, endn3, endo;
	Pixel col, ncol;
	ImageData *buf;
	int smooth[9];
	int fil[9] = {
		-1,-1,-1,
		-1, 8,-1,
		-1,-1,-1 };
	int sobel1[9] = {
		1, 0,-1,
		2, 0,-2,
		1, 0,-1 };
	int sobel2[9] = {
		1, 2, 1,
		0, 0, 0,
		-1,-2,-1 };
	double	factor[7] = { 1.25, 1.5, 1.75, 2.0, 2.5, 3.0, 4.0 };
	int x1, y1, x2, y2;

	x1 = 0;
	y1 = 0;
	x2 = img->width - 1;
	y2 = img->height - 1;


	sum = 0;
	for (ff = 0; ff<9; ff++) {
		if (ff == 4) {
			smooth[ff] = 10;
			sum += 10;
		}
		else {
			smooth[ff] = sm;
			sum += sm;
		}
	}

	f1 = (int)((double)FACTOR * factor[act]);
	f2 = (int)((double)FACTOR * (factor[act] - 1.0) / 2.0);
	buf = createImage(img->width, img->height, 24);

	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			val = getPixel(img, x, y, &col);
			rr = col.r;
			gg = col.g;
			bb = col.b;
			ro = (rr*f1 - gg*f2 - bb*f2) / FACTOR;
			go = (-rr*f2 + gg*f1 - bb*f2) / FACTOR;
			bo = (-rr*f2 - gg*f2 + bb*f1) / FACTOR;
			col.r = ro;
			col.g = go;
			col.b = bo;
			setPixel(buf, x, y, &col);
		}
	}

	st2 = st / 2;
	con = 66;
	//銳化處理
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			err = egg = ebb = 0;
			sr_x = sg_x = sb_x = 0;
			sr_y = sg_y = sb_y = 0;
			ff = 0;
			for (yy = -1; yy <= 1; yy++) {
				for (xx = -1; xx <= 1; xx++) {
					//要求邊緣的對象是原影像
					val = getPixel(img, x + xx, y + yy, &col);
					//取得像素資訊

					//銳化濾波器
					err -= col.r*fil[ff];
					egg -= col.g*fil[ff];
					ebb -= col.b*fil[ff];

					//使用sobel filter 抽出邊緣
					sr_x += col.r*sobel1[ff];
					sg_x += col.g*sobel1[ff];
					sb_x += col.b*sobel1[ff];
					sr_y += col.r*sobel2[ff];
					sg_y += col.g*sobel2[ff];
					sb_y += col.b*sobel2[ff];

					ff++;
				}
			}

			val = getPixel(buf, x, y, &col);
			r1 = col.r;
			g1 = col.g;
			b1 = col.b;

			//強化邊緣部分
			err = err*st / 100 + (int)sqrt(sr_x*sr_x + sr_y*sr_y)*st2 / 100;
			egg = egg*st / 100 + (int)sqrt(sg_x*sg_x + sg_y*sg_y)*st2 / 100;
			ebb = ebb*st / 100 + (int)sqrt(sb_x*sb_x + sb_y*sb_y)*st2 / 100;
			rr = getVal(err, (int)col.r, con);
			gg = getVal(egg, (int)col.g, con);
			bb = getVal(ebb, (int)col.b, con);
			col.r = rr;
			col.g = gg;
			col.b = bb;
			setPixel(buf, x, y, &col);	// 
		}
	}

	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			err = egg = ebb = 0;
			ff = 0;
			for (yy = -1; yy <= 1; yy++) {
				for (xx = -1; xx <= 1; xx++) {
					val = getPixel(buf, x + xx, y + yy, &col);

					err += col.r*smooth[ff];
					egg += col.g*smooth[ff];
					ebb += col.b*smooth[ff];

					ff++;
				}
			}
			col.r = err / sum;
			col.g = egg / sum;
			col.b = ebb / sum;
			setPixel(outimg, x, y, &col);
		}
	}
	disposeImage(buf);
	return TRUE;
}



int effect_1(ImageData *img, ImageData *outimg, int ol, int gammaint)//沙畫風
{
	int val;
	int x, y;
	int xx, yy;
	int hh;
	int du, dd, dl, dr;
	int endn;
	int c1, c2;
	double gamma, a;
	int rrx, ggx, bbx;
	int rry, ggy, bby;
	int rrr, ggg, bbb;
	int rr, gg, bb, gray;
	int rate;
	int res1, res2, res;
	int th1, th2, th3;
	int r1, g1, b1;
	Pixel col, ncol;
	int *sobel;
	int sadr;
	int sobel1[9] = {
		1, 0,-1,
		2, 0,-2,
		1, 0,-1 };
	int sobel2[9] = {
		1, 2, 1,
		0, 0, 0,
		-1,-2,-1 };
	int x1, y1, x2, y2;

	x1 = 0;
	y1 = 0;
	x2 = img->width - 1;
	y2 = img->height - 1;

	gamma = (double)gammaint / 100.0;
	gamma = 1.0 / gamma;
	a = 128.0 / 255.0;
	th1 = (int)(pow(a, gamma)*255.0);
	a = 96.0 / 255.0;
	th2 = (int)(pow(a, gamma)*255.0);
	a = 64.0 / 255.0;
	th3 = (int)(pow(a, gamma)*255.0);
	for (y = y1; y <= y2; y++) {
		for (x = x1; x <= x2; x++) {
			rrx = ggx = bbx = 0;
			rry = ggy = bby = 0;
			sadr = 0;
			for (yy = 0; yy<3; yy++) {
				for (xx = 0; xx<3; xx++) {
					val = getPixel(img, x + xx - 1, y + yy - 1, &col);
					rr = col.r;
					gg = col.g;
					bb = col.b;
					rrx += rr*sobel1[sadr];
					rry += rr*sobel2[sadr];
					ggx += gg*sobel1[sadr];
					ggy += gg*sobel2[sadr];
					bbx += bb*sobel1[sadr];
					bby += bb*sobel2[sadr];
					sadr++;
				}
			}
			rrr = (int)(sqrt((double)(rrx*rrx + rry*rry)) / 8.0);
			ggg = (int)(sqrt((double)(ggx*ggx + ggy*ggy)) / 8.0);
			bbb = (int)(sqrt((double)(bbx*bbx + bby*bby)) / 8.0);
			if (rrr>ggg) rate = rrr;
			else rate = ggg;
			if (bbb>rate) rate = bbb;

			if (rate<ol) res1 = 255;
			else if (rate<ol + GAP) {
				res1 = 255 - 255 * (rate - ol) / GAP;
			}
			else res1 = 0;

			val = getPixel(img, x, y, &col);
			rr = col.r;
			gg = col.g;
			bb = col.b;
			gray = (bb * 28 + 77 * rr + gg * 151) / 256;

			if (gray>th1)     res2 = 255;
			else if (gray>th2) res2 = 128;
			else if (gray>th3) res2 = 64;
			else res2 = 0;

			res = res1*res2 / 256;
			col.r = res;
			col.g = res;
			col.b = res;
			setPixel(outimg, x, y, &col);
		}
	}
	return TRUE;
}

//--------------------------------------------------------------

void SET_4B(U_CHAR *header, int num, int site) {
	int temp = 256;
	header[site] = num % temp;
	num = num - header[site];
	for (int n = 1; n < 4;n++) {
		temp = pow(256.0, n);
		header[n + site] = num / temp;
		num -= header[n + site] * temp;
	}
}

void SET_2B(U_CHAR *header, int num, int site) {
	int temp = 256;
	header[site] = num % temp;
	header[site + 1] = num / temp;
}

void ReadImageFheader(char *name, U_CHAR *bmpfileheader, INT32 *length)
{
	FILE *input_file = 0;
	errno_t err;
	INT32 FileSize = 0;
	INT32 bfOffBits = 0;
	U_CHAR InfoLength[4] = { 0 };

	/* 開啟檔案 */
	if ((err = fopen_s(&input_file, name, "rb")) != 0) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	FREAD(input_file, bmpfileheader, 14);

	if (GET_2B(bmpfileheader, 0) == 0x4D42) /* 'BM' */
		fprintf(stdout, "BMP file.\n");
	else {
		fprintf(stdout, "Not bmp file.\n");
		exit(0);
	}

	FileSize = GET_4B(bmpfileheader, 2);
	bfOffBits = GET_4B(bmpfileheader, 10);

	printf("FileSize = %ld \n"
		"DataOffset = %ld \n"
		, FileSize, bfOffBits);

	FREAD(input_file, InfoLength, 4);
	*length = GET_4B(InfoLength, 0);

	fclose(input_file);
}

void ReadImageIheader(char *name, U_CHAR *bmpinfoheader, INT32 *length) {
	
	FILE *input_file = 0;
	errno_t err;	

	INT32 FileSize = 0;
	INT32 bfOffBits = 0;
	INT32 headerSize = 0;
	INT32 biWidth = 0;
	INT32 biHeight = 0;
	INT16 biPlanes = 0;
	INT16 BitCount = 0;
	INT32 biCompression = 0;
	INT32 biImageSize = 0;
	INT32 biXPelsPerMeter = 0, biYPelsPerMeter = 0;
	INT32 biClrUsed = 0;
	INT32 biClrImp = 0;

	/* 開啟檔案 */
	if ((err = fopen_s(&input_file, name, "rb")) != 0) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	fseek(input_file, 14, SEEK_SET);
	FREAD(input_file, bmpinfoheader, *length);

	headerSize = GET_4B(bmpinfoheader, 0);
	biWidth = GET_4B(bmpinfoheader, 4);
	biHeight = GET_4B(bmpinfoheader, 8);
	biPlanes = GET_2B(bmpinfoheader, 12);
	BitCount = GET_2B(bmpinfoheader, 14);
	biCompression = GET_4B(bmpinfoheader, 16);
	biImageSize = GET_4B(bmpinfoheader, 20);
	biXPelsPerMeter = GET_4B(bmpinfoheader, 24);
	biYPelsPerMeter = GET_4B(bmpinfoheader, 28);
	biClrUsed = GET_4B(bmpinfoheader, 32);
	biClrImp = GET_4B(bmpinfoheader, 36);

	/*printf(
		"HeaderSize = %ld \n"
		"Width = %ld \n"
		"Height = %ld \n"
		"Planes = %d \n"
		"BitCount = %d \n"
		"Compression = %ld \n"
		"ImageSize = %ld \n"
		"XpixelsPerM = %ld \n"
		"YpixelsPerM = %ld \n"
		"ColorsUsed = %ld \n"
		"ColorsImportant = %ld \n",
		headerSize, biWidth, biHeight, biPlanes, BitCount, biCompression,
		biImageSize, biXPelsPerMeter, biYPelsPerMeter, biClrUsed, biClrImp);*/

	fclose(input_file);
}

void ReadImageBMdata(char *name, U_CHAR *data, INT32 bfOffBits, int length) {
	
	FILE *input_file = 0;
	errno_t err;

	/* 開啟檔案 */
	if ((err = fopen_s(&input_file, name, "rb")) != 0) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	fseek(input_file, bfOffBits, SEEK_SET);
	FREAD(input_file, data, length);

	fclose(input_file);
}

int ReadDataSize(char *name)
{
	errno_t err;
	FILE *input_file = 0;
	U_CHAR bmpfileheader[14] = { 0 };
	U_CHAR bmpinfoheader[40] = { 0 };

	INT32 biWidth = 0;
	INT32 biHeight = 0;
	INT16 BitCount = 0;

	/* 開啟檔案 */
	if ((err = fopen_s(&input_file, name, "rb")) != 0) {
		fprintf(stderr, "File can't open.\n");
		exit(0);
	}

	FREAD(input_file, bmpfileheader, 14);
	FREAD(input_file, bmpinfoheader, 40);

	if (GET_2B(bmpfileheader, 0) == 0x4D42) /* 'BM' */
		fprintf(stdout, "BMP file.\n");
	else {
		fprintf(stdout, "Not bmp file.\n");
		exit(0);
	}

	biWidth = GET_4B(bmpinfoheader, 4);
	biHeight = GET_4B(bmpinfoheader, 8);
	BitCount = GET_2B(bmpinfoheader, 14);

	/*if (BitCount != 8) {
		fprintf(stderr, "Not a 8-bit file.\n");
		fclose(input_file);
		exit(0);
	}*/

	// 
	fclose(input_file);

	return ((biWidth * 1 + 3) / 4 * 4)*biHeight * 3;
}

void fisheye(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length) 
{
	INT32 biWh = biWidth / 2;
	INT32 biHh = biHeight / 2;
	INT32 biSide = sqrt(pow(biWh, 2) + pow(biHh, 2)) + 1;
	U_CHAR *data1;
	data1 = (U_CHAR *)malloc(length);
	for (int n = 0;n < length;n++) {
		data1[n] = 255;
	}

	int biWidth4 = ((biWidth * 1 + 3) / 4 * 4);
	for (int n = 0;n < biHeight;n++) {
		int new_n = n - biHh;
		for (int m = 0;m < biWidth;m++) {
			int k = n * biWidth4 + m;
			int new_m = m - biWh;

			if (new_m == 0 && new_n == 0)continue;

			float a = 1.008;
			float ncr = 0.01;
			float cr = biHeight * 0.5;
			float fr = sqrt(pow(new_n, 2) + pow(new_m, 2));
			float rf = -1 * (log(1 * (1 - pow(a, -biHeight)))) / (log(a));

			if (fr > cr) continue;

			int Difx = new_m;
			int Dify = new_n * -1;
			float RaidX = acos(Difx / fr);
			float RaidY = asin(Dify / fr);
			float r = 0;
			if (fr < cr * ncr) {
				r = fr*rf / (cr * ncr);
			}
			else {
				r = (log((cr * pow(a, cr)) / (cr * pow(a, cr) - fr * (pow(a, cr) - 1)))) / log(a);
			}

			int temp_m = (r * cos(RaidX)) + biWh;
			int temp_n = biHh - (r * sin(RaidY));


			int temp_k = temp_n * biWidth4 + temp_m;
			data1[k] = data[temp_k];
		}
	}
	for (int n = 0;n < length;n++) {
		data[n] = data1[n];
	}
	free(data1);
}

void correctedPos(int *new_i, int *new_j, int i, int j, int M, int N)
{
	*new_i = i;
	*new_j = j;
	if (i >= 0 && i < M && j >= 0 && j < N)
		return;

	if (i < 0)
		*new_i = 0;
	else if (i >= M)
		*new_i = M - 1;

	if (j < 0)
		*new_j = 0;
	else if (j >= N)
		*new_j = N - 1;
}

void aver_arg(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length, int time)
{
	for (int exec_time = 0; exec_time < time; exec_time++)
	{		
		// Process the file
		int a = (SIZE - 1) / 2;
		int size_square = SIZE*SIZE;

		aver(data, biWidth, biHeight, length, time, a, size_square);		
	}
}

void aver(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length, int time, int a ,int size_square)//需要由exec_aver呼叫
{
	int i, j,mi, i1, mj, j1, ni, nj, k, idx_col;
	U_CHAR *data1;
	data1 = (U_CHAR *)malloc(length);

	for (i = 0; i < biHeight; i++)
	{
		int sum = 0;
		// fill the dataArray
		mi = 0;
		for (i1 = i - a; i1 <= i + a; i1++)
		{
			mj = 0;
			for (j1 = 0 - a; j1 <= a; j1++)
			{
				correctedPos(&ni, &nj, i1, j1, biHeight, biWidth);
				dataArray[mi][mj] = data[ni*biWidth4 + nj];
				sum = sum + dataArray[mi][mj];
				mj++;
			}
			mi++;
		}
		k = i* biWidth4;
		data1[k] = sum / size_square;
		k = k + 1;
		//
		idx_col = 0;
		for (j = 1; j < biWidth; j++)
		{
			// update the dataArray
			// remove old data and add new data
			mi = 0;
			for (i1 = i - a; i1 <= i + a; i1++)
			{
				correctedPos(&ni, &nj, i1, j, biHeight, biWidth);
				sum = sum - dataArray[mi][idx_col];
				dataArray[mi][idx_col] = data[ni*biWidth4 + nj];
				sum = sum + dataArray[mi][idx_col];
				mi++;
			}
			data1[k] = sum / size_square;
			idx_col = (idx_col + 1) % SIZE;
			k = k + 1;
		}
	}

	for (int n = 0;n < length;n++) {
		data[n] = data1[n];
	}
	free(data1);
}

void edge(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length)
{
	// Process the file
	int a = (SIZE - 1) / 2;
	int size_square = SIZE*SIZE;
	U_CHAR *data1;
	data1 = (U_CHAR *)malloc(length);

	//處理
	//Sobel operator
	int mask_g1[SIZE][SIZE];
	mask_g1[0][0] = -1;
	mask_g1[0][1] = 0;
	mask_g1[0][2] = 1;
	mask_g1[1][0] = -2;
	mask_g1[1][1] = 0;
	mask_g1[1][2] = 2;
	mask_g1[2][0] = -1;
	mask_g1[2][1] = 0;
	mask_g1[2][2] = 1;

	int mask_g2[SIZE][SIZE];
	mask_g2[0][0] = -1;
	mask_g2[0][1] = -2;
	mask_g2[0][2] = -1;
	mask_g2[1][0] = 0;
	mask_g2[1][1] = 0;
	mask_g2[1][2] = 0;
	mask_g2[2][0] = 1;
	mask_g2[2][1] = 2;
	mask_g2[2][2] = 1;

	//
	int i, j, k, mi, i1, j1, ni, nj;
	for (i = 0; i < biHeight; i++)
	{

		k = i* biWidth4 + 0;
		for (j = 0; j < biWidth; j++)
		{
			int sum = 0;
			int sum2 = 0;
			//DATA = (OLD DATA)-C1+C3
			mi = 0;//所在的那一列
			for (i1 = 0 - a; i1 <= 0 + a; i1++)
			{
				for (j1 = 0 - a; j1 <= 0 + a; j1++)
				{
					correctedPos(&ni, &nj, i + i1, j + j1, biHeight, biWidth);
					sum = sum + data[ni*biWidth4 + nj] * mask_g1[i1 + a][j1 + a];
					sum2 = sum2 + data[ni*biWidth4 + nj] * mask_g2[i1 + a][j1 + a];
				}
			}
			sum = abs(sum + sum2);

			//problem a without scaling use this
			//just clipping the gray level 
			if (sum > 255)data1[k] = 255;
			if (sum < 0)data1[k] = 0;
			else data1[k] = sum;

			//data2[k] = (int)((float)(sum-f_min)/(float)(f_max-f_min)*255);
			k = k + 1;
		}
	}

	for (int n = 0;n < length;n++) {
		data[n] = data1[n];
	}
	free(data1);
}

void negative(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length)//負片
{
	// Process the file
	int a = (SIZE - 1) / 2;
	int size_square = SIZE*SIZE;
	//i = ReadDataSize("new2.bmp");
	U_CHAR *data1;
	data1 = (U_CHAR *)malloc(length);

	//
	int i, j, k;
	for (i = 0; i < biHeight; i++)
	{
		k = i* ((biWidth * 1 + 3) / 4 * 4);
		for (j = 0; j < biWidth; j++)
		{
			data1[k] = 255 - data[k];

			k = k + 1;
		}
	}

	for (int n = 0;n < length;n++) {
		data[n] = data1[n];
	}
	free(data1);
}

void relief(U_CHAR *data, INT32 biWidth, INT32 biHeight, int length) {
	U_CHAR *data1, *data2, *data3, *data4;
	int *temparr;
	int i, j, k, temp, temp2, temp3, min, max;
	//int histo_table[256] = { 0 };


	data1 = (U_CHAR *)malloc(length);
	for (int n = 0;n < length;n++) {
		data1[n] = data[n];
	}

	max = 0;
	min = 0;
	temp2 = 0;
	temp3 = 0;

	//
	//i = ReadDataSize("new2.bmp");
	data2 = (U_CHAR *)malloc(length);
	data3 = (U_CHAR *)malloc(length);
	data4 = (U_CHAR *)malloc(length);

	temparr = (int *)malloc(length * sizeof(int));
	biWidth4 = ((biWidth * 1 + 3) / 4) * 4;
	// Process the file
	for (i = 0; i < biHeight; i++)
	{
		k = i* ((biWidth * 1 + 3) / 4 * 4);
		for (j = 0; j < biWidth; j++)
		{
			if (i == 0 && j == 0)
			{
				temp = data1[k] * 2 - data1[k + 1] - data1[k + biWidth4];

			}
			else if (j == 0)
			{
				temp = data1[k] * 3 - data1[k + 1] - data1[k + biWidth4] - data1[k - biWidth4];

			}
			else if (i == biHeight - 1 && j == 0)
			{
				temp = data1[k] * 2 - data1[k + 1] - data1[k - biWidth4];

			}
			else if (j == biWidth - 1)
			{
				temp = data1[k] * 3 - data1[k - 1] - data1[k + biWidth4] - data1[k - biWidth4];

			}
			else if (i == 0 && j == biWidth - 1)
			{
				temp = data1[k] * 2 - data1[k + biWidth4] - data1[k - 1];

			}
			else if (i == 0)
			{
				temp = data1[k] * 3 - data1[k + biWidth4] - data1[k + 1] - data1[k - 1];

			}
			else if (i == biHeight - 1 && j == biWidth - 1)
			{
				temp = data1[k] * 2 - data1[k - 1] - data1[k - biWidth4];

			}
			else if (i == biHeight - 1)
			{
				temp = data1[k] * 3 - data1[k - biWidth4] - data1[k + 1] - data1[k - 1];

			}
			else
			{
				temp = (data1[k] * 4 - data1[k + 1] - data1[k - 1] - data1[k + biWidth4] - data1[k - biWidth4]);
			}
			if (max < temp)
				max = temp;
			if (min > temp)
				min = temp;
			temparr[k] = temp;

			k = k + 1;
		}
	}
	//printf("%d %d", max, min);
	//system("pause");

	for (i = 0; i < biHeight; i++)
	{
		k = i* ((biWidth * 1 + 3) / 4 * 4);
		for (j = 0; j < biWidth; j++)
		{
			temparr[k] = ((double)temparr[k] - min) / (max - min) * 255;//double 讓數字變正常 思考一下int存取上限 double讓他強制升格 數字變正常
			data2[k] = temparr[k];
			k = k + 1;
		}
	}

	biWidth4 = ((biWidth * 1 + 3) / 4) * 4;
	//執行銳化圖片
	for (i = 0; i < biHeight; i++)
	{
		k = i* ((biWidth * 1 + 3) / 4 * 4);
		for (j = 0; j < biWidth; j++)
		{
			if (i == 0 && j == 0)
			{
				temp3 = data2[k] * 2 - data2[k + 1] - data2[k + biWidth4];
				temp3 = temp3;
			}
			else if (j == 0)
			{
				temp3 = data2[k] * 3 - data2[k + 1] - data2[k + biWidth4] - data2[k - biWidth4];
				temp3 = temp3;
			}
			else if (i == biHeight - 1 && j == 0)
			{
				temp3 = data2[k] * 2 - data2[k + 1] - data2[k - biWidth4];
				temp3 = temp3;
			}
			else if (j == biWidth - 1)
			{
				temp3 = data2[k] * 3 - data2[k - 1] - data2[k + biWidth4] - data2[k - biWidth4];
				temp3 = temp3;
			}
			else if (i == 0 && j == biWidth - 1)
			{
				temp3 = data2[k] * 2 - data2[k + biWidth4] - data2[k - 1];
				temp3 = temp3;
			}
			else if (i == 0)
			{
				temp3 = data2[k] * 3 - data2[k + biWidth4] - data2[k + 1] - data2[k - 1];
				temp3 = temp3;
			}
			else if (i == biHeight - 1 && j == biWidth - 1)
			{
				temp3 = data2[k] * 2 - data2[k - 1] - data2[k - biWidth4];
				temp3 = temp3;
			}
			else if (i == biHeight - 1)
			{
				temp3 = data2[k] * 3 - data2[k - biWidth4] - data2[k + 1] - data2[k - 1];
				temp3 = temp3;
			}
			else
			{
				temp3 = (data2[k] * 4 - data2[k + 1] - data2[k - 1] - data2[k + biWidth4] - data2[k - biWidth4]);
				temp3 = temp3;
			}


			temp3 = (temp3 * 2) + data2[k];

			if (temp3 > 255)
				temp3 = 255;
			else if (temp3 < 0)
				temp3 = 0;
			data4[k] = temp3;
			k = k + 1;
		}
	}
	for (int n = 0;n < length;n++) {
		data[n] = data4[n];
	}
	free(data1);
	free(data2);
	free(data3);
	free(data4);
	free(temparr);
}

