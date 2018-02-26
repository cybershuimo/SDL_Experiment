//Main program file
//basic actions, moving, eating food and growing

#include "snake.h"

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

//check memory leak
#include <vld.h>


//Snake body length limit
const int BODY_LIMIT = 100;

//The snake that will be moving around on the screen
Snake snake;
//Snake body tiles
SnakeBody* snakeBody[ BODY_LIMIT ];
//Food to be eaten; initialized position
Food food( 20 * 20, 20 * 20 );


int main( int argc, char* args[] )
//int main()
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

            int snakeBodyLength = snake.getLength();
            int posX = 360;   // set it as the offset X of snake head 
            int posY = 200;   // set it as the offset Y of snake head

            //Initialize snake body
            for (int i = 0; i < snakeBodyLength; ++i)
            {
                posX += 20;
                snakeBody[ i ] = new SnakeBody( posX, posY );
            }

            //Keeps track of time between steps
            LTimer stepTimer;
            stepTimer.start();

            //The camera area
            //SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

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

                    //Handle input for the camera
                    snake.handleEvent( e );
                }

                // //Calculate time step; why 1000, move too slowly?
                // float timeStep = stepTimer.getTicks() / 100.f;

                //Move the snake
                //Every 0.5s (this could change) moves one step (= head length)
                if ( stepTimer.getTicks() > 200 )
                {
                    //Snake head moves
                    snake.move( posX, posY );

                    //Snake body moves
                    snakeBody[ 0 ]->move( posX, posY, snake.getBox() );
                    for (int i = 1; i < snakeBodyLength; ++i)
                    {
                        snakeBody[ i ]->move( posX, posY, snakeBody[ i - 1 ]->getBox() );
                    }

                    //Restart timer
                    stepTimer.start();
                }

                //Clear screen; black background color
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
                SDL_RenderClear( gRenderer );

                //Render objects
                snake.render();
                for (int i = 0; i < snakeBodyLength; ++i)
                {
                    snakeBody[ i ]->render();
                }

                //Check if Food eaten or not
                if ( !food.eaten( snake.getBox() ) )
                {
                    food.render();
                }
                //Eating food
                else
                {
                    //Generate a new food
                    food.generate();
                    //Add snake length
                    snake.addLength();
                    snakeBodyLength = snake.getLength();
                    if ( snakeBodyLength <= BODY_LIMIT )
                    {
                        snakeBody[ snakeBodyLength - 1 ] = new SnakeBody( posX, posY );
                    }   
                }

                //Update screen
                SDL_RenderPresent( gRenderer );
            }

            //Deallocate snake body tiles
            for( int i = 0; i < snakeBodyLength; ++i )
            {
                 if( snakeBody[ i ] != NULL )
                 {
                    delete snakeBody[ i ];
                    snakeBody[ i ] = NULL;
                 }
            }
        }

    }

    //Free resources and close SDL
    close();

    return 0;
}