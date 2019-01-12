#include <glut.h>
#include <iostream>
#include "Z_Buffer.h"
#include "Obj_Loader.h"
#include "gtc/matrix_transform.hpp"

using namespace std;
using glm::vec3;
using glm::mat4;

const GLint image_width = 600, image_height = 600, drawMode = 0;
Object test;
Z_Buffer buf;
vector<float> pixel_data;
vector<vec3> vec;
vector<vector<vec3>> face;

vec3 trans_v(0, 0, 0);
vec3 rotate_v(0, 1, 0);
float angle = 0, s_scale = 1;
mat4 model = glm::mat4(1.0f);

void compute_model()
{
	model = mat4(1.0f);
	model = glm::translate(model, trans_v);
	if (angle)
		model = glm::rotate(model, angle, rotate_v);
	model = glm::scale(model, glm::vec3(s_scale, s_scale, s_scale));
}

void init()
{
	// init obj
	test.read_obj("test8.obj");
	test.get_vertices(vec);
	test.get_faces(face);

	/*std::vector<glm::vec3> vec;
	test.get_vertices(vec);
	cout << vec.size() << endl;
	for (int i = 0; i < vec.size(); ++i)
	{
	cout << i << ": " << vec[i].x << " " << vec[i].y << " " << vec[i].z << endl;
	}*/

	/*std::vector<std::vector<int>> face;
	test.get_faces(face);
	cout << face.size() << endl;
	for (int i = 0; i < face.size(); ++i)
	{
	for (int j = 0; j < face[i].size(); ++j)
	{
	cout << face[i][j] << " ";
	}
	cout << endl;
	}*/

	/*std::vector<std::vector<std::vector<int>>> face;
	test.get_faces(face);
	cout << face.size() << endl;
	for (int k = 0; k < face.size(); k++)
	{
	for (int i = 0; i < face[k].size(); ++i)
	{
	for (int j = 0; j < face[k][i].size(); ++j)
	{
	cout << face[k][i][j] << " ";
	}
	cout << endl;
	}
	}*/

	buf.set_size(image_width, image_height);
	compute_model();
	buf.init(face, model);
	buf.draw();
	buf.get_buffer(pixel_data);
}

void get_FPS()
{
	static int frame = 0, time, timebase = 0;
	static char buffer[256];

	char mode[64];
	if (drawMode == 0)
		strcpy_s(mode, "naive");
	else if (drawMode == 1)
		strcpy_s(mode, "vertex array");
	else
		strcpy_s(mode, "display list");

	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000) {
		sprintf_s(buffer, "FPS:%4.2f %s",
			frame*1000.0 / (time - timebase), mode);
		timebase = time;
		frame = 0;
	}

	glutSetWindowTitle(buffer);
}

void redraw()
{
	//buf.update(eye, center);
	/*buf.draw();
	buf.get_buffer(pixel_data);*/

	//GLubyte pixel_data[image_width*image_height*3];
	//for (int i = 0; i < image_width*image_height; i++)
	//{
	//	pixel_data[i * 3] = i /*/ 500.0 * 255*/;
	//	pixel_data[i * 3 + 1] = 0;
	//	pixel_data[i * 3 + 2] = 0;
	//}
	//glDrawPixels(image_width, image_height,
	//	GL_BGR_EXT, GL_UNSIGNED_BYTE, pixel_data);
	
	glClear(GL_COLOR_BUFFER_BIT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glRasterPos3d(0, 0, 0);
	glDrawPixels(image_width, image_height,
			GL_BGR_EXT, GL_FLOAT, pixel_data.data());

//	glClear(GL_COLOR_BUFFER_BIT);
//	glBegin(GL_TRIANGLES);
////#pragma omp parallel for
//	for (int i = 0; i < 3; i++)
//	{
//		glm::vec3 tmp = vec3(i / 4.0, i / 4.0, i / 4.0)/*face[3][i]*/;
//		cout << i <<" "<< tmp.x <<" " << tmp.z << " " << tmp.y << endl;
//		//glVertex3f(tmp.x, tmp.z, tmp.y);
//		glColor3f(1.0, 1.0, 0.0);
//		glVertex2f(tmp.x,  tmp.y);
//	}
//	glEnd();

	get_FPS();
	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (w <= h)
		glOrtho(-50, 50, -50, 50, -50.0, 50.0);
	else
		glOrtho(-1.5*(GLfloat)w / (GLfloat)h, 1.5*(GLfloat)w / (GLfloat)h, -1.5, 1.5, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
	glutPostRedisplay();
}

void key(unsigned char k, int x, int y)
{
	switch (k)
	{
	case 27:
	case 'q': {exit(0); break; }

	case 'a': {
		trans_v[0] -= 100.0f;
		break;
	}
	case 'd': {
		trans_v[0] += 100.0f;
		break;
	}
	case 'w': {
		trans_v[1] += 100.0f;
		break;
	}
	case 's': {
		trans_v[1] -= 100.0f;
		break;
	}
	case 'j': {
		s_scale += 1;
		break;
	}
	}
	compute_model();
	buf.update_model(model);
	buf.draw();
	buf.get_buffer(pixel_data);
}

int main(int argc, char *argv[])
{
	init();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(image_width, image_height);
	int windowHandle = glutCreateWindow("Gmy Scanline Z-buffer");
	
	glutKeyboardFunc(key);
	glutDisplayFunc(redraw);
	//glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutMainLoop();

	return 0;
}
