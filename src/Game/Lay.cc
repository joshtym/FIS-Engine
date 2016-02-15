/*******************************************************************************
* Class Name: Lay [Implementation]
* Date Created: February 14, 2016
* Inheritance: None
* Description: A lay ins an underlay, midlay, or overlay Sprite rendered on
*              the map or in Battle. A lay is seamless with any velocity
*              movement (x, y).
*
* Notes
* -----
*
* [1]:
*
* See .h file for TODOs
******************************************************************************/
#include "Game/Lay.h"

/*=============================================================================
 * CONSTANTS - See .h for Details
 *============================================================================*/

const float Lay::kABS_MAX_VELOCITY_X{700};
const float Lay::kABS_MAX_VELOCITY_Y{300};

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description:
 *
 * Inputs:
 */
Lay::Lay()
    : animation_time{0},
      config{nullptr},
      flags{static_cast<LayState>(0)},
      path{""},
      renderer{nullptr},
      lay_type{LayType::NONE}
{
}

/*
 * Description:
 *
 * Inputs:
 */
Lay::Lay(std::string path, Floatinate velocity, LayType lay_type,
         SDL_Renderer* renderer, Options* config)
{
  this->path = path;
  this->lay_type = lay_type;
  this->velocity = velocity;

  setFlag(LayState::SCREEN_SIZE, true);
  setFlag(LayState::PLAYER_RELATIVE, true);

  setConfig(config);
  setRenderer(renderer);

  if(this->lay_type != LayType::NONE)
  {
    auto success = createTiledLays();

#ifdef UDEBUG
    if(!success)
    {
      std::cout << "[ERROR] Creating tile overlays with path: " << path
                << std::endl;
    }
#endif
  }
}

/*
 * Description:
 *
 * Inputs: std::string path - the path to the lay's sprite to tile
 *         Floatinate velocity - (x, y) velocity to stream the sprite
 *         LayType lay_type - enumerated type of the lay (under, over, etc.)
 *         SDL_Renderer* renderer - pointer to the renderer
 *         Options* config - pointer to running configuration
 */
Lay::Lay(std::string path, uint32_t animation_time, Floatinate velocity,
         LayType lay_type, SDL_Renderer* renderer, Options* config)
    : Lay()
{
  this->path = path;
  this->animation_time = animation_time;
  this->lay_type = lay_type;
  this->velocity = velocity;

  setFlag(LayState::SCREEN_SIZE, true);
  setFlag(LayState::PLAYER_RELATIVE, false);

  setConfig(config);
  setRenderer(renderer);
  setVelocity(velocity);

  if(this->lay_type != LayType::NONE)
  {
    auto success = createTiledLays();

#ifdef UDEBUG
    if(!success)
    {
      std::cout << "[ERROR] Creating tile overlays with path: " << path
                << std::endl;
    }
#endif
  }
}

/*
 * Description: Annihilates a Lay object. Upon destruction, each lay tile
 *              and lay tile sprite must be cleared.
 */
Lay::~Lay()
{
  for(auto& tile : lay_tiles)
  {
    if(tile)
      delete tile;

    tile = nullptr;
  }

  lay_tiles.clear();
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Renders the lay at its given position. If the lay flag
 *              is set to screen_size, the configuration screen width will be
 *              used as the size to render the lay.
 *
 * Inputs: none
 * Output: bool - true if rendering was successful
 */
bool Lay::render()
{
  bool success{true};

  if(renderer)
  {

    for(const auto& lay_tile : lay_tiles)
    {
      if(getFlag(LayState::SCREEN_SIZE) && lay_tile && lay_tile->lay_sprite &&
         config)
      {
        success &= lay_tile->lay_sprite->render(
            renderer, lay_tile->location.x, lay_tile->location.y,
            config->getScreenWidth(), config->getScreenHeight());
      }
    }
  }

  return success;
}

/*
 * Description: Updates the position of the Lay based upon its (x, y) velocity
 *              if it is not a player_relative lay. This method will shuffle
 *              the lays back into appropriate positions to create the seamless
 *              transfer effect for any value of (x, y) velocity.
 *
 * Inputs: int32_t cycle_time - the update cycle time, affecting the velocity
 * Output: none
 */
void Lay::update(int32_t cycle_time)
{
  assert(config);

  error.x += cycle_time * velocity.x;
  error.y += cycle_time * velocity.y;

  auto dist_x = std::floor(error.x);
  auto dist_y = std::floor(error.y);

  error.x -= dist_x;
  error.y -= dist_y;

  for(auto& lay_tile : lay_tiles)
  {
    if(lay_tile)
    {
      if(velocity.x > 0)
      {
        if((lay_tile->location.x + dist_x) > config->getScreenWidth())
          lay_tile->location.x -= config->getScreenWidth() * 2;

        lay_tile->location.x += dist_x;
      }

      if(velocity.y > 0)
      {
        if((lay_tile->location.y + dist_y) > config->getScreenHeight())
          lay_tile->location.y -= config->getScreenHeight() * 2;

        lay_tile->location.y += dist_y;
      }

      if(velocity.x < 0)
      {
        if((lay_tile->location.x + dist_x) < -config->getScreenWidth())
          lay_tile->location.x += config->getScreenWidth() * 2;

        lay_tile->location.x += dist_x;
      }

      if(velocity.y < 0)
      {
        if((lay_tile->location.y + dist_y) < -config->getScreenHeight())
          lay_tile->location.y += config->getScreenHeight() * 2;

        lay_tile->location.y += dist_y;
      }

      if(lay_tile->lay_sprite)
        lay_tile->lay_sprite->update(animation_time);
    }
  }
}

/*
 * Description: Evaluates and returns a given LayState flag.
 *
 * Inputs: const LayState& test_flag - given flag to find the value for
 * Output: bool - the value of the given flag
 */
bool Lay::getFlag(const LayState& test_flag)
{
  return static_cast<bool>((flags & test_flag) == test_flag);
}

/*
 * Description: Assigns the configuration for the Lay object.
 *
 * Inputs: Options* config - pointer to the running config
 * Output: none
 */
void Lay::setConfig(Options* config)
{
  this->config = config;
}

/*
 * Description: Assigns a given LayState flag to a given boolean value.
 *
 * Inputs: const LayState& flag - enumerated flag to be assigned
 *         const bool& set_value - value to assign to the flag
 * Output: none
 */
void Lay::setFlag(const LayState& flag, const bool& set_value)
{
  (set_value) ? (flags |= flag) : (flags &= ~flag);
}

/*
 * Description: Assigns the rendering pointer for the Lay object.
 *
 * Inputs: SDL_Renderer* renderer - pointer to the renderer
 * Output: none
 */
void Lay::setRenderer(SDL_Renderer* renderer)
{
  this->renderer = renderer;
}

/*
 * Description: Assigns a given Floatinate velocity (x, y) such that it will
 *              be within acceptable parameters
 *
 * Inputs: Floatinate new_velocity - the velocity (x, y) to be assigned
 * Output: none
 */
void Lay::setVelocity(Floatinate new_velocity)
{
  velocity.x = Helpers::setInRange(new_velocity.x, -kABS_MAX_VELOCITY_X,
                                   kABS_MAX_VELOCITY_X);
  velocity.y = Helpers::setInRange(new_velocity.y, -kABS_MAX_VELOCITY_Y,
                                   kABS_MAX_VELOCITY_Y);
}

/*
 * Description: Creates a tiled lay given an enumerated index (NORTH_EAST,
 *              EAST, etc.). To create a lay, the path must be split to
 *              determine which Sprite constructor is to be called. This method
 *              assings the appropriate starting (x, y) for the given tiled lay.
 *
 * Inputs: LayIndex lay_index - enumerated index of tiled lay to create
 * Output: bool - true if the tiled lay was created successfully
 */
bool Lay::createTiledLay(LayIndex lay_index)
{
  if(renderer && config && path != "")
  {
    auto new_tile = new LayTile();

    auto split = Helpers::split(path, '|');
    auto num_frames = 1;

    if(split.size() == 3)
    {
      num_frames = std::stoi(split.at(1));
      new_tile->lay_sprite =
          new Sprite(split.at(0), num_frames, split.at(2), renderer);
    }
    else if(split.size() == 1)
    {
      new_tile->lay_sprite = new Sprite(path, renderer);
    }

    if((split.size() == 3 || split.size() == 1) && new_tile->lay_sprite)
    {
      new_tile->lay_sprite->setAnimationTime(animation_time);
      new_tile->lay_sprite->setNonUnique(true, num_frames);
      new_tile->lay_sprite->createTexture(renderer);

      auto x = config->getScreenWidth();
      auto y = config->getScreenHeight();

      if(lay_index == LayIndex::NORTH_WEST)
        new_tile->location = {-x, -y};
      else if(lay_index == LayIndex::NORTH)
        new_tile->location = {0, -y};
      else if(lay_index == LayIndex::NORTH_EAST)
        new_tile->location = {x, -y};
      else if(lay_index == LayIndex::WEST)
        new_tile->location = {-x, 0};
      else if(lay_index == LayIndex::CENTRE)
        new_tile->location = {0, 0};
      else if(lay_index == LayIndex::EAST)
        new_tile->location = {x, 0};
      else if(lay_index == LayIndex::SOUTH_WEST)
        new_tile->location = {-x, y};
      else if(lay_index == LayIndex::SOUTH)
        new_tile->location = {0, y};
      else if(lay_index == LayIndex::SOUTH_EAST)
        new_tile->location = {x, y};

      lay_tiles.push_back(new_tile);

      return true;
    }
  }

  return false;
}

/*
 * Description: Determines which lay tiles (NORTH_EAST, EAST, etc.) need to
 *              be created for the given velocity (x, y) floatinate. This
 *              method will call for the creation of the needed lays and
 *              return whether they were successfully constructed.
 *
 * Inputs: none
 * Output: bool - true if the lays were created successfully
 */
bool Lay::createTiledLays()
{
  bool success{true};

  success &= createTiledLay(LayIndex::CENTRE);

  if(velocity.x < 0)
    success &= createTiledLay(LayIndex::EAST);
  if(velocity.x > 0)
    success &= createTiledLay(LayIndex::WEST);
  if(velocity.y < 0)
    success &= createTiledLay(LayIndex::SOUTH);
  if(velocity.y > 0)
    success &= createTiledLay(LayIndex::NORTH);

  if(velocity.x > 0 && velocity.y > 0)
    success &= createTiledLay(LayIndex::NORTH_WEST);
  else if(velocity.x > 0 && velocity.y < 0)
    success &= createTiledLay(LayIndex::SOUTH_WEST);
  else if(velocity.x < 0 && velocity.y > 0)
    success &= createTiledLay(LayIndex::NORTH_EAST);
  else if(velocity.x < 0 && velocity.y < 0)
    success &= createTiledLay(LayIndex::SOUTH_EAST);

  return success;
}
