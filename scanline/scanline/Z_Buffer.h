#pragma once
#include <vector>
#include <list>
#include <glm.hpp>

struct EdgeTable 
{
	int dy, poly_id;
	float x, y_max, dx;
};

struct ActiveEdgeTable
{
	int dy, poly_id;
	float x, dx, z, dzx, dzy;
	bool operator < (const ActiveEdgeTable& o) const
	{
		return x < o.x;
	}
};

struct PolyTable 
{
	int poly_id, dy;
	float a, b, c, d;
	glm::vec3 color;
	bool in_out_flag;
};

class Z_Buffer
{
private:
	int _width, _height;
	std::vector<float> _frame_buffer;
	std::vector<std::list<PolyTable>> _poly_table;
	std::vector<PolyTable*> _poly_table_index;
	std::vector<std::list<EdgeTable>> _edge_table;
	std::vector<int> _active_poly;
	std::list<ActiveEdgeTable> _active_edge;
	//glm::vec3 _background_color;

	void active(const int& y);
	void draw_line(const int& y);
	void draw_region(const int& x1, const int& x2, const int& y, const glm::vec3& color);
	int count_active_poly_flag();
	void update(const int& y);
	void model_to_clip(std::vector<std::vector<glm::vec3>>& faces);

public:
	void set_size(const int& w, const int& h);
	void init(std::vector<std::vector<glm::vec3>>& faces);
	void draw();
	void get_buffer(std::vector<float>& buf);
};