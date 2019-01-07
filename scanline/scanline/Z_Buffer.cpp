#include <iostream>
#include <algorithm>
#include "Z_Buffer.h"

using namespace glm;
using namespace std;

void comput_plane_coefficient(const vector<vec3>& point,
	float& a, float& b, float& c, float& d)
{
	// A(x-x0) + B(y-y0) + C(z-z0) = 0
	if (point.size() < 3)
	{
		cout << "must given >= 3 points to compute the plane coefficient." << endl;
		return;
	}
	vec3 v1 = point[1] - point[0],
		v2 = point[2] - point[0],
		v = normalize(cross(v1, v2));
	a = v.x;
	b = v.y;
	c = v.z;
	d = -dot(v, point[1]);
}

void Z_Buffer::init(const vector<vector<vec3>>& faces)
{
	/*_frame_buffer.resize(_width*_height * 3);
	for (int i = 0; i < _width*_height; i++)
	{
		_frame_buffer[i * 3] = i *1.0 / _width / _height;
		_frame_buffer[i * 3 + 1] = 0;
		_frame_buffer[i * 3 + 2] = 0;
	}*/
	_frame_buffer.resize(_width*_height * 3, 0.5);
	//_background_color = vec3(0, 0, 0);
	_poly_table.resize(_height);
	_edge_table.resize(faces.size());
	_poly_table_index.resize(faces.size(), 0);
	for (int face_id = 0; face_id < faces.size(); face_id++)  // redo
	//for (int face_id = 0; face_id < 1; face_id++)
	{
		PolyTable poly;
		comput_plane_coefficient(faces[face_id], poly.a, poly.b, poly.c, poly.d);
		if (poly.c > -1e-8 && poly.c < 1e-8) // ignore: vertical to xOy
			continue;
		poly.poly_id = face_id;
		poly.in_out_flag = false;
		poly.color = vec3(poly.a, poly.b, poly.c);
		int poly_y_max = -1;
		for (int edge_id = 0; edge_id < faces[face_id].size(); edge_id++)
		{
			vec3 p1 = faces[face_id][edge_id], 
				p2 = faces[face_id][(edge_id + 1) % faces[face_id].size()];
			p1.x = p1.x;
			p1.y = p1.y;
			p2.x = p2.x;
			p2.y = p2.y;
			EdgeTable edge;
			if (round(p1.y) == round(p2.y))
				continue;
			else if (p1.y > p2.y)
			{
				edge.y_max = p1.y;
				edge.x = p1.x;
			}
			else
			{
				edge.y_max = p2.y;
				edge.x = p2.x;
			}
			edge.dy = abs(p1.y - p2.y);
			edge.dx = (p1.x - p2.x) / (p2.y - p1.y);
			edge.poly_id = face_id;
			_edge_table[edge.poly_id].push_back(edge);
			if (poly_y_max < round(edge.y_max))
				poly_y_max = round(edge.y_max);
		}
		_poly_table[poly_y_max].push_back(poly);
		_poly_table_index[face_id] = &(_poly_table[poly_y_max].back());
	}

	//for (int i = 0; i < _poly_table.size(); i++)
	//{
	//	if (_poly_table[i].size() != 0)
	//	{
	//		for (auto a = _poly_table[i].begin(); a != _poly_table[i].end(); a++)
	//		{
	//			cout << a->poly_id <<" " << a->a <<" " << a->b << " " << a->c << " " << a->d << endl;
	//			//_poly_table_index[a->poly_id] = &(*a);
	//		}
	//	}
	//}
	//for (int i =0;i<_poly_table_index.size();++i)
	//{
	//	cout << i << endl;
	//	if (_poly_table_index[i])
	//		cout << "poly " << _poly_table_index[i]->poly_id 
	//		<< " " << _poly_table_index[i]->a
	//		<< " " << _poly_table_index[i]->b
	//		<< " " << _poly_table_index[i]->c
	//		<<" "<< _poly_table_index[i]->d << endl;
	//}
	//cout << endl;
}

void Z_Buffer::set_size(const int& w, const int& h)
{
	_width = w;
	_height = h;
};

void Z_Buffer::get_buffer(vector<float>& buf)
{
	buf = _frame_buffer;
};

void Z_Buffer::active(const int& y)
{
	if (y<0 || y>=_poly_table.size())
	{
		cout << "scanline " << y << "out of range!" << endl;
		return;
	}
	for (auto p: _poly_table[y])
	{
		//cout <<"test poly_id:"<< (*(&p)).poly_id << endl;
		_active_poly.push_back(p.poly_id);
		for (auto e :_edge_table[p.poly_id])
		{
			// cout << e.x << " " << e.dx << " " << e.dy << " " << e.y_max << endl;
			if (round(e.y_max) == y)
			{
				ActiveEdgeTable ae;
				ae.x = e.x;
				ae.dx = e.dx;
				ae.dy = e.dy;
				ae.poly_id = p.poly_id;
				ae.z = -(p.a*e.x + p.b*e.y_max + p.d) / p.c;
				ae.dzx = -p.a / p.c;
				ae.dzy = p.b / p.c;
				_active_edge.push_back(ae);
			}
		}
	}
	_active_edge.sort();
	//cout << _active_edge.size() << endl;
	//cout << _active_poly.size() << endl;
	//cout <<"tt "<< (*_poly_table_index[_active_poly[0]]).poly_id << endl;
	/*for (auto id : _active_poly)
	{
		cout << id << " ";
		cout << (*(_poly_table_index[id])).poly_id << endl;
	}
	cout << endl;*/
}

int Z_Buffer::count_active_poly_flag()
{
	int flag_true = 0;
	for (auto id : _active_poly)
	{
		if(_poly_table_index[id]->in_out_flag)
			flag_true++;
	}
	return flag_true;
}

void Z_Buffer::draw_region(const int& x1, const int& x2, const int& y, const glm::vec3& color)
{
	int offset = y*_width*3;
	for (int i = x1; i < x2; i++)
	{
		_frame_buffer[offset + i * 3] = color.x;
		_frame_buffer[offset + i * 3 + 1] = color.y;
		_frame_buffer[offset + i * 3 + 2] = color.z;
	}
}

void Z_Buffer::draw_line(const int& y)
{
	list<ActiveEdgeTable>::iterator e1 = _active_edge.begin(), e2;
	vec3 color;
	int index = 0;
	if (_active_edge.size() > 0)
	{
		for (auto e = ++_active_edge.begin(); e != _active_edge.end(); e++)
		{
			_poly_table_index[e1->poly_id]->in_out_flag = !_poly_table_index[e1->poly_id]->in_out_flag;
			e2 = e;
			int in_out_flag = count_active_poly_flag();
			if (in_out_flag == 0)
				continue;
			else if (in_out_flag == 1)
			{
				color = _poly_table_index[e1->poly_id]->color;
				draw_region(round(e1->x), round(e2->x), y, color);
				/*cout << e1->poly_id << " " << e1->x << " " << e1->dx << " " << e2->x << " " << y
					<< " (" << color.x << "," << color.y << "," << color.z << ")" << endl;*/
				cout << y << " " << e1->x << " " << e1->dx << "," << e2->x << " " << e2->dx << endl;

			}
			e1->x += e1->dx;
			e1->dy -= 1;
			if (e1->dy < 0)
				_active_edge.erase(e1);
			e1 = e2;
		}
		ActiveEdgeTable* end = &_active_edge.back();
		_poly_table_index[end->poly_id]->in_out_flag = !_poly_table_index[end->poly_id]->in_out_flag;
		cout << end->x << " " << end->dx << endl;
		end->x += end->dx;
		cout << end->x << " " << end->dx << endl;
		end->dy -= 1;
		if (e1->dy < 0)
			_active_edge.erase(e1);
	}
}

void Z_Buffer::draw()
{
	for (int j = 0; j <= 37; j++)
	{
		for (int i = 16; i <= 35; i++)
		{
			int index = j*_width * 3;
			_frame_buffer[index + i * 3] = 1;
			_frame_buffer[index + i * 3 + 1] = 0;
			_frame_buffer[index + i * 3 + 2] = 0;
		}
	}

	for (int y = _height - 1; y >= 0; y--)
	{
		active(y);
		draw_line(y);
	}
}