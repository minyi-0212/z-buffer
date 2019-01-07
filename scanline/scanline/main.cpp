#include <glut.h>
#include "Z_Buffer.h"
#include "Obj_Loader.h"

#include <iostream>
using namespace std;
using glm::vec3;

const GLint image_width = 600, image_height = 600;
Object test;
Z_Buffer buf;
vector<float> pixel_data;

void init()
{
	// init obj
	test.read_obj("test7.obj");
	vector<vec3> vec;
	vector<vector<vec3>> face;
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
	buf.init(face);
	buf.draw();
	buf.get_buffer(pixel_data);
}

void redraw()
{
	// TODO£º GL_FLOATµÄÎÊÌâ

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

	/*glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 3; i++)
	{
		glm::vec3 tmp = vec[face[0][3][i] - 1];
		cout << face[0][3][i] <<" "<< tmp.x <<" " << tmp.z << " " << tmp.y << endl;
		glVertex3f(tmp.x,  tmp.z, tmp.y);
		glColor3f(0.0, 0.0, 1.0);
	}
	glEnd();*/

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

int main(int argc, char *argv[])
{
	init();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(image_width, image_height);
	int windowHandle = glutCreateWindow("Gmy Scanline Z-buffer");

	//glutReshapeFunc(reshape);
	glutDisplayFunc(&redraw);
	glutMainLoop();

	return 0;
}
