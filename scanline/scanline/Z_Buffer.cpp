#include <iostream>
#include <algorithm>
#include "Z_Buffer.h"

using namespace glm;
using namespace std;

void output(const vec3& v)
{
	cout << "(" << v.x << "," << v.y << "," << v.z << ")" << endl;
}

void output(const mat4& m)
{
	cout <<"["<< m[0][0] << "," << m[0][1] << "," << m[0][2] << "," << m[0][3] <<endl
		<< m[1][0] << "," << m[1][1] << "," << m[1][2] << "," << m[1][3] << endl
		<< m[2][0] << "," << m[2][1] << "," << m[2][2] << "," << m[2][3] << endl
		<< m[3][0] << "," << m[3][1] << "," << m[3][2] << "," << m[3][3] << "]" << endl;
}

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

void Z_Buffer::model_to_clip(vector<vector<vec3>>& faces)
{
	/*mat4 model = glm::mat4(1.0f);
	model = translate(model, trans_v);
	if(angle)
		model = rotate(model, angle, rotate_v);
	float s_scale = 1;
	model = scale(model, glm::vec3(s_scale, s_scale, s_scale));
	mat4 view = glm::lookAt(vec3(0, 0, 1), vec3(0, 0, 0), vec3(0, 1, 0));
	float x1 = 0.0f, x2 = 600.0f,
		y1 = 0.0f, y2 = 600.0f;
	mat4 projection = glm::ortho(x1, x2, y1, y2, -50.0f, 30.f);
	mat4 mvp = projection*view*model;*/
	mat4 mvp = _proj*_view*_model;
	
	// model world view clip
	for (auto &f : faces)
	{
		for (auto& e : f)
		{
			vec4 tmp(e, 1);
			tmp = mvp*tmp;
			//e = tmp;
			e.x = (tmp.x + 1.0)*_width / 2;
			e.y = (tmp.y + 1.0)*_height / 2;
			e.z = -tmp.z * __min(_width, _height);
			//output(e);
		}
	}
}

void Z_Buffer::init(const vector<vector<vec3>>& faces, const glm::mat4& model,
	const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up,
	const float& x1, const float& x2,
	const float& y1, const float& y2,
	const float& z1, const float& z2)
{
	/*_faces.resize(faces.size());
	for (int i =0;i<faces.size();++i)
	{
		_faces[i].resize(faces[i].size());
		_faces[i].assign(faces[i].begin(), faces[i].end());
	}*/
	_faces = faces;
	//output(faces[0][0]);
	look_at(eye, center, up);
	set_ortho(x1, x2, y1, y2, z1, z2);
	update_model(model);
}

void Z_Buffer::clear()
{
	_frame_buffer.clear();
	_poly_table.clear();
	_edge_table.clear();
	_poly_table_index.clear();
	_active_poly.clear();
	_active_edge.clear();
}

bool judge(const EdgeTable &e1, const EdgeTable &e2)
{
	glm::vec2 a(e1.x, e1.y_max), b(e1.x+e1.dx*e1.dy, e1.y_max-e1.dy),
		c(e2.x, e2.y_max), d(e2.x + e2.dx*e2.dy, e2.y_max - e2.dy);
	
	return ((c.x - a.x)*(b.y - a.y) - (b.x - a.x)*(c.y - a.y))*((d.x - a.x)*(b.y - a.y) - (b.x - a.x)*(d.y - a.y)) < 0 &&
		((a.x - c.x)*(d.y - c.y) - (d.x - c.x)*(a.y - c.y))*((b.x - c.x)*(d.y - c.y) - (d.x - c.x)*(b.y - c.y)) < 0;
}

void cross_point(const EdgeTable &e1, const EdgeTable &e2, glm::vec2& p)
{
	glm::vec2 a(e1.x, e1.y_max), b(e1.x + e1.dx*e1.dy, e1.y_max - e1.dy),
		c(e2.x, e2.y_max), d(e2.x + e2.dx*e2.dy, e2.y_max - e2.dy);

	/*cout << "cross!! "
		<< glm::dot((a - c), (d - c))*glm::dot((b - c), (d - c)) << " "
		<< glm::dot((c - a), (b - a))*glm::dot((d - a), (b - a)) << endl << endl
		<< (a - c).x << " " << (a - c).y << endl
		<< (d - c).x << " " << (d - c).y << endl
		<< (b - c).x << " " << (b - c).y << endl
		<< (d - c).x << " " << (d - c).y << endl
		<< endl;

	cout << a.x << " " << a.y << endl
		<< b.x << " " << b.y << endl
		<< c.x << " " << c.y << endl
		<< d.x << " " << d.y << endl
		<< endl;*/

	double a1, b1, c1, a2, b2, c2;
	a1 = a.y - b.y;
	b1 = b.x - a.x;
	c1 = a.x*b.y - b.x*a.y;

	a2 = c.y - d.y;
	b2 = d.x - c.x;
	c2 = c.x*d.y - d.x*c.y;

	double D = a1 * b2 - a2 * b1;
	p.x = (b1 * c2 - b2 * c1) / D;
	p.y = (a2 * c1 - a1 * c2) / D;
	
	/*cout << a1 << " " << b1 << " " << c1 << " r= " << a1 * a.x + b1 * a.y + c1 << " " << a1 * b.x + b1 * b.y + c1 << endl
		<< a2 << " " << b2 << " " << c2 << " r= " << a2 * c.x + b2 * c.y + c2 << " " << a2 * d.x + b2 * d.y + c2 << endl
		<< a1 * p.x + b1 * p.y + c1 << endl
		<< a2 * p.x + b2 * p.y + c2 << endl
		<< -(a1 * p.x + c1) / b1 << endl
		<< -(a2 * p.x + c2) / b2 << endl;*/
}

void Z_Buffer::update_model(const glm::mat4& model)
{
	vector<vector<vec3>> faces = _faces;
	_model = model;
	model_to_clip(faces);
	/*_frame_buffer.resize(_width*_height * 3);
	for (int i = 0; i < _width*_height; i++)
	{
	_frame_buffer[i * 3] = i *1.0 / _width / _height;
	_frame_buffer[i * 3 + 1] = 0;
	_frame_buffer[i * 3 + 2] = 0;
	}*/
	clear();
	_frame_buffer.resize(_width*_height * 3, 0.5);
	_poly_table.resize(_height);
	_edge_table.resize(faces.size());
	_poly_table_index.resize(faces.size(), 0);
	for (int face_id = 0; face_id < faces.size(); face_id++)  // redo
	//for (int face_id = 4; face_id < 5; face_id++)
	{
		PolyTable poly;
		comput_plane_coefficient(faces[face_id], poly.a, poly.b, poly.c, poly.d);
		if (poly.c > -1e-8 && poly.c < 1e-8) // ignore: vertical to xOy
			continue;
		poly.poly_id = face_id;
		poly.in_out_flag = false;
		poly.color = vec3(poly.a, poly.b, poly.c);
		//poly.color = vec3(abs(poly.a), abs(poly.b), abs(poly.c));
		//poly.color = vec3(poly.a, poly.a, poly.a);
		int poly_y_max = INT_MIN, poly_y_min = INT_MAX;
		for (int edge_id = 0; edge_id < faces[face_id].size(); edge_id++)
		{
			vec3 p1 = faces[face_id][edge_id],
				p2 = faces[face_id][(edge_id + 1) % faces[face_id].size()];
			EdgeTable edge;
			/*if (poly.poly_id == 4991)
			cout << face_id << " " << p1.x << " " << p1.y << " " << p1.z << endl;*/
			if (round(p1.y) == round(p2.y)
				|| (p1.x < 0 && p2.x < 0 && p1.y < 0 && p2.y < 0)
				|| (p1.x > _width && p2.x > _width && p1.y > _height && p2.y > _height))
				continue;
			else if (p1.y > p2.y)
			{
				edge.y_max = round(p1.y);
				edge.x = p1.x;
				if (poly_y_min > round(p2.y))
					poly_y_min = round(p2.y);
			}
			else
			{
				edge.y_max = round(p2.y);
				edge.x = p2.x;
				if (poly_y_min > round(p1.y))
					poly_y_min = round(p1.y);
			}
			edge.dy = abs(round(p1.y) - round(p2.y));
			edge.dx = (p1.x - p2.x) / (round(p2.y) - round(p1.y));
			if (edge.y_max > _height)
			{
				edge.dy -= (edge.y_max - _height);
				edge.x += edge.dx*(edge.y_max - _height);
				edge.y_max = _height - 1;
			}
			edge.poly_id = face_id;

			{
				EdgeTable edge_change = edge;
				for (int j = 0; j < face_id; j++)
				{
					for (auto& e2 : _edge_table[j])
					{
						if (judge(edge, e2))
						{
							if(e2.poly_id == -1)
								continue;
							//if (!_poly_table_index[e2.poly_id])
							//{
							//	cout << _edge_table[j].size() << endl;
							//	cout << j << " " << e2.poly_id << " " << _poly_table_index.size() << endl;
							//	//cout << "id:" << _poly_table_index[e2.poly_id]->poly_id << endl;
							//}
							const PolyTable* poly_2 = &(*_poly_table_index[e2.poly_id]);
							float z1= -(poly.a*edge.x + poly.b*edge.y_max + poly.d) / poly.c,
								z2 = - (poly_2->a*e2.x + poly_2->b*e2.y_max + poly_2->d) / poly_2->c,
								z3 = z1 - (poly.a*edge.dx*edge.dy - poly.b*edge.dy) / poly.c,
								z4 = z2 - (poly_2->a*e2.dx*e2.dy - poly_2->b*e2.dy) / poly_2->c;
							if ((z1 - z2)*(z3 - z4) < 0)
							{
								/*cout << _edge_table.size() << " [" << edge.poly_id << "] " << edge.dx
									<< " [" << e2.poly_id << "] " << e2.dx << " " << endl;*/
								vec2 tmp;
								cross_point(edge, e2, tmp);
								/*cout << "x1 =" << edge.x << " y1=" << edge.y_max << " " << edge.dx << " " << edge.dy << endl
									<< "x2 =" << e2.x << " y2=" << e2.y_max << " " << e2.dx << " " << e2.dy << endl
									<< "x =" << tmp.x << " y=" << tmp.y << " " << edge.y_max << endl << endl;*/
								

								EdgeTable edge2;
								edge2.x = round(tmp.x);
								edge2.dx = edge.x;
								edge2.dy = edge.dy- (edge.y_max - round(tmp.y));
								edge2.poly_id = -1;
								edge2.y_max = round(tmp.y);

								/*_edge_table[face_id].push_back(edge2);
								edge_change.dy = edge.y_max - round(tmp.y);*/
								//cout << edge.poly_id << " " << edge2.x << " " << edge2.dy << " " << edge_change.dy << endl;
							}
						}
					}
				}
				edge = edge_change;
			}
			_edge_table[face_id].push_back(edge);

			if (poly_y_max < round(edge.y_max))
				poly_y_max = round(edge.y_max);

			/*if (poly.poly_id == 4991)
				cout << p1.x << " " << p1.y << " " << p1.z << " "
				<< edge.x << " " 
				<< edge.dx << " " 
				<< edge.dy << " " << edge.poly_id << " "
				<< endl;*/
		}
		if (poly_y_max != INT_MIN && poly_y_max>=0 && poly_y_max<_height)
		{
			//cout <<"push _poly_table : ["<< face_id << "], " << poly_y_max << endl;
			poly.dy = poly_y_max - poly_y_min;
			_poly_table[poly_y_max].push_back(poly);
			_poly_table_index[face_id] = &(_poly_table[poly_y_max].back());
		}
		else
		{
			_edge_table[face_id].clear();
		}
	}

	{
		int index = 0;
		for (auto e : _edge_table)
		{
			cout << endl << index++ << endl;
			for (auto ee : e)
			{
				cout << "[" << ee.poly_id << "], x=" << ee.x << " dy=" << ee.dy << "   ";
			}
		}
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

// active the poly and edge
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
				ae.poly_id = e.poly_id;
				ae.z = -(p.a*e.x + p.b*e.y_max + p.d) / p.c;
				ae.dzx = -p.a / p.c;
				ae.dzy = p.b / p.c;
				_active_edge.push_back(ae);
			}
		}
	}
	_active_edge.sort();

	{
		//for (auto e : _active_edge)
		//{
		//	//cout << e.x << " ";
		//	if (e.x > 150)
		//	{
		//	cout <<y<< ":: " << _active_edge.size() << endl;
		//	cout << e.poly_id << " " << e.x << " " << e.dy << endl;
		//	}
		//}
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
	int offset = y*_width * 3;
	for (int i = __max(x1, 0); i <= x2&&i<_width; i++)
	{
		_frame_buffer[offset + i * 3] = color.x;
		_frame_buffer[offset + i * 3 + 1] = color.y;
		_frame_buffer[offset + i * 3 + 2] = color.z;
	}
}

float compute_z(const int& x, const int& y, PolyTable* p)
{
	return -(p->a * x + p->b * y + p->d) / p->c;
}

void Z_Buffer::draw_line(const int& y)
{
	list<ActiveEdgeTable>::iterator e1 = _active_edge.begin(), e2;
	vec3 color(0,0,1);
	int index = 0;
	//cout << endl << y << endl;
	if (_active_edge.size() > 0)
	{
		//cout << endl << y << " active edge size : " << _active_edge.size() << endl;
		for (auto e = ++_active_edge.begin(); e != _active_edge.end(); e++)
		{
			if (e1->poly_id != -1)
			{
				//cout << y << ": !flag :[" << e1->poly_id << "]" << endl;
				_poly_table_index[e1->poly_id]->in_out_flag = !_poly_table_index[e1->poly_id]->in_out_flag;
			}
			e2 = e;
			int in_out_flag_size = count_active_poly_flag();
		/*	cout << e1->poly_id << " " << e1->x << " "
				<< e2->poly_id << " " << e2->x << " " << in_out_flag_size << endl;*/
			if ((in_out_flag_size == 1) && (round(e1->x) != round(e2->x)))
			{
				//color = _poly_table_index[e1->poly_id]->color;
				for (auto id : _active_poly)
				{
					if (_poly_table_index[id]->in_out_flag)
					{
						color = _poly_table_index[id]->color;
					}
				}
				draw_region(round(e1->x), round(e2->x), y, color);
				/*cout << "1 : "
					<< e1->poly_id << " " << e1->x << " " << e1->dx << ", "
					<< e2->poly_id << " " << e2->x << " " << e2->dx << " "
					<< " (" << color.x << "," << color.y << "," << color.z << ")" << endl;*/
				//cout << y << " " << e1->x << " " << e1->dx << "," << e2->x << " " << e2->dx << endl;
			}
			else if (in_out_flag_size != 0 /*&& (round(e1->x) != round(e2->x))*/)
			{
				int judge_x = (e1->x + e2->x) / 2;
				float max_z = INT_MIN;
				int test_id = 0;  // for test
				for (auto id : _active_poly)
				{
					if (_poly_table_index[id]->in_out_flag)
					{
						float z = compute_z(judge_x, y, _poly_table_index[id]);
						if (max_z < z)
						{
							max_z = z;
							color = _poly_table_index[id]->color;
							test_id = id;// for test
						}
						else
						{
							//cout << max_z << "" << z << " " << endl;
						}
					}
				}
				draw_region(round(e1->x), round(e2->x), y, color);
				//cout << ">=2 : "
				//	<< e1->poly_id << " " << round(e1->x) /*<< " " << e1->dx*/ << ", "
				//	<< e2->poly_id << " " << round(e2->x) /*<< " " << e2->dx*/ << ", "
				//	<< "color [" << test_id << "]" << endl;
			}
			/*e1->x += e1->dx;
			e1->dy -= 1;
			if (e1->dy < 0)
				_active_edge.erase(e1);*/
			e1 = e2;
		}
		if (e1->poly_id != -1)
		{
			_poly_table_index[e1->poly_id]->in_out_flag = !_poly_table_index[e1->poly_id]->in_out_flag;
		}
		//cout << "!flag: [" << e1->poly_id << "]" << endl;
		//ActiveEdgeTable* end = &_active_edge.back();
		/*e1->x += e1->dx;
		e1->dy -= 1;
		if (e1->dy < 0)
			_active_edge.erase(e1);*/
	}
}

// after the draw, update the poly and edge
void Z_Buffer::update(const int& y)
{
	//cout << _active_poly.size() << endl;
	for (auto p = _active_poly.begin(); p != _active_poly.end();)
	{
		_poly_table_index[*p]->dy--;
		if (_poly_table_index[*p]->dy < 0)
			p = _active_poly.erase(p);
		else
			p++;
	}
	//cout << _active_edge.size() << endl;
	int index = 0;
	for (auto e = _active_edge.begin(); e != _active_edge.end();)
	{
		cout << endl << index++ << endl;
		e->x += e->dx;
		cout << "x=" << e->x << " dy=" << e->dy << " [" << e->poly_id << "]" << endl;
		e->dy--;
		if (e->dy < 0)
		{
			//cout << "dy<0 " << e->poly_id << endl;
			if (e->poly_id == -1)
			{
				cout << "!!!" << endl;
			}
			if (e->poly_id != -1 && count(_active_poly.begin(), _active_poly.end(), e->poly_id) != 0)
			{
				//cout << "---" << endl;
				for (auto edge_add : _edge_table[e->poly_id])
				{
					// cout << e.x << " " << e.dx << " " << e.dy << " " << e.y_max << endl;
					//cout << edge_add.y_max << endl;
					//cout << e->poly_id << endl;
					if (round(edge_add.y_max) == y)
					{
						ActiveEdgeTable ae;
						ae.x = edge_add.x;
						ae.dx = edge_add.dx;
						ae.dy = edge_add.dy;
						ae.poly_id = e->poly_id;
						ae.z = e->z;
						ae.dzx = e->dzx;
						ae.dzy = e->dzy;
						_active_edge.push_back(ae);
						cout << "push " << e->poly_id << endl;
					}
				}
			}
			e = _active_edge.erase(e);
		}
		else
			e++;
	}
}

void Z_Buffer::draw()
{
	for (int j = 0; j <= 300; j++)
	{
		for (int i = 0; i <= 420; i++)
		{
			int index = j*_width * 3;
			_frame_buffer[index + i * 3] = 1;
			_frame_buffer[index + i * 3 + 1] = 0;
			_frame_buffer[index + i * 3 + 2] = 0;
		}
	}

	for (int y = _height - 1; y >= 0; y--)
	{
		cout << y << endl;
		active(y);
		draw_line(y);
		update(y);
	}
}