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


//небольшой дефайн для упрощения кода
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile *model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //массивчик для десяти шейдеров
Shader frac;
Shader cassini;




//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;
	//double fp1, fp2;

	
	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
		
		

	}

	
	//считает позицию камеры, исходя из углов поворота, вызывается движком
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



}  camera;   //создаем объект камеры


//класс недоделан!
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


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 5);
	}

	
	//рисует сферу и линии под источником света, вызывается движком
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
			//линия от источника света до окружности
				glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
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

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom=1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;


int flag = 0;
int flag2 = 0;
//обработчик движения мыши
void mouseEvent(OpenGL *ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
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


	
	//двигаем свет по плоскости, в точку где мышь
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

//обработчик вращения колеса  мыши
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

int speed = 15;          // скорость движения всеё машины
int angle2speed = 5;     //скорость поворотом колёс


int changeTex = 0;

//обработчик нажатия кнопок клавиатуры
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

//выполняется перед первым рендером
void initRender(OpenGL *ogl)
{

	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);
	
	


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH); 


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/*
	//texture1.loadTextureFromFile("textures\\texture.bmp");   загрузка текстуры из файла
	*/


	frac.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	frac.FshaderFileName = "shaders\\frac.frag"; //имя файла фрагментного шейдера
	frac.LoadShaderFromFile(); //загружаем шейдеры из файла
	frac.Compile(); //компилируем

	cassini.VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	cassini.FshaderFileName = "shaders\\cassini.frag"; //имя файла фрагментного шейдера
	cassini.LoadShaderFromFile(); //загружаем шейдеры из файла
	cassini.Compile(); //компилируем
	

	s[0].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[0].FshaderFileName = "shaders\\light.frag"; //имя файла фрагментного шейдера
	s[0].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[0].Compile(); //компилируем

	s[1].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //имя файла фрагментного шейдера
	s[1].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[1].Compile(); //компилируем

	s[2].VshaderFileName = "shaders\\v.vert"; //имя файла вершинного шейдер
	s[2].FshaderFileName = "shaders\\light_texture.frag"; //имя файла фрагментного шейдера
	s[2].LoadShaderFromFile(); //загружаем шейдеры из файла
	s[2].Compile(); //компилируем

	

	 //так как гит игнорит модели *.obj файлы, так как они совпадают по расширению с объектными файлами, 
	 // создающимися во время компиляции, я переименовал модели в *.obj_m
	
	//дорога
	
	loadModel("models\\roadY.obj_m", &roadY);
	roadYTex.loadTextureFromFile("textures//roadtex.bmp");
	roadYTex.bindTexture();

	//мост и тунели
		loadModel("models\\mosti.obj_m", &mosti);
		mostiTex.loadTextureFromFile("textures//mostiTex.bmp");
		mostiTex.bindTexture();

    //тело машины
  // loadModel("models\\body.obj_m", &prob);
  // tetxbTEx.loadTextureFromFile("textures//cartext.bmp");
  // tetxbTEx.bindTexture();

   
   //tetxbTEx2.loadTextureFromFile("textures//cartext2.bmp");
  // tetxbTEx2.bindTexture();



		//дома
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
	rec.setText("T - вкл/выкл текстур\nF-свет из камеры\nG - двигать свет по горизонтали\nG+ЛКМ двигать свет по вертекали\nP-показать/скрыть машину\nS-остановить машину\nV-ускорить машину\nB-замедлить машину\nN-вернуть обычную скорость\nC-изменить текстуру машины",0,0,0);

	
}

//////////////////////////////


double f(double p0, double p1, double p2, double p3, double t)// формула кривой Безье

{

	return (p0 * (1 - t) * (1 - t) * (1 - t)) + (3 * t * (1 - t) * (1 - t) * p1) + (3 * t * t * (1 - t) * p2) + t * t * t * p3; //посчитанная формула

}

double t_max = 0;


double Angle_Vectors(double B[])

{
	double A[] = { 1,0 };
	double length = sqrt(B[0] * B[0] + B[1] * B[1] + 0);
	B[0] = B[0] / length;
	B[1] = B[1] / length;
	//скалярное произведение векторов
	double scalar = A[0] * B[0] + A[1] * B[1];
	//модуль векторов
	double modul_A = sqrt(pow(A[0], 2) + pow(A[1], 2));
	double modul_B = sqrt(pow(B[0], 2) + pow(B[1], 2));
	//расчет косинуса угла между векторами
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
	
	
	t_max += Time / speed; //t_max становится = 1 за 10 секунд
	if (t_max > 1) t_max = 0; //после обнуляется
	double P1[] = { 0,-40,0 }; //Наши точки
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
		//glVertex3dv(P_new); //Рисуем точку P
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
	

	//загрузка тестуры машины(тела)
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




//передние колёса

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



 //задние колёса
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



  void CarView() //МАШИНА КОТОРАЯ КРУТИТСЯ
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



	//задние колёса
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


	  //загрузка тестуры машины(тела)
	 // Shader::DontUseShaders();
	  glPushMatrix();
	  glRotated(90, 0, 0, 1);
	  if (changeTex == 0)
		  tetxbTEx.bindTexture();
	  else
		  tetxbTEx2.bindTexture();
	  
	  prob.DrawObj();
	  glPopMatrix();


	  //передние колёса

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


	  //задние колёса
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

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	//GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };  // old
	//GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	//GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	//GLfloat sh = 0.1f * 256;

	GLfloat amb[] = { 0.05, 0.05, 0.05, 1. };// белые текстуры больше не зеленеют
	GLfloat dif[] = { 0.5, 0.5, 0.5, 1. };
	GLfloat spec[] = { 0.7, 0.7, 0.7, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	//размер блика
	glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//Прогать тут 
	

	s[2].UseShader();

	//передача параметров в шейдер.  Шаг один - ищем адрес uniform переменной по ее имени. 
	int location = glGetUniformLocationARB(s[2].program, "light_pos");
	//Шаг 2 - передаем ей значение
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
	
	
	//дорога

	//location = glGetUniformLocationARB(s[2].program, "Is");
	//glUniform3fARB(location, 1, 0.05, 0.05);

	glPushMatrix();
	roadYTex.bindTexture();
	roadY.DrawObj();
	glPopMatrix();

	//location = glGetUniformLocationARB(s[2].program, "Is");
	//glUniform3fARB(location, .7, .7, .7);



	//мост и тунели
	//Shader::DontUseShaders();
	glPushMatrix();
	mostiTex.bindTexture();
	mosti.DrawObj();
	glPopMatrix();

	//дома
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





	

	
	
	
	
	
////пол
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






	

	




	//машина

//	Shader::DontUseShaders();
//	glPushMatrix();
//	//glRotated(-90, 0, 0, 1);
//	wheelTex.bindTexture();
//	car1.DrawObj();
//	glPopMatrix();



	
	




	
	

	//////Рисование фрактала

	
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
	
	
	//////Овал Кассини
	
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

	
	
}   //конец тела функции


bool gui_init = false;

//рисует интерфейс, вызывется после обычного рендера
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

