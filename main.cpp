#define _USE_MATH_DEFINES

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>
#include <time.h>
#include "GL\glew.h"
#include "GL\freeglut.h"
#include "glm/ext.hpp"
#include "vector"
#include "string"
#include <numeric>

#include "shaderLoader.h"
#include "tekstura.h"

#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"


int screen_width = 640;
int screen_height = 480;

int pozycjaMyszyX;
int pozycjaMyszyY;
int mbutton;

int kd = 0;
bool stop = false;
double kameraX = 50.0;
double kameraZ = 0.0;
double kameraD = -3.0;
double kameraPredkosc;
double kameraKat = 20;
double kameraPredkoscObrotu;
double poprzednie_kameraX;
double poprzednie_kameraZ;
double poprzednie_kameraD;

double rotation = 0;

int arenorm = 0;
int mode = 0;
int changeShape = 0;
double dx = 0;
double dy = 0;
float dw = 0;

glm::mat4 MV;
glm::mat4 P;
glm::vec3 lightPos(0.0f, -5.0f, 0.0f);

std::vector< glm::vec3 > vertices1;
std::vector< glm::vec2 > uvs1;
std::vector< glm::vec3 > normals1;

std::vector< glm::vec3 > vertices2;
std::vector< glm::vec2 > uvs2;

std::vector< glm::vec3 > vertices12;
std::vector< glm::vec3 > vertices22;

GLuint objectColor_id = 0;
GLuint lightColor_id = 0;
GLuint lightPos_id = 0;
GLuint viewPos_id = 0;

GLuint materialambient_id = 0;
GLuint materialdiffuse_id = 0;
GLuint materialspecular_id = 0;
GLuint materialshininess_id = 0;


bool objload(
	const char* path,
	std::vector < glm::vec3 >& out_vertices,
	std::vector < glm::vec2 >& out_uvs,
	std::vector < glm::vec3 >& out_normals
) {
	std::vector< unsigned int > vertexIndices, uvIndices, normalIndices;
	std::vector< glm::vec3 > temp_vertices;
	std::vector< glm::vec2 > temp_uvs;
	std::vector< glm::vec3 > temp_normals;

	FILE* file;
	errno_t err;
	if ((err = fopen_s(&file, path, "r")) != 0) {
		printf("Impossible to open the file !\n");
		return false;
	}

	while (1) {
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));
		//int uvCount = 0;
		if (res == EOF) break;
		else
		{
			if (strcmp(lineHeader, "o") == 0) {
				//if (uvCount > 0) {
				//	for (int i = 0; i < uvCount; i++) 
				//	{
				//		push.back()
				//	}
				//}

			}
			else if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				if (arenorm == 0) arenorm = 1;
				glm::vec3 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0) {
				char charIndex;
				int normCheck = 0, num = 0;
				unsigned int vertexIndex[4], uvIndex[4], normalIndex[4], tempIndex;

				while (1) {
					fscanf_s(file, "%d", &tempIndex);
					if (normCheck == 0) vertexIndex[num] = tempIndex;
					else if (normCheck == 1) uvIndex[num] = tempIndex;
					else if (normCheck == 2) normalIndex[num] = tempIndex;
					else {
						printf("File can't be read :c\n");
						return false;
					}
					fscanf_s(file, "%c", &charIndex);
					if (charIndex == '/') {
						++normCheck;
					}
					else if (charIndex == ' ') {
						++num;
						normCheck = 0;
					}
					else if (charIndex == '\n') {
						break;
					}
					else {
						printf("File can't be read :cc\n");
						return false;
					}
				}

				while (num >= 0) {
					vertexIndices.push_back(vertexIndex[num]);
					if (normCheck > 0) {
						uvIndices.push_back(uvIndex[num]);
						if (normCheck == 2) normalIndices.push_back(normalIndex[num]);
					}
					--num;
				}
			}
		}
	}
	for (unsigned int i = 0; i < std::size(vertexIndices); i++) {
		glm::vec3 vertex = temp_vertices[vertexIndices[i] - 1];
		out_vertices.push_back(vertex);
	}
	for (unsigned int i = 0; i < uvIndices.size(); i++) {
		glm::vec2 uv = temp_uvs[uvIndices[i] - 1];
		out_uvs.push_back(uv);
	}
	for (unsigned int i = 0; i < normalIndices.size(); i++) {
		glm::vec3 normal = temp_normals[normalIndices[i] - 1];
		out_normals.push_back(normal);
	}
}

void quads_to_triangles(
	std::vector< glm::vec3 > ver1,		//quads vertices
	std::vector< glm::vec3 >& ver2,		//triangles vertices
	std::vector< glm::vec3 >& ver22,	//triangles vertices w/o plate
	std::vector< glm::vec2> tex1,		//quads texture vertices
	std::vector< glm::vec2>& tex2		//triangles texture vertices
) {
	float dot0, ls01, ls02, angle0;
	float dot1, ls11, ls12, angle1;
	std::vector< glm::vec3 > vec;

	for (int i = 0; i < ver1.size(); i = i + 4) {
		vec.insert(vec.end(), { (ver1[i + 1] - ver1[i]), (ver1[i + 1] - ver1[i + 2]),(ver1[i + 3] - ver1[i + 2]), (ver1[i + 3] - ver1[i]) });

		dot0 = (vec[0].x * vec[1].x) + (vec[0].y * vec[1].y) + (vec[0].z * vec[1].z);
		if (dot0 == 0) angle0 = 1.57;
		else
		{
			ls01 = (vec[0].x * vec[0].x) + (vec[0].y * vec[0].y) + (vec[0].z * vec[0].z);
			ls02 = (vec[1].x * vec[1].x) + (vec[1].y * vec[1].y) + (vec[1].z * vec[1].z);
			angle0 = cos(dot0 / sqrt(ls01 * ls02));
		}
		//std::cout << angle0 << "\n";

		dot1 = (vec[1].x * vec[2].x) + (vec[1].y * vec[2].y) + (vec[1].z * vec[2].z);
		if (dot1 == 0) angle1 = 1.57;
		else
		{
			ls11 = (vec[1].x * vec[1].x) + (vec[1].y * vec[1].y) + (vec[1].z * vec[1].z);
			ls12 = (vec[2].x * vec[2].x) + (vec[2].y * vec[2].y) + (vec[2].z * vec[2].z);
			angle1 = cos(dot1 / sqrt(ls11 * ls12));
		}
		//std::cout << angle1 << "\n";

		bool choice = angle0 < angle1;
		if (dot0 == 0 && dot1 == 0) {
			glm::vec3 aa = (ver1[i + 2] - ver1[i]) * (ver1[i + 2] - ver1[i]);
			float ac= sqrt(aa.x + aa.y + aa.z);
			aa = (ver1[i + 3] - ver1[i + 1]) * (ver1[i + 3] - ver1[i + 1]);
			float bd = sqrt(aa.x + aa.y + aa.z);
			choice = ac < bd;
		}
		if (!choice)
		{
			ver2.insert(ver2.end(), { ver1[i + 2], ver1[i + 3],ver1[i + 1], ver1[i + 1], ver1[i + 3], ver1[i] });
			if (i >= 4) tex2.insert(tex2.end(), { tex1[i - 2], tex1[i - 1], tex1[i - 3], tex1[i - 3], tex1[i - 1], tex1[i - 4] });
		}
		else
		{
			ver2.insert(ver2.end(), { ver1[i + 1], ver1[i + 2],ver1[i], ver1[i], ver1[i + 2], ver1[i + 3] });
			if (i >= 4) tex2.insert(tex2.end(), { tex1[i - 3], tex1[i - 2], tex1[i - 4], tex1[i - 4], tex1[i - 2], tex1[i - 1] });
		}
	}
	ver22 = ver2;
	ver22.erase(ver22.begin(), ver22.begin() + 6);
}


void triangles_normals(
	std::vector< glm::vec3 > ver
) {
	float triangle_size, test;
	int nr = 0;
	std::vector< glm::vec3 > trinorm;
	for (int i = 0; i < ver.size(); i = i + 3) {
		std::vector< glm::vec3 > edge, triangle;
		edge.push_back(ver[i + 1] - ver[i]);
		edge.push_back(ver[i + 2] - ver[i]);
		triangle.push_back(cross(edge[0], edge[1]));
		triangle_size = sqrt((triangle[0].x * triangle[0].x) + (triangle[0].y * triangle[0].y) + (triangle[0].z * triangle[0].z));
		trinorm.push_back(triangle[0] / triangle_size);
	}

	for (int i = 0; i < ver.size(); i++) {
		int na = 0;
		float vnsize = 0;
		std::vector<int> nrtri;
		std::vector< glm::vec3 > vno, svno;
		for (int j = 0; j < ver.size(); j++) {
			if (ver[j] == ver[i]) {
				if (j < 3) nrtri.push_back(0);
				else if ((j + 1) % 3 == 1) nrtri.push_back(j / 3);
				else if ((j + 1) % 3 == 2) nrtri.push_back((j - 1) / 3);
				else nrtri.push_back((j - 2) / 3);
				//std::cout << nrtri[na] << "\n";
				//std::cout << "T: " << triangles[na].x << ", " << triangles[na + 1].y << ", " << triangles[na + 2].z << "\n";
				vno.push_back(trinorm[nrtri[na]]);
				na = na + 1;
			}
		}
		//std::cout << "\n";
		svno.push_back({ 0, 0, 0 });
		for (int k = 0; k < vno.size(); k++) svno[0] = svno[0] + vno[k];
		vnsize = sqrt((svno[0].x * svno[0].x) + (svno[0].y * svno[0].y) + (svno[0].z * svno[0].z));
		normals1.push_back(svno[0] / vnsize);
		//std::cout << vnormal[i].x << "," << vnormal[i].y << "," << vnormal[i].z << "\n";
	}
}

void colors(
	std::vector < glm::vec3 >& col1,
	std::vector < glm::vec3 >& col2,
	std::vector< glm::vec3 >& ver1,
	std::vector< glm::vec3 >& ver2
) {
	std::vector<float> hh;
	for (int i = 0; i < ver1.size(); i++) {
		hh.push_back(ver1[i].y);
	}
	std::vector<float>::iterator vmin = min_element(hh.begin(), hh.end());
	std::vector<float>::iterator vmax = max_element(hh.begin(), hh.end());
	//std::cout << *vmin << "\n" << *vmax << "\n";

	float min = *vmin;
	float max = (*vmax) - min;

	for (int i = 0; i < hh.size(); i++) hh[i] = (hh[i] - min) / max;
	glm::vec3 co;
	for (int i = 0; i < hh.size(); i++) {
		if (hh[i] == 0.0) co = { 0, 0, 0 };
		else if (hh[i] < 0.25) co = { 0, (hh[i] * 4), 1 };
		else if (hh[i] < 0.5) co = { 0, 1, 1 - ((hh[i] - 0.25) * 4) };
		else if (hh[i] < 0.75) co = { ((hh[i] - 0.5) * 4), 1, 0 };
		else if (hh[i] < 1.0) co = { 1, 1 - ((hh[i] - 0.75) * 4), 0 };
		else co = { 1, 0, 0 };

		col1.push_back(co);	
	}


	std::vector<float> gg;
	for (int i = 0; i < ver2.size(); i++) {
		gg.push_back(ver2[i].y);
	}
	vmin = min_element(gg.begin(), gg.end());
	vmax = max_element(gg.begin(), gg.end());
	//std::cout << *vmin << "\n" << *vmax << "\n";

	min = *vmin;
	max = (*vmax) - min;

	for (int i = 0; i < gg.size(); i++) gg[i] = (gg[i] - min) / max;
	glm::vec3 co1;
	for (int i = 0; i < gg.size(); i++) {
		if (gg[i] == 0.0) co1 = { 0, 0, 0 };
		else if (gg[i] < 0.25) co1 = { 0, (gg[i] * 4), 1 };
		else if (gg[i] < 0.5) co1 = { 0, 1, 1 - ((gg[i] - 0.25) * 4) };
		else if (gg[i] < 0.75) co1 = { ((gg[i] - 0.5) * 4), 1, 0 };
		else if (gg[i] < 1.0) co1 = { 1, 1 - ((gg[i] - 0.75) * 4), 0 };
		else co1 = { 1, 0, 0 };

		col2.push_back(co1);
	}
}

GLuint programID = 0;
GLuint programID1 = 0;
GLuint programID2 = 0;
GLuint programID3 = 0;

GLuint normalbuffer;
GLint uniformTex;
GLuint tex_id;
unsigned int VBO3p, VBO4np, VBO3np, VBOsc, VBOscQ, VBO4p, vtex4, vtex3, VAO[8];
/*###############################################################*/
void mysz(int button, int state, int x, int y)
{
	mbutton = button;
	switch (state)
	{
	case GLUT_UP:
		break;
	case GLUT_DOWN:
		pozycjaMyszyX = x;
		pozycjaMyszyY = y;
		poprzednie_kameraX = kameraX;
		poprzednie_kameraZ = kameraZ;
		poprzednie_kameraD = kameraD;
		break;

	}
}
/*******************************************/
void mysz_ruch(int x, int y)
{
	if (mbutton == GLUT_LEFT_BUTTON)
	{
		kameraX = poprzednie_kameraX - (pozycjaMyszyX - x) * 0.1;
		kameraZ = poprzednie_kameraZ - (pozycjaMyszyY - y) * 0.1;
	}
	if (mbutton == GLUT_RIGHT_BUTTON)
	{
		kameraD = poprzednie_kameraD + (pozycjaMyszyY - y) * 0.1;
	}
}
/******************************************/

void klawisz(GLubyte key, int x, int y)
{
	switch (key) {

	case 27:
		exit(1);
		break;

	case 'd':
		lightPos.x += 0.5;
		dx += 5;
		break;
	case 'a':
		lightPos.x -= 0.5;
		dx -= 5;
		break;
	case 'w':
		lightPos.z -= 0.5;
		dy += 5;
		break;
	case 's':
		lightPos.z += 0.5;
		dy -= 5;
		break;
	case 'e':
		dw += 5;
		break;
	case 'q':
		dw -= 5;
		break;
	case 'r':
		if (changeShape == 0) lightPos = { 0.0f, -5.0f, 0.0f };
		break;
	case '1':

		break;
	case 't':
		if (mode == 0) mode = 1;
		else if (mode == 1) mode = 2;
		else if (mode == 2) mode = 3;
		else if (mode == 3) mode = 0;
		break;
	case 'f':
		if (changeShape == 0) changeShape = 1;
		else if (changeShape == 1) changeShape = 0;
		break;
	case 'p':
		if (stop)
			stop = false;
		else
			stop = true;
		break;
	case ',':
		--kd;
		break;
	case '.':
		++kd;
		break;
	}
}
/*###############################################################*/
void rozmiar(int width, int height)
{
	screen_width = width;
	screen_height = height;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_width, screen_height);

	P = glm::perspective(glm::radians(60.0f), (GLfloat)screen_width / (GLfloat)screen_height, 1.0f, 1000.0f);

	glutPostRedisplay();
}

/*###############################################################*/
void rysuj(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Kasowanie ekranu
	GLfloat color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glClearBufferfv(GL_COLOR, 0, color);

	if (mode == 0)
	{
		glUseProgram(programID);

		MV = glm::mat4(1.0f);
		MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		glm::mat4 MVP = P * MV;
		GLuint MVP_id = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		if (changeShape == 0)
		{
			glBindVertexArray(VAO[0]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3f(objectColor_id, 0.0f, 0.0f, 0.0f);
			glDrawArrays(GL_TRIANGLES, 0, 3 * vertices22.size());
			glLineWidth(1);
		}
		else
		{
			glBindVertexArray(VAO[3]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glUniform3f(objectColor_id, 0.0f, 0.0f, 0.0f);
			glDrawArrays(GL_QUADS, 0, 4 * vertices12.size());
			glLineWidth(1);
		}
	}
	else if (mode == 1)
	{
		glUseProgram(programID2);

		MV = glm::mat4(1.0f);
		MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		glm::mat4 MVP = P * MV;
		GLuint MVP_id = glGetUniformLocation(programID2, "MVP");
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		if (changeShape == 0)
		{
			glBindVertexArray(VAO[1]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform3f(materialambient_id, 0.1f, 0.1f, 0.1f);
			glUniform3f(materialdiffuse_id, 0.4f, 0.4f, 0.4f);
			glUniform3f(materialspecular_id, 0.1f, 0.1f, 0.1f);
			glUniform1f(materialshininess_id, 1.0f);
			glDrawArrays(GL_TRIANGLES, 0, 3 * vertices2.size());
		}
		else 
		{
			glBindVertexArray(VAO[6]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glUniform3f(materialambient_id, 0.1f, 0.1f, 0.1f);
			glUniform3f(materialdiffuse_id, 0.4f, 0.4f, 0.4f);
			glUniform3f(materialspecular_id, 0.1f, 0.1f, 0.1f);
			glUniform1f(materialshininess_id, 1.0f);
			glDrawArrays(GL_QUADS, 0, 4 * vertices1.size());
		}
	}
	else if (mode == 2)
	{
		glUseProgram(programID1);

		MV = glm::mat4(1.0f);
		MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		glm::mat4 MVP = P * MV;
		GLuint MVP_id = glGetUniformLocation(programID1, "MVP");
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));

		if (changeShape == 0)
		{
			glBindVertexArray(VAO[2]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, 3 * vertices2.size());
		}
		else
		{
			glBindVertexArray(VAO[7]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_QUADS, 0, 3 * vertices1.size());
		}

	}
	else if (mode == 3)
	{
		glUseProgram(programID3);

		MV = glm::mat4(1.0f);
		MV = glm::translate(MV, glm::vec3(0, 0, kameraD));
		MV = glm::rotate(MV, (float)glm::radians(kameraZ), glm::vec3(1, 0, 0));
		MV = glm::rotate(MV, (float)glm::radians(kameraX), glm::vec3(0, 1, 0));

		glm::mat4 MVP = P * MV;
		GLuint MVP_id = glGetUniformLocation(programID3, "MVP");
		glUniformMatrix4fv(MVP_id, 1, GL_FALSE, &(MVP[0][0]));



		if (changeShape == 0)
		{
			glBindVertexArray(VAO[4]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glUniform1i(uniformTex, 0);
			glDrawArrays(GL_TRIANGLES, 0, 3 * vertices22.size());
		}
		else
		{
			glBindVertexArray(VAO[5]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, tex_id);
			glUniform1i(uniformTex, 0);
			glDrawArrays(GL_QUADS, 0, 4 * vertices12.size());
		}
	}

	if (mode != 0) {
		glUniform3f(lightColor_id, 1.0f, 1.0f, 1.0f);
		glUniform3f(lightPos_id, lightPos[0], lightPos[1], lightPos[2]);
		float camx = kameraX;
		//if(camx > 0) while (camx >= 40) camx = camx - 40;
		//else while (camx <= -40) camx = camx + 40;
		//camx = camx / 360;

		float camz = kameraZ;
		//if (camz > 0) while (camz >= 360) camz = camz - 360;
		//else while (camz <= -360) camz = camz + 360;
		//camz = camz / 360;

		//float camd = kameraD;
		glUniform3f(viewPos_id, camx, camz, kameraD);
		//std::cout << camx << " " << camz << " " << kameraD << "\n";
	}
	if (changeShape == 0 && lightPos.y == 5.0f) lightPos.y = -5.0f;
	else if (changeShape != 0 && lightPos.y == -5.0f) lightPos.y = 5.0f;

	glFlush();
	glutSwapBuffers();
}

void idle()
{
	glutPostRedisplay();
}

int timmy = 0, r = 5, kat = 0;
float lpozx=0, lpozz=0;

void timer(int value)
{
	if (mode != 0 && changeShape != 0)
	{
		if (stop) {
			lightPos.x = lpozx;
			lightPos.z = lpozz;
		}
		else {
			if (kat == 360) kat = 0;
			lpozx = r * sin(kat * M_PI / 180);
			lpozz = r * cos(kat * M_PI / 180);
			kat = kat + 1 + kd;

			lightPos.x = lpozx;
			lightPos.z = lpozz;
		}
	}
	glutTimerFunc(20, timer, 0);
}

int main(int argc, char** argv)
{
	//wczytanie pliku
	bool res = objload("mount.blend.obj", vertices1, uvs1, normals1);

	//wierzcholki bez podstawki
	vertices12 = vertices1;
	vertices12.erase(vertices12.begin(), vertices12.begin() + 4);

	//podzial prostokatow na trojkaty
	quads_to_triangles(vertices1, vertices2, vertices22, uvs1, uvs2);

	//obliczenie wektorow normalnych
	if (arenorm == 0) triangles_normals(vertices2);

	//kolory dla modelu wysokosciowego
	std::vector < glm::vec3 > height_color, height_color_quads;
	colors(height_color, height_color_quads, vertices2, vertices1);


	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(screen_width, screen_height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Numeryczny Model Terenu");

	glewInit();

	glutDisplayFunc(rysuj);
	glutIdleFunc(idle);
	glutTimerFunc(20, timer, 0);
	glutReshapeFunc(rozmiar);

	glutKeyboardFunc(klawisz);
	glutMouseFunc(mysz);
	glutMotionFunc(mysz_ruch);

	glEnable(GL_DEPTH_TEST);


	glGenVertexArrays(8, VAO);

	//prostokaty z plyta
	glGenBuffers(1, &VBO4p);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4p);
	glBufferData(GL_ARRAY_BUFFER, vertices1.size() * sizeof(glm::vec3), &vertices1[0], GL_STATIC_DRAW);

	//prostokaty bez plyty
	glGenBuffers(1, &VBO4np);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4np);
	glBufferData(GL_ARRAY_BUFFER, vertices12.size() * sizeof(glm::vec3), &vertices12[0], GL_STATIC_DRAW);

	//trojkaty z plyta
	glGenBuffers(1, &VBO3p);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3p);
	glBufferData(GL_ARRAY_BUFFER, vertices2.size() * sizeof(glm::vec3), &vertices2[0], GL_STATIC_DRAW);

	//trojkaty bez plyty
	glGenBuffers(1, &VBO3np);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3np);
	glBufferData(GL_ARRAY_BUFFER, vertices22.size() * sizeof(glm::vec3), &vertices22[0], GL_STATIC_DRAW);

	//kolor dla modelu wysokosciowego
	glGenBuffers(1, &VBOsc);
	glBindBuffer(GL_ARRAY_BUFFER, VBOsc);
	glBufferData(GL_ARRAY_BUFFER, height_color.size() * sizeof(glm::vec3), &height_color[0], GL_STATIC_DRAW);

	//kolor dla modelu wysokosciowego quads
	glGenBuffers(1, &VBOscQ);
	glBindBuffer(GL_ARRAY_BUFFER, VBOscQ);
	glBufferData(GL_ARRAY_BUFFER, height_color_quads.size() * sizeof(glm::vec3), &height_color_quads[0], GL_STATIC_DRAW);

	//wektory normalne
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, normals1.size() * sizeof(glm::vec3), &normals1[0], GL_STATIC_DRAW);

	//wspolrzedne tekstur dla prostokatow
	glGenBuffers(1, &vtex4);
	glBindBuffer(GL_ARRAY_BUFFER, vtex4);
	glBufferData(GL_ARRAY_BUFFER, uvs1.size() * sizeof(glm::vec2), &uvs1[0], GL_STATIC_DRAW);

	//wspolrzedne tekstur dla trojkatow
	glGenBuffers(1, &vtex3);
	glBindBuffer(GL_ARRAY_BUFFER, vtex3);
	glBufferData(GL_ARRAY_BUFFER, uvs2.size() * sizeof(glm::vec2), &uvs2[0], GL_STATIC_DRAW);


	//program dla siatki
	programID = loadShaders("vertex_shader.glsl", "fragment_shader3.glsl");
	objectColor_id = glGetUniformLocation(programID, "objectColor");

	//siatka trojkatow
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3np);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//siatka prostokatow
	glBindVertexArray(VAO[3]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4np);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	//program dla modelu wysokosciowego
	programID1 = loadShaders("vertex_shader.glsl", "fragment_shader2.glsl");
	lightColor_id = glGetUniformLocation(programID1, "lightColor");
	lightPos_id = glGetUniformLocation(programID1, "lightPos");
	viewPos_id = glGetUniformLocation(programID1, "viewPos");

	//kolorki
	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3p);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBOsc);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	
	//kolorki quads
	glBindVertexArray(VAO[7]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4p);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, VBOscQ);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	

	//program dla szarego modelu
	programID2 = loadShaders("vertex_shader.glsl", "fragment_shader1.glsl");
	materialambient_id = glGetUniformLocation(programID2, "material.ambient");
	materialdiffuse_id = glGetUniformLocation(programID2, "material.diffuse");
	materialspecular_id = glGetUniformLocation(programID2, "material.specular");
	materialshininess_id = glGetUniformLocation(programID2, "material.shininess");

	//szary model
	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3p);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	
	//szary quads
	glBindVertexArray(VAO[6]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4p);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


	//program dla modelu z tekstura
	programID3 = loadShaders("vertex_shader.glsl", "fragment_shader.glsl");
	tex_id = WczytajTeksture("grass1.bmp");
	if (tex_id == -1)
	{
		MessageBox(NULL, "Nie znaleziono pliku z teksturą", "Problem", MB_OK | MB_ICONERROR);
		exit(0);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex_id);
	uniformTex = glGetUniformLocation(programID3, "tex");
	glUniform1i(uniformTex, 0);

	//model z tekstura
	glBindVertexArray(VAO[4]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3np);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vtex3);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//tekstura quads
	glBindVertexArray(VAO[5]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4np);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vtex4);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);


	glutMainLoop();

	glDeleteBuffers(1, &VBO4p);
	glDeleteBuffers(1, &VBO3p);
	glDeleteBuffers(1, &VBO4np);
	glDeleteBuffers(1, &VBO3np);
	glDeleteBuffers(1, &VBOsc);
	glDeleteBuffers(1, &vtex4);
	glDeleteBuffers(1, &vtex3);
	glDeleteVertexArrays(8, VAO);

	return(0);
}