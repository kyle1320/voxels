#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

static char * readFile(const char * fname);

GLuint loadShaders(const char * vertex_file_path, const char * fragment_file_path) {

	// Create the shaders
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	char * vertexShaderCode = readFile(vertex_file_path);

	// Read the Fragment Shader code from the file
	char * fragmentShaderCode = readFile(fragment_file_path);

	GLint result = GL_FALSE;
	int infoLogLength;

	// Compile Vertex Shader
	//printf("Compiling shader : %s\n", vertex_file_path);
	glShaderSource(vertexShaderID, 1, &vertexShaderCode , NULL);
	glCompileShader(vertexShaderID);
	free(vertexShaderCode);

	// Check Vertex Shader
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	char * vertexShaderErrorMessage = malloc(infoLogLength * sizeof(char));
	glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, vertexShaderErrorMessage);
	if (!result)
		printf("Error compiling: %s\n%s\n", vertex_file_path, vertexShaderErrorMessage);
	free(vertexShaderErrorMessage);

	// Compile Fragment Shader
	//printf("Compiling shader : %s\n", fragment_file_path);
	glShaderSource(fragmentShaderID, 1, &fragmentShaderCode , NULL);
	glCompileShader(fragmentShaderID);
	free(fragmentShaderCode);

	// Check Fragment Shader
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
	glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
	char * fragmentShaderErrorMessage = malloc(infoLogLength * sizeof(char));
	glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, fragmentShaderErrorMessage);
	if (!result)
		printf("Error compiling %s:\n%s\n", fragment_file_path, fragmentShaderErrorMessage);
	free(fragmentShaderErrorMessage);

	// Link the program
	//puts("Linking program");
	GLuint programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);

	// Check the program
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	char * programErrorMessage = malloc((infoLogLength > 1 ? infoLogLength : 1) * sizeof(char));
	glGetProgramInfoLog(programID, infoLogLength, NULL, programErrorMessage);
	if (!result)
		printf("Error linking program:\n%s\n", programErrorMessage);
	free(programErrorMessage);

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);

	return programID;
}

static char * readFile(const char * fname) {
	FILE *fp;
	long lSize;
	char *buffer;

	fp = fopen ( fname , "rb" );
	if( !fp ) perror(fname),exit(1);

	fseek( fp , 0L , SEEK_END);
	lSize = ftell( fp );
	rewind( fp );

	/* allocate memory for entire content */
	buffer = calloc( 1, lSize+1 );
	if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);

	/* copy the file into the buffer */
	if( 1!=fread( buffer , lSize, 1 , fp) )
	 	fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);

	fclose(fp);

	return buffer;
}