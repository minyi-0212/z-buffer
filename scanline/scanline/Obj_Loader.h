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
	std::vector<glm::vec3> _vertices;//����
	std::vector<int> _faces;//��
	std::vector<std::pair<float, float>> _texturecoords;//��������
	std::vector<glm::vec3> _normals;//������
	std::vector<std::pair<int, int>> _row_col;//�����
	std::vector<std::string> _materials;
};
