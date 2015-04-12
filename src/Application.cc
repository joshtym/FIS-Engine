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
const uint8_t Application::kUPDATE_CHANGE_LIMIT = 5;
const uint8_t Application::kUPDATE_RATE = 16;

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

Application::Application(std::string base_path)
{
  /* Initialize the variables */
  this->base_path = base_path;
  initialized = false;
  renderer = NULL;
  system_options = new Options(base_path);
  window = NULL;
  
  /* Initialize update variables */
  update_rate = kUPDATE_RATE;
  update_sync = 0;
  
  /* Set the title screen parameters */
  game_handler.setConfiguration(system_options);
  title_screen.setConfiguration(system_options);
  
  /* Sets the current mode */
  title_screen.setMusic();
  changeMode(TITLESCREEN);
  title_screen.enableView(true);
}

Application::~Application()
{
  /* Clear the options class */
  delete system_options;
  system_options = NULL;
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
  if(allow && this->mode != mode)
  {
    /* Changes to execute on the views closing */
    if(this->mode == TITLESCREEN)
      title_screen.enableView(false);
    
    this->mode = mode;
    
    /* Changes to execute on the views opening */
    if(this->mode == TITLESCREEN)
      title_screen.enableView(true);
  }
    
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
      SDL_KeyboardEvent press_event = event.key;
      
      /* Send the key to the relevant view */
      if(press_event.keysym.sym == SDLK_F7)
      {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
      }
      else if(press_event.keysym.sym == SDLK_F8)
      {
        SDL_SetWindowFullscreen(window, 0);
      }
      else if(mode == TITLESCREEN)
      {
        title_screen.keyDownEvent(press_event);
      }
      else if(mode == GAME)
      {
        /* If the key event returns true, exit the game view */
        if(game_handler.keyDownEvent(press_event))
          changeMode(TITLESCREEN);
      }
    }
    else if(event.type == SDL_KEYUP)
    {
      SDL_KeyboardEvent release_event = event.key;
      
      /* Send the key to the relevant view */
      if(mode == TITLESCREEN)
        title_screen.keyUpEvent(release_event);
      else if(mode == GAME)
        game_handler.keyUpEvent(release_event);
    }
    else if (event.type == SDL_WINDOWEVENT)
    {
      if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
      {

      }
      else if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
      {

      }
      else if (event.window.event == SDL_WINDOWEVENT_MOVED)
      {

      }
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

/* Returns the latched cycle time */
int Application::updateCycleTime(int cycle_time)
{
  // uint8_t update_step = kUPDATE_RATE / 2;
  uint8_t update_time = 0;
  //std::cout << "Cycle time: " << cycle_time << std::endl;
  
  /* Parse the cycle time and find the category */
  if(cycle_time < 0)
  {
    update_time = 0;
  }
  else if(cycle_time <= kUPDATE_RATE)
  {
    update_time = kUPDATE_RATE;
  }
  else
  {
    update_time = ((cycle_time + kUPDATE_RATE / 2) / kUPDATE_RATE) 
                                                   * kUPDATE_RATE;
  }

  // /* Check if the update time is different */
  // if(update_time == update_rate)
  // {
    // if(update_sync > 0)
      // update_sync--;
  // }
  // else
  // {
    // update_sync++;
  // }
  
  // /* Determine if an update is required */
  // if(update_sync > kUPDATE_CHANGE_LIMIT)
  // {
    // update_rate = update_time;
    // update_sync = 0;
  // }

  return update_time;
}

bool Application::updateViews(int cycle_time)
{
  bool quit = false;
  
  /* Handle any appropriate actions of the individual views */
  if(mode == TITLESCREEN)
  {

    /* Update the title screen, which returns if an action is available */
    if(title_screen.update(cycle_time))
    {
      /* If action is available, get it, and parse it to change the mode */
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

  /* Only proceed if successful */
  if(success)
  {
    /* Create window for display */
    window = SDL_CreateWindow("Univursa", SDL_WINDOWPOS_CENTERED, 
                              SDL_WINDOWPOS_CENTERED, 
                              system_options->getScreenWidth(), 
                              system_options->getScreenHeight(), 
                              SDL_WINDOW_SHOWN);// | SDL_WINDOW_FULLSCREEN);
    if(window == NULL)
    {
      std::cerr << "[ERROR] Window could not be created. SDL error: " 
                << SDL_GetError() << std::endl;
      success = false;
    }
  }
  
  /* Create the renderer for the window */
  if(success)
  {
    /* Set up the renderer flags */
    uint32_t flags = SDL_RENDERER_ACCELERATED;
    if(system_options->isVsyncEnabled())
      flags |= SDL_RENDERER_PRESENTVSYNC;

    auto renderer_index = -1;

#ifdef _WIN32_OPENGL
    /* Force OpenGL to be used as the rendering driver */
    if (SDL_GetNumRenderDrivers() > 1)
    {
      std::string open_gl = "opengl";
      auto renderer_0_info = new SDL_RendererInfo;
      auto renderer_1_info = new SDL_RendererInfo;

      auto error_index   = SDL_GetRenderDriverInfo(0, renderer_0_info);
      auto error_2_index = SDL_GetRenderDriverInfo(1, renderer_1_info);

      if (error_index > -1 && renderer_0_info != nullptr)
      {
        if (std::strcmp(open_gl.c_str(), renderer_0_info->name) == 0)
          renderer_index = 0;
      }
      else
      {
        std::cerr << "[ERROR]: Error attempting to discern rendering drivers" 
           << std::endl;
      }
      if (error_2_index > -1 && renderer_1_info != nullptr)
      {
        if (std::strcmp(open_gl.c_str(), renderer_1_info->name) == 0)
          renderer_index = 1;
      }
      else
      {
        std::cerr << "[ERROR]: Error attempting to discern rendering drivers" 
            << std::endl;
      }

      /* Create the renderer */
      renderer = SDL_CreateRenderer(window, renderer_index, flags);

      /* Attempt to override SDL Rendering Engine with OpenGL */
      // SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "opengl", 
      //     SDL_HINT_OVERRIDE);

      if (renderer != nullptr)
      {
        auto renderer_info = new SDL_RendererInfo;
        SDL_GetRenderDriverInfo(0, renderer_info);
        error_index = SDL_GetRendererInfo(renderer, renderer_info);
    
        if (error_index > -1 && renderer_info != nullptr)
        {
          std::cout << "Rendering Driver: " << renderer_info->name << std::endl;
        }
        else
        {
          std::cerr << "[ERROR] Unable to get rendering driver info." 
            << std::endl;
        }
       
        delete renderer_info;
      }

      delete renderer_0_info;
      delete renderer_1_info;

    }
#else //_WIN32
    else
    {
      /* If only one renderer exists, let SDL 'choose' */
      renderer = SDL_CreateRenderer(window, -1, flags);
    }
#endif

    if(renderer == NULL)
    {
      std::cerr << "[ERROR] Renderer could not be created. SDL error: "
                << SDL_GetError() << std::endl;
      success = false;
    }

    else
    {
      SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

      /* Create helper graphical portions */
      Helpers::createWhiteMask(renderer);
      
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
      
      /* Render logo */
      Frame logo;
      logo.setTexture(system_options->getBasePath() + "sprites/Title/logo.png", 
                      renderer);
      logo.render(renderer, 0, 0, system_options->getScreenWidth(), 
                  system_options->getScreenHeight());
      SDL_RenderPresent(renderer);
    }
  }
  
  /* Test the font engine */
  if(!system_options->confirmFontSetup())
  {
    std::cerr << "[ERROR] Could not create font. This is either a library "
              << "issue or the font files are missing or invalid." 
              << std::endl;
    success = false;
  }
  
  /* If successful, confirm the initialization */
  initialized = true;
  if(success)
  {
    /* Set the title screen background - TODO: Encapsulate in load?? */
    title_screen.setBackground("sprites/Title/old_title.png", renderer);
  }
  /* Uninitialize everything, if the init sequence failed */
  else if(!isInitialized())
  {
    uninitialize();
  }
  
	return success;
}

/* Returns the status if the application libraries have been initialized */
bool Application::isInitialized()
{
  return initialized;
}

/* Runs the application */
bool Application::run(std::string test_map)
{
  uint32_t count = 1;
  uint32_t cycle_time = kUPDATE_RATE;
  bool quit = false;
  uint32_t ticks = 0;
  
  /* Font testing - TODO: Remove */
  Text text1;
  text1.setFont("fonts/colab_light.otf", 16, TTF_STYLE_BOLD);
  text1.setText(renderer, "341", {255, 255, 255, 255});
  Text text2;
  text2.setFont("fonts/colab_light.otf", 16, TTF_STYLE_BOLD);
  TTF_SetFontOutline(text2.getFont(), 2);
  text2.setText(renderer, "341", {0, 0, 0, 255});
  
  if(isInitialized())
  {
    /* If the test map isn't empty, jump straight to game map */
    if(!test_map.empty())
    {
      game_handler.setTestMap(test_map);
      changeMode(GAME);
    }

    /* Main application loop */
    while(!quit)
    {
      /* Determine the previous cycle time for using throughout the update
       * sequence for rendering */
      uint32_t new_ticks = SDL_GetTicks();
      if(new_ticks > ticks + 200)
      {
        cycle_time = 200 / count;
        count = 0;
        ticks = new_ticks;
      }

      //std::cout << cycle_time << std::endl;
      //cycle_time = new_ticks - ticks;
      //ticks = new_ticks;
      //if(system_options->isVsyncEnabled())
      //  cycle_time = updateCycleTime(cycle_time);
      
      /* Handle events - key press, window events, and such */
      handleEvents();

      /* Update the view control (moving sprites, players, etc.)
       * This returns true if the application should shut down */
      if(updateViews(cycle_time))
        quit = true;

      /* Clear screen */
      SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
      SDL_RenderClear(renderer);
      
      /* Render the application view */
      render(cycle_time);

      /* Font testing - TODO: Remove */
      text2.render(renderer, 48, 48);
      text1.render(renderer, 50, 50);
     
      /* Update screen */
      SDL_RenderPresent(renderer);
      count++;
      
      /* Delay if VSync is not enabled */
      if(!system_options->isVsyncEnabled())
        SDL_Delay(12);
    }
        
    return true;
  }
  
  return false;
}
  
void Application::uninitialize()
{
  if(initialized)
  {
    /* Clean up the renderer */
    if(renderer != NULL)
    {
      Helpers::deleteWhiteMask();
      SDL_DestroyRenderer(renderer);
    }
    renderer = NULL;
  
    /* Clean up the window */
    if(window != NULL)
      SDL_DestroyWindow(window);
    window = NULL;
    
    initialized = false;
  }
}
