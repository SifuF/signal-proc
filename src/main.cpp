#include <iostream>

#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static constexpr unsigned kernelSize = 7;

static float blurKernel[kernelSize * kernelSize] = {
	0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
	0.00f, 0.00f, 0.01f, 0.01f, 0.01f, 0.00f, 0.00f,
	0.00f, 0.01f, 0.05f, 0.11f, 0.05f, 0.01f, 0.00f,
	0.00f, 0.01f, 0.11f, 0.25f, 0.11f, 0.01f, 0.00f,
	0.00f, 0.01f, 0.05f, 0.11f, 0.05f, 0.01f, 0.00f,
	0.00f, 0.00f, 0.01f, 0.01f, 0.01f, 0.00f, 0.00f,
	0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
};

struct YUV
{
	float * Y = nullptr;
	float * U = nullptr;
	float * V = nullptr;
	int w;
	int h;

	//RGBA to YUV
	YUV(unsigned char* image, int w, int h) : w(w), h(h) {
		Y = new float[w * h];
		U = new float[w * h];
		V = new float[w * h];
		for (int i{ 0 }, j{ 0 }; i < w * h * 4; i += 4, ++j) {
			Y[j] = (0.299 * image[i] + 0.587 * image[i + 1] + 0.114 * image[i + 2]);
			U[j] = (-0.169 * image[i] - 0.331 * image[i + 1] + 0.500 * image[i + 2]);
			V[j] = (0.500 * image[i] - 0.419 * image[i + 1] - 0.081 * image[i + 2]);
		}
	}

	YUV(const YUV & obj)  {
		*this = obj;
	}

	YUV & operator=(const YUV & obj) {
		Y = new float[w * h];
		U = new float[w * h];
		V = new float[w * h];
		w = obj.w;
		h = obj.h;
		std::memcpy(Y, obj.Y, w * h * sizeof(*Y));
		std::memcpy(U, obj.U, w * h * sizeof(*U));
		std::memcpy(V, obj.V, w * h * sizeof(*V));
		return *this;
	}

	~YUV() { delete[] Y, U, V; }
};

void printGreyscaleImage(float* Y, unsigned w, unsigned h, unsigned target = 0) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			unsigned ind = j + i * w;
			if (ind == target) {
				SetConsoleTextAttribute(hConsole, 2);
			}
			else {
				SetConsoleTextAttribute(hConsole, 15);
			}
			std::cout << Y[ind] << "|";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

YUV applyKernel(const float kernel[], unsigned kernelSize, const YUV & image) {
	YUV output{ image };
	for (unsigned y = 0; y <= image.h - kernelSize; ++y) {
		for (unsigned x = 0; x <= image.w - kernelSize; ++x) {
			float totalY = 0.0f;
			float totalU = 0.0f;
			float totalV = 0.0f;
			for (int i = 0; i < kernelSize; ++i) {
				for (int j = 0; j < kernelSize; ++j) {
					totalY += blurKernel[j + kernelSize * i] * image.Y[(x + j) + image.w * (y + i)];
					totalU += blurKernel[j + kernelSize * i] * image.U[(x + j) + image.w * (y + i)];
					totalV += blurKernel[j + kernelSize * i] * image.V[(x + j) + image.w * (y + i)];
				}
			}
			unsigned index = (x + kernelSize / 2) + image.w * (y + kernelSize / 2);
			output.Y[index] = totalY;
			output.U[index] = totalU;
			output.V[index] = totalV;
		}
	}
	return output;
}

int main() {
	int w;
	int h;
	int comp;
	std::string filename{"test2.png"};

	unsigned char* image = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
	
	if (image == nullptr)
		throw(std::string("Failed to load texture"));

	printGreyscaleImage(blurKernel, kernelSize, kernelSize);

	YUV yuv{ image, w, h };
	//printGreyscaleImage(yuv.Y, w, h);
	YUV yuv_output = applyKernel(blurKernel, kernelSize, yuv);
	for (int i = 0; i < 9; ++i) {
		yuv_output = applyKernel(blurKernel, kernelSize, yuv_output);
	}
	//printGreyscaleImage(yuv_output.Y, w, h);

	unsigned char * out_bw = new unsigned char[w * h];
	for (int i = 0; i < w * h; ++i) {
		out_bw[i] = std::min<int>(std::floor(yuv_output.Y[i]), 255);
	}
	stbi_write_png("blured_bw.png", w, h, 1, out_bw, w);

	unsigned char * out = new unsigned char[w * h * 4];
	for (int i{ 0 }, j{0}; i < w * h * 4; i += 4, ++j) {
		out[i] = yuv_output.Y[j] + 1.403 * yuv_output.V[j];
		out[i + 1] = yuv_output.Y[j] - 0.344 * yuv_output.U[j] - 0.714 * yuv_output.V[j];
		out[i + 2] = yuv_output.Y[j] + 1.770 * yuv_output.U[j];
		out[i + 3] = 255;
	}
    stbi_write_png("blured.png", w, h, 4, out, 4*w);

	stbi_image_free(image);
	return 0;
}