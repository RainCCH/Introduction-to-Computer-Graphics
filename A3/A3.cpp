// Termm-Fall 2020

#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <math.h>
#include <stack>
#include <queue>

using namespace glm;

static bool show_gui = true;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.85, 0.85, 0.85, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj"),
			getAssetFilePath("cylinder.obj"),
			getAssetFilePath("plane.obj")
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();

	reset();

	m_node_count = m_rootNode->totalSceneNodes();
	selected.resize(m_node_count, false);

	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

void A3::reset(){
	m_interaction_mode = Interaction_Mode::POSITION;
	options_zbuffer = true;
	options_backface_culling = false;
	options_frontface_culling = false;
	options_circle = false;
	m_trackball_rotation = glm::vec3(0.0f);
	m_model_rotation_matrix = glm::mat4(1.0f);
	m_model_translation_matrix = glm::mat4(1.0f);
	do_picking = false;
	m_original_nodes_transformations.clear();
	std::queue<SceneNode*> q;
	q.push(m_rootNode.get());
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		cur->isSelected = false;
		m_original_nodes_transformations[cur->m_name] = cur->get_transform();
		for(SceneNode *child: cur->children){
			q.push(child);
		}
	}
	resetAll();
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could Not Open " << filename << std::endl;
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("Phong.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("Phong.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(10.0f, 10.0f, 10.0f);
	m_light.rgbIntensity = vec3(1.0f); // light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;

		location = m_shader.getUniformLocation("picking");
		glUniform1i( location, do_picking ? 1 : 0 );
		CHECK_GL_ERRORS;

		if(!do_picking){
			//-- Set LightSource uniform for the scene:
			{
				location = m_shader.getUniformLocation("light.position");
				glUniform3fv(location, 1, value_ptr(m_light.position));
				location = m_shader.getUniformLocation("light.rgbIntensity");
				glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
				CHECK_GL_ERRORS;
			}

			//-- Set background light ambient intensity
			{
				location = m_shader.getUniformLocation("ambientIntensity");
				vec3 ambientIntensity(0.25f);
				glUniform3fv(location, 1, value_ptr(ambientIntensity));
				CHECK_GL_ERRORS;
			}
		}
		
	}
	m_shader.disable();
}

void A3::resetPosition(){
	m_model_translation_matrix = glm::mat4(1.0f);
}

void A3::resetOrientation(){
	m_model_rotation_matrix = glm::mat4(1.0f);
}

void A3::resetJoints(){
	std::queue<SceneNode*> q;
	q.push(m_rootNode.get());
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		if(cur->m_nodeType == NodeType::JointNode){
			cur->set_transform(m_original_nodes_transformations[cur->m_name]);
		}
		for(SceneNode *child: cur->children){
			q.push(child);
		}
	}
	joint_index = 0;
	m_joints_transformations.clear();
	m_joints_transformations.push_back(m_original_nodes_transformations);
	std::fill(selected.begin(), selected.end(), false);
}

void A3::resetAll(){
	resetPosition();
	resetOrientation();
	resetJoints();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	windowFlags |= ImGuiWindowFlags_MenuBar;
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...
		if( ImGui::BeginMenuBar() ){
			if (ImGui::BeginMenu("Edit")){
				if( ImGui::MenuItem( "Undo    (U)" )) undo_joints();
				if( ImGui::MenuItem( "Redo    (R)" )) redo_joints();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Application"))
			{
				if (ImGui::MenuItem("Reset Position       (I)")) resetPosition();
				if (ImGui::MenuItem("Reset Orientation    (O)")) resetOrientation();
				if (ImGui::MenuItem("Reset Joints         (S)")) resetJoints();
				if (ImGui::MenuItem("Reset All            (A)")) resetAll();
				if( ImGui::MenuItem("Quit                 (Q)")) glfwSetWindowShouldClose(m_window, GL_TRUE);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Options"))
			{
				ImGui::Checkbox("Circle               (C)", &options_circle);
				ImGui::Checkbox("Z-buffer             (Z)", &options_zbuffer);
				ImGui::Checkbox("Backface Culling     (B)", &options_backface_culling);
				ImGui::Checkbox("Frontface Culling    (F)", &options_frontface_culling);
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::RadioButton("Position/Orientation    (P)", (int*)&m_interaction_mode, POSITION);
		ImGui::RadioButton("Joints                  (J)", (int*)&m_interaction_mode, JOINTS);


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & modelMatrix,
		const bool & do_picking,
		const std::vector<bool> & selected
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * modelMatrix;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;
		if(do_picking){
			float r = float(node.m_nodeId&0xff) / 255.0f;
			float g = float((node.m_nodeId>>8)&0xff) / 255.0f;
			float b = float((node.m_nodeId>>16)&0xff) / 255.0f;

			location = shader.getUniformLocation("material.kd");
			glUniform3f( location, r, g, b );
			CHECK_GL_ERRORS;
		}
		else{
			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;

			vec3 kd = node.material.kd;
			vec3 ks = node.material.ks;
			float shininess = node.material.shininess;

			if(selected[node.m_nodeId]){
				kd = vec3(0.9f, 0.9f, 0.9f);
				ks = vec3(1.0f, 1.0f, 1.0f);
				shininess = 1.0f;
			}

			//-- Set Material values:
			location = shader.getUniformLocation("material.kd");
			glUniform3fv(location, 1, value_ptr(kd));
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("material.ks");
			glUniform3fv(location, 1, value_ptr(ks));
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("material.shininess");
			glUniform1f(location, shininess);
			CHECK_GL_ERRORS;
		}
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

	if(options_zbuffer){
		glEnable( GL_DEPTH_TEST );
	}
	if(options_backface_culling && options_frontface_culling){
		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT_AND_BACK );
	}
	else if(options_backface_culling){
		glEnable( GL_CULL_FACE );
		glCullFace( GL_BACK );
	}
	else if(options_frontface_culling){
		glEnable( GL_CULL_FACE );
		glCullFace( GL_FRONT );
	}
	renderSceneGraph(*m_rootNode);
	if(options_circle){
		renderArcCircle();
	}

	if(options_zbuffer){
		glDisable( GL_DEPTH_TEST );
	}
	else if(options_backface_culling || options_frontface_culling){
		glDisable( GL_CULL_FACE );
	}
	
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.

	// for (const SceneNode * node : root.children) {

	// 	if (node->m_nodeType != NodeType::GeometryNode)
	// 		continue;

	// 	const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

	// 	updateShaderUniforms(m_shader, *geometryNode, m_view, do_picking);


	// 	// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
	// 	BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

	// 	//-- Now render the mesh:
	// 	m_shader.enable();
	// 	glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
	// 	m_shader.disable();
	// }
	mat4 root_trans = root.get_transform();
	stack<mat4> st;
	// renderSceneNode(&root, m_model_translation_matrix * m_model_rotation_matrix, st);
	renderSceneNode(&root, m_model_translation_matrix * root_trans * m_model_rotation_matrix * inverse(root_trans), st);
	// cout << "translate: " << m_model_translation_matrix << endl;
	// cout << "rotation: " << m_model_rotation_matrix << endl;

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void A3::renderSceneNode(const SceneNode *node, mat4 model, stack<mat4> st){
	if(node == nullptr) return;

	mat4 M_push = node->get_transform();
	st.push(M_push);
	model = model * M_push;

	if (node->m_nodeType == NodeType::GeometryNode) {
		const GeometryNode *geometryNode = static_cast<const GeometryNode *>(node);

		updateShaderUniforms(
			m_shader, 
			*geometryNode, 
			m_view,
			model,
			do_picking,
			selected
		);

		BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

		m_shader.enable();
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
		m_shader.disable();
	}

	for (const SceneNode *child: node->children) {
		renderSceneNode(child, model, st);
	}

	mat4 M_pop = st.top();
	st.pop();
	model = model * glm::inverse(M_pop);
}
//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

void A3::update_postion_orientation(double x, double y){
	double delta_x = (x - m_prev_mouse_x)/300.0f;
	double delta_y = (y - m_prev_mouse_y)/300.0f;
	vec3 translate = vec3(0.0f);
	if(ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT)){
		translate.x = delta_x;
		translate.y = -delta_y;
	}
	else if(ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_MIDDLE)){
		translate.z = delta_y;
	}
	else if(ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT)){
		// trackball rotation
		// cout << "trackball rotation" << endl;
		update_trackball_rotation(x, y);
	}
	m_model_translation_matrix = glm::translate(m_model_translation_matrix, translate);
}

void A3::update_joints(double x, double y){
	// cout << "update joints" << endl;
	double delta_x = (x - m_prev_mouse_x)/300.0f;
	double delta_y = (y - m_prev_mouse_y)/300.0f;
	if(m_interaction_mode == POSITION) return;
	std::queue<SceneNode*> q;
	q.push(m_rootNode.get());
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		// cout << cur->m_name << endl;
		if(selected[cur->m_nodeId] && ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_LEFT) && cur->m_name != "head"){
			// cout << "selected" << endl;
			JointNode* parent = findParentJoint(cur);
			if(parent == nullptr) continue;
			// cout << parent->m_name << endl;
			parent->rotate('x', radiansToDegrees(delta_y));
			parent->rotate('y', radiansToDegrees(delta_x));
		}
		if(selected[cur->m_nodeId] && ImGui::IsMouseDragging(GLFW_MOUSE_BUTTON_RIGHT) && cur->m_name == "head"){
			// cout << "selected" << endl;
			JointNode* parent = findParentJoint(cur);
			if(parent == nullptr) continue;
			parent->rotate('x', radiansToDegrees(delta_y));
			parent->rotate('y', radiansToDegrees(delta_x));
		}
		for(SceneNode *child: cur->children){
			q.push(child);
		}
	}
}

void A3::update_joints_transformations(){
	std::map<string, glm::mat4> joint_trans;
	std::queue<SceneNode*> q;
	q.push(m_rootNode.get());
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		if(cur->m_nodeType == NodeType::JointNode){
			joint_trans[cur->m_name] = cur->get_transform();
		}
		for(SceneNode *child: cur->children){
			q.push(child);
		}
	}
	m_joints_transformations.push_back(joint_trans);
}

void A3::update_joints_from_data(){
	if(joint_index >= m_joints_transformations.size()) return;
	std::queue<SceneNode*> q;
	q.push(m_rootNode.get());
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		if(cur->m_nodeType == NodeType::JointNode){
			cur->set_transform(m_joints_transformations[joint_index][cur->m_name]);
		}
		for(SceneNode *child: cur->children){
			q.push(child);
		}
	}
}

void A3::undo_joints(){
	if(joint_index <= 0) return;
	joint_index--;
	update_joints_from_data();
}

void A3::redo_joints(){
	if(joint_index >= m_joints_transformations.size()-1) return;
	joint_index++;
	update_joints_from_data();
}
//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(m_interaction_mode == POSITION){
		update_postion_orientation(xPos, yPos);
		eventHandled = true;
	}
	else if(m_interaction_mode == JOINTS){
		update_joints(xPos, yPos);
		eventHandled = true;
	}
	m_prev_mouse_x = xPos;
	m_prev_mouse_y = yPos;

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (button == GLFW_MOUSE_BUTTON_LEFT && actions == GLFW_PRESS && m_interaction_mode == JOINTS) {
		double xpos, ypos;
		glfwGetCursorPos( m_window, &xpos, &ypos );

		do_picking = true;

		uploadCommonSceneUniforms();
		glClearColor(1.0, 1.0, 1.0, 1.0 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glClearColor(0.35, 0.35, 0.35, 1.0);

		draw();

		// I don't know if these are really necessary anymore.
		// glFlush();
		// glFinish();

		CHECK_GL_ERRORS;

		// Ugly -- FB coordinates might be different than Window coordinates
		// (e.g., on a retina display).  Must compensate.
		xpos *= double(m_framebufferWidth) / double(m_windowWidth);
		// WTF, don't know why I have to measure y relative to the bottom of
		// the window in this case.
		ypos = m_windowHeight - ypos;
		ypos *= double(m_framebufferHeight) / double(m_windowHeight);

		GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
		// A bit ugly -- don't want to swap the just-drawn false colours
		// to the screen, so read from the back buffer.
		glReadBuffer( GL_BACK );
		// Actually read the pixel at the mouse location.
		glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
		CHECK_GL_ERRORS;

		// Reassemble the object ID.
		unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

		if( what < m_node_count && what >= 0 ) {
			selected[what] = !selected[what];
		}

		do_picking = false;

		CHECK_GL_ERRORS;
	}
	if (!ImGui::IsMouseHoveringAnyWindow() &&
		actions == GLFW_RELEASE &&
		m_interaction_mode == JOINTS &&
		(button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_RIGHT)) {
			m_joints_transformations.erase(m_joints_transformations.begin()+1+joint_index, m_joints_transformations.end());
			joint_index++;
			update_joints_transformations();
		}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A3::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A3::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
		if(key == GLFW_KEY_Q) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
		}
		if(key == GLFW_KEY_A){
			reset();
			eventHandled = true;
		}
		if(key == GLFW_KEY_I){
			resetPosition();
			eventHandled = true;
		}
		if(key == GLFW_KEY_O){
			resetOrientation();
			eventHandled = true;
		}
		if(key == GLFW_KEY_S){
			resetJoints();
			eventHandled = true;
		}
		if(key == GLFW_KEY_C){
			options_circle = !options_circle;
			eventHandled = true;
		}
		if(key == GLFW_KEY_Z){
			options_zbuffer = !options_zbuffer;
			eventHandled = true;
		}
		if(key == GLFW_KEY_B){
			options_backface_culling = !options_backface_culling;
			eventHandled = true;
		}
		if(key == GLFW_KEY_F){
			options_frontface_culling = !options_frontface_culling;
			eventHandled = true;
		}
		if(key == GLFW_KEY_P){
			m_interaction_mode = POSITION;
			eventHandled = true;
		}
		if(key == GLFW_KEY_J){
			m_interaction_mode = JOINTS;
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}

void A3::update_trackball_rotation(double x, double y){
	float fDiameter = (m_windowWidth<m_windowHeight) ? m_windowWidth*0.5f : m_windowHeight*0.5f;
	float fVecX, fVecY, fVecZ;
	vCalcRotVec(x - m_windowWidth/2.0f, y - m_windowHeight/2.0f, m_prev_mouse_x - m_windowWidth/2.0f, m_prev_mouse_y - m_windowHeight/2.0f, fDiameter, &fVecX, &fVecY, &fVecZ);

	mat4 mNewMat;
	vAxisRotMatrix(fVecX, fVecY, fVecZ, mNewMat);
	m_model_rotation_matrix = transpose(mNewMat) * m_model_rotation_matrix;
}

JointNode* A3::findParentJoint(SceneNode *node){
	SceneNode *root = m_rootNode.get();
	std::queue<SceneNode*> q;
	q.push(root);
	while(!q.empty()){
		SceneNode *cur = q.front();
		q.pop();
		if(cur->m_nodeType == NodeType::JointNode){
			JointNode *joint = static_cast<JointNode*>(cur);
			for(SceneNode *child: joint->children){
				if(child == node){
					return joint;
				}
				q.push(child);
			}
		}
		else{
			for(SceneNode *child: cur->children){
				q.push(child);
			}
		}
	}
	return nullptr;
}

void vCopyMatrix(mat4 mSource, mat4 mDestination) 
{
    int i, j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            mDestination[i][j] = mSource[i][j];
        }
    }
}

void A3::vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
                 float *fVecX, float *fVecY, float *fVecZ) {
   long  nXOrigin, nYOrigin;
   float fNewVecX, fNewVecY, fNewVecZ,        /* Vector corresponding to new mouse location */
         fOldVecX, fOldVecY, fOldVecZ,        /* Vector corresponding to old mouse location */
         fLength;

   /* Vector pointing from center of virtual trackball to
    * new mouse position
    */
   fNewVecX    = fNewX * 2.0 / fDiameter;
   fNewVecY    = fNewY * 2.0 / fDiameter;
   fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);

   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fNewVecZ < 0.0) {
      fLength = sqrt(1.0 - fNewVecZ);
      fNewVecZ  = 0.0;
      fNewVecX /= fLength;
      fNewVecY /= fLength;
   } else {
      fNewVecZ = sqrt(fNewVecZ);
   }

   /* Vector pointing from center of virtual trackball to
    * old mouse position
    */
   fOldVecX    = fOldX * 2.0 / fDiameter;
   fOldVecY    = fOldY * 2.0 / fDiameter;
   fOldVecZ    = (1.0 - fOldVecX * fOldVecX - fOldVecY * fOldVecY);
 
   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fOldVecZ < 0.0) {
      fLength = sqrt(1.0 - fOldVecZ);
      fOldVecZ  = 0.0;
      fOldVecX /= fLength;
      fOldVecY /= fLength;
   } else {
      fOldVecZ = sqrt(fOldVecZ);
   }

   /* Generate rotation vector by calculating cross product:
    * 
    * fOldVec x fNewVec.
    * 
    * The rotation vector is the axis of rotation
    * and is non-unit length since the length of a crossproduct
    * is related to the angle between fOldVec and fNewVec which we need
    * in order to perform the rotation.
    */
   *fVecX = fOldVecY * fNewVecZ - fNewVecY * fOldVecZ;
   *fVecY = fOldVecZ * fNewVecX - fNewVecZ * fOldVecX;
   *fVecZ = fOldVecX * fNewVecY - fNewVecX * fOldVecY;
}

/*******************************************************
 * void vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, Matrix mNewMat)
 *    
 *    Calculate the rotation matrix for rotation about an arbitrary axis.
 *    
 *    The axis of rotation is specified by (fVecX,fVecY,fVecZ). The length
 *    of the vector is the amount to rotate by.
 *
 * Parameters: fVecX,fVecY,fVecZ - Axis of rotation
 *             mNewMat - Output matrix of rotation in column-major format
 *                       (ie, translation components are in column 3 on rows
 *                       0,1, and 2).
 *
 *******************************************************/
void A3::vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, mat4& mNewMat) {
    float fRadians, fInvLength, fNewVecX, fNewVecY, fNewVecZ;

    /* Find the length of the vector which is the angle of rotation
     * (in radians)
     */
    fRadians = sqrt(fVecX * fVecX + fVecY * fVecY + fVecZ * fVecZ);

    /* If the vector has zero length - return the identity matrix */
    if (fRadians > -0.000001 && fRadians < 0.000001) {
        vCopyMatrix(mat4(1.0f), mNewMat);
        return;
    }

    /* Normalize the rotation vector now in preparation for making
     * rotation matrix. 
     */
    fInvLength = 1 / fRadians;
    fNewVecX   = fVecX * fInvLength;
    fNewVecY   = fVecY * fInvLength;
    fNewVecZ   = fVecZ * fInvLength;

    /* Create the arbitrary axis rotation matrix */
    double dSinAlpha = sin(fRadians);
    double dCosAlpha = cos(fRadians);
    double dT = 1 - dCosAlpha;

    mNewMat[0][0] = dCosAlpha + fNewVecX*fNewVecX*dT;
    mNewMat[0][1] = fNewVecX*fNewVecY*dT + fNewVecZ*dSinAlpha;
    mNewMat[0][2] = fNewVecX*fNewVecZ*dT - fNewVecY*dSinAlpha;
    mNewMat[0][3] = 0;

    mNewMat[1][0] = fNewVecX*fNewVecY*dT - dSinAlpha*fNewVecZ;
    mNewMat[1][1] = dCosAlpha + fNewVecY*fNewVecY*dT;
    mNewMat[1][2] = fNewVecY*fNewVecZ*dT + dSinAlpha*fNewVecX;
    mNewMat[1][3] = 0;

    mNewMat[2][0] = fNewVecZ*fNewVecX*dT + dSinAlpha*fNewVecY;
    mNewMat[2][1] = fNewVecZ*fNewVecY*dT - dSinAlpha*fNewVecX;
    mNewMat[2][2] = dCosAlpha + fNewVecZ*fNewVecZ*dT;
    mNewMat[2][3] = 0;

    mNewMat[3][0] = 0;
    mNewMat[3][1] = 0;
    mNewMat[3][2] = 0;
    mNewMat[3][3] = 1;
}