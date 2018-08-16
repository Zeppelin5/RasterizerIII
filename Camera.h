//2.	����������͡�����������ƶ�����ת��ͶӰ�Ȳ�����
#ifndef __CAMERA_H
#define __CAMERA_H

#include "Math.h"

class Camera{//UVN�����µ�Camera
private:
	Vector3 posCamera;
	Matrix44 matCamera;

	Vector3 u;//������
	Vector3 v;//������
	Vector3 n;//ע������

	double fov;//ˮƽ��������,�Ƕ�
	double nearClip;//���ü���
	double farClip;//Զ�ü���
	double viewDistance;//�Ӿ�

	double aspect;//��߱�
	double width;//��Ļ���
	double height;//��Ļ�߶�

public:
	Camera(Vector3 posCamera, Vector3 target, Vector3 v, double nearClip, double farClip,
		double fov, double viewDistance, double aspect) :
		posCamera(posCamera),
		u(u),
		v(v),
		nearClip(nearClip),
		farClip(farClip),
		fov(fov),
		viewDistance(viewDistance),
		aspect(aspect){
		SetTarget(target, v);
		width = (double)(viewDistance*tan(fov*PI / 360.0f) * 2);
		height =  width / aspect;
	}

	~Camera(){
	}

	void SetTarget(Vector3& target, Vector3& vv){//�������ע�ӵ�Ŀ��
		//v = vv;
		//n = target - posCamera;
		//u = Vector3::Vector3Cross(n, v);
		//Vector3::Vector3Normalize(u);
		//Vector3::Vector3Normalize(v);
		//Vector3::Vector3Normalize(n);
		//getCameraMat();

		v = vv;
		n = target - posCamera;
		u = Vector3::Vector3Cross(v, n);
		v = Vector3::Vector3Cross(n, u);
		Vector3::Vector3Normalize(u);
		Vector3::Vector3Normalize(v);
		Vector3::Vector3Normalize(n);
		getCameraMat();
	}

	void getCameraMat(){//����������
		matCamera = {
			u.x, v.x, n.x, 0,
			u.y, v.y, n.y, 0,
			u.z, v.z, n.z, 0,
			-posCamera.x, -posCamera.y, -posCamera.z, 1 };
	}

	void RotateHorizontal(double angel){
		Matrix44 m;
		Matrix44::Matrix44Rotate(m, v.x, v.y, v.z, angel);
		n = Vector3MulMatirx44(n, m);
		u = Vector3::Vector3Cross(v, n);
		v = Vector3::Vector3Cross(n, u);

		Vector3::Vector3Normalize(u);
		Vector3::Vector3Normalize(v);
		Vector3::Vector3Normalize(n);

		getCameraMat();
	}

	void SetPosCamera(double xx, double yy, double zz){
		this->posCamera.x = xx;
		this->posCamera.y = yy;
		this->posCamera.z = zz;
		getCameraMat();
	}

	Vector3 GetPosCamera(){
		return posCamera;
	}

	double GetAspect(){
		return aspect;
	}

	double GetNear(){
		return nearClip;
	}

	double GetFar(){
		return farClip;
	}

	Vector3 WorldToCamera(Vector3& posWorld){
		Vector3 res;
		res= Vector3MulMatirx44(posWorld, matCamera);
		return res;
	}

	//��ȡ˽�г�Ա
	double GetViewDistance(){
		return this->viewDistance;
	}
	double GetFov(){ 
		return this->fov; 
	}
	double GetCameraWidth(){ 
		return this->width; 
	}
	double GetCameraHeight(){
		return this->height; 
	}
};






#endif