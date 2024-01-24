// Termm--Fall 2020

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <sys/types.h>
#include <unistd.h>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
static const int m_stack_size = 20;
static const int m_sector_size = 20;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 ), maze(DIM)
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;

	cube_colour[0] = 0.5f;
	cube_colour[1] = 0.0f;
	cube_colour[2] = 0.0f;

	floor_colour[0] = 0.0f;
	floor_colour[1] = 0.5f;
	floor_colour[2] = 0.0f;

	avatar_colour[0] = 0.0f;
	avatar_colour[1] = 0.0f;
	avatar_colour[2] = 0.5f;

	m_avatar_pos.first = -1;
	m_avatar_pos.second = -1;

	m_cube_height = 1;
	m_scale_factor = 1.0f;
	m_rotation_angle = 0.0f;
	m_mouse_is_pressed = false;
	m_shift_is_pressed = false;

	m_last_mouse_pos = 0.0f;
	m_curr_mouse_pos = 0.0f;
	m_dynamic_offset = 0.0f;
	maze.reset();
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Initialize random number generator
	int rseed=getpid();
	srandom(rseed);
	// Print random number seed in case we want to rerun with
	// same random numbers
	cout << "Random number seed = " << rseed << endl;

	// DELETE FROM HERE...
	// Maze m(DIM);
	// m.digMaze();
	// m.printMaze();
	// ...TO HERE
	
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );

	initGrid();
	initCube();
	initFloor();
	initAvatar();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, 2.*float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );

	proj = glm::perspective( 
		glm::radians( 30.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::initFloor(){
	float floor_vertices[] = {
		0.0f, 0.0f, 0.0f,
		(float)DIM, 0.0f, 0.0f,
		0.0f, 0.0f, (float)DIM,
		(float)DIM, 0.0f, (float)DIM
	};
	size_t sz = sizeof(floor_vertices);
	glGenVertexArrays(1, &m_floor_vao);
	glBindVertexArray(m_floor_vao);

	glGenBuffers(1, &m_floor_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_floor_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz, floor_vertices, GL_STATIC_DRAW);

	GLint posAttrib = m_shader.getAttribLocation("position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// delete [] floor_vertices;

	CHECK_GL_ERRORS;
}

float* A1::generateCubeVerts(){
	size_t sz = 3*3*6*2;
	float *verts = new float[sz];
	float x = 0.0f, y = 0.0f, z = 0.0f;
	verts[0] = x, verts[1] = y, verts[2] = z;
	verts[3] = x + 1, verts[4] = y, verts[5] = z;
	verts[6] = x, verts[7] = y + 1, verts[8] = z;
	verts[9] = x + 1, verts[10] = y, verts[11] = z;
	verts[12] = x, verts[13] = y + 1, verts[14] = z;
	verts[15] = x + 1, verts[16] = y + 1, verts[17] = z;

	verts[18] = x, verts[19] = y, verts[20] = z + 1;
	verts[21] = x + 1, verts[22] = y, verts[23] = z + 1;
	verts[24] = x, verts[25] = y + 1, verts[26] = z + 1;
	verts[27] = x + 1, verts[28] = y, verts[29] = z + 1;
	verts[30] = x, verts[31] = y + 1, verts[32] = z + 1;
	verts[33] = x + 1, verts[34] = y + 1, verts[35] = z + 1;

	verts[36] = x, verts[37] = y, verts[38] = z;
	verts[39] = x, verts[40] = y + 1, verts[41] = z;
	verts[42] = x, verts[43] = y, verts[44] = z + 1;
	verts[45] = x, verts[46] = y + 1, verts[47] = z;
	verts[48] = x, verts[49] = y, verts[50] = z + 1;
	verts[51] = x, verts[52] = y + 1, verts[53] = z + 1;

	verts[54] = x + 1, verts[55] = y, verts[56] = z;
	verts[57] = x + 1, verts[58] = y + 1, verts[59] = z;
	verts[60] = x + 1, verts[61] = y, verts[62] = z + 1;
	verts[63] = x + 1, verts[64] = y + 1, verts[65] = z;
	verts[66] = x + 1, verts[67] = y, verts[68] = z + 1;
	verts[69] = x + 1, verts[70] = y + 1, verts[71] = z + 1;

	verts[72] = x, verts[73] = y, verts[74] = z;
	verts[75] = x + 1, verts[76] = y, verts[77] = z;
	verts[78] = x, verts[79] = y, verts[80] = z + 1;
	verts[81] = x + 1, verts[82] = y, verts[83] = z;
	verts[84] = x, verts[85] = y, verts[86] = z + 1;
	verts[87] = x + 1, verts[88] = y, verts[89] = z + 1;
	
	verts[90] = x, verts[91] = y + 1, verts[92] = z;
	verts[93] = x + 1, verts[94] = y + 1, verts[95] = z;
	verts[96] = x, verts[97] = y + 1, verts[98] = z + 1;
	verts[99] = x + 1, verts[100] = y + 1, verts[101] = z;
	verts[102] = x, verts[103] = y + 1, verts[104] = z + 1;
	verts[105] = x + 1, verts[106] = y + 1, verts[107] = z + 1;

	return verts;
}

void A1::initCube(){
	int sz = 3*3*6*2;
	
	float* verts = generateCubeVerts();

	glGenVertexArrays(1, &m_cube_vao);
	glBindVertexArray(m_cube_vao);

	glGenBuffers(1, &m_cube_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
	glBufferData(GL_ARRAY_BUFFER, sz*sizeof(float), verts, GL_STATIC_DRAW);

	GLint posAttrib = m_shader.getAttribLocation("position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::coor_trans(float radius, float u, float v, float* verts){
	verts[0] = radius * cosf(u) * cosf(v);
	verts[1] = radius * sinf(u);
	verts[2] = radius * cosf(u) * sinf(v); 
}

float* A1::generateSphereVerts(){
	// Reference: http://songho.ca/opengl/gl_sphere.html
	float x, y, z, xy;
	float radius = 0.5f;
	float sectorCount = m_sector_size, stackCount = m_stack_size;
	size_t number_triangles = (stackCount - 1) * sectorCount * 2;
	float* verts = new float[3*3*number_triangles];

	float sectorStep = 2 * M_PI / sectorCount;
	float stackStep = M_PI / stackCount;
	float sectorAngle, stackAngle;
	size_t ct = 0;
	for(int i = 0; i < stackCount; i++){
		coor_trans(radius, M_PI/2, 0.0f, &verts[ct]); ct += 3;
		coor_trans(radius, M_PI/2+stackStep, i*sectorStep, &verts[ct]); ct += 3;
		coor_trans(radius, M_PI/2+stackStep, (i-1)*sectorStep, &verts[ct]); ct += 3;
		coor_trans(radius, M_PI/2, 0.0f, &verts[ct]); ct += 3;
		coor_trans(radius, M_PI/2-stackStep, (i-1)*sectorStep, &verts[ct]); ct += 3;
		coor_trans(radius, M_PI/2-stackStep, i*sectorStep, &verts[ct]); ct += 3;
	}
	for (int i = 1; i < sectorCount-1; i++) {
		for (int j = 0; j < stackCount; j++) {
			float u = M_PI / 2 + i * stackStep;
			float v = j * sectorStep;
			coor_trans(radius, u, v, &verts[ct]); ct += 3;
			coor_trans(radius, u, v + sectorStep, &verts[ct]); ct += 3;
			coor_trans(radius, u + stackStep, v + sectorStep, &verts[ct]); ct += 3;
			coor_trans(radius, u, v, &verts[ct]); ct += 3;
			coor_trans(radius, u + stackStep, v + sectorStep, &verts[ct]); ct += 3;
			coor_trans(radius, u + stackStep, v, &verts[ct]); ct += 3;
		}
	}
	
	return verts;
}

void A1::initAvatar(){
	float* verts = generateSphereVerts();

	glGenVertexArrays(1, &m_avatar_vao);
	glBindVertexArray(m_avatar_vao);

	glGenBuffers(1, &m_avatar_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_avatar_vbo);
	glBufferData(GL_ARRAY_BUFFER, ((m_stack_size-1)*m_sector_size*2)*3*3*sizeof(float), verts, GL_STATIC_DRAW);

	GLint posAttrib = m_shader.getAttribLocation("position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// glGenBuffers(1, &m_avatar_ebo);
	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_avatar_ebo);
	// glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete [] verts;

	CHECK_GL_ERRORS;
}

void A1::reset(){
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;

	cube_colour[0] = 0.5f;
	cube_colour[1] = 0.0f;
	cube_colour[2] = 0.0f;

	floor_colour[0] = 0.0f;
	floor_colour[1] = 0.5f;
	floor_colour[2] = 0.0f;

	avatar_colour[0] = 0.0f;
	avatar_colour[1] = 0.0f;
	avatar_colour[2] = 0.5f;

	m_avatar_pos.first = -1;
	m_avatar_pos.second = -1;

	m_cube_height = 1;
	m_scale_factor = 1.0f;
	m_rotation_angle = 0.0f;
	m_mouse_is_pressed = false;
	maze.reset();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
		if( ImGui::Button( "Quit Application" )) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "Reset" )) {
			reset();
		}
		if( ImGui::Button( "Dig" )) {
			maze.digMaze();
			maze.printMaze();
			m_avatar_pos = maze.start;
		}
		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour );
		ImGui::SameLine();
		if( ImGui::RadioButton( "Wall's Colour", &current_col, 0 ) ) {
			// Select this colour.
			// ImGui::ColorEdit3( "##Colour", (float*)&m_color[m_avatar_pos.first][m_avatar_pos.second] );
			cube_colour[0] = colour[0];
			cube_colour[1] = colour[1];
			cube_colour[2] = colour[2];
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Floor's Colour", &current_col, 1 ) ) {
			// Select this colour.
			// m_color[m_avatar_pos.first][m_avatar_pos.second] = glm::vec3(colour[0], colour[1], colour[2]);
			floor_colour[0] = colour[0];
			floor_colour[1] = colour[1];
			floor_colour[2] = colour[2];
		}
		ImGui::SameLine();
		if( ImGui::RadioButton( "Avatar's Colour", &current_col, 2 ) ) {
			// Select this colour.
			// m_color[m_avatar_pos.first][m_avatar_pos.second] = glm::vec3(colour[0], colour[1], colour[2]);
			avatar_colour[0] = colour[0];
			avatar_colour[1] = colour[1];
			avatar_colour[2] = colour[2];
		}
		
		ImGui::PopID();

		// ImGui::PushID( 1 );
		// ImGui::ColorEdit3( "##Colour", floor_colour );
		// ImGui::SameLine();
		// // if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
		// // 	// Select this colour.
		// // 	m_color[m_avatar_pos.first][m_avatar_pos.second] = glm::vec3(colour[0], colour[1], colour[2]);
		// // }
		// ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	if(abs(m_dynamic_offset) > 0.001f) m_rotation_angle += m_dynamic_offset;
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );
	W = glm::rotate(mat4(), m_rotation_angle, vec3(0.0f, 1.0f, 0.0f)) * W;
	W = glm::scale(mat4(), vec3(m_scale_factor)) * W;

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Draw the floor
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray( m_floor_vao );
		glUniform3f( col_uni, floor_colour[0], floor_colour[1], floor_colour[2] );
		// glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(W));
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

		// Just draw the grid for now.
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1.0f, 1.0f, 1.0f );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		glBindVertexArray(m_cube_vao);
		glUniform3f(col_uni, cube_colour[0], cube_colour[1], cube_colour[2]);
		for(int i = 0; i < DIM; i++){ // x
			for(int j = 0; j < DIM; j++){ // z
				if(maze.getValue(i, j) >= 1){
					for(int k = 0; k < m_cube_height; k++){
						mat4 Wi = glm::translate(W, vec3(j*1.0f, k*1.0f, i*1.0f));
						// glBindVertexArray(m_cube_vao);
						glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(Wi));
						glDrawArrays(GL_TRIANGLES, 0, 3*12);
					}
				}
				// for(int k = 0; k < maze.getValue(j, i); k++){ // y
				// 	// drawCube((float)i, float(k), float(j));
					
				// 	// glGenBuffers(1, &m_cube_vbo);
				// 	// glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);
				// }
			}
		}
		// Draw the avatar(using cube for now: TODO: draw sphere.)
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(m_avatar_vao);
		// glUniform3f(col_uni, colour[0], colour[1], colour[2]);
		glUniform3f(col_uni, avatar_colour[0], avatar_colour[1], avatar_colour[2]);
		mat4 Wi = glm::translate(W, vec3(m_avatar_pos.first*1.0f+0.5f, 0.0f, m_avatar_pos.second*1.0f));
		// glBindVertexArray(m_cube_vao);
		glUniformMatrix4fv(M_uni, 1, GL_FALSE, value_ptr(Wi));
		glDrawArrays(GL_TRIANGLES, 0, ((m_stack_size-1)*m_sector_size*2)*3*3);
		// Highlight the active square.

	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		m_curr_mouse_pos = xPos;
		if(!m_mouse_is_pressed){
			m_last_mouse_pos = m_curr_mouse_pos;
		}
		else{
			// m_rotation_angle += (m_curr_mouse_pos-m_last_mouse_pos)*2*M_PI/20000.0f;
			// m_last_mouse_pos = m_curr_mouse_pos;
			m_dynamic_offset = (m_curr_mouse_pos-m_last_mouse_pos)*2*M_PI/20000.0f;
			// cout << m_dynamic_offset << endl;
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (button == GLFW_MOUSE_BUTTON_LEFT) {
			if (actions == GLFW_PRESS) {
				// Left mouse button pressed
				m_mouse_is_pressed = true;
			} else if (actions == GLFW_RELEASE) {
				// Left mouse button released
				m_mouse_is_pressed = false;
			}
		}
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);
	// cout << "Moved: " << yOffSet << endl;
	// Zoom in or out.
	if(yOffSet != 0){
		m_scale_factor += yOffSet*0.05f;
		eventHandled = true;
	}
	
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
		switch (key)
		{
		case GLFW_KEY_Q:
			glfwSetWindowShouldClose(m_window, GL_TRUE);
			eventHandled = true;
			break;
		case GLFW_KEY_R:
			reset();
			eventHandled = true;
			break;
		case GLFW_KEY_D:
			maze.digMaze();
			maze.printMaze();
			m_avatar_pos = maze.start;
			eventHandled = true;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			m_shift_is_pressed = true;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			m_shift_is_pressed = true;
			break;
		case GLFW_KEY_SPACE:
			// if(m_avatar_pos.first == -1 || m_avatar_pos.first == DIM || m_avatar_pos.second == -1 || m_avatar_pos.second == DIM) break;
			// maze.setValue(m_avatar_pos.second, m_avatar_pos.first, maze.getValue(m_avatar_pos.second, m_avatar_pos.first)+1);
			m_cube_height++;
			eventHandled = true;
			break;
		case GLFW_KEY_BACKSPACE:
			// if(m_avatar_pos.first == -1 || m_avatar_pos.first == DIM || m_avatar_pos.second == -1 || m_avatar_pos.second == DIM) break;
			// if(maze.getValue(m_avatar_pos.second, m_avatar_pos.first) > 0){
			// 	maze.setValue(m_avatar_pos.second, m_avatar_pos.first, maze.getValue(m_avatar_pos.second, m_avatar_pos.first)-1);
			// }
			if(m_cube_height > 0) m_cube_height--;
			eventHandled = true;
			break;
		case GLFW_KEY_RIGHT:
			if(m_avatar_pos.first < DIM){
				if(maze.getValue(m_avatar_pos.second, m_avatar_pos.first+1) == 0) m_avatar_pos.first++;
				else if(m_shift_is_pressed){
					maze.setValue(m_avatar_pos.second, m_avatar_pos.first+1, 0);
					m_avatar_pos.first++;
				}
			}
			// else if(m_avatar_pos.first == DIM) m_avatar_pos.first++;
			eventHandled = true;
			break;
		case GLFW_KEY_LEFT:
			if(m_avatar_pos.first > 0){
				if(maze.getValue(m_avatar_pos.second, m_avatar_pos.first-1) == 0) m_avatar_pos.first--;
				else if(m_shift_is_pressed){
					maze.setValue(m_avatar_pos.second, m_avatar_pos.first-1, 0);
					m_avatar_pos.first--;
				}
			}
			// else if(m_avatar_pos.first == 0) m_avatar_pos.first--;
			eventHandled = true;
			break;
		case GLFW_KEY_UP:
			if(m_avatar_pos.second > 0){
				// if(m_avatar_pos.second == 0) m_avatar_pos.second--;
				if(maze.getValue(m_avatar_pos.second-1, m_avatar_pos.first) == 0) m_avatar_pos.second--;
				else if(m_shift_is_pressed){
					maze.setValue(m_avatar_pos.second-1, m_avatar_pos.first, 0);
					m_avatar_pos.second--;
				}
			}
			// else if(m_avatar_pos.second == 0) m_avatar_pos.second--;
			eventHandled = true;
			break;
		case GLFW_KEY_DOWN:
			if(m_avatar_pos.second < DIM+1 || m_avatar_pos.second == -1){
				// if(m_avatar_pos.second == DIM) m_avatar_pos.second++;
				if(maze.getValue(m_avatar_pos.second+1, m_avatar_pos.first) == 0) m_avatar_pos.second++;
				else if(m_shift_is_pressed){
					maze.setValue(m_avatar_pos.second+1, m_avatar_pos.first, 0);
					m_avatar_pos.second++;
				}
			}
			// else if(m_avatar_pos.second == DIM) m_avatar_pos.second++;
			// else if(m_avatar_pos.second == -1) m_avatar_pos.second++;
			eventHandled = true;
			break;
		default:
			break;
		}
	}
	else if(action == GLFW_RELEASE){
		if(key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT){
			m_shift_is_pressed = false;
		}
	}

	return eventHandled;
}
