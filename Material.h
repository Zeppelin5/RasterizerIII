//���ʸ�ʽ����
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
	double Kambient;//������ϵ��
	double Kdiffuse;//������ϵ��
	double Kspecular;//���淴��ϵ��
	double shininess;//�߹�ָ��
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