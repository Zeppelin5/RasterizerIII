//定义颜色
#ifndef __COLOR_H
#define __COLOR_H
#include "Math.h"

class Color{
public:
	double r;
	double b;
	double g;
	double a;

	Color(double r, double g, double b){
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 0;
	}

	Color(double r, double g, double b, double a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color(unsigned int color){
		//内存分布:
		//0xaarrggbb
		this->r = double((color & 0x00FF0000) >> 16) / 255;//提取出r值,归一化表示。FF右移16位为15*16+15=255，除255得1。
		this->g = double((color & 0x0000FF00) >> 8) / 255;//变化范围为00~FF，对应归一化为0~1
		this->b = double(color & 0x000000FF) / 255;
		this->a = double((color & 0xFF000000) >> 24) / 255;
	}

	Color(){}

	Color operator*(const double x){
		Color res;
		res.r *= x; res.g *= g; res.b *= b;
		return res;
	}

	Color operator+(const Color& c){
		Color res;
		res.r += c.r; res.g += c.g; res.b += c.b;
		res.a = 0;
		return res;
	}

	unsigned int Get32(){
		unsigned int res = ((unsigned int)(r * 255) << 16) + ((unsigned int)(g * 255) << 8) + ((unsigned int)(b * 255));
		return res;
	}
};






#endif