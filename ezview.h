#ifndef EZVIEW_H
#define EZVIEW_H

/*************
* Structs
*************/

// Create the structure for the vertex
typedef struct
{
	float Position[2];
	float TexCoord[2];
} Vertex;

// Create the structure for the image NOTE "don't care about the alpha channel"
typedef struct Pixmap
{
	int width, height, magicNumber;
	unsigned char *image;
} Pixmap;


/*************
* Prototypes
*************/

static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glCompileShaderOrDie(GLuint shader);
void glLinkProgramOrDie(GLuint program);

#endif