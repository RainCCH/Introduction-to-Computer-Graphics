// Termm-Fall 2020

#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "JointNode.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <stack>
#include <map>
#include <set>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

enum Interaction_Mode {
	POSITION,
	JOINTS
};


class A3 : public CS488Window {
public:
	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(const SceneNode &node);
	void renderArcCircle();

	void reset();

	void vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
                 float *fVecX, float *fVecY, float *fVecZ);
	void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, glm::mat4 &mNewMat);
	void update_postion_orientation(double x, double y);
	void update_trackball_rotation(double x, double y);
	void update_joints(double x, double y);
	void update_joints_transformations();
	void undo_joints();
	void redo_joints();
	void renderSceneNode(const SceneNode *node, glm::mat4 model, std::stack<glm::mat4> st);

	void resetPosition();
	void resetOrientation();
	void resetJoints();
	void resetAll();

	JointNode* findParentJoint(SceneNode *node);

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;
	int m_node_count;
	// My variables
	Interaction_Mode m_interaction_mode;
	glm::mat4 m_model_translation_matrix;
	glm::mat4 m_model_rotation_matrix;
	// previous mouse position
	double m_prev_mouse_x;
	double m_prev_mouse_y;

	bool options_circle;
	bool options_zbuffer;
	bool options_backface_culling;
	bool options_frontface_culling;
	bool do_picking;
	std::vector<bool> selected;
	// std::map<string, bool> joints_selected;

	glm::vec3 m_trackball_rotation;
	std::pair<float, float> m_window_size;
	std::map<std::string, glm::mat4> m_original_nodes_transformations;

	std::vector<std::map<std::string, glm::mat4>> m_joints_transformations;
	int joint_index;
};
