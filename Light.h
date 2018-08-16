//3.	定义方向光源类型，包括光线方向、光源颜色、光照强度等基本属性。
#ifndef __LIGHT_H
#define __LIGHT_H

#include "Geometry.h"
#include <algorithm>

//光源类型
const int LIGHTTYPE_AMBIENT = 0;
const int LIGHTTYPE_DIRECTION = 1;


class Light{//环境光
public:
	int id;
	int state;
	int attr;
	Vector3 posLight;//平行光不考虑光源位置
	Color color;

	//	virtual Color CalculateColor(const Vertex3D &vertex, const Material &material, const Point3D &objectPos)
	virtual Color getColor(Vertex3& ModelPoint, Material& material, Vector3& camPos){//Ambient Light Intensity
		//material.ka = (material.ka) > 1.0 ? 1.0 : (material.ka);//不能直接改
		double r = material.Kambient;
		double g = material.Kambient;
		double b = material.Kambient;

		r = r > 1.0 ? 1.0 : r;//最高为1,最亮时为白光
		g = g > 1.0 ? 1.0 : g;
		b = b > 1.0 ? 1.0 : b;
		return Color(color.r*r, color.g*g, color.b*b);
	}

	virtual Color getColorGeneral(Vertex3& ModelPoint, Material& material, Vector3& camPos){//Ambient Light Intensity
		//material.ka = (material.ka) > 1.0 ? 1.0 : (material.ka);//不能直接改
		double r = material.Kambient;
		double g = material.Kambient;
		double b = material.Kambient;

		r = r > 1.0 ? 1.0 : r;//最高为1,最亮时为白光
		g = g > 1.0 ? 1.0 : g;
		b = b > 1.0 ? 1.0 : b;
		return Color(color.r*r, color.g*g, color.b*b);
	}
};

class DirectionLight: public Light{
public:
	Vector3 dirLight;

	Color getColor(Vertex3& ModelPoint, Material& material, Vector3& camPos){
						//物体上的照射点			//该照射点的材质		//相机位置
		double rLight = 0, gLight = 0, bLight = 0;

		//**计算diffuse漫反射光强  Lambert模型 Kdiffuse*Lightcolor*dot(N·L),其中Lightcolor即(r,g,b)
		Vector3 L;//光源入射方向，等于平行光的方向角度
		L.x = -dirLight.x;
		L.y = -dirLight.y;
		L.z = -dirLight.z;
		ModelPoint.bitangent = Vector3::Vector3Cross(ModelPoint.normal, ModelPoint.tangent);
		L.x = Vector3::Vector3Dot(ModelPoint.tangent, L);
		L.y = Vector3::Vector3Dot(ModelPoint.bitangent, L);
		L.z = Vector3::Vector3Dot(ModelPoint.normal, L);//转换L到切空间中

		Vector3::Vector3Normalize(L);
		double cosTheta = Vector3::Vector3Dot(ModelPoint.normalMap, L);//a·b=|a||b|cosθ，当a、b为单位向量时，a·b=cosθ
		//材质没有法线信息时，cosTheTa总是为0，此时无漫反射

		//光衰减系数
		double attenuation = 1;
		
		if (cosTheta > 0){
			rLight += material.Kdiffuse*cosTheta / attenuation;
			gLight += material.Kdiffuse*cosTheta / attenuation;
			bLight += material.Kdiffuse*cosTheta / attenuation;
		}

		//**计算specular镜面反射光强  Phone模型 Kspecular*Lightcolor*dot(R·V)^shininess,其中Lightcolor即(r,g,b)
		//反射光线R:L-2*N*dot(N*L)
		//L = dirLight;
		Vector3 R;
		R.x = L.x - 2 * ModelPoint.normalMap.x*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.y = L.y - 2 * ModelPoint.normalMap.y*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.z = L.z - 2 * ModelPoint.normalMap.z*Vector3::Vector3Dot(ModelPoint.normalMap, L);
		R.w = 1;

		//视线V:相机点-照射点
		Vector3 V = camPos - ModelPoint.vertex;
		//转换到切空间计算
		V.x = Vector3::Vector3Dot(ModelPoint.tangent, V);
		V.y = Vector3::Vector3Dot(ModelPoint.bitangent, V);
		V.z = Vector3::Vector3Dot(ModelPoint.normal, V);

		Vector3::Vector3Normalize(R);
		Vector3::Vector3Normalize(V);

		//dot(R·V)^shiness大于0时进行计算
		double RVMulShiness = pow(Vector3::Vector3Dot(R, V), material.shininess);
		if (RVMulShiness > 0){
			rLight += material.Kspecular*RVMulShiness / attenuation;
			gLight += material.Kspecular*RVMulShiness / attenuation;
			bLight += material.Kspecular*RVMulShiness / attenuation;
		}
		rLight = rLight > 1.0 ? 1.0 : rLight;//最大为1.0
		gLight = gLight > 1.0 ? 1.0 : gLight;
		bLight = bLight > 1.0 ? 1.0 : bLight;

		//**最终颜色
		Color res;
		res.r = color.r*rLight;
		res.g = color.g*gLight;
		res.b = color.b*bLight;
		return res;
	}

	Color getColorGeneral(Vertex3& ModelPoint, Material& material, Vector3& camPos){
		//物体上的照射点			//该照射点的材质		//相机位置
		double rLight = 0, gLight = 0, bLight = 0;

		//**计算diffuse漫反射光强  Lambert模型 Kdiffuse*Lightcolor*dot(N·L),其中Lightcolor即(r,g,b)
		Vector3 L;//光源入射方向，等于平行光的方向角度
		L.x = -dirLight.x;
		L.y = -dirLight.y;
		L.z = -dirLight.z;
		Vector3::Vector3Normalize(L);
		double cosTheta = Vector3::Vector3Dot(ModelPoint.normal, L);//a·b=|a||b|cosθ，当a、b为单位向量时，a·b=cosθ
		//材质没有法线信息时，cosTheTa总是为0，此时无漫反射

		//光衰减系数
		double attenuation = 1;

		if (cosTheta > 0){
			rLight += material.Kdiffuse*cosTheta / attenuation;
			gLight += material.Kdiffuse*cosTheta / attenuation;
			bLight += material.Kdiffuse*cosTheta / attenuation;
		}

		//**计算specular镜面反射光强  Phone模型 Kspecular*Lightcolor*dot(R·V)^shininess,其中Lightcolor即(r,g,b)
		//反射光线R:L-2*N*dot(N*L)
		L = dirLight;
		Vector3 R;
		R.x = L.x - 2 * ModelPoint.normal.x*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.y = L.y - 2 * ModelPoint.normal.y*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.z = L.z - 2 * ModelPoint.normal.z*Vector3::Vector3Dot(ModelPoint.normal, L);
		R.w = 1;

		//视线V:相机点-照射点
		Vector3 V = camPos - ModelPoint.vertex;

		Vector3::Vector3Normalize(R);
		Vector3::Vector3Normalize(V);

		//dot(R·V)^shiness大于0时进行计算
		double test = Vector3::Vector3Dot(R, V);
		double RVMulShiness = pow(Vector3::Vector3Dot(R, V), material.shininess);
		if (RVMulShiness > 0){
			rLight += material.Kspecular*RVMulShiness / attenuation;
			gLight += material.Kspecular*RVMulShiness / attenuation;
			bLight += material.Kspecular*RVMulShiness / attenuation;
		}
		rLight = rLight > 1.0 ? 1.0 : rLight;//最大为1.0
		gLight = gLight > 1.0 ? 1.0 : gLight;
		bLight = bLight > 1.0 ? 1.0 : bLight;

		//**最终颜色
		Color res;
		res.r = color.r*rLight;
		res.g = color.g*gLight;
		res.b = color.b*bLight;
		return res;
	}
};











#endif