// Termm--Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>

using namespace glm;

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;
const vec3 RED = vec3(1.0f, 0.0f, 0.0f);
const vec3 GREEN = vec3(0.0f, 1.0f, 0.0f);
const vec3 BLUE = vec3(0.0f, 0.0f, 1.0f);
const vec3 WHITE = vec3(1.0f, 1.0f, 1.0f);
const vec3 BLACK = vec3(0.0f, 0.0f, 0.0f);
const vec3 YELLOW = vec3(1.0f, 1.0f, 0.0f);
const vec3 PINK = vec3(1.0f, 0.0f, 1.0f);
const vec3 TIFFANY = vec3(0.0f, 1.0f, 1.0f);

class line3d{
	public:
		line3d(vec4 st, vec4 e, vec3 c){
			start = st;
			end = e;
			colour = c;
		};
		vec4 start;
		vec4 end;
		vec3 colour;
};

enum IntersectionMode{
	ROTATE_VIEW,
	TRANSLATE_VIEW,
	PERSPECTIVE,
	ROTATE_MODEL,
	TRANSLATE_MODEL,
	SCALE_MODEL,
	VIEWPORT
};

// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);
	void reset();
	void draw3dlines(std::vector<line3d> lines);
	glm::mat4 get_rotation_matrix(float theta, int coor);
	glm::mat4 get_translate_matrix(float dx, float dy, float dz);
	glm::mat4 get_scale_matrix(float sx, float sy, float sz);
	glm::mat4 get_projection_matrix();

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object
	float m_prev_mouse_x;
	float m_prev_mouse_y;
	bool m_mouse_clicked[3];

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;
	glm::mat4 m_rotation_model;
	glm::mat4 m_translate_model;
	glm::mat4 m_scale_model;
	glm::mat4 m_rotation_view;
	glm::mat4 m_translate_view;

	float m_fov;
	float m_aspect;
	float m_near;
	float m_far;
	std::pair<vec2, vec2> m_viewport;

	// my own code:
	IntersectionMode interaction_mode;
	std::vector<line3d> getcubeLines();
	std::vector<line3d> getgnomonlines(const vec3& c1, const vec3& c2, const vec3& c3);
	std::vector<line3d> transformLines(std::vector<line3d> lines, glm::mat4 T);

	void clip_nearandfar(std::vector<line3d>& lines);
};
