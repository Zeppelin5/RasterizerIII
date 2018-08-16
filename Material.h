//材质格式设置
#ifndef __MATERIAL_H
#define __MATERIAL_H

#include "Math.h"
#include "Color.h"

class Material{
public:
	int id;
	int state;
	int attr;
	char name[64];
	double Kambient;//环境光系数
	double Kdiffuse;//漫反射系数
	double Kspecular;//镜面反射系数
	double shininess;//高光指数
	double width;
	double height;
	unsigned int *texBuffer;
	unsigned int *norBuffer;
	Color color;
	Color colorAmb;
	Color colorDif;
	Color colorSpe;

	Material(unsigned int *tB, double w, double h){
		this->texBuffer = tB;
		this->width = w;
		this->height = h;
	}
};









#endif