//1.	定义基本的3D数据类型，如向量、矩阵。定义其基本操作，如向量加法、向量减法、矩阵运算等
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;
#ifndef __MATH_H
#define __MATH_H

#define PI 3.14159265

double Interpolate(double x, double y, double t){//双线性插值
	return x + (y - x)*t;
}

class Matrix44;

class Vector3{
public:
	double x, y, z, w;

	Vector3 operator-(const Vector3& v){
		this->x = this->x - v.x;
		this->y = this->y - v.y;
		this->z = this->z - v.z;
		return *this;
	}

	Vector3 operator+(const Vector3& v){
		this->x = this->x + v.x;
		this->y = this->y + v.y;
		this->z = this->z + v.z;
		return *this;
	}

	Vector3 operator/(double v){
		this->x = this->x / v;
		this->y = this->y / v;
		this->z = this->z / v;
		return *this;
	}

	Vector3 operator*(double v){
		this->x = this->x*v;
		this->y = this->y*v;
		this->z = this->z*v;
		return *this;
	}

	bool operator==(Vector3 v){
		if (this->x == v.x && this->y == v.y && this->z == v.z && this->w == v.w)
			return true;
		else
			return false;
	}

	bool operator!=(Vector3 v){
		if (this->x != v.x || this->y != v.y || this->z != v.z || this->w != v.w)
			return true;
		else
			return false;
	}

	//Vector3& operator=(Vector3& other){
	//	if (this == &other)
	//		return *this;
	//	this->x = other.x;
	//	this->y = other.y;
	//	this->z = other.z;
	//	this->w = other.w;
	//	return *this;
	//}
	//
	//Vector3& operator=({ double x, double y, double z, double w }){
	//	this->x = x;
	//	this->y = y;
	//	this->z = z;
	//	this->w = w;
	//	return *this;
	//}

	static double Vector3Dot(Vector3& v1, Vector3& v2){//向量点乘
		double res = v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
		return res;
	}

	static Vector3 Vector3Cross(Vector3& v1, Vector3& v2){//向量叉乘
		Vector3 res;
		res.x = v1.y*v2.z - v1.z*v2.y;
		res.y = v1.z*v2.x - v1.x*v2.z;
		res.z = v1.x*v2.y - v1.y*v2.x;
		return res;
	}

	static Vector3 Vector3Interpolate(Vector3 &v1, Vector3& v2, double t){
		Vector3 res;
		res.x = Interpolate(v1.x, v2.x, t);
		res.y = Interpolate(v1.y, v2.y, t);
		res.z = Interpolate(v1.z, v2.z, t);
		res.w = 1.0f;
		return res;
	}

	static void Vector3Normalize(Vector3& v){
		//Vector3 res=(0,0,0);
		double denominator = sqrt(v.x*v.x + v.y*v.y + v.z*v.z);//分母
		if (denominator != 0){
			v.x /= denominator;
			v.y /= denominator;
			v.z /= denominator;
		}
	}

	static double Vector3Length(Vector3 v){
		return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	}
};

class Vector2{
public:
	double x, y;

	Vector2 operator-(const Vector2& v){
		this->x = this->x - v.x;
		this->y = this->y - v.y;
		return *this;
	}

	Vector2 operator+(const Vector2& v){
		this->x = this->x + v.x;
		this->y = this->y + v.y;
		return *this;
	}

	Vector2 operator/(double v){
		this->x = this->x / v;
		this->y = this->y / v;
		return *this;
	}

	Vector2 operator*(double v){
		this->x = this->x*v;
		this->y = this->y*v;
		return *this;
	}

	Vector2& operator=(const Vector2& other){
		if (this == &other)
			return *this;
		this->x = other.x;
		this->y = other.y;
		return *this;
	}

	static double Vector2Dot(Vector2& v1, Vector2& v2){
		double res = v1.x*v2.x + v1.y*v2.y;
		return res;
	}
};

class Matrix44{
public:
	double c[4][4];

	static Matrix44 Matrix44Add(Matrix44& m1, Matrix44& m2){//矩阵加法
		Matrix44 res;
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				res.c[i][j] = m1.c[i][j] + m2.c[i][j];
			}
		}
		return res;
	}

	static Matrix44 Matrix44Sub(Matrix44& m1, Matrix44& m2){//矩阵减法
		Matrix44 res;
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				res.c[i][j] = m1.c[i][j] - m2.c[i][j];
			}
		}
		return res;
	}

	static Matrix44 Matrix44Mul(Matrix44& m1, Matrix44& m2){//矩阵叉乘
		Matrix44 res;
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				res.c[j][i] = m1.c[j][0] * m2.c[0][i] +
					m1.c[j][1] * m2.c[1][j] +
					m1.c[j][2] * m2.c[2][j] +
					m1.c[j][3] * m2.c[3][j];
			}
		}
		return res;
	}

	static Matrix44 Matrix44Sca(Matrix44& m, double s){//矩阵整体缩放
		Matrix44 res;
		for (int i = 0; i < 4; i++){
			for (int j = 0; j < 4; j++){
				res.c[i][j] = m.c[i][j] * s;
			}
		}
		return res;
	}

	static Matrix44 MakeAIdentity(){//变成一个单位矩阵
		Matrix44 res;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				res.c[i][j] = 0;
		res.c[0][0] = res.c[1][1] = res.c[2][2] = res.c[3][3];
		return res;
	}

	static Matrix44 MakeAZero(){//矩阵元素清零
		Matrix44 res;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				res.c[i][j] = 0;
		return res;
	}

	static void Matrix44Translate(Matrix44& m, double x, double y, double z){//平移
		m.c[3][0] = x;
		m.c[3][1] = y;
		m.c[3][2] = z;	
	}

	static void Matrix44Scale(Matrix44& m, double x, double y, double z){//通过矩阵缩放模型
		m.c[0][0] = x;
		m.c[1][1] = y;
		m.c[2][2] = z;
	}

	static void Matrix44Rotate(Matrix44 &m, double x, double y, double z, double t){//???通过矩阵旋转模型
		double qsin = (double)sin(t * 0.5f);
		double qcos = (double)cos(t * 0.5f);
		Vector3 vec = { x, y, z, 1.0f };
		double w = qcos;
		Vector3::Vector3Normalize(vec);
		x = vec.x * qsin;
		y = vec.y * qsin;
		z = vec.z * qsin;
		m.c[0][0] = 1 - 2 * y * y - 2 * z * z;
		m.c[1][0] = 2 * x * y - 2 * w * z;
		m.c[2][0] = 2 * x * z + 2 * w * y;
		m.c[0][1] = 2 * x * y + 2 * w * z;
		m.c[1][1] = 1 - 2 * x * x - 2 * z * z;
		m.c[2][1] = 2 * y * z - 2 * w * x;
		m.c[0][2] = 2 * x * z - 2 * w * y;
		m.c[1][2] = 2 * y * z + 2 * w * x;
		m.c[2][2] = 1 - 2 * x * x - 2 * y * y;
		m.c[0][3] = m.c[1][3] = m.c[2][3] = 0.0f;
		m.c[3][0] = m.c[3][1] = m.c[3][2] = 0.0f;
		m.c[3][3] = 1.0f;
	}
};

class Transform{
public:
	Matrix44 world;
	Matrix44 view;
	Matrix44 projection;
	Matrix44 transformWVP;//三者相乘
	double width, height;//屏幕宽高
};

void TransformUpdate(Transform &t){//矩阵的更新
	Matrix44 m;
	m = Matrix44::Matrix44Mul(t.world, t.view);
	t.transformWVP = Matrix44::Matrix44Mul(m, t.projection);
}

Vector3 Vector3MulMatirx44(Vector3& v, Matrix44& m){//向量乘以矩阵
	Vector3 res;
	res.x = v.x*m.c[0][0] + v.y*m.c[1][0] + v.z*m.c[2][0] + v.w*m.c[3][0];
	res.y = v.x*m.c[0][1] + v.y*m.c[1][1] + v.z*m.c[2][1] + v.w*m.c[3][1];
	res.z = v.x*m.c[0][2] + v.y*m.c[1][2] + v.z*m.c[2][2] + v.w*m.c[3][2];
	res.w = v.x*m.c[0][3] + v.y*m.c[1][3] + v.z*m.c[2][3] + v.w*m.c[3][3];
	return res;
}

template<class T>
void Swap(T& a, T& b){
	T tmp = a;
	a = b;
	b = tmp;
}

double InterpolateTriangle(//输入三角的三个点，输出三角形内的该点的插值
	double x0, double y0, double value0,
	double x1, double y1, double value1,
	double x2, double y2, double value2,
	double x, double y){

	Vector3 v0 = { x0 - x, y0 - y };
	Vector3 v1 = { x1 - x, y1 - y, 0 };
	Vector3 v2 = { x2 - x, y2 - y, 0 };

	Vector3 v12 = Vector3::Vector3Cross(v1, v2);
	Vector3 v01 = Vector3::Vector3Cross(v0, v1);
	Vector3 v02 = Vector3::Vector3Cross(v0, v2);

	double area12 = Vector3::Vector3Length(v12);//内点和两个三角形顶点所构成三角形面积。面积为向量积的模
	double area01 = Vector3::Vector3Length(v01);
	double area02 = Vector3::Vector3Length(v02);

	double areaAll = area12 + area01 + area02;//三角形的面积
	
	return (value0*area12 + value1*area02 + value2*area01) / areaAll;
	
}

#endif