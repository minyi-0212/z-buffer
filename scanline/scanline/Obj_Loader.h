#pragma once

#include <vector>
#include <string>
#include <map>
#include <glm.hpp>

class Object
{
public:
	void read_obj(std::string filename);
	void get_vertices(std::vector<glm::vec3>& vertices) 
	{
		vertices = _vertices;
	};
	void get_faces(std::vector<std::vector<std::vector<int>>>& faces);
	void get_faces(std::vector<std::vector<int>>& faces);
	void get_faces(std::vector<std::vector<glm::vec3>>& faces);

private:
	std::vector<glm::vec3> _vertices;//顶点
	std::vector<int> _faces;//面
	std::vector<std::pair<float, float>> _texturecoords;//纹理坐标
	std::vector<glm::vec3> _normals;//法向量
	std::vector<std::pair<int, int>> _row_col;//面参数
	std::vector<std::string> _materials;
};
