//51_opengl3.0, making a minimalist OpenGL 3+ core program.
//ps. opengl32.lib, glu32.lib should be added as dependency libraries


//Using SDL, SDL OpenGL, standard IO, and, strings
#include <iostream>
#include <SDL.h>
#include <gl\glew.h>    //GLEW is an extension library; set it before compiling
#include <SDL_opengl.h>
//#include <GL/gl.h>	//OpenGL header
#include <GL/glu.h>	//OpenGL Utilities header
#include <stdio.h>
#include <string>

//check memory leak
#include <vld.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Shader loading utility programs
void printProgramLog( GLuint program );
void printShaderLog( GLuint shader );

//Graphics program
GLuint gProgramID = 0;
GLint gVertexPos2DLocation = -1;
GLuint gVBO = 0;
GLuint gIBO = 0;



//Starts up SDL, creates window, and initializes OpenGL
bool init();

//Initializes matrices and clear color
bool initGL();

//Input handler
void handleKeys( unsigned char key, int x, int y );

//Per frame update
void update();

//Renders quad to the screen
void render();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//OpenGL context
SDL_GLContext gContext;

//Render flag
bool gRenderQuad = true;

bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Use OpenGL 2.1
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
        SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
        
        //Create window
        gWindow = SDL_CreateWindow( "SDL Tutorial_OpenGL3.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create the OpenGL rendering context
            gContext = SDL_GL_CreateContext( gWindow );
            if( gContext == NULL )
            {
                printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize GLEW
                glewExperimental = GL_TRUE; 
                GLenum glewError = glewInit();
                if( glewError != GLEW_OK )
                {
                    printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
                }

                //Use Vsync
                if( SDL_GL_SetSwapInterval( 1 ) < 0 )
                {
                    printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
                }

                //Initialize OpenGL
                if( !initGL() )
                {
                    printf( "Unable to initialize OpenGL!\n" );
                    success = false;
                }
            }
        }
    }

    return success;
}

//
bool initGL()
{
    //Success flag
    bool success = true;

    //Generate program
    gProgramID = glCreateProgram();

    //Create vertex shader
    GLuint vertexShader = glCreateShader( GL_VERTEX_SHADER );

    //Get vertex source
    const GLchar* vertexShaderSource[] =
    {
        "#version 140\nin vec2 LVertexPos2D; void main() { gl_Position = vec4( LVertexPos2D.x, LVertexPos2D.y, 0, 1 ); }"
    };

    //Set vertex source
    glShaderSource( vertexShader, 1, vertexShaderSource, NULL );

    //Compile vertex source
    glCompileShader( vertexShader );

    //Check vertex shader for errors
    GLint vShaderCompiled = GL_FALSE;
    glGetShaderiv( vertexShader, GL_COMPILE_STATUS, &vShaderCompiled );
    if( vShaderCompiled != GL_TRUE )
    {
        printf( "Unable to compile vertex shader %d!\n", vertexShader );
        printShaderLog( vertexShader );
        success = false;
    }

    
    
    return success;
}

void handleKeys( unsigned char key, int x, int y )
{
	//Toggle quad
    if( key == 'q' )
    {
        gRenderQuad = !gRenderQuad;
    }
}

void update()
{
    //No per frame update needed?
}

void render()
{
    //Clear color buffer
    glClear( GL_COLOR_BUFFER_BIT );
    
    //Render quad
    if( gRenderQuad )
    {
    	//OpenGL uses normalized coordinates, thus x, y from -1 to 1
        glBegin( GL_QUADS );	//delimit the vertices of a primitive or a group of like primitives
            glVertex2f( -0.5f, -0.5f );	//specify a vertex, GLfloat format
            glVertex2f( 0.5f, -0.5f );
            glVertex2f( 0.5f, 0.5f );
            glVertex2f( -0.5f, 0.5f );
        glEnd();
    }
}

void close()
{
	//Destroy window
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;

    //Quit SDL subsystems
    //Clean up all dynamically loaded library handles, freeing memory
    SDL_Quit();

    printf( "Unload resource successfully. Close the window as you like.\n" );
    std::cin.get(); 
}

int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
		//Main loop flag
        bool quit = false;

        //Event handler
        SDL_Event e;

        //Enable text input
    	SDL_StartTextInput();

        //While application is running
        while( !quit )
        {
            //Handle events on queue
            while( SDL_PollEvent( &e ) != 0 )
            {
                //User requests quit
                if( e.type == SDL_QUIT )
                {
                    quit = true;
                }
                //Handle keypress with current mouse position
                else if( e.type == SDL_TEXTINPUT )
                {
                    int x = 0, y = 0;
                    SDL_GetMouseState( &x, &y );
                    handleKeys( e.text.text[ 0 ], x, y );
                }
            }

            //Render quad
            render();
            
            //Update screen
            SDL_GL_SwapWindow( gWindow );
        }
        
        //Disable text input
        SDL_StopTextInput();	
	}

	//Free resources and close SDL
	close();

	return 0;
}