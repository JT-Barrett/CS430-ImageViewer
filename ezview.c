/*****************************************************************************************************************************
*	Title: Ezview.c
*	Author: JT Barrett
*	Last Updated: 12/8/2016
*	Purpose: Loads in a P3 or P6 image and displays it in a window. The user can then transform the image using the keyboard
*****************************************************************************************************************************/

#define GLFW_DLL 1
#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <GLFW/linmath.h>
#include <assert.h>
#include <string.h>
#include "ezview.h"

// Create 6 verticies (2 triangles) that will render the image, 0.99999 is used to avoid roundoff error artifacts
Vertex vertexes[] = {
	{ { 0.99999, -0.99999 },{ 0.99999, 0.99999 } },
	{ { 0.99999, 0.99999 },{ 0.99999, 0 } },
	{ { -0.99999, 0.99999 },{ 0, 0 } },
	{ { -0.99999, 0.99999 },{ 0, 0 } },
	{ { -0.99999, -0.99999 },{ 0, 0.99999 } },
	{ { 0.99999, -0.99999 },{ 0.99999, 0.99999 } }
};

Transformations trans = { 0, 1, 0, 0, 0, 0 };

// Same vertex shader from the texDemo
static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

// Same fragment shader from the texDemo
static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

// This loads the image into a buffer and displays it ina  window for trasformation by the user
int main(int argc, char *argv[])
{
	GLFWwindow* window;
	GLuint program;
	GLint mvp_location;

	// Load the image into a buffer
	Pixmap *buffer = (Pixmap *)malloc(sizeof(Pixmap));
	load_ppm(argv[1], buffer);

	if (!glfwInit()) exit(EXIT_FAILURE);

	//create the window with the demensions of the image
	init_window(buffer, window);
	window = glfwCreateWindow(buffer->width, buffer->height, "ezview", NULL, NULL);
	if (!window)
	{
		fprintf(stderr, "\nError: could not open window");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// link key callback to the function we created for it
	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	//set the image into the window
	init_image(buffer, &mvp_location, &program);

	//update the image as the user inputs transformations
	update_window(window, program, mvp_location);

	// free buffer and exit program
	free(buffer);
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void init_window(Pixmap *buffer, GLFWwindow *window) {
	// window hints from demo
	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
}

void init_image(Pixmap *buffer, GLint *mvp_location, GLuint *program) {
	GLuint vertex_buffer, vertex_shader, fragment_shader;
	GLint vpos_location;

	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

	// set up the vertex and fragment shaders
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	glCompileShaderOrDie(vertex_shader);

	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	glCompileShaderOrDie(fragment_shader);


	// make and link up the program
	*program = glCreateProgram();
	glAttachShader(*program, vertex_shader);
	glAttachShader(*program, fragment_shader);
	glLinkProgram(*program);
	glLinkProgramOrDie(*program);

	//initialize locations
	*mvp_location = glGetUniformLocation(*program, "MVP");
	assert(*mvp_location != -1);

	vpos_location = glGetAttribLocation(*program, "vPos");
	assert(vpos_location != -1);

	GLint texcoord_location = glGetAttribLocation(*program, "TexCoordIn");
	assert(texcoord_location != -1);

	GLint tex_location = glGetUniformLocation(*program, "Texture");
	assert(tex_location != -1);

	//set up vertexes
	glEnableVertexAttribArray(vpos_location);
	glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(texcoord_location);
	glVertexAttribPointer(texcoord_location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(float) * 2));

	//set up the textures as the loaded image buffer
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, buffer->width, buffer->height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer->image);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(tex_location, 0);
}

//calculate the transformation matrix and update the window with the new image
void update_window(GLFWwindow *window, GLuint program, GLint mvp_location) {
	while (!glfwWindowShouldClose(window))
	{
		int width, height;

		//initialize transformation matricies
		mat4x4 r, h, s, t, rh, rhs, mvp;

		//add current values into the appropriate locations for each transformation matrix
		mat4x4_identity(s);
		s[0][0] = s[0][0] * trans.scale;
		s[1][1] = s[1][1] * trans.scale;

		mat4x4_identity(r);
		mat4x4_rotate_Z(r, r, trans.rotation);

		mat4x4_identity(t);
		mat4x4_translate(t, trans.translateX, trans.translateY, 0);

		mat4x4_identity(h);
		h[0][1] = trans.shearX;
		h[1][0] = trans.shearY;

		//transform the image (final MVP = ((R*H)*S)*T))
		mat4x4_mul(rh, r, h);
		mat4x4_mul(rhs, rh, s);
		mat4x4_mul(mvp, rhs, t);

		glfwGetFramebufferSize(window, &width, &height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		// update the image with transformation
		glUseProgram(program);
		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glfwSwapBuffers(window);

		// Process keyboard input
		glfwPollEvents();
	}
}

// Handle for the user input transoformations
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Hit escape to quite the ez-view program
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

	// Zoom in and out using R and F
	if (key == GLFW_KEY_R && action == GLFW_PRESS) trans.scale *= SCALE_UP;
	if (key == GLFW_KEY_F && action == GLFW_PRESS) trans.scale *= SCALE_DOWN;

	// translate operations using arrow keys
	if (key == GLFW_KEY_UP && action == GLFW_PRESS) trans.translateY += MIN_TRANS;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) trans.translateY -= MIN_TRANS;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) trans.translateX += MIN_TRANS;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) trans.translateX -= MIN_TRANS;

	// rotate 90 degrees using Q and E
	if (key == GLFW_KEY_Q && action == GLFW_PRESS) trans.rotation += NINETY_DEG;
	if (key == GLFW_KEY_E && action == GLFW_PRESS) trans.rotation -= NINETY_DEG;

	// Shear image using WASD
	if (key == GLFW_KEY_D && action == GLFW_PRESS) trans.shearY += MIN_TRANS;
	if (key == GLFW_KEY_A && action == GLFW_PRESS) trans.shearY -= MIN_TRANS;
	if (key == GLFW_KEY_W && action == GLFW_PRESS) trans.shearX += MIN_TRANS;
	if (key == GLFW_KEY_S && action == GLFW_PRESS) trans.shearX -= MIN_TRANS;
}

// shade compile checker from demo, checks if shader compiles correctly
void glCompileShaderOrDie(GLuint shader)
{
	GLint compiled;
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		char* info = malloc(infoLen + 1);
		GLint done;
		glGetShaderInfoLog(shader, infoLen, &done, info);
		printf("Error: Unable to compile shader %s\n", info);
		exit(-1);
	}
}

// check if program links properly
void glLinkProgramOrDie(GLuint program)
{
	int link_success;

	glGetProgramiv(program, GL_LINK_STATUS, &link_success);

	if (!link_success)
	{
		GLchar message[MSG_MAX];
		glGetProgramInfoLog(program, sizeof(message), 0, &message[0]);
		printf("Error: unable to link program %s\n", message);
		exit(-1);
	}
}

// Prints out an appropriate error
static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

//Extract header information from the ppm file and store it in a dedicated struct (From Project 1)
PPMHeader get_header_info(FILE *file) {
	char header[HEAD_ELEM][HEAD_ELEM_MAX_LEN];
	char full[HEAD_MAX_LEN];
	char *full_temp = full;
	int header_size = 0;

	rewind(file);

	//load each relevant header element separated by '\n' into an array of strings
	for (int i = 0; i < HEAD_ELEM; i++) {
		char *current_str = malloc(HEAD_ELEM_MAX_LEN);
		char *temp = current_str;
		while (1) {
			int current_char = getc(file);
			*full_temp = (char)current_char;
			*full_temp++;
			header_size++;
			if (current_char != ' ' && current_char != '\n' && current_char != '\r' && current_char != '#') {
				*temp = current_char;
				*temp++;
			}
			else if (current_char == '#') {
				while (1) {
					int comment_char = getc(file);
					*full_temp = (char)comment_char;
					*full_temp++;
					header_size++;
					if (comment_char == '\n')
						break;
				}
			}
			else {
				*temp = '\0';
				break;
			}
		}
		strcpy(header[i], current_str);
	}

	//load info from string array into struct
	PPMHeader extracted_header;
	extracted_header.header_size = header_size;
	strcpy(extracted_header.full_header, full);
	strcpy(extracted_header.format, header[0]);
	extracted_header.width = atoi(header[1]);
	extracted_header.height = atoi(header[2]);
	extracted_header.max_depth = atoi(header[3]);

	return extracted_header;
}

// This method parses the ppm file and stores it's contents into an appropriately formatted image buffer (From Project 1)
void load_ppm(char *filename, Pixmap *buffer) {
	printf("load_ppm entered");
	FILE *file;

	if (file = fopen(filename, "rb")) {
		PPMHeader myHeader;
		myHeader = get_header_info(file);
		int size = myHeader.width * myHeader.height * 3;
		printf("\n| format:%s || width:%d || height:%d || max depth:%d |\n", myHeader.format, myHeader.width, myHeader.height, myHeader.max_depth);
		buffer->image = malloc(myHeader.width * myHeader.height * 3);

		int current_pixel = 0;
		if (strcmp("P3", myHeader.format) == 0) {
			int pixel;
			for (int i = 0; i<myHeader.height; i++)
			{
				for (int j = 0; j<myHeader.width; j++)
				{
					fscanf(file, "%d ", &pixel);
					//memcpy(buffer->image[current_pixel++], pixel, sizeof(unsigned char));
					buffer->image[current_pixel++] = pixel;
					fscanf(file, "%d ", &pixel);
					buffer->image[current_pixel++] = pixel;
					fscanf(file, "%d ", &pixel);
					buffer->image[current_pixel++] = pixel;
				}
			}
		}

		else if (strcmp("P6", myHeader.format) == 0) {
			while (1) {
				int data_char = getc(file);
				if (data_char != EOF) {
					buffer->image[current_pixel++] = data_char;
				}
				else if (data_char == EOF) {
					break;
				}
			}
		}
		else {
			fprintf(stderr, "Error: PPM format read as %s, was not 3 or 6.\n", myHeader.format);
			exit(1);
		}
		printf("\nbuffer loaded");
		buffer->height = myHeader.height;
		buffer->width = myHeader.width;
	}
	else
	{
		fprintf(stderr, "Error: The input file specified could not be opened or does not exist.\n");
		exit(1);
	}
	fclose(file);
}