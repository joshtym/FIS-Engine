/*******************************************************************************
 * Class Name: Application
 * Date Created: November 17, 2013
 * Inheritance: none
 * Description: The Main Application that handles all the displays for the 
 *              entire game. This includes the primary switching mechanism to 
 *              switch between views as needed according to the selection on 
 *              the screen.
 ******************************************************************************/
#include "Application.h"

/* Constant Implementation - see header file for descriptions */
//const short Application::kTICK_DELAY = 10;

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

Application::Application()
{
  /* Initialize the variables */
  initialized = false;
  mode = TITLESCREEN;
  renderer = NULL;
  window = NULL;
  
  /* Set the title screen parameters */
  game_handler.setConfiguration(&system_options);
  title_screen.setConfiguration(&system_options);
}

Application::~Application()
{
  uninitialize();
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
  
/* Change the mode that the application is running */
bool Application::changeMode(AppMode mode)
{
  bool allow = true;
  
  /* Run logic to determine if mode switch is allowed - currently empty */
  
  /* If allowed, make change */
  if(allow)
    this->mode = mode;
    
  return allow;
}

/* Goes through all available events that are currently on the stack */
void Application::handleEvents()
{
  SDL_Event event;
  
  while(SDL_PollEvent(&event) != 0)
  {
    /* If quit initialized, end the game loop */
    if(event.type == SDL_QUIT)
    {
      changeMode(EXIT);
    }
    /* Otherwise, pass the key down events on to the active view */
    else if(event.type == SDL_KEYDOWN)
    {
      SDL_Keysym symbol = event.key.keysym;
      
      /* Send the key to the relevant view */
      if(mode == TITLESCREEN)
      {
        title_screen.keyDownEvent(symbol);
      }
      else if(mode == GAME)
      {
        /* If the key event returns true, exit the game view */
        if(game_handler.keyDownEvent(symbol))
          changeMode(TITLESCREEN);
      }
    }
    else if(event.type == SDL_KEYUP)
    {
      SDL_Keysym symbol = event.key.keysym;
      
      /* Send the key to the relevant view */
      if(mode == TITLESCREEN)
        title_screen.keyUpEvent(symbol);
      else if(mode == GAME)
        game_handler.keyUpEvent(symbol);
    }
  }
}
  
void Application::render(uint32_t cycle_time)
{
  /* Handle the individual action items, depending on whats running */
  if(mode == TITLESCREEN)
    title_screen.render(renderer);
  else if(mode == GAME)
    game_handler.render(renderer);
  else if(mode == OPTIONS)
    cycle_time = cycle_time; // DO OPTIONS EXECUTION
}

bool Application::updateViews(int cycle_time)
{
  bool quit = false;
  
  /* Handle any appropriate actions of the individual views */
  if(mode == TITLESCREEN)
  {
    /* Check if there is a title screen action */
    if(title_screen.isActionOnQueue())
    {
      TitleScreen::MenuItems action_item = title_screen.getAction();
      if(action_item == TitleScreen::EXIT)
        changeMode(EXIT);
      else if(action_item == TitleScreen::GAME)
        changeMode(GAME);
    }
  }
  /* Otherwise, update the game and check if the game is finished */
  else if(mode == GAME)
  {
    if(game_handler.update(cycle_time))
      changeMode(TITLESCREEN);
  }
  /* If exit, return true to notify the running thread the application is 
   * done */
  else if(mode == EXIT)
  {
    quit = true;
  }
  
  return quit;
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/* Provides initialization of the SDL engine. Required for running. */
bool Application::initialize()
{
	bool success = !isInitialized();

  /* Initialize SDL */
  if(success && SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    printf("[ERROR] SDL could not initialize. SDL error: %s\n", SDL_GetError());
    success = false;
  }

  /* Only proceed if successful */
  if(success)
  {
    /* Create window for display */
    window = SDL_CreateWindow("Univursa", SDL_WINDOWPOS_CENTERED, 
                              SDL_WINDOWPOS_CENTERED, 
                              system_options.getScreenWidth(), 
                              system_options.getScreenHeight(), 
                              SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(window == NULL)
    {
      printf("[ERROR] Window could not be created. SDL error: %s\n", 
             SDL_GetError());
      success = false;
    }
  }
  
  /* Create the renderer for the window */
  if(success)
  {
    /* Set up the renderer flags */
    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if(system_options.isVsyncEnabled())
      flags |= SDL_RENDERER_PRESENTVSYNC;
    
    /* Create the renderer */
    renderer = SDL_CreateRenderer(window, -1, flags);
    if(renderer == NULL)
    {
      printf("[ERROR] Renderer could not be created. SDL error: %s\n", 
             SDL_GetError());
      success = false;
    }
    else
    {
      SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    }
  }
  
  /* Initialize SDL Image library */
  if(success)
  {
    if(!IMG_Init(IMG_INIT_PNG))
    {
      printf("[ERROR] SDL_Image could not initialize. SDL_image error: %s\n", 
             IMG_GetError());
      success = false;
    }
  }
  
  /* Initialize SDL TTF library */
  if(success)
  {
    if(TTF_Init() == -1)
    {
      printf("[ERROR] SDL_ttf could not initialize. SDL_ttf error: %s\n",
             TTF_GetError());
      success = false;
    }
    
    success = system_options.confirmFontSetup();
  }

  /* If successful, confirm the initialization */
  if(success)
  {
    initialized = true;
    
    /* Set the title screen background */
    title_screen.setBackground("pictures/univursatitle.png", renderer);
  }
  /* Uninitialize everything, if the init sequence failed */
  else if(!isInitialized())
    uninitialize();
  
	return success;
}

/* Returns the status if the application libraries have been initialized */
bool Application::isInitialized()
{
  return initialized;
}

/* Runs the application */
bool Application::run()
{
  bool quit = false;
  uint32_t ticks = 0;
  
  // TODO: Remove - testing variables
  int angle = 0;
  double brightness = 1.0;
  double brightness2 = 1.0;
  bool increasing = true;
  
  if(isInitialized())
  {
    // TODO: Remove - sprite setup
    Frame white_mask("sprites/white_mask.png", renderer);
    SDL_SetTextureBlendMode(white_mask.getTexture(), SDL_BLENDMODE_ADD);
    
    Sprite image_sprite("sprites/raven_AA_D|3|.png", renderer);
    image_sprite.setWhiteMask(white_mask.getTexture());
    image_sprite.setBrightness(1.2f);
    image_sprite.setColorBalance(128, 128, 255);
    //image_sprite.flipHorizontal();
    //image_sprite.flipVertical(true);
    image_sprite.updateSprite(0, renderer);
    
    Sprite bubby_sprite("sprites/bubby_AA_A00.png", renderer);
    bubby_sprite.setWhiteMask(white_mask.getTexture());
    bubby_sprite.setOpacity(128);
    bubby_sprite.updateSprite(0, renderer);
    
    Sprite coin_sprite("sprites/coins_AA_A00.png", renderer);
    coin_sprite.setWhiteMask(white_mask.getTexture());
    coin_sprite.updateSprite(0, renderer);
    
    /* Main application loop */
    while(!quit)
    {
      // TODO: Remove - Sprite logic during rendering
      angle++;
      if(angle == 180)
        image_sprite.flipVertical(true);
      else if(angle == 360)
      {
        image_sprite.flipVertical(false);
        image_sprite.removeTail();
        angle = 0;
      }
      
      if(increasing)
      {
        brightness += 0.01;
        if(brightness > 1.3)
          increasing = false;
      }
      else
      {
        brightness -= 0.01;
        if(brightness <= 1.0)
          increasing = true;
      }
      brightness2 += 0.01;
      if(brightness2 >= 2.0)
        brightness2 = 0.0;
      
      /* Determine the previous cycle time for using throughout the update
       * sequence for rendering */
      uint32_t cycle_time = SDL_GetTicks() - ticks;
      ticks = SDL_GetTicks();
      
      /* Handle events - key press, window events, and such */
      handleEvents();
      
      /* Update the view control (moving sprites, players, etc.)
       * This returns true if the application should shut down */
      if(updateViews(cycle_time))
        quit = true;
      
      /* Clear screen */
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
      SDL_RenderClear(renderer);
      
      /* Render the application view */
      render(cycle_time);
      
      // TODO: Remove - Sprite rendering
      image_sprite.setBrightness(brightness2);
      image_sprite.setRotation(angle);
      image_sprite.updateSprite(10, renderer);
      image_sprite.render(renderer, 512, 512);
      
      bubby_sprite.setBrightness(brightness);
      bubby_sprite.updateSprite(10, renderer);
      bubby_sprite.render(renderer, 512, 525);
      
      coin_sprite.setBrightness(brightness);
      coin_sprite.updateSprite(10, renderer);
      coin_sprite.render(renderer, 700, 300);
      
      /* Update screen */
      SDL_RenderPresent(renderer);
    }
        
    return true;
  }
  
  return false;
}
  
void Application::uninitialize()
{
  /* Clean up the renderer */
  if(renderer != NULL)
    SDL_DestroyRenderer(renderer);
  renderer = NULL;
  
  /* Clean up the window */
  if(window != NULL)
    SDL_DestroyWindow(window);
  window = NULL;

	/* Quit SDL sub-systems */
	IMG_Quit();
	SDL_Quit();
}
