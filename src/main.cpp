#include <iostream>
#include <algorithm>

#include <windows.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static constexpr unsigned kernelSize = 7;

static float blurKernel[kernelSize * kernelSize] = {
	0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
	0.00f, 0.00f, 0.01f, 0.01f, 0.01f, 0.00f, 0.00f,
	0.00f, 0.01f, 0.05f, 0.10f, 0.05f, 0.01f, 0.00f,
	0.00f, 0.01f, 0.10f, 0.25f, 0.10f, 0.01f, 0.00f,
	0.00f, 0.01f, 0.05f, 0.10f, 0.05f, 0.01f, 0.00f,
	0.00f, 0.00f, 0.01f, 0.01f, 0.01f, 0.00f, 0.00f,
	0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f, 0.00f,
};

struct YUV
{
	unsigned char* Y = nullptr;
	unsigned char* U = nullptr;
	unsigned char* V = nullptr;
	int w;
	int h;

	YUV(int w, int h) : w(w), h(h) {
		Y = new unsigned char[w * h];
		U = new unsigned char[w * h];
		V = new unsigned char[w * h];
		for (int i{ 0 }, j{ 0 }; i < w * h * 4; i += 4, ++j) {
			Y[j] = 0;
			U[j] = 0;
			V[j] = 0;
		}
	}

	//RGBA to YUV
	YUV(unsigned char* image, int w, int h) : w(w), h(h) {
		Y = new unsigned char[w * h];
		U = new unsigned char[w * h];
		V = new unsigned char[w * h];
		for (int i{ 0 }, j{ 0 }; i < w * h * 4; i += 4, ++j) {
			Y[j] = (0.299 * image[i] + 0.587 * image[i + 1] + 0.114 * image[i + 2]);
			U[j] = (-0.169 * image[i] - 0.331 * image[i + 1] + 0.500 * image[i + 2]) + 128;
			V[j] = (0.500 * image[i] - 0.419 * image[i + 1] - 0.081 * image[i + 2]) + 128;
		}
	}

	YUV(const YUV & obj)  {
		*this = obj;
	}

	YUV & operator=(const YUV & obj) {
		Y = new unsigned char[w * h];
		U = new unsigned char[w * h];
		V = new unsigned char[w * h];
		w = obj.w;
		h = obj.h;
		std::memcpy(Y, obj.Y, w * h * sizeof(*Y));
		std::memcpy(U, obj.U, w * h * sizeof(*U));
		std::memcpy(V, obj.V, w * h * sizeof(*V));
		return *this;
	}

	~YUV() { delete[] Y, U, V; }
};

void printKernel(float* Y, unsigned w, unsigned h, unsigned target = 0) {
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

void printRgbaImage(unsigned char* Y, unsigned w, unsigned h) {
	std::cout << " RGBA image: " << std::endl;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w*4; j+=4) {
			unsigned ind = j + i * w*4;
			SetConsoleTextAttribute(hConsole, 12); if (Y[ind] < 100) { std::cout << " "; } if (Y[ind] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(Y[ind]) << " ";
			SetConsoleTextAttribute(hConsole, 2); if (Y[ind+1] < 100) { std::cout << " "; } if (Y[ind+1] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(Y[ind+1]) << " ";
			SetConsoleTextAttribute(hConsole, 9); if (Y[ind+2] < 100) { std::cout << " "; } if (Y[ind+2] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(Y[ind+2]) << " ";
			SetConsoleTextAttribute(hConsole, 15); if (Y[ind+3] < 100) { std::cout << " "; } if (Y[ind+3] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(Y[ind+3]) << " | ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void printYuvImage(const YUV & yuv, unsigned w, unsigned h) {
	std::cout << " YUV image: " << std::endl;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	for (int i = 0; i < h; ++i) {
		for (int j = 0; j < w; ++j) {
			unsigned ind = j + i * w;
			SetConsoleTextAttribute(hConsole, 12); if (yuv.Y[ind] < 100) { std::cout << " "; } if (yuv.Y[ind] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(yuv.Y[ind]) << " ";
			SetConsoleTextAttribute(hConsole, 2); if (yuv.U[ind] < 100) { std::cout << " "; } if (yuv.U[ind] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(yuv.U[ind]) << " ";
			SetConsoleTextAttribute(hConsole, 9); if (yuv.V[ind] < 100) { std::cout << " "; } if (yuv.V[ind] < 10) { std::cout << " "; }
			std::cout << static_cast<unsigned>(yuv.V[ind]) << " ";
			SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
			std::cout << "| ";
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
	SetConsoleTextAttribute(hConsole, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
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
			output.Y[index] = static_cast<unsigned>(totalY);
			output.U[index] = static_cast<unsigned>(totalU);
			output.V[index] = static_cast<unsigned>(totalV);
		}
	}
	return output;
}

void guassianBlur(const YUV& yuv) {
	//printYuvImage(yuv, w, h);

	//YUV yuv_output = yuv;
	YUV yuv_output = applyKernel(blurKernel, kernelSize, yuv);
	//for (int i = 0; i < 9; ++i) {
	//	yuv_output = applyKernel(blurKernel, kernelSize, yuv_output);
	//}

	unsigned char* out_bw = new unsigned char[yuv.w * yuv.h];
	for (int i = 0; i < yuv.w * yuv.h; ++i) {
		out_bw[i] = std::min<int>(std::floor(yuv_output.Y[i]), 255);
	}
	stbi_write_png("blured_bw.png", yuv.w, yuv.h, 1, out_bw, yuv.w);
	delete[] out_bw;

	unsigned char* out = new unsigned char[yuv.w * yuv.h * 4];
	for (int i{ 0 }, j{ 0 }; i < yuv.w * yuv.h * 4; i += 4, ++j) {
		out[i] = std::clamp<int>(std::floor(yuv_output.Y[j] + 1.403 * (yuv_output.V[j] - 128)), 0, 255);
		out[i + 1] = std::clamp<int>(std::floor(yuv_output.Y[j] - 0.344 * (yuv_output.U[j] - 128) - 0.714 * (yuv_output.V[j] - 128)), 0, 255);
		out[i + 2] = std::clamp<int>(std::floor(yuv_output.Y[j] + 1.770 * (yuv_output.U[j] - 128)), 0, 255);
		out[i + 3] = 255;
	}
	stbi_write_png("blured.png", yuv.w, yuv.h, 4, out, 4 * yuv.w);
	//printRgbaImage(out, w, h);
	delete[] out;
}

static inline int16_t us_shift_clamp_s15(int64_t v) {
	v >>= 14;
	if (v > 16383)
		v = 16383;
	else if (v < -16384)
		v = -16384;
	return v;
}

static void inline apply_kernel(unsigned char* dest, unsigned dest_stride, const unsigned char* src, unsigned src_stride,
	unsigned size) {

	if (size == 0)
		return;

	assert(size >= 4); // Head and tail have been unrolled

	int32_t kernel[] = { 0, 16384, 0, 0 };

	// First sample
	int64_t ds = 0x2000;
	for (int k = 0; k < 4; ++k)
		ds += kernel[3 - k] * src[src_stride * std::clamp(k - 2, 0, (int)size - 1)];

	//*dest = us_shift_clamp_s15(ds);
	dest += dest_stride;

	// Second pair
	int64_t ds0 = 0x2000, ds1 = 0x2000;
	for (int k = 0; k < 4; ++k) {
		const int32_t sample = src[src_stride * std::clamp(k - 1, 0, (int)size - 1)];
		ds0 += kernel[k] * sample;
		ds1 += kernel[3 - k] * sample;
	}
	*dest = us_shift_clamp_s15(ds0);
	dest += dest_stride;
	*dest = us_shift_clamp_s15(ds1);
	dest += dest_stride;

	// Pairs of output samples
	for (int s = 1; s < (int)(size - 2); ++s) {
		int64_t d0 = 0x2000, d1 = 0x2000;
		for (int k = 0; k < 4; ++k) {
			const int32_t sample = src[src_stride * (s + k - 1)];
			d0 += kernel[k] * sample;
			d1 += kernel[3 - k] * sample;
		}
		dest[0] = us_shift_clamp_s15(d0);
		dest[dest_stride] = us_shift_clamp_s15(d1);

		dest += 2 * dest_stride;
	}

	// Last pair
	int64_t de0 = 0x2000, de1 = 0x2000;
	for (int k = 0; k < 4; ++k) {
		const int32_t sample = src[src_stride * std::clamp(((int)size - 3) + k, 0, (int)size - 1)];
		de0 += kernel[k] * sample;
		de1 += kernel[3 - k] * sample;
	}
	*dest = us_shift_clamp_s15(de0);
	dest += dest_stride;
	*dest = us_shift_clamp_s15(de1);
	dest += dest_stride;

	// Last sample
	int64_t de = 0x2000;
	for (int k = 0; k < 4; ++k)
		de += kernel[k] * src[src_stride * std::clamp(((int)size - 2) + k, 0, (int)size - 1)];

	*dest = us_shift_clamp_s15(de);

}

void upscale(const YUV& yuv) {
	const unsigned width = yuv.w;
	const unsigned height = yuv.h;

	// Intermediate is w,2h
	YUV v_dest{ yuv.w, yuv.h * 2 };

	// Vertical scale
	for (unsigned x = 0; x < width; ++x) {
		apply_kernel(v_dest.Y + x, width, yuv.Y + x, width, height);
		apply_kernel(v_dest.U + x, width, yuv.U + x, width, height);
		apply_kernel(v_dest.V + x, width, yuv.V + x, width, height);
	}
	
	// Final is 2w,2h
	YUV h_dest{ yuv.w * 2, yuv.h * 2 };

	// Horizontal scale
	for (unsigned y = 0; y < height * 2; ++y) {
		apply_kernel(h_dest.Y + y * h_dest.w, 1, v_dest.Y + y * v_dest.w, 1, width);
		apply_kernel(h_dest.U + y * h_dest.w, 1, v_dest.U + y * v_dest.w, 1, width);
		apply_kernel(h_dest.V + y * h_dest.w, 1, v_dest.V + y * v_dest.w, 1, width);
	}

	unsigned char* out_bw = new unsigned char[h_dest.w * h_dest.h];
	for (int i = 0; i < h_dest.w * h_dest.h; ++i) {
		out_bw[i] = std::min<int>(std::floor(h_dest.Y[i]), 255);
	}
	stbi_write_png("blured_bw.png", h_dest.w, h_dest.h, 1, out_bw, h_dest.w);
	delete[] out_bw;

	unsigned char* out = new unsigned char[h_dest.w * h_dest.w * 4];
	for (int i{ 0 }, j{ 0 }; i < h_dest.w * h_dest.w * 4; i += 4, ++j) {
		out[i] = std::clamp<int>(std::floor(h_dest.Y[j] + 1.403 * (h_dest.V[j] - 128)), 0, 255);
		out[i + 1] = std::clamp<int>(std::floor(h_dest.Y[j] - 0.344 * (h_dest.U[j] - 128) - 0.714 * (h_dest.V[j] - 128)), 0, 255);
		out[i + 2] = std::clamp<int>(std::floor(h_dest.Y[j] + 1.770 * (h_dest.U[j] - 128)), 0, 255);
		out[i + 3] = 255;
	}
	stbi_write_png("blured.png", h_dest.w, h_dest.h, 4, out, 4 * h_dest.w);
	delete[] out;
}

int main() {
	int w;
	int h;
	int comp;
	std::string filename{"test8.png"};

	unsigned char* image = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
	
	if (image == nullptr)
		throw(std::string("Failed to load texture"));

	printKernel(blurKernel, kernelSize, kernelSize);
	//printRgbaImage(image, w, h);

	YUV yuv{ image, w, h };
	//guassianBlur(yuv);
	upscale(yuv);

	stbi_image_free(image);
	return 0;
}
