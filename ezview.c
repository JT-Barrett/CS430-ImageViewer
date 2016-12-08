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
#include "linmath.h"
#include <assert.h>
#include "ezview.h"


// vertexes for displaying the image, use 0.99999 to eliminate red line rounding artifact. Use two triangles to display image.
Vertex vertexes[] = {
	{{ 1, -1 },{ 0.99999, 0.99999 }}, {{ 1, 1 },{ 0.99999, 0 }}, {{ -1, 1 },{ 0, 0 }},
	{{ -1, 1 },{ 0, 0 }}, {{ -1, -1 },{ 0, 0.99999 }}, {{ 1, -1 },{ 0.99999, 0.99999 }}
};

// vertex shader from demo
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

// fragment shader from demo
static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

// this will load the p3 or p6 image and place it into the ezview window for the user to transform
int main(int argc, char *argv[])
{
	return 0;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

}


static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

// shade checker from demo, make sure shader compiles correctly
void glCompileShaderOrDie(GLuint shader)
{
	GLint compiled;
	glCompileShader(shader);
	glGetShaderiv(shader,
		GL_COMPILE_STATUS,
		&compiled);

	if (!compiled)
	{
		GLint infoLen = 0;
		glGetShaderiv(shader,
			GL_INFO_LOG_LENGTH,
			&infoLen);
		char* info = malloc(infoLen + 1);
		GLint done;
		glGetShaderInfoLog(shader, infoLen, &done, info);
		printf("Unable to compile shader: %s\n", info);
		exit(-1);
	}
}

// check that program is linked properly
void glLinkProgramOrDie(GLuint program)
{
	int link_success = GL_FALSE;

	glGetProgramiv(program, GL_LINK_STATUS, &link_success);

	if (link_success == GL_FALSE)
	{
		GLchar message[256];
		glGetProgramInfoLog(program, sizeof(message), 0, &message[0]);
		fprintf(stderr, "Link Error: %s\n", message);
		exit(-1);
	}
}
