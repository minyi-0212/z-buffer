#pragma once
#include <vector>
#include <list>
#include <glm.hpp>
#include "gtc/matrix_transform.hpp"

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
	std::vector<std::vector<glm::vec3>> _faces;

	glm::mat4 _model, _view, _proj;

	void active(const int& y);
	void draw_line(const int& y);
	void draw_region(const int& x1, const int& x2, const int& y, const glm::vec3& color);
	int count_active_poly_flag();
	void update(const int& y);
	void model_to_clip(std::vector<std::vector<glm::vec3>>& faces);
	void clear();

public:
	void set_size(const int& w, const int& h);
	void look_at(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
	{
		_view = glm::lookAt(eye, center, up);
	};
	void set_ortho(const float& x1, const float& x2, const float& y1, const float& y2, const float& z1, const float& z2)
	{
		_proj = glm::ortho(x1, x2, y1, y2, z1, z2);
	};
	void init(const std::vector<std::vector<glm::vec3>>& faces, const glm::mat4& model,
		const glm::vec3& eye = glm::vec3(0, 0, 1),
		const glm::vec3& center = glm::vec3(0, 0, 0),
		const glm::vec3& up = glm::vec3(0, 1, 0),
		const float& x1 = 0, const float& x2 = 600,
		const float& y1 = 0, const float& y2 = 600,
		const float& z1 = -30, const float& z2 = 50);
	void update_model(const glm::mat4& model);
	void draw();
	void get_buffer(std::vector<float>& buf);
};