// Termm--Fall 2020

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.resize(kMaxVertices);
	colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{
	interaction_mode = ROTATE_MODEL;
	// m_rotation_model = mat4(1.0f);
	m_rotation_model = mat4(1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, 1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 1.0f, 0.0f,
							0.0f, 0.0f, 0.0f, 1.0f);
	m_translate_model = mat4(1.0f);
	m_scale_model = mat4(1.0f);
	m_rotation_view = mat4(1.0f);
	m_translate_view = mat4(1.0f);
	m_near = 1.0f;
	m_far = 20.0f;
	m_aspect = 1.0f;
	m_fov = glm::radians(100.0f);
	m_viewport = make_pair(vec2(-0.9f, 0.9f), vec2(0.9f, -0.9f));
	m_initial_camera_pos = vec3(0.0f, 0.0f, 5.0f);
	m_viewport_mouse_clicked = false;

	m_prev_mouse_x = 0.0f;
	m_prev_mouse_y = 0.0f;
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

void A2::reset(){
	interaction_mode = ROTATE_MODEL;
	m_rotation_model = mat4(1.0f);
	m_translate_model = mat4(1.0f);
	m_scale_model = mat4(1.0f);
	m_rotation_view = mat4(1.0f);
	m_translate_view = mat4(1.0f);
	m_near = 1.0f;
	m_far = 20.0f;
	m_aspect = 1.0f;
	m_fov = glm::radians(100.0f);
	m_viewport = make_pair(vec2(-0.9f, 0.9f), vec2(0.9f, -0.9f));
	m_initial_camera_pos = vec3(0.0f, 0.0f, 5.0f);
	m_viewport_mouse_clicked = false;

	m_prev_mouse_x = 0.0f;
	m_prev_mouse_y = 0.0f;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//---------------------------------------------------------------------------------------- Spring 2020
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & V0,   // Line Start (NDC coordinate)
		const glm::vec2 & V1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = V0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = V1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

vector<line3d> A2::getcubeLines(){
	const float l = 1.0f;

	vec4 p1 = vec4(-l, -l, -l, 1.0f);
	vec4 p2 = vec4(-l, -l,  l, 1.0f);
	vec4 p3 = vec4(-l,  l,  l, 1.0f);
	vec4 p4 = vec4(-l,  l, -l, 1.0f);
	vec4 p5 = vec4( l, -l, -l, 1.0f);
	vec4 p6 = vec4( l, -l,  l, 1.0f);
	vec4 p7 = vec4( l,  l,  l, 1.0f);
	vec4 p8 = vec4( l,  l, -l, 1.0f);
	
	vector<line3d> cubelines;
	// Face 1
	cubelines.push_back(line3d(p1, p2, WHITE));
	cubelines.push_back(line3d(p2, p3, WHITE));
	cubelines.push_back(line3d(p3, p4, WHITE));
	cubelines.push_back(line3d(p4, p1, WHITE));
	// Face 2
	cubelines.push_back(line3d(p5, p6, WHITE));
	cubelines.push_back(line3d(p6, p7, WHITE));
	cubelines.push_back(line3d(p7, p8, WHITE));
	cubelines.push_back(line3d(p8, p5, WHITE));
	// Link them
	cubelines.push_back(line3d(p1, p5, WHITE));
	cubelines.push_back(line3d(p2, p6, WHITE));
	cubelines.push_back(line3d(p3, p7, WHITE));
	cubelines.push_back(line3d(p4, p8, WHITE));

	return cubelines;
}

vector<line3d> A2::getgnomonlines(const vec3& c1, const vec3& c2, const vec3& c3){
	const float l = 0.5f;
	vector<line3d> lines;
	vec4 p1 = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 p2 = vec4(   l, 0.0f, 0.0f, 1.0f);
	vec4 p3 = vec4(0.0f,    l, 0.0f, 1.0f);
	vec4 p4 = vec4(0.0f, 0.0f,    l, 1.0f);
	lines.push_back(line3d(p1, p2, c1));
	lines.push_back(line3d(p1, p3, c2));
	lines.push_back(line3d(p1, p4, c3));

	return lines;
}

void A2::draw3dlines(vector<line3d> lines){
	for(line3d line: lines){
		setLineColour(line.colour);
		drawLine(vec2(line.start), vec2(line.end));
		// cout << vec2(line.start) << " " << vec2(line.end) << endl;
	}
	// cout << endl;
}

vector<line3d> A2::transformLines(vector<line3d> lines, mat4 T){
	vector<line3d> lines_transform;
	for(line3d line: lines){
		vec4 t_start = T * line.start;
		vec4 t_end = T * line.end;
		lines_transform.push_back(line3d(t_start, t_end, line.colour));
	}
	return lines_transform;
}
//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();
	
	// Draw outer square:
	// setLineColour(vec3(1.0f, 0.7f, 0.8f));
	// drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
	// drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
	// drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
	// drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));


	// // Draw inner square:
	// setLineColour(vec3(0.2f, 1.0f, 1.0f));
	// drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
	// drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
	// drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
	// drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));
	// vector<vec3> cube_gnomon_colors{RED, GREEN, BLUE};
	vector<line3d> all_lines;
	vector<line3d> cubelines = getcubeLines();
	vector<line3d> cube_gnomonlines = getgnomonlines(RED, GREEN, BLUE);
	all_lines.insert(all_lines.end(), cubelines.begin(), cubelines.end());
	all_lines.insert(all_lines.end(), cube_gnomonlines.begin(), cube_gnomonlines.end());
	all_lines = transformLines(all_lines, m_rotation_model);
	all_lines = transformLines(all_lines, m_scale_model);
	all_lines = transformLines(all_lines, m_translate_model);
	// cout << "Model Translation: " << m_translate_model << endl;
	// cout << "View Translation" << m_translate_view << endl;

	vector<line3d> world_gnomonlines = getgnomonlines(YELLOW, PINK, TIFFANY);
	// transformLines(world_gnomonlines, get_translate_matrix(m_initial_camera_pos.x, m_initial_camera_pos.y, m_initial_camera_pos.z));
	all_lines.insert(all_lines.end(), world_gnomonlines.begin(), world_gnomonlines.end());
	mat4 Projection = get_projection_matrix();
	// cubelines.insert(cubelines.begin(), gnomonlines.begin(), gnomonlines.end());
	// cubelines = transformLines(cubelines, m_rotation_model); 
	// cubelines = transformLines(cubelines, m_scale_model);
	// cubelines = transformLines(cubelines, m_translate_model);
	// cubelines = transformLines(cubelines, Projection);
	// cube_gnomonlines = transformLines(cube_gnomonlines, m_rotation_model);
	// cube_gnomonlines = transformLines(cube_gnomonlines, m_translate_model);
	// cube_gnomonlines = transformLines(cube_gnomonlines, Projection);
	all_lines = transformLines(all_lines, m_rotation_view);
	all_lines = transformLines(all_lines, get_translate_matrix(m_initial_camera_pos.x, m_initial_camera_pos.y, m_initial_camera_pos.z));
	all_lines = transformLines(all_lines, m_translate_view);
	all_lines = transformLines(all_lines, Projection);
	clip_nearandfar(all_lines);
	clip_viewport(all_lines);

	draw3dlines(all_lines);
	drawViewport();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


	// Add more gui elements here here ...
	ImGui::RadioButton("Rotation View   (o)", (int*)&interaction_mode, ROTATE_VIEW);
	ImGui::RadioButton("Translate View  (e)", (int*)&interaction_mode, TRANSLATE_VIEW);
	ImGui::RadioButton("Perspective     (p)", (int*)&interaction_mode, PERSPECTIVE);
	ImGui::RadioButton("Rotation Model  (r)", (int*)&interaction_mode, ROTATE_MODEL);
	ImGui::RadioButton("Translate Model (t)", (int*)&interaction_mode, TRANSLATE_MODEL);
	ImGui::RadioButton("Scale Model     (s)", (int*)&interaction_mode, SCALE_MODEL);
	ImGui::RadioButton("Viewport        (v)", (int*)&interaction_mode, VIEWPORT);

	// Create Button, and check if it was clicked:
	if( ImGui::Button( "Quit Application(q)" ) ) {
		glfwSetWindowShouldClose(m_window, GL_TRUE);
	}
	if( ImGui::Button( "Reset(a)" ) ) {
		reset();
	}

	ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
	ImGui::Text( "Near: %.1f, Far: %.1f", m_near, m_far );
	// For Debug:
	// ImGui::Text( "FOV: %.1f", m_fov );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	float theta = (xPos-m_prev_mouse_x)/300.0f;
	// theta += (yPos-m_prev_mouse_y)/300.0f;
	if(m_mouse_clicked[0]){ // Left -> x axis
		if(interaction_mode == ROTATE_MODEL){
			mat4 T = get_rotation_matrix(theta, 0);
			m_rotation_model = T * m_rotation_model;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_MODEL){
			mat4 T = get_translate_matrix(theta, 0.0f, 0.0f);
			m_translate_model = T * m_translate_model;
			eventHandled = true;
		}
		else if(interaction_mode == SCALE_MODEL){
			mat4 T = get_scale_matrix(1.0f+theta, 1.0f, 1.0f);
			m_scale_model = T * m_scale_model;
			eventHandled = true;
		}
		else if(interaction_mode == ROTATE_VIEW){
			mat4 T = get_rotation_matrix(theta, 0);
			m_rotation_view = T * m_rotation_view;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_VIEW){
			mat4 T = get_translate_matrix(theta, 0.0f, 0.0f);
			m_translate_view = T * m_translate_view;
			eventHandled = true;
		}
		else if(interaction_mode == PERSPECTIVE){
			if(m_fov+theta >= radians(5.0f) && m_fov+theta <= radians(160.0f)){
				m_fov += theta;
				// cout << radians(5.0f) << " " << radians(160.0f) << endl;
			}
			eventHandled = true;
		}
	}
	if(m_mouse_clicked[1]){ // Middle -> y axis
		if(interaction_mode == ROTATE_MODEL){
			mat4 T = get_rotation_matrix(theta, 1);
			m_rotation_model = T * m_rotation_model;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_MODEL){
			mat4 T = get_translate_matrix(0.0f, theta, 0.0f);
			m_translate_model = T * m_translate_model;
			eventHandled = true;
		}
		else if(interaction_mode == SCALE_MODEL){
			mat4 T = get_scale_matrix(1.0f, 1.0f+theta, 1.0f);
			m_scale_model = T * m_scale_model;
			eventHandled = true;
		}
		else if(interaction_mode == ROTATE_VIEW){
			mat4 T = get_rotation_matrix(theta, 1);
			m_rotation_view = T * m_rotation_view;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_VIEW){
			mat4 T = get_translate_matrix(0.0f, theta, 0.0f);
			m_translate_view = T * m_translate_view;
			eventHandled = true;
		}
		else if(interaction_mode == PERSPECTIVE){
			if(m_near+theta >= 0.1f){
				m_near += theta;
			}
			eventHandled = true;
		}
	}
	if(m_mouse_clicked[2]){ // Right -> z axis
		if(interaction_mode == ROTATE_MODEL){
			mat4 T = get_rotation_matrix(theta, 2);
			m_rotation_model = T * m_rotation_model;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_MODEL){
			mat4 T = get_translate_matrix(0.0f, 0.0f, theta);
			m_translate_model = T * m_translate_model;
			eventHandled = true;
		}
		else if(interaction_mode == SCALE_MODEL){
			mat4 T = get_scale_matrix(1.0f, 1.0f, 1.0f+theta);
			m_scale_model = T * m_scale_model;
			eventHandled = true;
		}
		else if(interaction_mode == ROTATE_VIEW){
			mat4 T = get_rotation_matrix(theta, 2);
			m_rotation_view = T * m_rotation_view;
			eventHandled = true;
		}
		else if(interaction_mode == TRANSLATE_VIEW){
			mat4 T = get_translate_matrix(0.0f, 0.0f, theta);
			m_translate_view = T * m_translate_view;
			eventHandled = true;
		}
		else if(interaction_mode == PERSPECTIVE){
			if(m_far+theta <= 30.0f){
				m_far += theta;
			}
			eventHandled = true;
		}
	}
	if(interaction_mode == VIEWPORT && m_viewport_mouse_clicked){
		float new_y = m_window_size.second - yPos;
		m_viewport.second = vec2((xPos-m_window_size.first/2.0f)/(m_window_size.first/2.0f), (new_y-m_window_size.second/2.0f)/(m_window_size.second/2.0f));
		// viewport_judge();
		eventHandled = true;
	}
	m_prev_mouse_x = xPos;
	m_prev_mouse_y = yPos;
	// cout << xPos << " " << yPos << endl;
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(button == GLFW_MOUSE_BUTTON_LEFT){
		if(actions == GLFW_PRESS){
			m_mouse_clicked[0] = true;
			if(interaction_mode == VIEWPORT){
				float new_y = m_window_size.second - m_prev_mouse_y;
				if(!m_viewport_mouse_clicked){
					m_viewport.first = vec2((m_prev_mouse_x-m_window_size.first/2.0f)/(m_window_size.first/2.0f), (new_y-m_window_size.second/2.0f)/(m_window_size.second/2.0f));
				}
				else{
					m_viewport.second = vec2((m_prev_mouse_x-m_window_size.first/2.0f)/(m_window_size.first/2.0f), (new_y-m_window_size.second/2.0f)/(m_window_size.second/2.0f));
					viewport_judge();
				}
				m_viewport_mouse_clicked = !m_viewport_mouse_clicked;
			}
			eventHandled = true;
		}
		else if(actions == GLFW_RELEASE){
			m_mouse_clicked[0] = false;
			eventHandled = true;
		}
	}
	else if(button == GLFW_MOUSE_BUTTON_MIDDLE){
		if(actions == GLFW_PRESS){
			m_mouse_clicked[1] = true;
			eventHandled = true;
		}
		else if(actions == GLFW_RELEASE){
			m_mouse_clicked[1] = false;
			eventHandled = true;
		}
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT){
		if(actions == GLFW_PRESS){
			m_mouse_clicked[2] = true;
			eventHandled = true;
		}
		else if(actions == GLFW_RELEASE){
			m_mouse_clicked[2] = false;
			eventHandled = true;
		}
	}
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
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
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	m_window_size = make_pair((float)width, (float)height);

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(action == GLFW_PRESS){
		switch (key)
		{
		case GLFW_KEY_Q:
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
			break;
		case GLFW_KEY_O:
			interaction_mode = ROTATE_VIEW;
			eventHandled = true;
			break;
		case GLFW_KEY_E:
			interaction_mode = TRANSLATE_VIEW;
			eventHandled = true;
			break;
		case GLFW_KEY_P:
			interaction_mode = PERSPECTIVE;
			eventHandled = true;
			break;
		case GLFW_KEY_R:
			interaction_mode = ROTATE_MODEL;
			eventHandled = true;
			break;
		case GLFW_KEY_T:
			interaction_mode = TRANSLATE_MODEL;
			eventHandled = true;
			break;
		case GLFW_KEY_S:
			interaction_mode = SCALE_MODEL;
			eventHandled = true;
			break;
		case GLFW_KEY_V:
			interaction_mode = VIEWPORT;
			eventHandled = true;
			break;
		default:
			break;
		}
	}

	return eventHandled;
}

mat4 A2::get_rotation_matrix(float theta, int coor){
	mat4 T = mat4(1.0f);
	if(coor == 0){ // Rx
		T = mat4(1.0f,       0.0f,        0.0f, 0.0f,
				 0.0f, cos(theta), -sin(theta), 0.0f,
				 0.0f, sin(theta),  cos(theta), 0.0f,
				 0.0f,       0.0f,        0.0f, 1.0f);
	}
	else if(coor == 1){ // Ry
		T = mat4( cos(theta), 0.0f, sin(theta), 0.0f,
				        0.0f, 1.0f,       0.0f, 0.0f,
				 -sin(theta), 0.0f, cos(theta), 0.0f,
				        0.0f, 0.0f,       0.0f, 1.0f);
	}
	else if(coor == 2){ // Rz
		T = mat4(cos(theta), -sin(theta), 0.0f, 0.0f,
				 sin(theta),  cos(theta), 0.0f, 0.0f,
				       0.0f,        0.0f, 1.0f, 0.0f,
				       0.0f,        0.0f, 0.0f, 1.0f);
	}
	// m_rotation_model = transpose(T)*m_rotation_model;
	return transpose(T);
}

mat4 A2::get_translate_matrix(float dx, float dy, float dz){
	mat4 T = mat4(1.0f, 0.0f, 0.0f, dx,
				  0.0f, 1.0f, 0.0f, dy,
				  0.0f, 0.0f, 1.0f, dz,
				  0.0f, 0.0f, 0.0f, 1.0f);
	// m_translate_model = transpose(T)*m_translate_model;
	return transpose(T);
}

mat4 A2::get_scale_matrix(float sx, float sy, float sz){
	mat4 T = mat4(sx  , 0.0f, 0.0f, 0.0f,
				  0.0f, sy  , 0.0f, 0.0f,
				  0.0f, 0.0f, sz  , 0.0f,
				  0.0f, 0.0f, 0.0f, 1.0f);
	// m_scale_model = transpose(T)*m_scale_model;
	return transpose(T);
}

mat4 A2::get_projection_matrix(){
	mat4 T = mat4((1/tan(m_fov/2.0f))/m_aspect, 0.0f, 0.0f, 0.0f,
				  0.0f,(1/tan(m_fov/2.0f)), 0.0f, 0.0f,
				  0.0f, 0.0f, (m_far+m_near)/(m_far-m_near), (-2.0f*m_far*m_near)/(m_far-m_near),
				  0.0f, 0.0f, 1.0f, 0.0f);
	return transpose(T);
}

void A2::clip_nearandfar(vector<line3d>& lines){
	vector<line3d> after_clipped;
	// clip near plane
	for(line3d line: lines){
		if(line.start.z < m_near && line.end.z < m_near){
			continue; // trivially reject
		}
		else if(line.start.z >= m_near && line.end.z >= m_near){
			after_clipped.push_back(line); // trivally accept
		}
		else if(line.end.z < m_near){
			float t = (m_near - line.start.z)/(line.end.z - line.start.z);
			after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start + t*line.end, line.colour));
		}
		else{
			float t = (m_near - line.start.z)/(line.end.z - line.start.z);
			after_clipped.push_back(line3d((1.0f-t)*line.start + t*line.end, line.end, line.colour));
		}
	}
	lines = after_clipped;
	after_clipped.clear();
	// clip far plane
	for(line3d line: lines){
		if(line.start.z > m_far && line.end.z > m_far){
			continue; // trivially reject
		}
		else if(line.start.z <= m_far && line.end.z <= m_far){
			after_clipped.push_back(line); // trivally accept
		}
		else if(line.start.z <= m_far){
			float t = (m_far - line.start.z)/(line.end.z - line.start.z);
			after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start + t*line.end, line.colour));
		}
		else{
			float t = (m_far - line.start.z)/(line.end.z - line.start.z);
			after_clipped.push_back(line3d((1.0f-t)*line.start + t*line.end, line.end, line.colour));
		}
	}
	lines = after_clipped;
}

void A2::drawViewport(){
	// float w = m_viewport.second.x - m_viewport.first.x;
	// float h = m_viewport.first.y - m_viewport.second.y;
	vec2 tl = m_viewport.first;
	vec2 br = m_viewport.second;
	vec2 tr = vec2(br.x, tl.y);
	vec2 bl = vec2(tl.x, br.y);

	setLineColour(ORANGE);
	drawLine(tl, tr);
	drawLine(tr, br);
	drawLine(br, bl);
	drawLine(bl, tl);
}

void A2::clip_viewport(vector<line3d>& lines){
	vec2 tl = m_viewport.first;
	vec2 br = m_viewport.second;
	vec2 tr = vec2(br.x, tl.y);
	vec2 bl = vec2(tl.x, br.y);

	vector<line3d> after_clipped;
	// clip left line
	for(line3d line: lines){
		if(line.start.x < tl.x && line.end.x < tl.x){
			continue; // trivially reject
		}
		else if(line.start.x >= tl.x && line.end.x >= tl.x){
			after_clipped.push_back(line); // trivially accept
		}
		else{
			float t = (tl.x - line.start.x)/(line.end.x - line.start.x);
			if(line.start.x < tl.x){
				after_clipped.push_back(line3d((1.0f-t)*line.start+t*line.end, line.end, line.colour));
			}
			else{
				after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start+t*line.end, line.colour));
			}
		}
	}
	lines = after_clipped; 
	after_clipped.clear();
	// clip right line
	for(line3d line: lines){
		if(line.start.x > br.x && line.end.x > br.x){
			continue; // trivially reject
		}
		else if(line.start.x <= br.x && line.end.x <= br.x){
			after_clipped.push_back(line); // trivially accept
		}
		else{
			float t = (br.x - line.start.x)/(line.end.x - line.start.x);
			if(line.start.x > br.x){
				after_clipped.push_back(line3d((1.0f-t)*line.start+t*line.end, line.end, line.colour));
			}
			else{
				after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start+t*line.end, line.colour));
			}
		}
	}
	lines = after_clipped; 
	after_clipped.clear();
	// clip top line
	for(line3d line: lines){
		if(line.start.y > tl.y && line.end.y > tl.y){
			continue; // trivially reject
		}
		else if(line.start.y <= tl.y && line.end.y <= tl.y){
			after_clipped.push_back(line); // trivially accept
		}
		else{
			float t = (tl.y - line.start.y)/(line.end.y - line.start.y);
			if(line.start.y > tl.y){
				after_clipped.push_back(line3d((1.0f-t)*line.start+t*line.end, line.end, line.colour));
			}
			else{
				after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start+t*line.end, line.colour));
			}
		}
	}
	lines = after_clipped; 
	after_clipped.clear();
	// clip bottom line
	for(line3d line: lines){
		if(line.start.y < br.y && line.end.y < br.y){
			continue; // trivially reject
		}
		else if(line.start.y >= br.y && line.end.y >= br.y){
			after_clipped.push_back(line); // trivially accept
		}
		else{
			float t = (br.y - line.start.y)/(line.end.y - line.start.y);
			if(line.start.y < br.y){
				after_clipped.push_back(line3d((1.0f-t)*line.start+t*line.end, line.end, line.colour));
			}
			else{
				after_clipped.push_back(line3d(line.start, (1.0f-t)*line.start+t*line.end, line.colour));
			}
		}
	}
	lines = after_clipped; 
}

void A2::viewport_judge(){
	if(m_viewport.first.x < m_viewport.second.x && m_viewport.first.y > m_viewport.second.y){
		return;
	}
	else{
		if(m_viewport.first.x > m_viewport.second.x){
			float temp = m_viewport.first.x;
			m_viewport.first.x = m_viewport.second.x;
			m_viewport.second.x = temp;
		}
		if(m_viewport.first.y < m_viewport.second.y){
			float temp = m_viewport.first.y;
			m_viewport.first.y = m_viewport.second.y;
			m_viewport.second.y = temp;
		}
	}
}