//49_mutex (mutually exclusive) to prevent the two threads 
//from grabbing the same piece of data and conditions 
//to let the threads know when they can consume and can produce

//Using SDL, SDL Threads, SDL_image, standard IO, and, strings
#include <iostream>
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>

//check memory leak
#include <vld.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Texture wrapper class
class LTexture
{
    public:
        //Initializes variables
        LTexture();

        //Deallocates memory
        ~LTexture();

        //Loads image at specified path
        bool loadFromFile( std::string path );

        #ifdef _SDL_TTF_H
        //Creates image from font string
        //If SDL_ttf is not defined, ignore this piece of code.
        bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
        #endif

        //Deallocates texture
        void free();

        //Set color modification of texture
        void setColor( Uint8 red, Uint8 green, Uint8 blue );

        //Set blending
        void setBlendMode( SDL_BlendMode blending );

        //Set alpha modulation
        void setAlpha( Uint8 alpha );

        //Renders texture at given point, and whether it is a clip
        void render( int x, int y, SDL_Rect* clip = NULL, 
        	double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

        //Gets image dimensions
        int getWidth();
        int getHeight();

    private:
        //The actual hardware texture
        SDL_Texture* mTexture;

        //Image dimensions
        int mWidth;
        int mHeight;
};


//Our worker functions
int producer( void* data );
int consumer( void* data );
void produce();
void consume();

//Our test thread function
int threadFunction( void* data );

int threadFunction( void* data )
{
    //Print incoming data
    printf( "Running thread with value = %d\n", (int)data );

    return 0;
}

bool init();

bool loadMedia();

void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//
LTexture gSplashTexture;

//The protective mutex
SDL_mutex* gBufferLock = NULL;

//The conditions
SDL_cond* gCanProduce = NULL;
SDL_cond* gCanConsume = NULL;

//The "data buffer"
int gData = -1;

// Now to build the class methods
// Initializes
LTexture::LTexture()
{
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

// Deallocates texture
LTexture::~LTexture()
{
    //Free loaded image
    free();
}

// Load image from specific path
bool LTexture::loadFromFile( std::string path )
{
    //Initialization flag
    bool success = true;
    //Get rid of preexisting texture
    free();

    //Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
        success = false;
    }
    else
    {
        //Color key image, use SDL_SetColorKey() to set the color key (transparent pixel) in a surface
        //SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0, 0 ) );
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
            success = false;
        }
        else
        {
            //Fill width, height of the image
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;

        }

        //Get rid of old loaded surface
        SDL_FreeSurface( loadedSurface );
    }

    return success;
}


#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Get rid of preexisting texture
    free();

    //Render text surface
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Get image dimensions
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        //Get rid of old surface
        SDL_FreeSurface( textSurface );
    }
    
    //Return success
    return mTexture != NULL;
}
#endif


void LTexture::free()
{
    //Free texture if it exists
    if ( mTexture != NULL )
    {
    	SDL_DestroyTexture( mTexture );
	    mTexture = NULL;
	    mWidth = 0;
	    mHeight = 0;
    }
}

//Renders texture at given point
void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    //Set destination rectangle
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };

    //Set renderQuad width and height same as source rect
    if ( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    
    SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
    //SDL_RenderCopy( renderer, texture, &srcrect, &dstrect );
}


//Set color modulation
void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	SDL_SetTextureColorMod( mTexture, red, green, blue );
	//srcC = srcC * (color / 255)
	//return -1 if color modulation is not supported.
}

//Set alpha modulation
void LTexture::setAlpha( Uint8 alpha )
{
	SDL_SetTextureAlphaMod( mTexture, alpha );
	//srcA = srcA * (alpha / 255)
	//return -1 if alpha modulation is not supported.
}

//Set blending, e.g.alpha blending
void LTexture::setBlendMode( SDL_BlendMode blending )
{
	SDL_SetTextureBlendMode( mTexture, blending );
}


//Get image dimensions
int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}


bool init()
{
    //Initialization flag
    bool success = true;

    //Initialize SDL
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
    {
        printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {
        //Create window
        gWindow = SDL_CreateWindow( "SDL Test_49_mutex_and_conditions", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            //Create vsynced renderer for window
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Initialize renderer color, white?
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }

                // //Initialize SDL_ttf
                // if( TTF_Init() == -1 )
                // {
                //     printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                //     success = false;
                // }
            }
        }
    }

    return success;
    //Filled gWindow, gRenderer
}

bool loadMedia()
{
    //Loading success flag
    bool success = true;

    //Create the mutex
    gBufferLock = SDL_CreateMutex();
            
    //Allocate conditions
    gCanProduce = SDL_CreateCond();
    gCanConsume = SDL_CreateCond();

    //Initialize semaphore
    //gDataLock = SDL_CreateSemaphore( 1 );   // SDL_sem* SDL_CreateSemaphore( initial_value )

    if( !gSplashTexture.loadFromFile( "47_semaphores/splash.png" ) )
    {
        printf( "Failed to load dot texture!\n" );
        success = false;
    }

    return success;
}

void close()
{
    //Free loaded image
    gSplashTexture.free();

    //Free condition and mutex variable
    SDL_DestroyCond( gCanProduce );
    SDL_DestroyCond( gCanConsume );
    SDL_DestroyMutex( gBufferLock );

    gBufferLock = NULL;
    gCanConsume = NULL;
    gCanProduce = NULL;
    
    //Destroy window
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gRenderer = NULL;
    gWindow = NULL;

    //Quit SDL subsystems
    //Clean up all dynamically loaded library handles, freeing memory
    //TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    printf( "Unload resource successfully. Close the window as you like.\n" );
    std::cin.get(); 
}

int producer( void* data )
{
    printf( "\nProducer started...\n" );

    //Seed thread random
    srand( SDL_GetTicks() );
    
    //Produce
    for( int i = 0; i < 5; ++i )
    {
        //Wait
        SDL_Delay( rand() % 1000 );
        
        //Produce
        produce();
    }

    printf( "\nProducer finished!\n" );
    
    return 0;
}

int consumer( void* data )
{
    printf( "\nConsumer started...\n" );

    //Seed thread random
    srand( SDL_GetTicks() );

    for( int i = 0; i < 5; ++i )
    {
        //Wait
        SDL_Delay( rand() % 1000 );
        
        //Consume
        consume();
    }
    
    printf( "\nConsumer finished!\n" );

    return 0;
}

void produce()
{
    //Lock
    SDL_LockMutex( gBufferLock );
    
    //If the buffer is full
    if( gData != -1 )
    {
        //we can wait on a condition with SDL_CondWait and unlock the mutex for other threads
        //Wait for buffer to be cleared
        printf( "\nProducer encountered full buffer, waiting for consumer to empty buffer...\n" );
        SDL_CondWait( gCanProduce, gBufferLock );
        //int SDL_CondWait(SDL_cond* cond, SDL_mutex* mutex);

    }

    //Fill and show buffer
    gData = rand() % 255;
    printf( "\nProduced %d\n", gData );
    
    //Unlock
    SDL_UnlockMutex( gBufferLock );
    
    //Signal consumer
    SDL_CondSignal( gCanConsume );
}

void consume()
{
    //Lock
    SDL_LockMutex( gBufferLock );
    
    //If the buffer is empty
    if( gData == -1 )
    {
        //Wait for buffer to be cleared
        printf( "\nConsumer encountered empty buffer, waiting for produce to fill buffer...\n" );
        SDL_CondWait( gCanConsume, gBufferLock );
        //int SDL_CondWait(SDL_cond* cond, SDL_mutex* mutex);
    }

    //Show and empty buffer
    printf( "\nConsumed %d\n", gData );
    gData = -1;
    
    //Unlock
    SDL_UnlockMutex( gBufferLock );
    
    //Signal producer
    SDL_CondSignal( gCanProduce );
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
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{	
			//Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;

            //Run the threads
            srand( SDL_GetTicks() );
            SDL_Thread* threadA = SDL_CreateThread( producer, "Thread A", (void*)"Thread A" );
            SDL_Delay( 16 + rand() % 32 );
            SDL_Thread* threadB = SDL_CreateThread( consumer, "Thread B", (void*)"Thread B" );

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
                }
                
                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                SDL_RenderClear( gRenderer );

                //Render objects
                gSplashTexture.render( 0, 0 );

                //Update screen
                SDL_RenderPresent( gRenderer );
            }

            //Wait for threads to finish
            SDL_WaitThread( threadA, NULL );
            SDL_WaitThread( threadB, NULL );
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}
