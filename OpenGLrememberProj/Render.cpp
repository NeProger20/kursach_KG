#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;


//��������� ������ ��� ��������� ����
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //��������� ��� ������ ��������
Shader frac;
Shader cassini;




//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;
	//double fp1, fp2;

	
	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
		
		

	}

	
	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist*cos(fi2)*cos(fi1),
			camDist*cos(fi2)*sin(fi1),
			camDist*sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ���������!
class WASDcamera :public CustomCamera
{
public:
		
	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize()*(-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;
			
		}

		LookAt();
	}

} WASDcam;


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 5);
	}

	
	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);
		
		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 1);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale*1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����



//������ ���������� ����
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;


int flag = 0;
int flag2 = 0;
//���������� �������� ����
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01*dx;
		camera.fi2 += -0.01*dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0*dx/ogl->getWidth()/zoom;
		offsetY += 1.0*dy/ogl->getHeight()/zoom;
	}


	
	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y,60,ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}


	
	
		

}

//���������� �������� ������  ����
void mouseWheelEvent(OpenGL *ogl, int delta)
{


	float _tmpZ = delta*0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2*zoom*_tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01*delta;
}

int speed = 15;          // �������� �������� ��� ������
int angle2speed = 5;     //�������� ��������� ����


int changeTex = 0;

//���������� ������� ������ ����������
void keyDownEvent(OpenGL *ogl, int key)
{
	//if (OpenGL::isKeyPressed('L'))
	//{
	//	lightMode = !lightMode;
	//}

	if (OpenGL::isKeyPressed('T'))
	{
		textureMode = !textureMode;
	}	   

	//if (OpenGL::isKeyPressed('R'))
	//{
	//	camera.fi1 = 30;
	//	camera.fi2 = -5;
	//	camera.camDist = 15;
	//
	//	light.pos = Vector3(1, 1, 3);
	//}

	if (OpenGL::isKeyPressed('F'))
	{
		light.pos = camera.pos;
	}

	//if (OpenGL::isKeyPressed('S'))
	//{
	//	frac.LoadShaderFromFile();
	//	frac.Compile();
	//
	//	s[0].LoadShaderFromFile();
	//	s[0].Compile();
	//
	//	cassini.LoadShaderFromFile();
	//	cassini.Compile();
	//}

	//if (OpenGL::isKeyPressed('Q'))
	//	Time = 0;

	if (OpenGL::isKeyPressed('P'))
	{
		if (flag == 0)
			flag = 1;
		else
			flag = 0;

		
		
		
	}

	if (OpenGL::isKeyPressed('C'))//change text car
	{
		if (changeTex == 0)
			changeTex = 1;
		else
			changeTex = 0;



	}

	if (OpenGL::isKeyPressed('S'))
	{
		if (flag2 == 0)
			flag2 = 1;
		else
			flag2 = 0;




	}

	
	if (OpenGL::isKeyPressed('V'))//bistro
	{
		speed = 5;
		angle2speed = 50;




	}
	if (OpenGL::isKeyPressed('B'))//medleno
	{
		speed = 35;
		angle2speed = 2;

	


	}
	if (OpenGL::isKeyPressed('N'))//obichno
	{
		 speed = 15;
		 angle2speed = 5;


		 
	}
	

	




}



void keyUpEvent(OpenGL *ogl, int key)
{
	

}


void DrawQuad()
{
	double A[] = { 0,0 };
	double B[] = { 1,0 };
	double C[] = { 1,1 };
	double D[] = { 0,1 };
	glBegin(GL_QUADS);
	glColor3d(.5, 0, 0);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 0);
	glVertex2dv(A);
	glTexCoord2d(1, 0);
	glVertex2dv(B);
	glTexCoord2d(1, 1);
	glVertex2dv(C);
	glTexCoord2d(0, 1);
	glVertex2dv(D);
	glEnd();
}


ObjFile objModel, pol, body, FrontRightWheel,probW,roadY,prob, mosti,home, wrld, polM;

Texture car1Tex,polTex, wheelTex, tetxbTEx, roadYTex, mostiTex, homeTex, wrldTex, polMTEx, tetxbTEx2;

//����������� ����� ������ ��������
void initRender(OpenGL *ogl)
{

	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);
	
	


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH); 


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   �������� �������� �� �����
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	frac.FshaderFileName = "shaders\\frac.frag"; //��� ����� ������������ �������
	frac.LoadShaderFromFile(); //��������� ������� �� �����
	frac.Compile(); //�����������

	cassini.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	cassini.FshaderFileName = "shaders\\cassini.frag"; //��� ����� ������������ �������
	cassini.LoadShaderFromFile(); //��������� ������� �� �����
	cassini.Compile(); //�����������
	

	s[0].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[0].FshaderFileName = "shaders\\light.frag"; //��� ����� ������������ �������
	s[0].LoadShaderFromFile(); //��������� ������� �� �����
	s[0].Compile(); //�����������

	s[1].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //��� ����� ������������ �������
	s[1].LoadShaderFromFile(); //��������� ������� �� �����
	s[1].Compile(); //�����������

	s[2].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[2].FshaderFileName = "shaders\\light_texture.frag"; //��� ����� ������������ �������
	s[2].LoadShaderFromFile(); //��������� ������� �� �����
	s[2].Compile(); //�����������

	

	 //��� ��� ��� ������� ������ *.obj �����, ��� ��� ��� ��������� �� ���������� � ���������� �������, 
	 // ������������ �� ����� ����������, � ������������ ������ � *.obj_m
	
	//������
	
	loadModel("models\\roadY.obj_m", &roadY);
	roadYTex.loadTextureFromFile("textures//roadtex.bmp");
	roadYTex.bindTexture();

	//���� � ������
		loadModel("models\\mosti.obj_m", &mosti);
		mostiTex.loadTextureFromFile("textures//mostiTex.bmp");
		mostiTex.bindTexture();

 //���� ������
 loadModel("models\\body.obj_m", &prob);
 tetxbTEx.loadTextureFromFile("textures//cartext.bmp");
 tetxbTEx.bindTexture();

 
 tetxbTEx2.loadTextureFromFile("textures//cartext2.bmp");
 tetxbTEx2.bindTexture();



		//����
		loadModel("models\\home.obj_m", &home);
		homeTex.loadTextureFromFile("textures//homeTex.bmp");
		homeTex.bindTexture();
	
		//world
		loadModel("models\\wrld.obj_m", &wrld);
		wrldTex.loadTextureFromFile("textures//wrldtex.bmp");
		wrldTex.bindTexture();
	
		//podstavka
		
			loadModel("models\\polM.obj_m", &polM);
			polMTEx.loadTextureFromFile("textures//rrr.bmp");
			polMTEx.bindTexture();



	//glActiveTexture(GL_TEXTURE0);
	loadModel("models\\front-right-wheel.obj_m", &FrontRightWheel);
	wheelTex.loadTextureFromFile("textures//wheel.bmp");
	wheelTex.bindTexture();





	//glActiveTexture(GL_TEXTURE0);
	
	

		
     
	




	


	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 200);
	rec.setPosition(10, ogl->getHeight() - 100-10);
	rec.setText("T - ���/���� �������\nF-���� �� ������\nG - ������� ���� �� �����������\nG+��� ������� ���� �� ���������\nP-��������/������ ������\nS-���������� ������\nV-�������� ������\nB-��������� ������\nN-������� ������� ��������\nC-�������� �������� ������",0,0,0);

	
}

//////////////////////////////


double f(double p0, double p1, double p2, double p3, double t)// ������� ������ �����

{

	return (p0 * (1 - t) * (1 - t) * (1 - t)) + (3 * t * (1 - t) * (1 - t) * p1) + (3 * t * t * (1 - t) * p2) + t * t * t * p3; //����������� �������

}

double t_max = 0;


double Angle_Vectors(double B[])

{
	double A[] = { 1,0 };
	double length = sqrt(B[0] * B[0] + B[1] * B[1] + 0);
	B[0] = B[0] / length;
	B[1] = B[1] / length;
	//��������� ������������ ��������
	double scalar = A[0] * B[0] + A[1] * B[1];
	//������ ��������
	double modul_A = sqrt(pow(A[0], 2) + pow(A[1], 2));
	double modul_B = sqrt(pow(B[0], 2) + pow(B[1], 2));
	//������ �������� ���� ����� ���������
	double cos_vec = scalar / (modul_A * modul_B);
	return acos(cos_vec);

}

double P_before[] = { 0,0,0 };
double vector_bef[] = { 0,0 };
double vector_new[] = { 0,0 };
double angle2 = 0;
double angle3 = 0;





void whl() {

	//Shader::DontUseShaders();

	//s[1].UseShader();
	//int l = glGetUniformLocationARB(s[1].program, "wheel");
	//glUniform1fARB(1, 0);
	glPushMatrix();
	wheelTex.bindTexture();
	FrontRightWheel.DrawObj();
	glPopMatrix();
}

double P_new[3];
double angle = 0;



void Bez3()

{
	glPushMatrix();
	
	
	t_max += Time / speed; //t_max ���������� = 1 �� 10 ������
	if (t_max > 1) t_max = 0; //����� ����������
	double P1[] = { 0,-40,0 }; //���� �����
	double P2[] = { 35,-10,0 };
	double P3[] = { -35,20,0 };
	double P4[] = { 35,60,0 };
	
	double P_old[3];
	
	glBegin(GL_LINE_STRIP);

	for (double t = 0; t <= t_max; t += 0.001)
	{
		P_new[0] = f(P1[0], P2[0], P3[0], P4[0], t);
		P_new[1] = f(P1[1], P2[1], P3[1], P4[1], t);
		P_new[2] = f(P1[2], P2[2], P3[2], P4[2], t);
		//glVertex3dv(P_new); //������ ����� P
	}
	glEnd();

	P_old[0] = f(P1[0], P2[0], P3[0], P4[0], t_max - 0.001);
	P_old[1] = f(P1[1], P2[1], P3[1], P4[1], t_max - 0.001);

	vector_new[0] = P_new[0] - P_old[0];
	vector_new[1] = P_new[1] - P_old[1];
	 angle = Angle_Vectors(vector_new);
	angle = angle * 180 / PI;

	glTranslated(P_new[0], P_new[1], P_new[2]);
	glRotated(angle, 0, 0, 1);


	
	glRotated(270, 0, 0, 1);
     
                                                         //car save front Y, top Z
	

	//�������� ������� ������(����)
//Shader::DontUseShaders();
glPushMatrix();
glRotated(90, 0, 0, 1);

if(changeTex==0)
tetxbTEx.bindTexture();
else
tetxbTEx2.bindTexture();
prob.DrawObj();
glPopMatrix();
	



if (angle2 >= 360)
	angle2 = 0;
angle2 += angle2speed;




//�������� �����

glPushMatrix();
 glTranslated(0.9, 1.48, 0.33); 
 glRotated(90, 0, 0, 1);
 glRotated(angle2, 0, 1, 0);
 whl();
 //FrontRightWheel.DrawObj();

 


 glTranslated(0, 1.85, 0); //1.83683  -3.01751 
 glRotated(180, 0, 0, 1);
 whl();//FrontRightWheel.DrawObj();
 glPopMatrix();



 //������ �����
 glPushMatrix();
 glTranslated(0.9, -1.35, 0.33); //1.83683  -3.01751 
 glRotated(90, 0, 0, 1);
 glRotated(angle2, 0, 1, 0);
 whl();//FrontRightWheel.DrawObj();


 glTranslated(0, 1.85, 0); //1.83683  -3.01751 
 glRotated(180, 0, 0, 1);
 whl();//FrontRightWheel.DrawObj();
 glPopMatrix();
 



	
	
	



	vector_bef[0] = vector_new[0];
	vector_bef[1] = vector_new[1];
	P_before[0] = P_new[0];
	P_before[1] = P_new[1];
	P_before[2] = P_new[2];
	//glColor3d(1, 0, 0);
	//glBegin(GL_POINTS);
	//glVertex3dv(P1);
	//glVertex3dv(P2);
	//glVertex3dv(P3);
	//glVertex3dv(P4);
	//glEnd();
	glPopMatrix();
}

double angle5 = 0;



  void CarView() //������ ������� ��������
{


	glPushMatrix();
	glTranslated(30, -5, 3);

	if (angle5 >= 360)
		angle5 = 0;
	angle5 += 0.7;
	glRotated(angle5, 0, 0, 1);


	//podstavka
	
	//Shader::DontUseShaders();
	glPushMatrix();
	polMTEx.bindTexture();
	polM.DrawObj();
	glPopMatrix();

	
	//body car
	//Shader::DontUseShaders();
	glPushMatrix();
	glRotated(90, 0, 0, 1);
	if (changeTex == 0)
		tetxbTEx.bindTexture();
	else
		tetxbTEx2.bindTexture();
	prob.DrawObj();
	glPopMatrix();

	//perednie kolesa
	glPushMatrix();
	glTranslated(0.9, 1.48, 0.33);
	glRotated(90, 0, 0, 1);
	//glRotated(angle2, 0, 1, 0);
	whl();
	//FrontRightWheel.DrawObj();




	glTranslated(0, 1.85, 0); //1.83683  -3.01751 
	glRotated(180, 0, 0, 1);
	whl();//FrontRightWheel.DrawObj();
	glPopMatrix();



	//������ �����
	glPushMatrix();
	glTranslated(0.9, -1.35, 0.33); //1.83683  -3.01751 
	glRotated(90, 0, 0, 1);
	//glRotated(angle2, 0, 1, 0);
	whl();//FrontRightWheel.DrawObj();


	glTranslated(0, 1.85, 0); //1.83683  -3.01751 
	glRotated(180, 0, 0, 1);
	whl();//FrontRightWheel.DrawObj();
	glPopMatrix();



	glPopMatrix();
}

  void StopCar() 
  {

	  glTranslated(P_new[0], P_new[1], P_new[2]);
	  glRotated(angle, 0, 0, 1);
	  glRotated(270, 0, 0, 1);


	  //�������� ������� ������(����)
	 // Shader::DontUseShaders();
	  glPushMatrix();
	  glRotated(90, 0, 0, 1);
	  if (changeTex == 0)
		  tetxbTEx.bindTexture();
	  else
		  tetxbTEx2.bindTexture();
	  
	  prob.DrawObj();
	  glPopMatrix();


	  //�������� �����

	  glPushMatrix();
	  glTranslated(0.9, 1.48, 0.33);
	  glRotated(90, 0, 0, 1);
	  glRotated(angle2, 0, 1, 0);
	  whl();
	  //FrontRightWheel.DrawObj();

	  glTranslated(0, 1.85, 0); //1.83683  -3.01751 
	  glRotated(180, 0, 0, 1);
	  whl();//FrontRightWheel.DrawObj();
	  glPopMatrix();


	  //������ �����
	  glPushMatrix();
	  glTranslated(0.9, -1.35, 0.33); //1.83683  -3.01751 
	  glRotated(90, 0, 0, 1);
	  glRotated(angle2, 0, 1, 0);
	  whl();//FrontRightWheel.DrawObj();


	  glTranslated(0, 1.85, 0); //1.83683  -3.01751 
	  glRotated(180, 0, 0, 1);
	  whl();//FrontRightWheel.DrawObj();
	  glPopMatrix();

  }

void Render(OpenGL *ogl)
{   
	
	tick_o = tick_n;
	tick_n = GetTickCount();
	Time = (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	//GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };  // old
	//GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	//GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	//GLfloat sh = 0.1f * 256;

	GLfloat amb[] = { 0.05, 0.05, 0.05, 1. };// ����� �������� ������ �� ��������
	GLfloat dif[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat spec[] = { 0.7, 0.7, 0.7, 1. };
	GLfloat sh = 0.1f * 256;

	//�������
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//��������
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//����������
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//������ �����
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//������� ��� 
	

	s[2].UseShader();

	//�������� ���������� � ������.  ��� ���� - ���� ����� uniform ���������� �� �� �����. 
	int location = glGetUniformLocationARB(s[2].program, "light_pos");
	//��� 2 - �������� �� ��������
	glUniform3fARB(location, light.pos.X(), light.pos.Y(), light.pos.Z());

	location = glGetUniformLocationARB(s[2].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[2].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[2].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[2].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[2].program, "md");
	glUniform3fARB(location, 0.5, 0.5, 0.5);

	location = glGetUniformLocationARB(s[2].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[2].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());

	location = glGetUniformLocationARB(s[2].program, "iTexture0");
	glUniform1iARB(location, 0);









	if (flag == 1)
		CarView();
	
	
	//������

	//location = glGetUniformLocationARB(s[2].program, "Is");
	//glUniform3fARB(location, 1, 0.05, 0.05);

	glPushMatrix();
	roadYTex.bindTexture();
	roadY.DrawObj();
	glPopMatrix();

	//location = glGetUniformLocationARB(s[2].program, "Is");
	//glUniform3fARB(location, .7, .7, .7);



	//���� � ������
	//Shader::DontUseShaders();
	glPushMatrix();
	mostiTex.bindTexture();
	mosti.DrawObj();
	glPopMatrix();

	//����
	//Shader::DontUseShaders();
	glPushMatrix();
	homeTex.bindTexture();
	home.DrawObj();
	glPopMatrix();

	//wrld
	//fShader::DontUseShaders();
	glPushMatrix();
	wrldTex.bindTexture();
	wrld.DrawObj();
	glPopMatrix();




	





	
	if (flag2 == 0)
	Bez3();
	else
	{
		StopCar();
	}





	//





	

	
	
	
	
	
////���
//Shader::DontUseShaders();
//glPushMatrix();
//glTranslated(0, 0, 0);
//polTex.bindTexture();
//pol.DrawObj();
//glPopMatrix();


	//pol.DrawObj();
	//roadY.DrawObj();


//	s[1].UseShader();
//	int l = glGetUniformLocationARB(s[1].program, "roadtex");
//	glUniform1fARB(1, 0);






	

	




	//������

//	Shader::DontUseShaders();
//	glPushMatrix();
//	//glRotated(-90, 0, 0, 1);
//	wheelTex.bindTexture();
//	car1.DrawObj();
//	glPopMatrix();



	
	




	
	

	//////��������� ��������

	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/
	
	
	//////���� �������
	
	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/

	
	
	

	
	//Shader::DontUseShaders();

	
	
}   //����� ���� �������


bool gui_init = false;

//������ ���������, ��������� ����� �������� �������
void RenderGUI(OpenGL *ogl)
{
	
	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);
	

	glActiveTexture(GL_TEXTURE0);
	rec.Draw();


		
	Shader::DontUseShaders(); 



	
}

void resizeEvent(OpenGL *ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 200);
}

