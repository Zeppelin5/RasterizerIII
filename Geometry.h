//5.	定义基本顶点结构，包括顶点位置、顶点法线等。

#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#include "Math.h"
#include "Color.h"
#include "Material.h"
#include <vector>
const int TRIANGLE_FOREFACE = 0;
const int TRIANGLE_BACKFACE = 1;
const int TRIANGLE_CLIPPED = 2;

const int CLIP_SIMPLE_NO = 0;
const int CLIP_SIMPLE = 1;

int triangleCount_g = 0;

class Vertex3{//基本vertex结构
public:
	Vector3 vertex;
	Vector3 normal;
	Color color;

	Vector3 tangent;
	Vector3 bitangent;
	Vector3 normalMap;

	Vertex3& operator=(Vertex3& other){
		this->vertex = other.vertex;
		this->normal = other.normal;
		this->color = other.color;
		return *this;
	}
};

class Triangle{
public:
	int id;
	int state;
	int stateClip;//开clip=1，不开=0
	int attribute;
	Vertex3* vertexList; //顶点数组
	int index[3];//索引
	Vector2 uv[3];//纹理uv坐标;
	Color color[3];
	Material *material;//材质

	Vector3 tangent[3];
	Vector3 bitangent[3];

	Vertex3 GetVertexOfTriangle(int i){
		int indexx = index[i];
		return vertexList[indexx];
	}

	Triangle(int id, int state, int attr, Vertex3* vertexList, int *idx, Vector2* uv, Color* color){
		this->id = id;
		this->state = state;
		this->attribute = attr;
		this->vertexList = vertexList;
		//this->material = material;
		this->index[0] = idx[0];this->index[1] = idx[1];this->index[2] = idx[2];
		this->uv[0] = uv[0];this->uv[1] = uv[1];this->uv[2] = uv[2];
		this->color[0] = color[0];this->color[1] = color[1];this->color[2] = color[2];
	}

	Triangle& operator=(Triangle& other){
		this->id=other.id;
		this->state = other.state;
		this->attribute = other.attribute;
		this->vertexList = other.vertexList;
		this->index[0] = other.index[0]; this->index[1] = other.index[1]; this->index[2] = other.index[2];
		this->uv[0] = other.uv[0]; this->uv[1] = other.uv[1]; this->uv[2] = other.uv[2];
		this->color[0] = other.color[0]; this->color[1] = other.color[1]; this->color[2] = other.color[2];
		return *this;
	}
};

class GameObject{
public:
	int id;
	int state;//TRIANGLE_FOREFACE=0, TRIANGLE_BACKFACE=1
	int attribute;
	char* name;
	Vector3 posGameObject;

	int vertexCount;
	Vertex3 *localVertexList;
	Vertex3 *transVertexList;

	int triangleCount;
	Triangle *triangleList;

	//GameObject(Point3D pos ,int id, int attr, char *name, int vertexCount, Vertex3D *localVertexList, int triangleCount, Triangle* triangleList)
	GameObject(Vector3 pos, int id, int attribute, char* name, int vertexCount, Vertex3* localVertexList, int triangleCount, Triangle* triangleList){
		this->posGameObject = pos;
		this->id = id;
		this->attribute = attribute;
		this->name = name;
		this->localVertexList = localVertexList;
		this->vertexCount = vertexCount;
		this->triangleList = triangleList;
		this->triangleCount = triangleCount;
		//this->transVertexList = transVertexList;
		this->transVertexList = nullptr;
	}

	virtual ~GameObject()
	{
		if (name == nullptr){delete name;name = nullptr;}
		if (localVertexList == nullptr){delete localVertexList;localVertexList = nullptr;}
		if (transVertexList == nullptr){delete transVertexList;transVertexList = nullptr;}
		if (triangleList == nullptr){delete triangleList;triangleList = nullptr;}
	}

	void AddTriangle(vector<Vector3> vary,int& jcur){
		int vc = this->vertexCount;
		if (vary.size() > 3){
			//根据vary.x对4个点进行排序
			Vertex3 vList[3];
			Vector2 uv[3];
			int idx[3];
			Color color[3];

			uv[0] = { 0.0, 1.0 };uv[1] = { 0.0, 0.0 };uv[2] = { 1.0, 0.0 };
			idx[0] = 0; idx[1] = 1; idx[2] = 2;
			color[0] = 0xFFFFFFFF;color[1] = 0xFFFFFFFF;color[2] = 0xFFFFFFFF;

			//whichVertex0 = objectList[i]->triangleList[j].index[0];
			//Vertex3 va = objectList[i]->triangleList[j].vertexList[whichVertex0];//vertexList[]是一个顶点数组，如012,621,510等
			//Triangle(int id, int state, int attr, Vertex3* vertexList, int *idx, Vector2* uv, Color* color)
			vList[0].vertex = vary[0]; vList[0].normal = { -5.0, 0.0, 0.0 }; vList[0].color = Color(0xFFFFFFFF);
			vList[1].vertex = vary[1]; vList[1].normal = { -5.0, 0.0, 0.0 }; vList[1].color = Color(0xFFFFFFFF);
			vList[2].vertex = vary[2]; vList[2].normal = { -5.0, 0.0, 0.0 }; vList[2].color = Color(0xFFFFFFFF);
			Triangle t1(1, 0, 1, vList, idx, uv, color);
			
			vList[0].vertex = vary[1];
			vList[1].vertex = vary[2];
			vList[2].vertex = vary[3];
			Triangle t2(2, 0, 1, vList, idx, uv, color);

			//Triangle(int id, int state, int attr, Vertex3* vertexList, int *idx, Vector2* uv, Color* color)
			//this->triangleList[jcur] = t1;
			this->triangleList[jcur].id=t1.id;
			this->triangleList[jcur].state = t1.state;
			this->triangleList[jcur].attribute = t1.attribute;
			this->triangleList[jcur].vertexList = t1.vertexList;
			this->triangleList[jcur].index[0] = t1.index[0]; this->triangleList[jcur].index[1] = t1.index[1]; this->triangleList[jcur].index[2] = t1.index[2];
			this->triangleList[jcur].uv[0] = t1.uv[0]; this->triangleList[jcur].uv[1] = t1.uv[1]; this->triangleList[jcur].uv[2] = t1.uv[2];
			this->triangleList[jcur].color[0] = t1.color[0]; this->triangleList[jcur].color[1] = t1.color[1]; this->triangleList[jcur].color[2] = t1.color[2];

			triangleCount_g++;
			int newElement = triangleCount_g - 1;			
			this->triangleList[newElement].id = t2.id;
			this->triangleList[newElement].state = t2.state;
			this->triangleList[newElement].attribute = t2.attribute;
			this->triangleList[newElement].vertexList = t2.vertexList;
			this->triangleList[newElement].index[0] = t2.index[0]; 
			this->triangleList[newElement].index[1] = t2.index[1];
			this->triangleList[newElement].index[2] = t2.index[2];
			this->triangleList[newElement].uv[0].x = t2.uv[0].x;
			this->triangleList[newElement].uv[0].x = t2.uv[0].y;
			this->triangleList[newElement].uv[1].x = t2.uv[1].x;
			this->triangleList[newElement].uv[1].y = t2.uv[1].y;
			this->triangleList[newElement].uv[2].x = t2.uv[2].x;
			this->triangleList[newElement].uv[2].y = t2.uv[2].y;
			this->triangleList[newElement].color[0].r = t2.color[0].r; this->triangleList[newElement].color[0].g = t2.color[0].g; this->triangleList[newElement].color[0].b = t2.color[0].b;
			this->triangleList[newElement].color[1].r = t2.color[1].r; this->triangleList[newElement].color[1].g = t2.color[1].g; this->triangleList[newElement].color[1].b = t2.color[1].b;
			this->triangleList[newElement].color[2].r = t2.color[2].r; this->triangleList[newElement].color[2].g = t2.color[2].g; this->triangleList[newElement].color[2].b = t2.color[2].b;
			
			
			for (int i = 0; i < vc; i++)
				Vector3 vtest = this->transVertexList[i].vertex;
			cout << "test";
		}
	}
};

class TriangleColored{
public:
	Vector3 vertex[3];//一个三角形三个三维顶点
	Vector2 uv[3];//一个三角形三个二维纹理点坐标
	Color color[3];
	Material *material;

	TriangleColored(){}

	//FinalTriange(Point3D p0, Point3D p1, Point3D p2, Point2D uv0, Point2D uv1, Point2D uv2, Color c0, Color c1, Color c2, Material *m)
	TriangleColored(Vector3 p0, Vector3 p1, Vector3 p2, Vector2 uv0, Vector2 uv1, Vector2 uv2, Color c0, Color c1, Color c2, Material *m){
		vertex[0] = p0; vertex[1] = p1; vertex[2] = p2;
		uv[0] = uv0; uv[1] = uv1; uv[2] = uv2;
		color[0] = c0; color[1] = c1; color[2] = c2;
		material = m;
	}

	void SortFromTop2Bottom(){//将三角形三个顶点按从上到下（y值逐渐减小）排列为0，1，2
		for (int i = 0; i < 2; i++){
			int index = i;
			for (int j = index; j < 3; j++){
				if (vertex[j].y < vertex[index].y)
					continue;
				index = j;
			}
			if (index != i){
				Swap(vertex[i], vertex[index]);
				Swap(uv[i], uv[index]);
				Swap(color[i], color[index]);
			}
		}

	}
};

#endif
