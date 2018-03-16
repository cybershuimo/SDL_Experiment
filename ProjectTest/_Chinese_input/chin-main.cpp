//Main program file
//basic actions, moving, eating food and growing

#include "chin.h"

//Using SDL, SDL_image, standard IO, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

//check memory leak
#include <vld.h>

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

            //The current input text (default content)
            std::string inputText = "";
            SDL_Color textColor = { 200, 0, 0 };
            //Enable text input, use SDL_StopTextInput() when finished
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
                    //Special key input
                    else if( e.type == SDL_KEYDOWN )
                    {
                        //Handle backspace
                        if( e.key.keysym.sym == SDLK_BACKSPACE && inputText.length() > 0 )
                        {
                            if( inputText != "" )
                            {
                                //lop off character; if input is chinese character, pop back TWICE?
                                inputText.pop_back();
                            }
                        }
                    }
                    else if( e.type == SDL_TEXTINPUT )
                    {
                        //Allow result string length less than 24 (8 Chinese characters)
                        if( inputText.size() + strlen(e.text.text) <= 24 )
                        {
                            //Append character
                            inputText += e.text.text;
                            //renderText = true;
                        }
                    }    
                }

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
                SDL_RenderClear( gRenderer );

                //Text is not empty
                if( inputText != "" )
                {
                    //Render new text
                    gTextTexture.loadFromRenderedText( inputText.c_str(), textColor );
                }
                //Text is empty
                else
                {
                    //Render space texture
                    gTextTexture.loadFromRenderedText( " ", textColor );
                }
                //Render textures to screen
                gPromptTexture.render( ( SCREEN_WIDTH - gPromptTexture.getWidth() ) / 3, ( SCREEN_HEIGHT - gPromptTexture.getHeight() ) / 2, NULL);
                gTextTexture.render( ( SCREEN_WIDTH - gPromptTexture.getWidth() ) / 3 + gPromptTexture.getWidth(), ( SCREEN_HEIGHT - gPromptTexture.getHeight() ) / 2, NULL);

                //Update screen
                SDL_RenderPresent( gRenderer );
            }

            //Disable text input
            SDL_StopTextInput();
        }
    }

    //Free resources and close SDL
    close();

    return 0;
}
