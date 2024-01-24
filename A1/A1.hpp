// Termm--Fall 2020

#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "maze.hpp"
#include <vector>

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

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

private:
	void initGrid();
	void initFloor();
	void initCube();
	void initAvatar();
	float* generateCubeVerts();
	void coor_trans(float radius, float u, float v, float* verts);
	float* generateSphereVerts();
	// void drawAvatar(float x, float y, float z);
	// void drawCube(float x, float y, float z);
	void reset();

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.

	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Fields related to floor geometry.
	GLuint m_floor_vao;
	GLuint m_floor_vbo;

	// Fields related to avatar geometry.
	GLuint m_avatar_vao;
	GLuint m_avatar_vbo;
	// GLuint m_avatar_ebo;
	// size_t m_avatar_elements;

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;
	float m_scale_factor;
	float m_rotation_angle;
	bool m_mouse_is_pressed;
	bool m_shift_is_pressed;
	float m_last_mouse_pos;
	float m_curr_mouse_pos;
	float m_dynamic_offset;

	float colour[3]; // colour on the colour editor.
	float cube_colour[3]; // cube's colour.
	float floor_colour[3]; // floor's colour.
	float avatar_colour[3]; // avatar's colour.
	int current_col;

	// Fields related to cube geometry.
	GLuint m_cube_vao;
	GLuint m_cube_vbo;
	GLuint m_cube_height;

	glm::vec3 m_color[18][18];
	std::pair<GLint, GLint> m_avatar_pos; // XZ
	Maze maze;
};
