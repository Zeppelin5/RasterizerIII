//3.	���巽���Դ���ͣ��������߷��򡢹�Դ��ɫ������ǿ�ȵȻ������ԡ�
#ifndef __LIGHT_H
#define __LIGHT_H

#include "Geometry.h"
#include <algorithm>

//��Դ����
const int LIGHTTYPE_AMBIENT = 0;
const int LIGHTTYPE_DIRECTION = 1;


class Light{//������
public:
	int id;
	int state;
	int attr;
	Vector3 posLight;//ƽ�йⲻ���ǹ�Դλ��
	Color color;

	//	virtual Color CalculateColor(const Vertex3D &vertex, const Material &material, const Point3D &objectPos)
	virtual Color getColor(Vertex3& ModelPoint, Material& material, Vector3& camPos){//Ambient Light Intensity
		//material.ka = (material.ka) > 1.0 ? 1.0 : (material.ka);//����ֱ�Ӹ�
		double r = material.Kambient;
		double g = material.Kambient;
		double b = material.Kambient;

		r = r > 1.0 ? 1.0 : r;//���Ϊ1,����ʱΪ�׹�
		g = g > 1.0 ? 1.0 : g;
		b = b > 1.0 ? 1.0 : b;
		return Color(color.r*r, color.g*g, color.b*b);
	}

	virtual Color getColorGeneral(Vertex3& ModelPoint, Material& material, Vector3& camPos){//Ambient Light Intensity
		//material.ka = (material.ka) > 1.0 ? 1.0 : (material.ka);//����ֱ�Ӹ�
		double r = material.Kambient;
		double g = material.Kambient;
		double b = material.Kambient;

		r = r > 1.0 ? 1.0 : r;//���Ϊ1,����ʱΪ�׹�
		g = g > 1.0 ? 1.0 : g;
		b = b > 1.0 ? 1.0 : b;
		return Color(color.r*r, color.g*g, color.b*b);
	}
};

class DirectionLight: public Light{
public:
	Vector3 dirLight;

	Color getColor(Vertex3& ModelPoint, Material& material, Vector3& camPos){
						//�����ϵ������			//�������Ĳ���		//���λ��
		double rLight = 0, gLight = 0, bLight = 0;

		//**����diffuse�������ǿ  Lambertģ�� Kdiffuse*Lightcolor*dot(N��L),����Lightcolor��(r,g,b)
		Vector3 L;//��Դ���䷽�򣬵���ƽ�й�ķ���Ƕ�
		L.x = -dirLight.x;
		L.y = -dirLight.y;
		L.z = -dirLight.z;
		ModelPoint.bitangent = Vector3::Vector3Cross(ModelPoint.normal, ModelPoint.tangent);
		L.x = Vector3::Vector3Dot(ModelPoint.tangent, L);
		L.y = Vector3::Vector3Dot(ModelPoint.bitangent, L);
		L.z = Vector3::Vector3Dot(ModelPoint.normal, L);//ת��L���пռ���

		Vector3::Vector3Normalize(L);
		double cosTheta = Vector3::Vector3Dot(ModelPoint.normalMap, L);//a��b=|a||b|cos�ȣ���a��bΪ��λ����ʱ��a��b=cos��
		//����û�з�����Ϣʱ��cosTheTa����Ϊ0����ʱ��������

		//��˥��ϵ��
		double attenuation = 1;
		
		if (cosTheta > 0){
			rLight += material.Kdiffuse*cosTheta / attenuation;
			gLight += material.Kdiffuse*cosTheta / attenuation;
			bLight += material.Kdiffuse*cosTheta / attenuation;
		}

		//**����specular���淴���ǿ  Phoneģ�� Kspecular*Lightcolor*dot(R��V)^shininess,����Lightcolor��(r,g,b)
		//�������R:L-2*N*dot(N*L)
		//L = dirLight;
		Vector3 R;
		R.x = L.x - 2 * ModelPoint.normalMap.x*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.y = L.y - 2 * ModelPoint.normalMap.y*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.z = L.z - 2 * ModelPoint.normalMap.z*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.w = 1;

		//����V:�����-�����
		Vector3 V = camPos - ModelPoint.vertex;
		//ת�����пռ����
		V.x = Vector3::Vector3Dot(ModelPoint.tangent, V);
		V.y = Vector3::Vector3Dot(ModelPoint.bitangent, V);
		V.z = Vector3::Vector3Dot(ModelPoint.normal, V);

		Vector3::Vector3Normalize(R);
		Vector3::Vector3Normalize(V);

		//dot(R��V)^shiness����0ʱ���м���
		double RVMulShiness = pow(Vector3::Vector3Dot(R, V), material.shininess);
		if (RVMulShiness > 0){
			rLight += material.Kspecular*RVMulShiness / attenuation;
			gLight += material.Kspecular*RVMulShiness / attenuation;
			bLight += material.Kspecular*RVMulShiness / attenuation;
		}
		rLight = rLight > 1.0 ? 1.0 : rLight;//���Ϊ1.0
		gLight = gLight > 1.0 ? 1.0 : gLight;
		bLight = bLight > 1.0 ? 1.0 : bLight;

		//**������ɫ
		Color res;
		res.r = color.r*rLight;
		res.g = color.g*gLight;
		res.b = color.b*bLight;
		return res;
	}

	Color getColorGeneral(Vertex3& ModelPoint, Material& material, Vector3& camPos){
		//�����ϵ������			//�������Ĳ���		//���λ��
		double rLight = 0, gLight = 0, bLight = 0;

		//**����diffuse�������ǿ  Lambertģ�� Kdiffuse*Lightcolor*dot(N��L),����Lightcolor��(r,g,b)
		Vector3 L;//��Դ���䷽�򣬵���ƽ�й�ķ���Ƕ�
		L.x = -dirLight.x;
		L.y = -dirLight.y;
		L.z = -dirLight.z;
		Vector3::Vector3Normalize(L);
		double cosTheta = Vector3::Vector3Dot(ModelPoint.normal, L);//a��b=|a||b|cos�ȣ���a��bΪ��λ����ʱ��a��b=cos��
		//����û�з�����Ϣʱ��cosTheTa����Ϊ0����ʱ��������

		//��˥��ϵ��
		double attenuation = 1;

		if (cosTheta > 0){
			rLight += material.Kdiffuse*cosTheta / attenuation;
			gLight += material.Kdiffuse*cosTheta / attenuation;
			bLight += material.Kdiffuse*cosTheta / attenuation;
		}

		//**����specular���淴���ǿ  Phoneģ�� Kspecular*Lightcolor*dot(R��V)^shininess,����Lightcolor��(r,g,b)
		//�������R:L-2*N*dot(N*L)
		L = dirLight;
		Vector3 R;
		R.x = L.x - 2 * ModelPoint.normal.x*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.y = L.y - 2 * ModelPoint.normal.y*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.z = L.z - 2 * ModelPoint.normal.z*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.w = 1;

		//����V:�����-�����
		Vector3 V = camPos - ModelPoint.vertex;

		Vector3::Vector3Normalize(R);
		Vector3::Vector3Normalize(V);

		//dot(R��V)^shiness����0ʱ���м���
		double test = Vector3::Vector3Dot(R, V);
		double RVMulShiness = pow(Vector3::Vector3Dot(R, V), material.shininess);
		if (RVMulShiness > 0){
			rLight += material.Kspecular*RVMulShiness / attenuation;
			gLight += material.Kspecular*RVMulShiness / attenuation;
			bLight += material.Kspecular*RVMulShiness / attenuation;
		}
		rLight = rLight > 1.0 ? 1.0 : rLight;//���Ϊ1.0
		gLight = gLight > 1.0 ? 1.0 : gLight;
		bLight = bLight > 1.0 ? 1.0 : bLight;

		//**������ɫ
		Color res;
		res.r = color.r*rLight;
		res.g = color.g*gLight;
		res.b = color.b*bLight;
		return res;
	}
};











#endif