#pragma once
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/hash.hpp"
#include<vector>

namespace tEngine {
	struct Vertex
	{
		Vertex() = default;
		Vertex(glm::vec3 pos) :Position(pos) {}
		glm::vec3 Position=glm::vec3(0,0,0);
		glm::vec3 Normal=glm::vec3(0,1,0);
		glm::vec2 TexCoords=glm::vec2(0,0);
		glm::vec4 Color=glm::vec4(1,1,1,1);
	
		bool operator==(const Vertex& v)const {
			return v.Position == Position && v.TexCoords == TexCoords && v.Color == Color ;
		}
	};
	struct Geo {
		
		Geo(std::vector<Vertex> _vertices,std::vector<uint32_t> _indices) :vertices(_vertices), indices(_indices) {}
		Geo() {};
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		size_t VertexBufferSize() {
			return vertices.size() * sizeof(Vertex);
		}
		size_t IndexBufferSize() {
			return indices.size() * sizeof(uint32_t);
		}
	};
	struct Line :public Geo {
		Line(std::vector<Vertex> _vertices) {
			vertices = _vertices;
			for (int i = 0; i < vertices.size(); ++i) {
				indices.push_back(i);
			}
		}
		Line(std::vector<Vertex> _vertices, std::vector<uint32_t> _indices) :Geo(_vertices, _indices) {}
		void Append(Vertex a, Vertex b) {
			indices.push_back(vertices.size());
			vertices.push_back(a);
			indices.push_back(vertices.size());
			vertices.push_back(b);
		}
	};
	struct Mesh:public Geo {
		
		Mesh(std::vector<Vertex> _vertices, std::vector<uint32_t> _indices) :Geo(_vertices, _indices) {}
		Mesh() {};
		
		void UpdateNormal() {
			for (auto& v : vertices) {
				v.Normal = glm::vec3(0, 0, 0);
			}
			for (size_t i = 0; i < indices.size(); i += 3) {
				int id_a = indices[i];
				int id_b = indices[i + 1];
				int id_c = indices[i + 2];
				Vertex a = vertices[id_a];
				Vertex b = vertices[id_b];
				Vertex c = vertices[id_c];

				auto normal = [](const Vertex& a, const Vertex& b, const Vertex& c) {
					auto triangleArea = [](const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
						float a = glm::length(p2 - p1);
						float b = glm::length(p2 - p3);
						float c = glm::length(p3 - p1);
						float p = (a + b + c) / 2.f;
						return sqrt(p * (p - a) * (p - b) * (p - c));
					};
					return glm::cross(b.Position - a.Position, c.Position - a.Position) * triangleArea(a.Position, b.Position, c.Position);
				};
				vertices[id_a].Normal += normal(a, b, c);
				vertices[id_b].Normal += normal(b, c, a);
				vertices[id_c].Normal += normal(c, a, b);

			}
			for (auto& v :vertices) {
				v.Normal = glm::normalize(v.Normal);
			}
		}
		
		static Mesh UnitBox(float length = 2) {
			float skyboxVertices[] = {
				// positions
				-1.0f,  1.0f, -1.0f,
				-1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f, -1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,

				-1.0f, -1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f,
				-1.0f, -1.0f,  1.0f,

				-1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f, -1.0f,
				 1.0f,  1.0f,  1.0f,
				 1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f,  1.0f,
				-1.0f,  1.0f, -1.0f,

				-1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f, -1.0f,
				 1.0f, -1.0f, -1.0f,
				-1.0f, -1.0f,  1.0f,
				 1.0f, -1.0f,  1.0f
			};

			std::vector<Vertex> vertices;
			std::vector<uint32_t>indices;
			for (int i = 0; i < 36; ++i) {
				Vertex temp;
				temp.Position = length / 2 * glm::vec3(skyboxVertices[3 * i], skyboxVertices[3 * i + 1], skyboxVertices[3 * i + 2]);
				indices.push_back(i);
				vertices.push_back(temp);
			}
			Mesh box(vertices, indices);
			box.UpdateNormal();
			return box;
		}
		static Mesh UnitSquare() {
			using namespace std;
			vector<Vertex>vertices;
			vector<unsigned int>indices;
			Vertex temp[4];
			temp[0].Position = glm::vec3(-1.0f, -1.0f, 0.0f);
			temp[0].TexCoords = glm::vec2(0, 0);
			temp[1].Position = glm::vec3(1.0f, -1.0f, 0.0f);
			temp[1].TexCoords = glm::vec2(1, 0);
			temp[2].Position = glm::vec3(-1.0f, 1.0f, 0.0f);
			temp[2].TexCoords = glm::vec2(0, 1);
			temp[3].Position = glm::vec3(1.0f, 1.0f, 0.0f);
			temp[3].TexCoords = glm::vec2(1, 1);
			for (int i = 0; i < 4; i++) {
				vertices.push_back(temp[i]);
				
			}
			indices = {0,1,2,2,1,3};
			Mesh mesh(vertices, indices);
			mesh.UpdateNormal();
			return mesh;
		}
	};
	
	inline void FlipFace(Mesh& mesh) {
		for (size_t i = 0; i < mesh.indices.size(); i+=3) {
			std::reverse(mesh.indices.begin() + i, mesh.indices.begin() + i + 2);
		}
		mesh.UpdateNormal();
	}
	
}
namespace std {
	template<> struct hash<tEngine::Vertex> {
		size_t operator()(tEngine::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.Position) ^
				(hash<glm::vec4>()(vertex.Color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.TexCoords) << 1);
		}
	};
}