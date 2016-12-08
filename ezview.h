#ifndef EZVIEW_H
#define EZVIEW_H

/*************
* Constants
*************/

#define HEAD_MAX_LEN 4096
#define FORMAT_LEN 2
#define COLOR_DEPTH 255
#define HEAD_ELEM 4
#define HEAD_ELEM_MAX_LEN 64


/*************
* Structs
*************/

// structure for vertex
typedef struct
{
	float Position[2];
	float TexCoord[2];
} Vertex;

// structure for the image with height and width
typedef struct Pixmap
{
	int width, height;
	unsigned char *image;
} Pixmap;

//struct to store ppm header info
typedef struct PPMHeader
{
	int header_size;
	char full_header[HEAD_MAX_LEN];
	char format[FORMAT_LEN];
	int width;
	int height;
	int max_depth;
} PPMHeader;


/*************
* Prototypes
*************/

static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void glCompileShaderOrDie(GLuint shader);
void glLinkProgramOrDie(GLuint program);
void init_window(Pixmap *buffer, GLFWwindow *window);
void init_image(Pixmap *buffer, GLint *mvp_location, GLuint *program);
void update_window(GLFWwindow *window, GLuint *program, GLint *mvp_location);

PPMHeader get_header_info(FILE *file);
void load_ppm(char *filename, Pixmap *buffer);

#endif