//2.	定义相机类型。定义相机的移动、旋转、投影等操作。
#ifndef __CAMERA_H
#define __CAMERA_H

#include "Math.h"

class Camera{//UVN坐标下的Camera
private:
	Vector3 posCamera;
	Matrix44 matCamera;

	Vector3 u;//右向量
	Vector3 v;//上向量
	Vector3 n;//注视向量

	double fov;//水平方向视域,角度
	double nearClip;//近裁减面
	double farClip;//远裁减面
	double viewDistance;//视距

	double aspect;//宽高比
	double width;//屏幕宽度
	double height;//屏幕高度

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

	void SetTarget(Vector3& target, Vector3& vv){//设置相机注视的目标
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

	void getCameraMat(){//获得相机矩阵
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

	//获取私有成员
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