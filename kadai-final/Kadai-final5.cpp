#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#pragma comment(lib,"winmm.lib")
#include <windows.h>
#include "glut.h"
#include "freeglut.h"
#include <GL/gl.h>
#include <math.h>
#include <string>
using namespace std;

#define pi 3.1415
#define T 10
#define RandomTime 6000
#define RandomTime2 8000
#define ItemTime 20000
#define	ObstacleTime 5000

LPCWSTR Sound, SoundSub;

int rver = -1, rhor = -1, rver2 = 2, rhor2 = 2, rpver = 1, rphor = 1,
	rCoinNum = 1, rCoinNum2 = 1, ScoreCoin = 0, ScoreDis = 0, ResultCoin = 0, ResultDis = 0,
	rover[8] = { 3, 2, 1, 0, -1, -2, -3, -4 }, rohor[8] = { 3, 2, 1, 0, -1, -2, -3, -4 },
	rObsNum = 0, UserLife = 3, ObsNum = 1;
double TraDis = -15.0, LoadDis = -15.0, ItemDis = -90.0, ObsDis = -90.0, 
	   RotAngle = 0.0, UserMove = 0.0, UserRot = 0.0, UserVer = 0.0, UserHor = 0.0,
	   CoinDis[] = { -90.0, -100.0, -110.0, -120.0, 130.0 }, 
	   CoinDis2[] = { -90.0, -100.0, -110.0, -120.0, 130.0 };
bool ItemFlag = false, ItemUseFlag = false, ObsFlag = false, GameOverFlag = false;

float yellow[] = { 1.0, 1.0, 0.0, 1.0 };
float white[] = { 1.0, 1.0, 1.0, 1.0 };
float black[] = { 0.0, 0.0, 0.0, 1.0 };
float gray[] = { 0.5, 0.5, 0.5, 1.0 };
float dark_gray[] = { 0.3, 0.3, 0.3, 1.0 };
float light_gray[] = { 0.9, 0.9, 0.9, 1.0 };
float red[] = { 1.0, 0.0, 0.0, 1.0 };
float building_color[] = { 0.9, 0.3, 0.3, 1.0 };
float amber[] = { 1.0, 0.5, 0.0, 1.0 };

float positionLR[][4] = { {25.0, 6.0, -10.0, 0.0}, {5.0, 6.0, -10.0, 1.0} };
float position_default[] = { 15.0, 6.0, -5.0, 1.0 };
float light_position[] = { 15.0, 10.0, 10.0, 0.0 };
float position_center[] = { 15.0, 0.0, -10.0, 1.0 };
float new_params[4];

float* Multiply(float params[], float val) {
	int i;
	for (i = 0; i < 3; i++)
		new_params[i] = params[i] * val;
	return new_params;
}
void Lighting(GLenum light, float color[], float diffuse, float specular, float ambient) {
	glLightfv(light, GL_DIFFUSE, Multiply(color, diffuse));
	glLightfv(light, GL_SPECULAR, Multiply(color, specular));
	glLightfv(light, GL_AMBIENT, Multiply(color, ambient));
}
void Prism(int N) {
	/*
	N角柱を描画する関数
	*/
	double theta = 2 * pi / N;
	int i;
	//上面
	glBegin(GL_POLYGON);
	glNormal3f(0, +1, 0);
	for (i = 0; i < N; i++) {
		glVertex3f(cos(i * theta), 1.0, sin(i * theta));
	}
	glEnd();
	//下面
	glBegin(GL_POLYGON);
	glNormal3f(0, -1, 0);
	for (i = 0; i < N; i++) {
		glVertex3f(cos(i * theta), -1.0, sin(i * theta));
	}
	glEnd();
	//側面
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= N; i++) {
		glNormal3f(cos((i - 0.5) * theta), 0, sin((i - 0.5) * theta));
		glVertex3f(cos(i * theta), +1.0, sin(i * theta));
		glVertex3f(cos(i * theta), -1.0, sin(i * theta));
	}
	glEnd();
}
void Rectangular(float ver, float hor) {
	/*
	ver×hor×2の直方体を描画する
	*/
	int i;
	ver /= 2, hor /= 2;
	float idx[][3] = { {ver, 0.0, hor}, {-ver, 0.0, hor}, {-ver, 0.0, -hor}, {ver, 0.0, -hor}, {ver, 0.0, hor} };
	glBegin(GL_POLYGON);//上面 
	glNormal3f(0, +1, 0);
	for (i = 0; i < 4; i++) {
		glVertex3f(idx[i][0], 1.0, idx[i][2]);
	}
	glEnd();
	glBegin(GL_POLYGON);//下面
	glNormal3f(0, -1, 0);
	for (i = 0; i < 4; i++) {
		glVertex3f(idx[i][0], -1.0, idx[i][2]);
	}
	glEnd();
	glBegin(GL_QUAD_STRIP);//側面
	for (i = 0; i <= 4; i++) {
		glNormal3f(idx[i][0], 0.0, idx[i][2]);
		glVertex3f(idx[i][0], 1.0, idx[i][2]);
		glVertex3f(idx[i][0], -1.0, idx[i][2]);
	}
	glEnd();
}
void Plane(void) {
	/*
	平面を描画する。
	*/
	float coordinate[][3] = { {1, 0, 1}, {-1, 0, 1}, {-1, 0, -1}, {1, 0, -1} };
	glBegin(GL_POLYGON);
	glNormal3f(0, 1.0, 0);
	for (int i = 0; i < 4; i++)
		glVertex3f(coordinate[i][0], -7.0, coordinate[i][2]);
	glEnd();
}
void Buildings(void) {
	/*
	建物を描画する関数
	*/
	int i, j, sidx = 0;
	for (j = 0; j < 2; j++) {
		glPushMatrix();
		glTranslatef(30.0 * j + TraDis, 0.0, TraDis);
		for (i = 0; i < 20; i++) {
			glPushMatrix();
			glTranslatef(-9.0 * i, 0.0, -9.0 * i);
			glScalef(7.0, 8.0, 7.0);
			Prism(4);
			glPopMatrix();
		}
		glPopMatrix();
	}
	glLightfv(GL_LIGHT0, GL_POSITION, position_default);
}
void Coin(void) {
	/*
	コインを描画する関数
	*/
	// 外枠
	glPushMatrix();
	glutSolidTorus(0.3, 2.0, 30, 30);
	glPopMatrix();
	// 側面
	glPushMatrix();
	glRotatef(90, 1, 0, 0);
	glScalef(2.0, 0.2, 2.0);
	Prism(30);
	glPopMatrix();
	// 中心のマーク
	glPushMatrix();
	glRotatef(45, 0, 1, 0);
	glScalef(0.5, 1.5, 0.5);
	Prism(4);
	glPopMatrix();
}
void Trapezoid(float top, float under) {
	/*
	Create a trapezoid with upper base [float top], lower base [float under], and height [= 2].

	Parameter:
		top [float]: upper base
		under [float]: under base
	*/
	int i;
	float body_top[][3] = { {top, 1, top}, {-top, 1, top}, {-top, 1, -top}, {top, 1, -top}, {top, 1, top} };
	float body_under[][3] = { {under, -1, under}, {-under, -1, under}, {-under, -1, -under}, {under, -1, -under}, {under, -1, under} };

	glBegin(GL_POLYGON);
	glNormal3f(0, 1, 0);
	for (i = 0; i < 4; i++) {
		glVertex3f(body_top[i][0], body_top[i][1], body_top[i][2]);
	}
	glEnd();
	glBegin(GL_POLYGON);
	glNormal3f(0, -1, 0);
	for (i = 0; i < 4; i++) {
		glVertex3f(body_under[i][0], body_under[i][1], body_under[i][2]);
	}
	glEnd();
	glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= 4; i++) {
		glNormal3f(body_under[i][0] - body_top[i][0], 1.5, body_under[i][2] - body_top[i][2]);
		glVertex3f(body_top[i][0], body_top[i][1], body_top[i][2]);
		glVertex3f(body_under[i][0], body_under[i][1], body_under[i][2]);
	}
	glEnd();
}
void PenguinLeg(void) {
	/*
	ペンギンの足を描画する関数
	*/
	glPushMatrix(); // ankle
	Rectangular(0.30, 0.30); // height=2
	glPopMatrix();
	glPushMatrix(); // leg
	glTranslatef(0.3, -1.0, 0.0);
	glRotatef(90, 0, 0, 1);
	glScalef(1.0, 0.6, 1.0);
	Rectangular(0.1, 0.6); // height=2
	glPopMatrix();
}
void Penguin(void) {
	/*
	ペンギンを描画する関数
	*/
	// head
	Lighting(GL_LIGHT0, black, 1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslated(0, 5, 0);
	glScalef(2.0, 2.0, 2.0);
	Trapezoid(1.0, 1.2); // top: 2.0, under: 2.4, height: 4
	glPopMatrix();
	// eyes
	Lighting(GL_LIGHT0, white, 1.0, 1.0, 1.0);
	glShadeModel(GL_SMOOTH);
	for (int i = -1; i < 2; i += 2) {
		glPushMatrix();
		glRotatef(90, 0, 1, 0);
		glTranslatef(1 * i, 5, 2.2);
		glutSolidSphere(0.3, 10, 10);
		Lighting(GL_LIGHT0, black, 1.0, 1.0, 1.0);
		glTranslatef(0.0, 0.0, 0.2);
		glutSolidSphere(0.15, 10, 10);
		glPopMatrix();
		Lighting(GL_LIGHT0, white, 1.0, 1.0, 1.0);
	}
	// mouth
	Lighting(GL_LIGHT0, yellow, 1.0, 1.0, 1.0);
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 4, 2);
	glutSolidOctahedron();
	glPopMatrix();
	Lighting(GL_LIGHT0, white, 1.0, 1.0, 1.0);
	// body
	glPushMatrix();
	glScalef(2.0, 3.0, 2.0);
	Trapezoid(1.0, 1.2); // top:2.0, under: 2.4, height: 6
	glPopMatrix();
	//hand
	Lighting(GL_LIGHT0, dark_gray, 1.0, 1.0, 1.0);
	// left hand
	glPushMatrix();
	glTranslatef(0.0, 2.0, 2.5);
	glRotatef(90, 0, 1, 0);
	glRotatef(-30, 0, 0, 1);
	glScalef(1.0, 3.0, 1.0);
	Rectangular(0.2, 2.0);
	glPopMatrix();
	// right hand
	glPushMatrix();
	glTranslatef(0.0, 2.0, -2.5);
	glRotatef(90, 0, 1, 0);
	glRotatef(30, 0, 0, 1);
	glScalef(1.0, 3.0, 1.0);
	Rectangular(0.2, 2.0);
	glPopMatrix();
	// leg
	glPushMatrix();
	glTranslatef(0.0, -4.0, -0.8);
	PenguinLeg();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, -4.0, 0.8);
	PenguinLeg();
	glPopMatrix();
}
void Heart(void) {
	/*
	ハートを描画する関数
	*/
	glBegin(GL_POLYGON);
	for (float x = -1.139; x <= 1.139; x += 0.001) {
		float delta = cbrt(pow(x, 2)) * cbrt(pow(x, 2)) - 4 * pow(x, 2) + 4;
		float y1 = (cbrt(pow(x, 2)) + sqrt(delta)) / 2;
		float y2 = (cbrt(pow(x, 2)) - sqrt(delta)) / 2;
		glVertex2f(x, y1);
		glVertex2f(x, y2);
	}
	glEnd();
}
static void DrawString(string str, int w, int h, int x0, int y0) {
	/*
	文字列(Times Roman font)を描画する関数
	*/
	glDisable(GL_LIGHTING);

	// モード変更
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	// 画面上にテキストを表示
	glRasterPos2f(x0, y0);
	int size = (int)str.size();
	glutBitmapString(GLUT_BITMAP_TIMES_ROMAN_24, reinterpret_cast<const unsigned char*>(str.c_str()));
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_LIGHTING);
}
void DrawString3D(const char* str, float charSize, float lineWidth){
	/*
	3Dの文字を描画する関数
	*/
	glPushMatrix();
	glPushAttrib(GL_LINE_BIT);
	glScaled(0.001 * charSize, 0.001 * charSize, 0.01);
	glLineWidth(lineWidth);
	while (*str) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *str);
		++str;
	}
	glPopAttrib();
	glPopMatrix();
}
void GameSound(const wchar_t filename[]) {
	/*
	windows APIを用いて音声を再生する関数
	*/
	DWORD dwReadSize;
	HANDLE FileHandle = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwFileSize = GetFileSize(FileHandle, NULL);
	Sound = (LPCWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	ReadFile(FileHandle, (LPVOID)Sound, dwFileSize, &dwReadSize, NULL);
	CloseHandle(FileHandle);
	PlaySound(Sound, NULL, SND_MEMORY | SND_ASYNC);
}
void display(void) {
	int i;
	// 初期化
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0);

	// 行列演算モード
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// ビルを生成
	glShadeModel(GL_FLAT);
	Lighting(GL_LIGHT0, building_color, 1.0, 1.0, 1.0);
	glLightfv(GL_LIGHT0, GL_POSITION, position_center);
	glPushMatrix();
	glTranslatef(4.0, 0.0, 4.0);
	// buildings関数は z = -x の方向に伸びていくので回転して座標軸を調整する。
	glRotatef(-45, 0, 1, 0);
	Buildings();
	glPopMatrix();

	// 道路を生成
	// 基盤
	Lighting(GL_LIGHT0, gray, 1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(14.8, 0.0, -60.0);
	glScalef(10.0, 1.0, 50.0);
	Plane();
	glPopMatrix();
	// 白線
	Lighting(GL_LIGHT0, white, 1.0, 1.0, 1.0);
	glPushMatrix();
	glTranslatef(14.8, 0.1, LoadDis);
	for (i = 0; i < 10; i++) {
		glPushMatrix();
		glTranslatef(0.0, 1.0, -8.0 * i);
		glScalef(0.3, 1.0, 2.0);
		Plane();
		glPopMatrix();
	}
	glPopMatrix();
	// Game Overの時
	if (GameOverFlag) {
		glPushMatrix();
		glColor3f(1.0, 0.0, 0.0);
		glTranslatef(10.5, 0.0, -11.0);
		Lighting(GL_LIGHT0, red, 1.0, 1.0, 1.0);
		DrawString3D("!!!GAME OVER!!!", 9.5, 10);
		DrawString("[Result]\nCoin: $" + to_string(ResultCoin) + "\nDistance Score: " + to_string(ResultDis / 10) + "m", 20, 20, 8, 13);
		glPopMatrix();
		glPushMatrix();
		glTranslatef(15.0, -3.5, -15.0);
		glRotatef(-90, 0, 1, 0);
		Penguin();
		glPopMatrix();

		glFlush();
		return;
	}
	// コインを生成
	glShadeModel(GL_SMOOTH);
	Lighting(GL_LIGHT0, yellow, 1.0, 1.0, 1.0);
	glLightfv(GL_LIGHT0, GL_POSITION, positionLR[0]);
	for (i = 0; i < rCoinNum; i++) {
		glPushMatrix();
		glTranslatef(15.0, 0.0, -15.0);
		glTranslatef(rhor, rver, CoinDis[i]);
		glScalef(0.5, 0.5, 0.5);
		glRotatef(30, 0, 1, 0);
		Coin();
		glPopMatrix();
	}
	for (i = 0; i < rCoinNum2; i++) {
		glPushMatrix();
		glTranslatef(15.0, 0.0, -15.0);
		glTranslatef(rhor2, rver2, CoinDis2[i]);
		glScalef(0.5, 0.5, 0.5);
		glRotatef(30, 0, 1, 0);
		Coin();
		glPopMatrix();
	}
	glShadeModel(GL_FLAT);

	// Penguinを生成
	glLightfv(GL_LIGHT0, GL_POSITION, position_default);
	glPushMatrix();
	glTranslatef(15.0, 0.0, -15.0); // initialization
	if (ObsFlag) {
		glEnable(GL_LIGHT1);
		Lighting(GL_LIGHT1, red, 1.0, 1.0, 1.0);
		glLightfv(GL_LIGHT1, GL_POSITION, position_default);
		glTranslatef(0.0, 0.0, 5.0);
		glRotatef(RotAngle, 0, 1, 0);
		glScalef(0.3, 0.3, 0.3);
		Penguin();
		glDisable(GL_LIGHT1);
	}
	else {
		glTranslatef(0.0, UserVer, 0.0); // User input
		glTranslatef(UserHor, 0.0, 0.0); // User input 
		glRotatef(90, 0, 1, 0);
		glRotatef(UserRot, 1, 0, 0);	 // User input
		glScalef(0.3, 0.3, 0.3);
		glRotatef(-90, 0, 0, 1);
		if (ItemUseFlag) glRotatef(RotAngle, 0, 1, 0);
		Penguin();
	}
	glPopMatrix();

	// Penguin Item
	Lighting(GL_LIGHT0, black, 1.0, 1.0, 1.0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glPushMatrix();
		glTranslatef(0.0, 1.5, 0.0);
		glPushMatrix();
			glTranslatef(15.0, -1.0, -15.0); // initialization
			glTranslatef(rphor, rpver, ItemDis);
			glRotatef(-90, 0, 1, 0);
			glScalef(0.1, 0.1, 0.1);
			Penguin();
		glPopMatrix();
	glPopMatrix();
	// 取得したItemを表示する部分
	Lighting(GL_LIGHT0, light_gray, 1.0, 1.0, 1.0);
	glPushMatrix();
		glTranslatef(15.0, 6.5, -15.0);
		glRotatef(90, 1, 0, 0);
		Plane();
	glPopMatrix();

	// Itemを取得した場合に画面に表示する
	if (ItemFlag) {
		glPushMatrix();
		glTranslatef(15.0, 4.6, -15.0);
		glRotatef(-90, 0, 1, 0);
		glScalef(0.1, 0.1, 0.1);
		Penguin();
		glPopMatrix();
	}		
	// おじゃまアイテムTeaPot
	Lighting(GL_LIGHT0, amber, 1.0, 1.0, 1.0);
	for (i = 0; i < ObsNum; i++) {
		glPushMatrix();
		glTranslatef(15.0, 0.0, -15.0);
		glTranslatef(rohor[i], rover[i], ObsDis);
		glRotatef(RotAngle, 0, 1, 0);
		glutSolidTeapot(0.8);
		glPopMatrix();
	}
	// Heart Life
	glColor3f(0.0, 0.0, 1.0);
	Lighting(GL_LIGHT0, red, 1.0, 1.0, 1.0);
	for (i = 0; i < UserLife; i++) {
		glPushMatrix();
		glTranslatef(9.6 + 1.2 * i, -5.9, -15.0);
		glScalef(0.4, 0.4, 0.4);
		Heart();
		glPopMatrix();
	}
	// Scoreを表字
	DrawString("Coin: $" + to_string(ScoreCoin), 20, 20, 5, 1);
	DrawString("Distance Score: " + to_string(ScoreDis / 10) + "m", 20, 20, 9, 1);

	glFlush();
}
void RandomObstacle(int timer) {
	/*
	障害物の座標を生成、初期化する関数
	*/
	int i;
	if (timer != 5) return;
	ObsNum = rand() % 8 + 1;
	for (i = 0; i < ObsNum; i++) {
		rover[i] = rand() % 7 - 3; // [-5, +5]
		rohor[i] = rand() % 9 - 4;  // [-4, +4]
	}
	ObsDis = -90.0;
	ObsFlag = false;
	glutTimerFunc(ObstacleTime, RandomObstacle, 5);
	glutPostRedisplay();
}
void RandomItem(int timer) {
	/*
	アイテムの座標を生成、初期化する関数
	*/
	if (timer != 4) return;
	if (ItemUseFlag) ItemUseFlag = false;
	rpver = rand() % 7 - 3; // [-3, +3]
	rphor = rand() % 9 - 4;  // [-4, +4]
	ItemDis = -100.0;
	if (GameOverFlag) exit(0);
	glutTimerFunc(ItemTime, RandomItem, 4);
	glutPostRedisplay();
}
void RandomCoin(int timer) {
	/*
	コインの座標を生成、初期化する関数
	*/
	if (timer != 2) return;
	srand(rand() % 4);
	rver = rand() % 7 - 3; // [-5, +5]
	rhor = rand() % 9 - 4;  // [-4, +4]
	rCoinNum = rand() % 5 + 1; // [1, 5]
	for (int i = 0; i < rCoinNum; i++)
		CoinDis[i] = -90.0 - 10.0 * i;
	glutTimerFunc(RandomTime, RandomCoin, 2);
	glutPostRedisplay();
}
void RandomCoin2(int timer) {
	/*
	コインの座標を生成、初期化する関数
	*/
	if (timer != 3) return;
	rver2 = rand() % 7 - 3; // [-5, +5]
	rhor2 = rand() % 9 - 4;  // [-4, +4]
	rCoinNum2 = rand() % 5 + 1; // [1, 5]
	for (int i = 0; i < rCoinNum2; i++)
		CoinDis2[i] = -90.0 - 10.0 * i;
	glutTimerFunc(RandomTime2, RandomCoin2, 3);
	glutPostRedisplay();
}
void IncAngle(int timer) {
	int i;
	if (timer != 1) return;

	// コインとUserの接触判定
	if (fabs(rver - UserVer) < 2.0 && fabs(rhor - UserHor) < 2.0) {
		for (i = 0; i < rCoinNum; i++) {
			if (CoinDis[i] > -5.0 && CoinDis[i] < 0.0) {
				CoinDis[i] = 50.0;
				ScoreCoin += 10;
				GameSound(L"coin.wav");
			}
		}
	}
	if (fabs(rver2 - UserVer) < 2.0 && fabs(rhor2 - UserHor) < 2.0) {
		for (i = 0; i < rCoinNum2; i++) {
			if (CoinDis2[i] > -5.0 && CoinDis2[i] < 0.0) {
				CoinDis2[i] = 50.0;
				ScoreCoin += 10;
				GameSound(L"coin.wav");
			}
		}
	}
	// ItemとUserの接触判定
	if (fabs(rpver - UserVer) < 1.0 && fabs(rphor - UserHor) < 1.2 &&
		ItemDis > -5.0 && ItemDis < 0.0) {
		ItemFlag = true;
		ItemDis = 50.0;
		GameSound(L"item.wav");
	}
	// ObstacleとUserの接触判定
	if (ObsDis > -5.0 && ObsDis < 0.0)
		for (i = 0; i < ObsNum; i++) {
			if (fabs(rover[i] - UserVer) < 1.5 && fabs(rohor[i] - UserHor) < 1.5) {
				ObsDis = 50.0;
				ObsFlag = true;
				UserLife -= 1;
				GameSound(L"damage.wav");
				if (UserLife == 0) {
					ResultCoin = ScoreCoin, ResultDis = ScoreDis;
					GameSound(L"gameover.wav");
				}
				break;
			}
		}
	if (UserLife <= 0) GameOverFlag = true;
	// 自動回転
	RotAngle += 5.0;
	if (RotAngle > 360.0) RotAngle -= 360.0;
	// 自動画面遷移
	double speed = 0.3;
	int dis = 1;
	if (ItemUseFlag) speed = 0.6, dis = 2;
	LoadDis += speed;
	TraDis += speed;
	ObsDis += speed;
	// Itemの操作
	ItemDis += speed;
	// 距離のScoreを更新
	ScoreDis += dis;
	// Coinの操作
	for (i = 0; i < rCoinNum; i++) CoinDis[i] += speed;
	for (i = 0; i < rCoinNum2; i++) CoinDis2[i] += speed;

	if (LoadDis > 15.0) LoadDis -= 24.0;
	if (TraDis > 15.0) TraDis -= 27.0;

	// Penguinの操作
	if (UserRot > 30.0) UserRot = 30.0;
	else if (UserRot < -30.0) UserRot = -30.0;
	if (UserVer > 3.5) UserVer = 3.5;
	else if (UserVer < -4.0) UserVer = -4.0;
	if (UserHor > 4.0) UserHor = 4.0;
	else if (UserHor < -4.0) UserHor = -4.0;
	// 水平が0だった時に傾きを戻す.
	if (UserHor == 0.0) UserRot = 0.0;

	glutTimerFunc(T, IncAngle, 1);
	glutPostRedisplay();
}
void KeyboardHandler(unsigned char key, int x, int y) {
	if (key == 27) // ESC Key
		exit(0);
	if (key == 'a') {
		UserRot += 5.0;
		UserHor -= 0.5;
		glutPostRedisplay();
	}
	if (key == 'd') {
		UserRot -= 5.0;
		UserHor += 0.5;
		glutPostRedisplay();
	}
	if (key == 'w') {
		UserVer += 0.5;
		glutPostRedisplay();
	}
	if (key == 's') {
		UserVer -= 0.5;
		glutPostRedisplay();
	}
	if (key == ' ') {
		if (ItemFlag) {
			ItemFlag = false;
			ItemUseFlag = true;
			GameSound(L"useitem.wav");
		}
		glutPostRedisplay();
	}
}
int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(800, 800);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("電19-0021 岩崎一輝");
	glClearColor(0.0, 0.9, 1.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1.0, 1.0, 100.0);
	gluLookAt(15.0, 1.0, 0.0, 15.0, 0.0, -15.0, 0, 1, 0);

	glShadeModel(GL_FLAT);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glLightfv(GL_LIGHT0, GL_POSITION, position_default);

	glutDisplayFunc(display);
	glutKeyboardFunc(KeyboardHandler);
	glutTimerFunc(T, IncAngle, 1);
	glutTimerFunc(RandomTime, RandomCoin, 2);
	glutTimerFunc(RandomTime2, RandomCoin2, 3);
	glutTimerFunc(ItemTime, RandomItem, 4);
	glutTimerFunc(ObstacleTime, RandomObstacle, 5);
	glutMainLoop();
}