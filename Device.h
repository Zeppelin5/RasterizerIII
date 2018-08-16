#ifndef __DEVICE_H
#define __DEVICE_H

#include "Math.h"
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include "ObjParser.h"
#include "Shader.h"
#include <vector>
#include <unordered_set>

//HDC hdc;
//TCHAR msg[] = _T("Hello World!");
const int RENDER_STATE_WIREFRAME = 1;
const int RENDER_STATE_COLOR = 2;
bool loadnow = false;
bool clipnow = false;

const int TEXTURE_WIDTH = 512;//148;
const int TEXTURE_HEIGHT = 512;// 149;
UINT TextureBuffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];
UINT NormalBuffer[TEXTURE_WIDTH * TEXTURE_HEIGHT];

const double eps = 0.0001;
typedef int OutCode;

const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

//渲染装置
class Device;
Device *device;

GameObject *obj0;

//立方体8个顶点
int vertexCount = 8;
Vertex3 *vertexList = (Vertex3*)malloc(sizeof(Vertex3) * vertexCount);
//立方体 6个面，每个面2个三角形，共12个三角形
int triangleCount = 12;
Triangle *triangleList = (Triangle*)malloc(sizeof(Triangle) * triangleCount);

class Device {
private:
	int width;
	int height;
	unsigned int *frameBuffer;		//图像缓存
	double *depthBuffer;		//深度缓存
	int renderState;
	unsigned int background;		//背景颜色
	unsigned int foreground;		//线框颜色
	double clipMaxX;
	double clipMaxY;
	double clipMinX;
	double clipMinY;

	Camera *camera;		//相机

	int objectListCount;
	GameObject *objectList[64];			//渲染物体列表

	int TriangleColoredCount;
	TriangleColored TriangleColoredList[64];

	int lightCount;
	Light *lightList[64];

	//Shader* pShader;
public:
	Device(Camera *camera, int width, int height, void *frameBuffer, int renderState, unsigned int foreground, unsigned int background)
	{
		Init(camera, width, height, frameBuffer, renderState, foreground, background);
	}

	void Init(Camera *camera, int width, int height, void *frameBuffer, int renderState, UINT foreground, UINT background)
	{
		this->camera = camera;
		this->width = width;
		this->height = height;
		this->frameBuffer = (UINT*)frameBuffer;
		depthBuffer = (double*)malloc(sizeof(double)*width * height);
		this->renderState = renderState;
		this->foreground = foreground;
		this->background = background;
		this->clipMinX = 0;
		this->clipMinY = 0;
		this->clipMaxX = width;//800
		this->clipMaxY = height;//600
		//objectListCount = 0;
		//finalTriangleCount = 0;
		lightCount = 0;
	}

	void VS(){
		//CalculateTangentArray();
		Model2World();
		RemoveBackfaceTriangle();
		World2Camera();
		Projection();
		ClipSimple();
	}

	void PS(){
	}

	void Render(){
		Clear();
		Vector2 p1 = { 400, 0 }; Vector2 p2 = { 400, 600 };//左上角是（0,0）,水平轴为x，垂直轴为y，x长800，y高600
		DrawLine(p1, p2, 0xFFFFFFFF);//画测试线
		Vector2 p3 = { 0, 300 }; Vector2 p4 = {800, 300 };
		DrawLine(p3, p4, 0xFFFFFFFF);
		if (loadnow)
			LoadExtraModel();
		VS();
		PerspectiveDiv();
		ViewTransform();
		GetLight();
		RenderObject();
	}

	void CalculateTangentArray(){
		for (int i = 0; i < objectListCount; i++){
			//ZeroMemory(tan1, vertexCount * sizeof(Vector3D) * 2);//清空数组
			Vector3 *Tv = new Vector3[vertexCount];
			Vector3 *Bv = new Vector3[vertexCount];
			for (int j = 0; j < triangleCount; j++){
				int index0 = objectList[i]->triangleList[j].index[0];//三角形顶点的索引，值为顶点列表的编号
				int index1 = objectList[i]->triangleList[j].index[1];
				int index2 = objectList[i]->triangleList[j].index[2];

				Vertex3& P0 = objectList[i]->triangleList[j].vertexList[index0];
				Vertex3& P1 = objectList[i]->triangleList[j].vertexList[index1];
				Vertex3& P2 = objectList[i]->triangleList[j].vertexList[index2];
				Vector3 Q1, Q2;
				Q1 = P1.vertex - P0.vertex;
				Q2 = P2.vertex - P0.vertex;

				Vector2 st0;
				st0.x = objectList[i]->triangleList[j].uv[0].x;//s0
				st0.y = objectList[i]->triangleList[j].uv[0].y;//t0
				Vector2 st1;
				st1.x = objectList[i]->triangleList[j].uv[1].x;//s1
				st1.y = objectList[i]->triangleList[j].uv[1].y;//t1
				Vector2 st2;
				st2.x = objectList[i]->triangleList[j].uv[2].x;//s2
				st2.y = objectList[i]->triangleList[j].uv[2].y;//t2

				float s1 = st1.x - st0.x;//<s1,t1>=<s1-s0,t1-t0>
				float t1 = st1.y - st0.y;
				float s2 = st2.x - st0.x;//<s2,t2>=<s2-s0,t2-t0>
				float t2 = st2.y - st0.y;

				float ratio = 1.0F / (s1 * t2 - s2 * t1);
				Vector3 T;//sdir
				T.x = (t2 * Q1.x - t1 * Q2.x) * ratio;//t2*Q1x-t1*Q2x
				T.y = (t2 * Q1.y - t1 * Q2.y) * ratio;
				T.z = (t2 * Q1.z - t1 * Q2.z) * ratio;
				Vector3 B;//tdir
				B.x = (s1 * Q2.x - s2 * Q1.x) * ratio;
				B.y = (s1 * Q2.y - s2 * Q1.y) * ratio;
				B.z = (s1 * Q2.z - s2 * Q1.z) * ratio;
				
				//向量平均值，直接相加就可以
				Tv[index0] = Tv[index0] + T;//index0点的T向量累加
				Bv[index0] = Tv[index0] + B;//index0点的B向量累加
				Tv[index1] = Tv[index1] + T;//index1点的T向量累加
				Bv[index1] = Tv[index1] + B;//index1点的B向量累加
				Tv[index2] = Tv[index2] + T;//index2点的T向量累加
				Bv[index2] = Tv[index2] + B;//index2点的B向量累加

				//Color color = Color(texBuffer[(UINT)u + (UINT)v * (UINT)texWidth]);
				//color.r *= r;
				//color.g *= g;
				//color.b *= b;
				UINT u, v; 
				u = objectList[i]->triangleList[j].uv[0].x;
				v = objectList[i]->triangleList[j].uv[0].y;
				Color color = Color(NormalBuffer[(UINT)u + (UINT)v*(UINT)TEXTURE_WIDTH]);
				Vector3 nortmp;
				nortmp.x = color.r * 2 - 1;
				nortmp.y = color.g * 2 - 1;
				nortmp.z = color.b * 2 - 1;
				objectList[i]->triangleList[j].GetVertexOfTriangle(0).normalMap = LoadNormalMap(i, j, 0);
				objectList[i]->triangleList[j].GetVertexOfTriangle(1).normalMap = LoadNormalMap(i, j, 1);
				objectList[i]->triangleList[j].GetVertexOfTriangle(2).normalMap = LoadNormalMap(i, j, 2);
			}

			for (int a = 0; a < objectList[i]->vertexCount; a++){
				Vector3& normal = Vector3::Vector3Cross(Bv[a], Tv[a]);
				Vector3::Vector3Normalize(normal);
				objectList[i]->transVertexList[a].normal = normal;//算出顶点N

				// 正交化
				Vector3& t = Tv[a];
				Vector3 tangent;
				tangent = (t - normal * Vector3::Vector3Dot(normal, t));//T'
				Vector3::Vector3Normalize(tangent);
				objectList[i]->transVertexList[a].tangent = tangent;//算出顶点T
				//计算偏手性w
				objectList[i]->transVertexList[a].tangent.w = (Vector3::Vector3Dot(Vector3::Vector3Cross(normal, t), Bv[a]) < 0.0F) ? -1.0F : 1.0F;
			}
			delete[] Tv;
			delete[] Bv;
		}
	}

	GameObject* Model2World(){//模型坐标到世界坐标
		for (int i = 0; i < objectListCount; i++)
		{
			if (objectList[i]->transVertexList != nullptr)
			{
				free(objectList[i]->transVertexList);//清除原有数据
				objectList[i]->transVertexList = nullptr;
			}
			objectList[i]->transVertexList = (Vertex3*)malloc(sizeof(Vertex3)*objectList[i]->vertexCount);//分配内存
			//将localVertexList里的vertexCount个元素拷到transVertexList中,localVertexList就是手动设置的8个顶点坐标
			memcpy(objectList[i]->transVertexList, objectList[i]->localVertexList, sizeof(Vertex3)* objectList[i]->vertexCount);

			Matrix44 translation = { 1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				objectList[i]->posGameObject.x, objectList[i]->posGameObject.y, objectList[i]->posGameObject.z, 1 };//平移矩阵

			for (int j = 0; j < objectList[i]->vertexCount; j++)
			{
				objectList[i]->transVertexList[j].vertex = Vector3MulMatirx44(objectList[i]->localVertexList[j].vertex, translation);//乘以平移矩阵
			}
			for (int j = 0; j < objectList[i]->triangleCount; j++)
			{
				objectList[i]->triangleList[j].vertexList = objectList[i]->transVertexList;
			}
		}
		return *objectList;
	}

	void RemoveBackfaceTriangle()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			for (int j = 0; j < objectList[i]->triangleCount; j++)
			{
				if (true)
				{
					Vector3 temp1 = objectList[i]->triangleList[j].GetVertexOfTriangle(1).vertex;
					Vector3 temp2 = objectList[i]->triangleList[j].GetVertexOfTriangle(0).vertex;
					Vector3 v1 = temp1 - temp2;
					Vector3 temp3 = objectList[i]->triangleList[j].GetVertexOfTriangle(0).vertex;
					Vector3 temp4 = objectList[i]->triangleList[j].GetVertexOfTriangle(2).vertex;
					Vector3 v2 = temp3 - temp4;

					//法线
					Vector3 normal;
					normal=Vector3::Vector3Cross(v1, v2);

					for (int k = 0; k < 3; k++)
					{
						Vector3 theNormal = objectList[i]->triangleList[j].GetVertexOfTriangle(k).normal;
						Vector3 n = theNormal + normal;
						int index = objectList[i]->triangleList[j].index[k];
						objectList[i]->triangleList[j].vertexList[index].normal.x = n.x;
						objectList[i]->triangleList[j].vertexList[index].normal.y = n.y;
						objectList[i]->triangleList[j].vertexList[index].normal.z = n.z;
					}

					Vector3 direction = objectList[i]->triangleList[j].GetVertexOfTriangle(0).vertex - camera->GetPosCamera();//方向为相机指向三角形

					double dot = Vector3::Vector3Dot(direction, normal);

					if (dot >= 0.0f)//cosθ大于0，θ<90度,则在背面，该三角形不渲染
					{
						objectList[i]->triangleList[j].state = TRIANGLE_BACKFACE;
					}
					else
					{
						objectList[i]->triangleList[j].state = TRIANGLE_FOREFACE;
					}
				}
			}
		}
	}


	void GetLight(){//给每个模型的每个点上色
		Vector3* tangent;
		//CalculateTangentArray(tangent);
		for (int i = 0; i < objectListCount; i++){//模型数objectListCount
			for (int j = 0; j < objectList[i]->vertexCount; j++){//单个模型包含的顶点数vertexCount	
				Vector3::Vector3Normalize(objectList[i]->transVertexList[j].normal);//单个顶点的法线归一化
				//Vector3 bitangle=Vector3::Vector3Cross(objectList[i]->transVertexList[j].normal,)

				Color color = Color(0);
				for (int k = 0; k < lightCount; k++){//光种类数
					Vertex3 tmp = objectList[i]->transVertexList[j];
					Color theColor = lightList[k]->getColorGeneral(tmp,
						*objectList[i]->triangleList[j].material, camera->GetPosCamera());
					color.r += theColor.r;
					color.g += theColor.g;
					color.b += theColor.b;
				}
				color.r = color.r > 1.0 ? 1.0 : color.r;
				color.g = color.g > 1.0 ? 1.0 : color.g;
				color.b = color.b > 1.0 ? 1.0 : color.b;
				objectList[i]->transVertexList[j].color.r *= color.r;//未定义赋值构造函数
				objectList[i]->transVertexList[j].color.g *= color.g;
				objectList[i]->transVertexList[j].color.b *= color.b;
			}
		}
	}

	void World2Camera(){//世界坐标到相机坐标
		for (int i = 0; i < objectListCount; i++)
			for (int j = 0; j < objectList[i]->vertexCount; j++)
				objectList[i]->transVertexList[j].vertex = camera->WorldToCamera(objectList[i]->transVertexList[j].vertex);
				//objectList[i]->transVertexList[j].vertex = Vector3MulMatirx44(objectList[i]->transVertexList[j].vertex, camera->matCamera);
	}


	void Projection(){//投影变换
		double angle = camera->GetFov()*PI / 180;
		Matrix44 perspectivehd = { 1 / (tan(angle / 2)*(width / height)), 0, 0, 0,
			0, 1 / (tan(angle / 2)), 0, 0,
			0, 0, camera->GetFar() / (camera->GetFar() - camera->GetNear()), 1,
			0, 0, (-1 * (camera->GetFar())*(camera->GetNear())) / (camera->GetFar() - camera->GetNear()), 0 };

		for (int i = 0; i < objectListCount; i++){
			for (int j = 0; j < objectList[i]->vertexCount; j++){
				objectList[i]->transVertexList[j].vertex = Vector3MulMatirx44(objectList[i]->transVertexList[j].vertex, perspectivehd);
			}
		}
	}

	//cvv裁剪
	void Clip()
	{
		//测试代码，证实有效
		Vector3 v0 = { 0, 20, 0, 10 };
		Vector3 v1 = { -10, 20, 0, 20 };
		Vector3* vary;
		vary = GetCrossPoint(v0, v1);

		for (int i = 0; i < objectListCount; i++){
			triangleCount_g = objectList[i]->triangleCount;
			for (int j = 0; j < 12; j++){
				objectList[i]->triangleCount = triangleCount_g;
				int whichVertex0,whichVertex1,whichVertex2;
				whichVertex0 = objectList[i]->triangleList[j].index[0];//返回手动设置的点的索引
				Vertex3 va = objectList[i]->triangleList[j].vertexList[whichVertex0];
				whichVertex1 = objectList[i]->triangleList[j].index[1];
				Vertex3 vb = objectList[i]->triangleList[j].vertexList[whichVertex1];
				whichVertex2 = objectList[i]->triangleList[j].index[2];
				Vertex3 vc = objectList[i]->triangleList[j].vertexList[whichVertex2];
				Vector3 a = va.vertex, b = vb.vertex, c = vc.vertex;

				vector<Vector3> vv;

				if (a.x / a.w < -1 || a.x / a.w>1 || a.y / a.w < -1 || a.y / a.w>1 || a.z / a.w>1 || a.z < 0
					|| b.x / b.w < -1 || b.x / b.w>1 || b.y / b.w < -1 || b.y / b.w>1 || b.z / b.w>1 || b.z < 0
					|| c.x / c.w < -1 || c.x / c.w>1 || c.y / c.w < -1 || c.y / c.w>1 || c.z / c.w>1 || c.z < 0){
					Vector3 vary[6];
					Vector3* vtmp;//线段交点
					vtmp = GetCrossPoint(a, b);
					vary[0] = vtmp[0];
					vary[1] = vtmp[1];
					vtmp = GetCrossPoint(b, c);
					vary[2] = vtmp[0];
					vary[3] = vtmp[1];
					vtmp = GetCrossPoint(c, a);
					vary[4] = vtmp[0];
					vary[5] = vtmp[1];

					for (int i = 0; i < 6; i++){
						if (vary[i] == vary[i + 1]){
							i++;
							vv.push_back(vary[i]);
						}
						else
							vv.push_back(vary[i]);
					}
					vector<Vector3>::iterator it = vv.begin();
					vector<Vector3>::iterator end = vv.end() - 1;

					if (*it == *end)
						vv.erase(vv.end() - 1);

					Vector3 zero = { 0, 0, 0, 0 };
					for (vector<Vector3>::iterator its = vv.begin(); its != vv.end();){
						if ((*its) == zero){
							its=vv.erase(its);
						}
						else
							++its;
					}

					//int newVertexCount = vv.size() - 3;
					//objectList[i]->vertexCount += newVertexCount;
					if (vv.size() > 3){
						clipnow = true;
						cout << vv.size() << endl;
						//vv为切割后的多边形顶点数组
						objectList[i]->AddTriangle(vv, j);
					}
				}//if{a.x/a.w>0}...
			}//for(objection[i]->triangleList[j]
		}//for(objection)
	}//function

	//判断端点的区域码
	OutCode ComputeOutCode(double x, double y, double w)
	{
		OutCode code;
		code = INSIDE;          // initialised as being inside of clip window

		if (x < -w)           // to the left of clip window
			code |= LEFT;
		else if (x > w)      // to the right of clip window
			code |= RIGHT;
		if (y < -w)           // below the clip window
			code |= BOTTOM;
		else if (y > w)      // above the clip window
			code |= TOP;

		return code;
	}
	// Cohen–Sutherland clipping algorithm clips a line from
	// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with 
	// diagonal from (xmin, ymin) to (xmax, ymax).
	//vector<Vector3> GetCrossPoint(Vector3 v0, Vector3 v1)//CohenSutherlandLineClip
	Vector3* GetCrossPoint(Vector3 v0, Vector3 v1)//CohenSutherlandLineClip
	{
		// compute outcodes for P0, P1, and whatever point lies outside the clip rectangle
		OutCode outcode0 = ComputeOutCode(v0.x, v0.y, v0.w);
		OutCode outcode1 = ComputeOutCode(v1.x, v1.y, v1.w);
		bool accept = false;//接受时输出该线，拒绝时不输出，不渲染
		Vector3 vout, vin; 
		bool isInValid = false;
		while (true) {
			if (!(outcode0 | outcode1)) { //相或为0，都在立方体内，接受并且退出循环
				accept = true;
				Vector3 res[2];
				res[0] = v0;
				res[1] = v1;
				return res;
				//break;
			}
			else if (outcode0 & outcode1) { // 相与为1，都在立方体外的一侧，拒绝且退出循环
				Vector3 res[2];
				res[0] = { 0, 0, 0, 0 };
				res[1] = { 0, 0, 0, 0 };
				return res;
				//break;
			}
			else {
				// failed both tests, so calculate the line segment to clip
				// from an outside point to an intersection with clip edge
				//double x, y;

				//找出在界外的点
				OutCode outcodeOut = outcode0 ? outcode0 : outcode1;//outcodeOut是在外头点的编码
				if (outcodeOut == outcode0){
					vout = v0;
					vin = v1;
				}
				else{
					vout = v1;
					vin = v0;
				}

				Vector3 cross;
				
				// 找出和边界相交的点
				// 使用点斜式 y = y0 + slope * (x - x0), x = x0 + (1 / slope) * (y - y0)
				if (outcodeOut & TOP) {           // point is above the clip rectangle
					//x=(y-y0)*(1/slope)+x0
					if (vout.y != vin.y){
						cross.x = vin.x + (vout.x - vin.x) / (vout.y - vin.y)*(vout.w - vin.y);
						cross.y = vout.w;
						cross.z = vout.z;
						cross.w = vout.w;
					}
					else{
						isInValid = true;
						break;
					}
				}
				else if (outcodeOut & BOTTOM) { // point is below the clip rectangle
					//x=(y-y0)*(1/slope)+x0
					if (vout.y != vin.y){
						cross.x = vin.x + (vout.x - vin.x) / (vout.y - vin.y)*(-vout.w - vin.y);
						cross.y = -vout.w;
						cross.z = vout.z;
						cross.w = vout.w;
					}
					else{
						isInValid = true;
						break;
					}
				}
				else if (outcodeOut & RIGHT) {  // point is to the right of clip rectangle
					//y=slope*(x-x0)+y0;
					if (vout.x != vin.x){
						cross.y = vin.y + (vout.y - vin.y) / (vout.x - vin.x)*(vout.w - vin.x);
						cross.x = vout.w;
						cross.z = vout.z;
						cross.w = vout.w;
					}
					else{
						isInValid = true;
						break;
					}
				}
				else if (outcodeOut & LEFT) {   // point is to the left of clip rectangle
					//y=slope*(x-x0)+y0;
					if (vout.x != vin.x){
						cross.y = vin.y + (vout.y - vin.y) / (vout.x - vin.x)*(-vout.w - vin.x);
						cross.x = -vout.w;
						cross.z = vout.z;
						cross.w = vout.w;
					}
					else{
						isInValid = true;
						break;
					}
				}

				// Now we move outside point to intersection point to clip
				// 为什么继续循环，因为另一个端点都有可能也在外面，需要将原来外面的点用算出来的交点取代，然后再进行一次判断，直到accept
				if (outcodeOut == outcode0) {
					v0 = cross;
					outcode0 = ComputeOutCode(v0.x, v0.y, v0.w);
				}
				else {
					v1 = cross;
					outcode1 = ComputeOutCode(v1.x, v1.y, v1.w);
				}
			}
		}
		if (accept) {
			// Following functions are left for implementation by user based on
			// their platform (OpenGL/graphics.h etc.)
			//DrawRectangle(xmin, ymin, xmax, ymax);
			//LineSegment(x0, y0, x1, y1);			//x0,y0,x1,y1为裁剪后屏幕内的点

			Vector3 res[2];
			res[0] = v0, res[1] = v1;
			return res;
		}
		else{
			if (isInValid == false){
				Vector3 res[2];
				res[0] = { 0, 0, 0, 0 };
				res[1] = { 0, 0, 0, 0 };
				return res;
			}
			else{//isInvalid==true;
				Vector3 res[2];
				res[0] = vin;
				res[1] = { 0, 0, 0, 0 };
				return res;
			}
		}

	}

	void ClipSimple(){
		for (int i = 0; i < objectListCount; i++){
			for (int j = 0; j < objectList[i]->triangleCount; j++){
				objectList[i]->triangleList[j].stateClip = CLIP_SIMPLE_NO;
				if (ClipSimpleCore(objectList[i]->triangleList[j].GetVertexOfTriangle(0)) == false
					|| ClipSimpleCore(objectList[i]->triangleList[j].GetVertexOfTriangle(0)) == false
					|| ClipSimpleCore(objectList[i]->triangleList[j].GetVertexOfTriangle(0)) == false){
					objectList[i]->triangleList[j].stateClip = CLIP_SIMPLE;
				}
			}
		}
	}

	bool ClipSimpleCore(Vertex3 v){
		//cvv为 x-1,1  y-1,1  z0,1
		if (v.vertex.x >= -v.vertex.w && v.vertex.x <= v.vertex.w &&
			v.vertex.y >= -v.vertex.w && v.vertex.y <= v.vertex.w &&
			v.vertex.z >= 0.f && v.vertex.z <= v.vertex.w)//是否在ndc内
			return true;//在ndc内，无需裁剪
		else
			return false;//有点在ncc外，需裁剪
	}

	void PerspectiveDiv(){
		for (int i = 0; i < objectListCount; i++){
			for (int j = 0; j < objectList[i]->vertexCount; j++){
				double wtmp = objectList[i]->transVertexList[j].vertex.w;
				objectList[i]->transVertexList[j].vertex.x /= wtmp;
				objectList[i]->transVertexList[j].vertex.y /= wtmp;
				objectList[i]->transVertexList[j].vertex.z /= wtmp;
				objectList[i]->transVertexList[j].vertex.w = 1;
			}
		}
	}

	void ViewTransform(){//视口变换,将坐标转化为屏幕窗口坐标
		for (int i = 0; i < objectListCount; i++){
			if (objectList[i]->vertexCount != 8){
				cout << "test";
			}
			for (int j = 0; j < objectList[i]->vertexCount; j++){
				double deltax = (objectList[i]->transVertexList[j].vertex.x + camera->GetCameraWidth() / 2) / (camera->GetCameraWidth());
				objectList[i]->transVertexList[j].vertex.x = Interpolate(0.0f, (double)width, deltax);
				double deltay = (objectList[i]->transVertexList[j].vertex.y + camera->GetCameraHeight() / 2) / (camera->GetCameraHeight());
				objectList[i]->transVertexList[j].vertex.y = (double)height - Interpolate(0.0f, (double)height, deltay);
				
			}
		}
	}

	//渲染物体列表
	void RenderObject()
	{
		for (int i = 0; i < objectListCount; i++)
		{
			int ii = i;
			for (int j = 0; j < objectList[i]->triangleCount; j++)
			{
				if (objectList[i]->triangleList[j].state == TRIANGLE_BACKFACE)
					continue;
				if (objectList[i]->triangleList[j].stateClip == CLIP_SIMPLE)
					continue;
				//objectList[i]->triangleList[j].vertexList = objectList[i]->transVertexList;//如果注释掉，无法显示另外添加的立方体
				TriangleColored &t = TriangleColoredList[TriangleColoredCount++];
				t.material = objectList[i]->triangleList[j].material;
				for (int k = 0; k < 3; k++)
				{
					int whichindex = objectList[i]->triangleList[j].index[k];
					t.vertex[k].x = objectList[i]->triangleList[j].vertexList[whichindex].vertex.x;
					t.vertex[k].y = objectList[i]->triangleList[j].vertexList[whichindex].vertex.y;
					t.vertex[k].z = objectList[i]->triangleList[j].vertexList[whichindex].vertex.z;

					t.uv[k].x = objectList[i]->triangleList[j].uv[k].x;
					t.uv[k].y = objectList[i]->triangleList[j].uv[k].y;
					t.color[k] = objectList[i]->triangleList[j].vertexList[whichindex].color;
				}
			}
		}
		for (int i = 0; i < TriangleColoredCount; i++)
		{
			DrawTriangle(TriangleColoredList[i]);
		}
		for (int i = 0; i < TriangleColoredCount; i++)
		{
			Vector2 p1 = { TriangleColoredList[i].vertex[0].x, TriangleColoredList[i].vertex[0].y };
			Vector2 p2 = { TriangleColoredList[i].vertex[1].x, TriangleColoredList[i].vertex[1].y };
			Vector2 p3 = { TriangleColoredList[i].vertex[2].x, TriangleColoredList[i].vertex[2].y };
			DrawLine(p1, p2, 0x00FF6100);
			DrawLine(p2, p3, 0x00FF6100);
			DrawLine(p3, p1, 0x00FF6100);
		}
	}

	Vector3 LoadNormalMap(int objIndex, int triIndex, int verIndex){
		UINT u, v;
		u = objectList[objIndex]->triangleList[triIndex].uv[verIndex].x;
		v = objectList[objIndex]->triangleList[triIndex].uv[verIndex].y;
		Color color = Color(NormalBuffer[(UINT)u + (UINT)v*(UINT)TEXTURE_WIDTH]);
		Vector3 nortmp;
		nortmp.x = color.r * 2 - 1;
		nortmp.y = color.g * 2 - 1;
		nortmp.z = color.b * 2 - 1;
		return nortmp;
	}

	void DrawLine(const Vector2& p1, const Vector2& p2, unsigned int color){
		DrawLineDDA(p1.x, p1.y, p2.x, p2.y, color);
	}

	void DrawLineDDA(int x1, int y1, int x2, int y2, unsigned int color){//绘线
		int dx = x2 - x1;
		int dy = y2 - y1;
		float x = x1, y = y1;
		int steps = (abs(dx)) > (abs(dy)) ? (abs(dx)) : (abs(dy));

		float xPerStep, yPerStep;
		xPerStep = float(dx) / (float)(steps);
		yPerStep = float(dy) / (float)(steps);

		DrawPoint(x1, y1, color);
		for (int i = 0; i <= steps; i++){
			x += xPerStep;
			y += yPerStep;
			DrawPoint(x, y, color);
		}
		DrawPoint(x2, y2, color);
	}

	void DrawPoint(int x, int y, unsigned int color){
		if (x<clipMinX || x>clipMaxX || y<clipMinY || y>clipMaxY)
			return;//剔除
		frameBuffer[(unsigned int)x + (unsigned int)y*(unsigned int)width] = color;//相当于一个二维数组bitmap来存储颜色数据
	}

	void RenderTexture(UINT *texTureBuffer, int width, int height)
	{
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				UINT color = texTureBuffer[j + width * i];
				DrawPoint(j, i, color);
			}
		}
	}

	void Clear(){
		TriangleColoredCount = 0;
		//memset(frameBuffer, 0xFFFF0000, sizeof(UINT)* width * height);//清除前一帧，消重影
		for (int i = 0; i < clipMaxX; i++){
			for (int j = 0; j < clipMaxY; j++){
				frameBuffer[(unsigned)i + (unsigned)j*width] = 0x002B9EB7;
			}
		}
		memset(depthBuffer, 0, sizeof(double)* width * height);
	}

	void LoadExtraModel(){
		//载入obj文件,获取顶点纹理法线数据
		ObjParser* pObjParser = new ObjParser();
		pObjParser->SetFileName("mayaCube.obj");
		pObjParser->ReadFileCounts();//统计顶点纹理和法线数mVertexCount、mTexcoordCount等等

		//将数据传入顶点、纹理、法线和面数组
		// After get enough memory, clear these counts to 0, use it as index
		VertexType* vertices = new VertexType[pObjParser->mVertexCount];
		VertexType* texcoords = new VertexType[pObjParser->mTexcoordCount];
		VertexType* normals = new VertexType[pObjParser->mNormalCount];
		FaceType* faces = new FaceType[pObjParser->mFaceCount];

		std::ifstream fin;
		fin.open(pObjParser->mInFile);
		if (fin.fail())
		{
			pObjParser->mErr << "Open input file : Failed." << std::endl;
			return;
		}
		char input;
		char ignore;
		int iver = 0, itex = 0, inor = 0, ifac = 0;
		fin.get(input);
		while (!fin.eof())
		{
			if (input == 'v')
			{
				fin.get(input);
				switch (input)
				{
				case ' ':
				{
					fin >> vertices[iver].x >> vertices[iver].y >> vertices[iver].z;//写入顶点数组
					//vertices[iver].z *= -1.0f;  // RH->LH
					iver++;
					break;
				}
				case 't':
				{
					fin >> texcoords[itex].x >> texcoords[itex].y;//写入纹理数组
					//texcoords[itex].y = 1.0f - texcoords[itex].y;  // RH->LH
					itex++;
					break;
				}
				case 'n':
				{
					fin >> normals[inor].x >> normals[inor].y >> normals[inor].z;//写入法线数组
					//normals[inor].z *= -1.0f; // RH->LH
					inor++;
					break;
				}
				}
			}
			if (input == 'f')
			{
				fin.get(input);
				if (input == ' ')
				{//写入三角面的数组,换个旋转方向，要不然会被背面剔除
					fin >> faces[ifac].vIndex3 >> ignore >> faces[ifac].tIndex3 >> ignore >> faces[ifac].nIndex3;
					fin >> faces[ifac].vIndex2 >> ignore >> faces[ifac].tIndex2 >> ignore >> faces[ifac].nIndex2;
					fin >> faces[ifac].vIndex1 >> ignore >> faces[ifac].tIndex1 >> ignore >> faces[ifac].nIndex1;
					ifac++;
				}
			}

			while (input != '\n')
				fin.get(input);

			fin.get(input);
		}
		fin.close();
		//以上代码将数据读取到vertices、texcoords、normals和faces数组中，之后再往三角形和顶点容器里写入

		//设置顶点、三角面等列表，之后再AddObject
		int vIndex = 0, tIndex = 0, nIndex = 0;
		Vertex3 *vList = (Vertex3*)malloc(sizeof(Vertex3)*pObjParser->mVertexCount);//8
		Triangle *tList = (Triangle*)malloc(sizeof(Triangle) * pObjParser->mFaceCount);//12

		for (int i = 0; i < pObjParser->mVertexCount; i++){//设置顶点的坐标、颜色和法线
			vList[i].vertex.x = vertices[i].x;
			vList[i].vertex.y = vertices[i].y;
			vList[i].vertex.z = vertices[i].z;
			vList[i].vertex.w = 1;
			vList[i].color = Color(0xFFFFFFFF);
			vList[i].normal = { -5.0, 0.0, 0.0 };//暂时先都设为常量
		}

		for (int i = 0; i < pObjParser->mFaceCount; i++)//pObjParser->mFaceCount应该等于vIndex最大值*3
		{
			tList[i].index[0] = faces[i].vIndex1 - 1;
			tList[i].index[1] = faces[i].vIndex2 - 1;
			tList[i].index[2] = faces[i].vIndex3 - 1;
			tList[i].uv[0].x = texcoords[faces[i].tIndex1 - 1].x;
			tList[i].uv[0].y = texcoords[faces[i].tIndex1 - 1].y;
			tList[i].uv[1].x = texcoords[faces[i].tIndex2 - 1].x;
			tList[i].uv[1].y = texcoords[faces[i].tIndex2 - 1].y;
			tList[i].uv[2].x = texcoords[faces[i].tIndex3 - 1].x;
			tList[i].uv[2].y = texcoords[faces[i].tIndex3 - 1].y;
		}

		//材质 纹理默认TEXTURE_HEIGHT=149, TextureBuffer=148
		Material *m = new Material(TextureBuffer, TEXTURE_WIDTH, TEXTURE_HEIGHT);
		m->id = 0;
		m->color = Color(0xFFFFFFFF);
		m->Kambient = 0.5;
		m->Kdiffuse = 0.9;
		m->Kspecular = 0.1;
		m->shininess = 1.2;
		m->colorAmb = Color(0xFFFFFFFF)*m->Kambient;
		m->colorDif = Color(0xFFFFFFFF)*m->Kdiffuse;
		m->colorSpe = Color(0xFFFFFFFF)*m->Kspecular;
		for (int i = 0; i < pObjParser->mFaceCount; i++)
			tList[i].material = m;

		if (vertices)
		{
			delete[] vertices;
			vertices = 0;
		}
		if (texcoords)
		{
			delete[] texcoords;
			texcoords = 0;
		}
		if (normals)
		{
			delete[] normals;
			normals = 0;
		}
		if (faces)
		{
			delete[] faces;
			faces = 0;
		}

		Vector3 obj0Pos1 = { 2.0, 0.5, 5.0, 1 };
		GameObject* objdotOBJ = new GameObject(obj0Pos1, 1, 1, "newOBJ", pObjParser->mVertexCount, vList, pObjParser->mFaceCount, tList);
		device->AddObjectList(objdotOBJ);
		//加载结束
		loadnow = false;
	}

	bool ZTest(double x, double y, double z){
		if (x<clipMinX || x>=clipMaxX || y<clipMinY || y>=clipMaxY)
			return false;
		double ZofCurrentPixel = depthBuffer[(unsigned int)x + (unsigned int)y*width];
		if (ZofCurrentPixel < z){//改顶点的z值大于深度缓冲区中该像素的当前深度值
			depthBuffer[(unsigned int)x + (unsigned int)y*width] = z;
			return true;
		}
		else
			return false;
	}

	Light *CreateLight(int type, unsigned int color){
		if (type == LIGHTTYPE_AMBIENT){
			Light* ambientLight = new Light();
			ambientLight->color = Color(color);
			return ambientLight;
		}
		else if (type == LIGHTTYPE_DIRECTION){
			DirectionLight* directionLight = new DirectionLight();
			directionLight->color = Color(color);
			return directionLight;
		}
	}

	Light *addLight(int type, unsigned int color){//初始化LightList
		Light *light = CreateLight(type, color);
		lightList[lightCount++] = light;
		return light;
	}

	void TopTriangle(const TriangleColored &t)
	{
		Vertex3 top, left, right;
		Vector2 topUV, leftUV, rightUV;
		Color topColor, leftColor, rightColor;
		if ((int)t.vertex[0].y == (int)t.vertex[1].y)
		{
			top.vertex = t.vertex[2];
			topUV = t.uv[2];
			topColor = t.color[2];
			left.vertex = t.vertex[0];
			leftUV = t.uv[0];
			leftColor = t.color[0];
			right.vertex = t.vertex[1];
			rightUV = t.uv[1];
			rightColor = t.color[1];
		}
		else if ((int)t.vertex[1].y == (int)t.vertex[2].y)
		{
			top.vertex = t.vertex[0];
			left.vertex = t.vertex[1];
			right.vertex = t.vertex[2];
			topUV = t.uv[0];
			leftUV = t.uv[1];
			rightUV = t.uv[2];
			topColor = t.color[0];
			leftColor = t.color[1];
			rightColor = t.color[2];
		}
		else
		{
			top.vertex = t.vertex[1];
			left.vertex = t.vertex[0];
			right.vertex = t.vertex[2];
			topUV = t.uv[1];
			leftUV = t.uv[0];
			rightUV = t.uv[2];
			topColor = t.color[1];
			leftColor = t.color[0];
			rightColor = t.color[2];

		}
		if (left.vertex.x > right.vertex.x)
		{
			Swap(left, right);
			Swap(leftUV, rightUV);
			Swap(leftColor, rightColor);
		}
		double leftDxDivDy = (top.vertex.x - left.vertex.x) / (top.vertex.y - left.vertex.y);
		double rightDxDivDy = (right.vertex.x - top.vertex.x) / (right.vertex.y - top.vertex.y);
		double xleft = left.vertex.x;
		double xRight = right.vertex.x;
		int y = left.vertex.y;

		//1/Z
		double leftDxDivDyReciprocalZ = ((1 / top.vertex.z) - (1 / left.vertex.z)) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyReciprocalZ = ((1 / right.vertex.z) - (1 / top.vertex.z)) / (right.vertex.y - top.vertex.y);
		double xLeftReciprocalZ = 1 / left.vertex.z;
		double xRightReciprocalZ = 1 / right.vertex.z;

		//U
		double leftDxDivDyColorU = (topUV.x / top.vertex.z - leftUV.x / left.vertex.z) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyColorU = (rightUV.x / right.vertex.z - topUV.x / top.vertex.z) / (right.vertex.y - top.vertex.y);
		double xLeftColorU = leftUV.x / left.vertex.z;
		double xRightColorU = rightUV.x / right.vertex.z;
		//V
		double leftDxDivDyColorV = (topUV.y / top.vertex.z - leftUV.y / left.vertex.z) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyColorV = (rightUV.y / right.vertex.z - topUV.y / top.vertex.z) / (right.vertex.y - top.vertex.y);
		double xLeftColorV = leftUV.y / left.vertex.z;
		double xRightColorV = rightUV.y / right.vertex.z;

		//R
		double leftDxDivDyColorR = (topColor.r - leftColor.r) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyColorR = (rightColor.r - topColor.r) / (right.vertex.y - top.vertex.y);
		double xLeftColorR = leftColor.r;
		double xRightColorR = rightColor.r;

		//G
		double leftDxDivDyColorG = (topColor.g - leftColor.g) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyColorG = (rightColor.g - topColor.g) / (right.vertex.y - top.vertex.y);
		double xLeftColorG = leftColor.g;
		double xRightColorG = rightColor.g;

		//B
		double leftDxDivDyColorB = (topColor.b - leftColor.b) / (top.vertex.y - left.vertex.y);
		double rightDxDivDyColorB = (rightColor.b - topColor.b) / (right.vertex.y - top.vertex.y);
		double xLeftColorB = leftColor.b;
		double xRightColorB = rightColor.b;



		double texWidth = t.material->width;
		double texHeight = t.material->height;
		UINT *texBuffer = t.material->texBuffer;

		while (y >= top.vertex.y)
		{
			double dxReciprocalZ = (xRightReciprocalZ - xLeftReciprocalZ) / (xRight - xleft);
			double reciprocalZ = xLeftReciprocalZ;

			double dxColorU = (xRightColorU - xLeftColorU) / (xRight - xleft);
			double ColorStarU = xLeftColorU;

			double dxColorV = (xRightColorV - xLeftColorV) / (xRight - xleft);
			double ColorStarV = xLeftColorV;

			double dxColorR = (xRightColorR - xLeftColorR) / (xRight - xleft);
			double ColorStarR = xLeftColorR;

			double dxColorG = (xRightColorG - xLeftColorG) / (xRight - xleft);
			double ColorStarG = xLeftColorG;

			double dxColorB = (xRightColorB - xLeftColorB) / (xRight - xleft);
			double ColorStarB = xLeftColorB;

			for (int i = (int)(xleft); i <= (int)(xRight); i++)
			{

				double u = (ColorStarU / reciprocalZ) * (texWidth - 1);
				double v = (ColorStarV / reciprocalZ) * (texHeight - 1);

				double r = ColorStarR;
				double g = ColorStarG;
				double b = ColorStarB;

				if (ZTest(i, y, reciprocalZ))
				{
					if (u < 0)
						u = 0;
					if (v < 0)
						v = 0;
					
					Color color = Color(texBuffer[(UINT)u + (UINT)v * (UINT)texWidth]);
					color.r *= r;
					color.g *= g;
					color.b *= b;
					//color.r = 1;
					//color.g = 0;
					//color.b = 0;
					DrawPoint(i, y, color.Get32());
				}

				ColorStarU += dxColorU;
				ColorStarV += dxColorV;
				reciprocalZ += dxReciprocalZ;
				ColorStarR += dxColorR;
				ColorStarG += dxColorG;
				ColorStarB += dxColorB;
			}
			y--;
			xleft -= leftDxDivDy;
			xRight -= rightDxDivDy;

			xLeftReciprocalZ -= leftDxDivDyReciprocalZ;
			xRightReciprocalZ -= rightDxDivDyReciprocalZ;

			xLeftColorU -= leftDxDivDyColorU;
			xRightColorU -= rightDxDivDyColorU;

			xLeftColorV -= leftDxDivDyColorV;
			xRightColorV -= rightDxDivDyColorV;

			xLeftColorR -= leftDxDivDyColorR;
			xLeftColorG -= leftDxDivDyColorG;
			xLeftColorB -= leftDxDivDyColorB;

			xRightColorR -= rightDxDivDyColorR;
			xRightColorG -= rightDxDivDyColorG;
			xRightColorB -= rightDxDivDyColorB;
		}
	}
	void BottomTriangle(const TriangleColored &t)
	{
		Vertex3 bottom, left, right;
		Vector2 bottomUV, leftUV, rightUV;
		Color bottomColor, leftColor, rightColor;
		if ((int)t.vertex[0].y == (int)t.vertex[1].y)//0,1在下，2在上
		{
			bottom.vertex = t.vertex[2];
			bottomUV = t.uv[2];
			left.vertex = t.vertex[0];
			leftUV = t.uv[0];
			right.vertex = t.vertex[1];
			rightUV = t.uv[1];
			bottomColor = t.color[2];
			leftColor = t.color[0];
			rightColor = t.color[1];
		}
		else if ((int)t.vertex[1].y == (int)t.vertex[2].y)
		{
			bottom.vertex = t.vertex[0];
			left.vertex = t.vertex[1];
			right.vertex = t.vertex[2];
			bottomUV = t.uv[0];
			leftUV = t.uv[1];
			rightUV = t.uv[2];
			bottomColor = t.color[0];
			leftColor = t.color[1];
			rightColor = t.color[2];
		}
		else
		{
			bottom.vertex = t.vertex[1];
			left.vertex = t.vertex[0];
			right.vertex = t.vertex[2];
			bottomUV = t.uv[1];
			leftUV = t.uv[0];
			rightUV = t.uv[2];
			bottomColor = t.color[1];
			leftColor = t.color[0];
			rightColor = t.color[2];
		}
		if (left.vertex.x > right.vertex.x)
		{
			Swap(left, right);
			Swap(leftUV, rightUV);
			Swap(leftColor, rightColor);
		}
		double leftDxDivDy = (bottom.vertex.x - left.vertex.x) / (bottom.vertex.y - left.vertex.y);//左边斜率
		double rightDxDivDy = (right.vertex.x - bottom.vertex.x) / (right.vertex.y - bottom.vertex.y);//右边斜率
		double xleft = left.vertex.x;//左点x坐标
		double xRight = right.vertex.x;//右点x坐标
		int y = left.vertex.y;//左点y坐标

		//1/Z
		double leftDxDivDyReciprocalZ = ((1 / bottom.vertex.z) - (1 / left.vertex.z)) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyReciprocalZ = ((1 / right.vertex.z) - (1 / bottom.vertex.z)) / (right.vertex.y - bottom.vertex.y);
		double xLeftReciprocalZ = 1 / left.vertex.z;
		double xRightReciprocalZ = 1 / right.vertex.z;

		//U
		double leftDxDivDyColorU = (bottomUV.x / bottom.vertex.z - leftUV.x / left.vertex.z) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyColorU = (rightUV.x / right.vertex.z - bottomUV.x / bottom.vertex.z) / (right.vertex.y - bottom.vertex.y);
		double xLeftColorU = leftUV.x / left.vertex.z;
		double xRightColorU = rightUV.x / right.vertex.z;
		//V
		double leftDxDivDyColorV = (bottomUV.y / bottom.vertex.z - leftUV.y / left.vertex.z) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyColorV = (rightUV.y / right.vertex.z - bottomUV.y / bottom.vertex.z) / (right.vertex.y - bottom.vertex.y);
		double xLeftColorV = leftUV.y / left.vertex.z;
		double xRightColorV = rightUV.y / right.vertex.z;

		//R
		double leftDxDivDyColorR = (bottomColor.r - leftColor.r) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyColorR = (rightColor.r - bottomColor.r) / (right.vertex.y - bottom.vertex.y);
		double xLeftColorR = leftColor.r;
		double xRightColorR = rightColor.r;

		//G
		double leftDxDivDyColorG = (bottomColor.g - leftColor.g) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyColorG = (rightColor.g - bottomColor.g) / (right.vertex.y - bottom.vertex.y);
		double xLeftColorG = leftColor.g;
		double xRightColorG = rightColor.g;

		//B
		double leftDxDivDyColorB = (bottomColor.b - leftColor.b) / (bottom.vertex.y - left.vertex.y);
		double rightDxDivDyColorB = (rightColor.b - bottomColor.b) / (right.vertex.y - bottom.vertex.y);
		double xLeftColorB = leftColor.b;
		double xRightColorB = rightColor.b;

		double texWidth = t.material->width;
		double texHeight = t.material->height;
		UINT *texBuffer = t.material->texBuffer;

		while (y <= bottom.vertex.y)
		{
			double dxReciprocalZ = (xRightReciprocalZ - xLeftReciprocalZ) / (xRight - xleft);
			double reciprocalZ = xLeftReciprocalZ;

			double dxColorU = (xRightColorU - xLeftColorU) / (xRight - xleft);
			double ColorStarU = xLeftColorU;

			double dxColorV = (xRightColorV - xLeftColorV) / (xRight - xleft);
			double ColorStarV = xLeftColorV;

			double dxColorR = (xRightColorR - xLeftColorR) / (xRight - xleft);
			double ColorStarR = xLeftColorR;

			double dxColorG = (xRightColorG - xLeftColorG) / (xRight - xleft);
			double ColorStarG = xLeftColorG;

			double dxColorB = (xRightColorB - xLeftColorB) / (xRight - xleft);
			double ColorStarB = xLeftColorB;
			for (int i = (int)(xleft); i <= (int)(xRight); i++)
			{

				double u = (ColorStarU / reciprocalZ) * (texWidth - 1);
				double v = (ColorStarV / reciprocalZ) * (texHeight - 1);
				double r = ColorStarR;
				double g = ColorStarG;
				double b = ColorStarB;
				if (ZTest(i, y, reciprocalZ))
				{
					if (u < 0)
						u = 0;
					if (v < 0)
						v = 0;
					Color color = Color(texBuffer[(UINT)u + (UINT)v * (UINT)texWidth]);
					color.r *= r;
					color.g *= g;
					color.b *= b;
					//color.r = 0;
					//color.g = 1;
					//color.b = 0;
					DrawPoint(i, y, color.Get32());
				}

				ColorStarU += dxColorU;
				ColorStarV += dxColorV;
				reciprocalZ += dxReciprocalZ;
				ColorStarR += dxColorR;
				ColorStarG += dxColorG;
				ColorStarB += dxColorB;
			}
			y++;
			xleft += leftDxDivDy;//递增1个delta值，在下一次进入时绘制下一个点
			xRight += rightDxDivDy;

			xLeftReciprocalZ += leftDxDivDyReciprocalZ;
			xRightReciprocalZ += rightDxDivDyReciprocalZ;

			xLeftColorU += leftDxDivDyColorU;
			xRightColorU += rightDxDivDyColorU;

			xLeftColorV += leftDxDivDyColorV;
			xRightColorV += rightDxDivDyColorV;

			xLeftColorR += leftDxDivDyColorR;
			xLeftColorG += leftDxDivDyColorG;
			xLeftColorB += leftDxDivDyColorB;

			xRightColorR += rightDxDivDyColorR;
			xRightColorG += rightDxDivDyColorG;
			xRightColorB += rightDxDivDyColorB;
		}
	}

	void DrawTriangle(TriangleColored &t)
	{
		//判断三角形是平顶还是平底
		if (abs(t.vertex[0].y - t.vertex[1].y) < 0.00001)
		{
			if (t.vertex[2].y > t.vertex[1].y)
			{
				BottomTriangle(t);
			}
			else if (t.vertex[2].y < t.vertex[1].y)
			{
				TopTriangle(t);
			}
		}
		else if (abs(t.vertex[0].y - t.vertex[2].y) < 0.00001)
		{
			if (t.vertex[1].y > t.vertex[0].y)
			{
				BottomTriangle(t);
			}
			else if (t.vertex[1].y < t.vertex[0].y)
			{
				TopTriangle(t);
			}
		}
		else if (abs(t.vertex[1].y - t.vertex[2].y) < 0.00001)
		{
			if (t.vertex[0].y > t.vertex[1].y)
			{
				BottomTriangle(t);
			}
			else if (t.vertex[0].y < t.vertex[1].y)
			{
				TopTriangle(t);
			}
		}
		//不是平底也不是平顶,需要拆分
		else
		{
			//从上到下排列顶点
			t.SortFromTop2Bottom();
			//(y - b) / k
			double k = (t.vertex[0].y - t.vertex[2].y) * 1.0f / (t.vertex[0].x - t.vertex[2].x);
			double m = t.vertex[2].y - k*t.vertex[2].x;
			double CenterPointX = (t.vertex[1].y - m) / k;

			if ((t.vertex[0].x - t.vertex[2].x) == 0)
			{
				CenterPointX = t.vertex[0].x;
			}

			double reciprocalZ = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, 1 / t.vertex[0].z,
				t.vertex[1].x, t.vertex[1].y, 1 / t.vertex[1].z,
				t.vertex[2].x, t.vertex[2].y, 1 / t.vertex[2].z,
				CenterPointX, t.vertex[1].y);

			double r = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, t.color[0].r,
				t.vertex[1].x, t.vertex[1].y, t.color[1].r,
				t.vertex[2].x, t.vertex[2].y, t.color[2].r,
				CenterPointX, t.vertex[1].y);

			double g = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, t.color[0].g,
				t.vertex[1].x, t.vertex[1].y, t.color[1].g,
				t.vertex[2].x, t.vertex[2].y, t.color[2].g,
				CenterPointX, t.vertex[1].y);

			double b = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, t.color[0].b,
				t.vertex[1].x, t.vertex[1].y, t.color[1].b,
				t.vertex[2].x, t.vertex[2].y, t.color[2].b,
				CenterPointX, t.vertex[1].y);

			double u = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, t.uv[0].x / t.vertex[0].z,
				t.vertex[1].x, t.vertex[1].y, t.uv[1].x / t.vertex[1].z,
				t.vertex[2].x, t.vertex[2].y, t.uv[2].x / t.vertex[2].z,
				CenterPointX, t.vertex[1].y) / reciprocalZ;

			double v = InterpolateTriangle(t.vertex[0].x, t.vertex[0].y, t.uv[0].y / t.vertex[0].z,
				t.vertex[1].x, t.vertex[1].y, t.uv[1].y / t.vertex[1].z,
				t.vertex[2].x, t.vertex[2].y, t.uv[2].y / t.vertex[2].z,
				CenterPointX, t.vertex[1].y) / reciprocalZ;


			Vector3 centerPoint = { CenterPointX, t.vertex[1].y, 1 / reciprocalZ };

			//TriangleColored(Vector3 p0, Vector3 p1, Vector3 p2, Vector2 uv0, Vector2 uv1, Vector2 uv2, Color c0, Color c1, Color c2, Material *m)
			TriangleColored bottom(t.vertex[0], centerPoint, t.vertex[1], t.uv[0], { u, v }, t.uv[1], t.color[0], Color(r, g, b), t.color[1], t.material);
			TriangleColored top(centerPoint, t.vertex[1], t.vertex[2], { u, v }, t.uv[1], t.uv[2], Color(r, g, b), t.color[1], t.color[2], t.material);

			TopTriangle(top);
			BottomTriangle(bottom);
		}
	}

	void AddObjectList(GameObject *obj){
		if (obj == nullptr)
			return;
		objectList[objectListCount++] = obj;
	}

};








#endif