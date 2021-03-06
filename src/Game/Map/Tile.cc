/*******************************************************************************
 * Class Name: Tile
 * Date Created: December 2, 2012
 * Inheritance: none
 * Description: This class handles the basic tile that is set up on the map.
 *              It is the overall structure. The tile class creates a base,
 *              enhancer, lower, upper, item, person, and thing to define
 *              all the possibilities on the tile. This also handles printing
 *              its own tile data and ensuring that movement isn't entered
 *              through the tile. For additional information, read the comments
 *              below for each function.
 ******************************************************************************/
#include "Game/Map/Tile.h"

/* Constant Implementation - see header file for descriptions */
const uint8_t Tile::kLOWER_COUNT_MAX = 5;
const uint8_t Tile::kMAX_ITEMS = 10;
const uint8_t Tile::kUPPER_COUNT_MAX = 5;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/*
 * Description: Constructor for this class. Set up the tile class to an empty
 *              null state. In this state, you will have to manually set
 *              the width, height, x, and y coordinates of the tile and then
 *              enable the tile using setStatus()
 *
 * Input: none
 */
Tile::Tile()
{
  event_handler = NULL;
  clear();
}

/*
 * Description: Constructor for this class. Takes information on the tile
 *              such as size, location and the parent and sets up the given
 *              tile. This will be framed by an upper class that handles
 *              overall control (Map).
 *
 * Inputs: EventHandler* event_handler - the handler for all events
 *         uint16_t width - the width of the tile in pixels
 *         uint16_t height - the height of the tile in pixels
 *         uint16_t x - the x location respective to the parent (in tile count)
 *         uint16_t y - the y location respective to the parent (in tile count)
 */
Tile::Tile(EventHandler* event_handler, uint16_t width, uint16_t height,
                                        uint16_t x, uint16_t y)
    : Tile()
{
  /* Reset the parameters based on this alternate constructor */
  setEventHandler(event_handler);
  setHeight(height);
  setStatus(ACTIVE);
  setWidth(width);
  setX(x);
  setY(y);
}

/*
 * Description: Destructor function
 */
Tile::~Tile()
{
  clear();
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

/*
 * Description: This increases the IO stack to allow for the indicated
 *              render level to be set in the stack. This will not exceed
 *              the max allowable by Helpers::getRenderDepth()
 *
 * Inputs: uint8_t render_level - the render level corresponding to the IO
 * Output: bool - status if the indicated level can be set
 */
bool Tile::growIOStack(uint8_t render_level)
{
  if(render_level < Helpers::getRenderDepth())
  {
    MapInteractiveObject* null_io = NULL;

    while(ios.size() <= render_level)
      ios.push_back(null_io);

    return true;
  }

  return false;
}

/*
 * Description: This increases the person stack to allow for the indicated
 *              render level to be set in the stack. This will not exceed
 *              the max allowable by Helpers::getRenderDepth()
 *
 * Inputs: uint8_t render_level - the render level corresponding to the person
 * Output: bool - status if the indicated level can be set
 */
bool Tile::growPersonStack(uint8_t render_level)
{
  if(render_level < Helpers::getRenderDepth())
  {
    MapPerson* null_person = NULL;

    /* Ensure array sizes */
    while(persons_main.size() <= render_level)
      persons_main.push_back(null_person);
    while(persons_prev.size() <= render_level)
      persons_prev.push_back(null_person);

    return true;
  }

  return false;
}

/*
 * Description: This increases the thing stack to allow for the indicated
 *              render level to be set in the stack. This will not exceed
 *              the max allowable by Helpers::getRenderDepth()
 *
 * Inputs: uint8_t render_level - the render level corresponding to the thing
 * Output: bool - status if the indicated level can be set
 */
bool Tile::growThingStack(uint8_t render_level)
{
  if(render_level < Helpers::getRenderDepth())
  {
    MapThing* null_thing = NULL;

    while(things.size() <= render_level)
      things.push_back(null_thing);

    return true;
  }

  return false;
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

/*
 * Description: Adds an item to the tile. It will only render the top one
 *              but can hold up to kMAX_ITEMS count. This allows for walkovers
 *              as well as pickups.
 *
 * Inputs: MapItem* item - new item pointer to add to the tile
 * Output: bool - if the item was successfully added
 */
bool Tile::addItem(MapItem* item)
{
  if(item != NULL && items.size() < kMAX_ITEMS)
  {
    items.push_back(item);
    return true;
  }

  return false;
}

/*
 * Description: Adds a passability to the given tile based on data from a file
 *              that is read in. The data is a comma delimited directional
 *              list, the classifier is if it's base or lower and the index
 *              corresponds to which lower layer (if applicable).
 *
 * Inputs: std::string data - the passability data, as mentioned above.
 *         std::string classifier - the layer classifier (base, lower)
 *         std::string index - the lower layer index
 * Output: bool - status if the set was successful
 */
bool Tile::addPassability(std::string data, std::string classifier,
                                            std::string index)
{
  bool success = true;
  Direction new_direction = Direction::DIRECTIONLESS;
  std::vector<std::string> data_list = Helpers::split(data, ',');

  /* Loop through each value of the data list to add */
  for(uint16_t i = 0; i < data_list.size(); i++)
  {
    /* Determine the data identifier first */
    if(data_list[i] == "N")
      new_direction = Direction::NORTH;
    else if(data_list[i] == "E")
      new_direction = Direction::EAST;
    else if(data_list[i] == "S")
      new_direction = Direction::SOUTH;
    else if(data_list[i] == "W")
      new_direction = Direction::WEST;

    /* Add the layer passability to the specific classifier */
    if(classifier == "base")
      success &= setBasePassability(new_direction, true);
    else if(classifier == "lower")
      success &= setLowerPassability(std::stoi(index), new_direction, true);
  }

  return success;
}

/*
 * Description: Adds in sprite data for the applicable frames. Based on a
 *              classifier, which indicates which layer and an index (which
 *              is only applicable for stackable layers, such as lower and
 *              upper).
 *
 * Inputs: Sprite* frames - the sprite data to add in (pointer only).
 *         std::string classifier - the layer classifier (base, lower, etc.)
 *         std::string index - the layering index (for lower and upper)
 * Output: bool - status if addition was successful
 */
bool Tile::addSprite(Sprite* frames, std::string classifier, std::string index)
{
  /* Add the layer data to the specific classifier */
  if(classifier == "base")
    return setBase(frames);
  else if(classifier == "enhancer")
    return setEnhancer(frames);
  else if(classifier == "lower")
    return insertLower(frames, std::stoi(index));
  else if(classifier == "upper")
    return insertUpper(frames, std::stoi(index));
  return false;
}

/*
 * Description: Animates all sprite layer sets on the tile. This allows for
 *              the fine control over when the timer hits and how it's updated.
 *
 * Inputs: none
 * Output: none
 */
void Tile::animate()
{
  /* Animate the base first */
  if(base != NULL)
    base->shiftNext();

  /* Then the enhancer sprite */
  if(enhancer != NULL)
    enhancer->shiftNext();

  /* Then animate the set of lower layers */
  for(uint8_t i = 0; i < lower.size(); i++)
    if(lower[i] != NULL)
      lower[i]->shiftNext();

  /* Finish by animating the upper set, if set */
  for(uint8_t i = 0; i < upper.size(); i++)
    if(upper[i] != NULL)
      upper[i]->shiftNext();
}

/*
 * Description: Clears the data within the tile. Usually used for cleaning
 *              up after the tile operation is complete. Has the functionality
 *              just to clean out the sprites and not the width/height and
 *              coordinates of the tile.
 *
 * Inputs: bool just_sprites - Only clean up the sprites or all of the class.
 * Output: none
 */
void Tile::clear(bool just_sprites)
{
  /* Clear sprite layer data */
  unsetBase();
  unsetEnhancer();
  unsetIOs();
  unsetItems();
  unsetLower();
  unsetPersons(true);
  unsetThings();
  unsetUpper();

  if(!just_sprites)
  {
    /* Reset tile parameters */
    setWidth(1);
    setHeight(1);
    setStatus(OFF);

    /* Reset coordinates */
    setX(0);
    setY(0);
  }

  clearEvents();
}

/*
 * Description: Clears all events stored within the class. Call only works if
 *              the event handler has already been set up and is available.
 *              Note that if there is no event handler, no events fire.
 *
 * Inputs: none
 * Output: bool - status if the events were cleared.
 */
bool Tile::clearEvents()
{
  event_enter.clear();
  event_exit.clear();
  return true;
}

/*
 * Description: Gets the base sprite sequence and returns it, if set.
 *
 * Inputs: none
 * Output: Sprite* - the base sprite pointer
 */
Sprite* Tile::getBase() const
{
  return base;
}

/*
 * Description: Gets the passability for the base portion of the tile. This
 *              only responds true passability for set sprites. Otherwise,
 *              it returns true.
 *
 * Inputs: Direction dir - the direction to get
 * Output: bool - the base passability directional status
 */
bool Tile::getBasePassability(Direction dir) const
{
  if(base != NULL)
  {
    if(dir != Direction::DIRECTIONLESS)
      return ((base_passability & static_cast<uint8_t>(dir)) > 0);
  }

  return false;
}

/*
 * Description: Gets the enhancer sprite sequence and returns it, if set.
 *
 * Inputs: none
 * Output: Sprite* - the enhancer sprite pointer
 */
Sprite* Tile::getEnhancer() const
{
  return enhancer;
}

/*
 * Description: Returns the tile enter event set reference. Will never return a
 *              nullptr
 *
 * Inputs: none
 * Output: EventSet* - the enter event set
 */
EventSet* Tile::getEventEnter()
{
  return &event_enter;
}

/*
 * Description: Returns the tile exit event set reference. Will never return a
 *              nullptr
 *
 * Inputs: none
 * Output: EventSet* - the exit event set
 */
EventSet* Tile::getEventExit()
{
  return &event_exit;
}

/*
 * Description: Gets the height stored within the tile (and all the layers).
 *
 * Inputs: none
 * Output: uint16_t - the height in pixels
 */
uint16_t Tile::getHeight() const
{
  return height;
}

/*
 * Description: Returns the IO stored on this tile at the indicated render
 *              level.
 *
 * Inputs: uint8_t render_level - integer of the render level on tile
 * Output: MapInteractiveObject* - IO reference stored. NULL if not set
 */
MapInteractiveObject* Tile::getIO(uint8_t render_level) const
{
  MapInteractiveObject* selected_io = NULL;

  if(ios.size() > render_level)
    selected_io = ios[render_level];

  return selected_io;
}

/*
 * Description: Returns all IOs stored on the tile. This is ordered based
 *              on render level. I.e., index 4 will correspond to render level
 *              4. If the render level is not set in the vector, it is NULL
 *              and not used.
 *
 * Inputs: none
 * Output: std::vector<MapInteractiveObject*> - stack of IOs stored in tile
 */
std::vector<MapInteractiveObject*> Tile::getIOs() const
{
  return ios;
}

/*
 * Description: Returns the number of items sitting on the tile.
 *
 * Inputs: none
 * Output: uint16_t - unsigned int of item count
 */
uint16_t Tile::getItemCount() const
{
  return items.size();
}

/*
 * Description: Returns a stack of all item pointers sitting on the tile.
 *              Do not delete item pointers; it would cause unknown results.
 *
 * Inputs: none
 * Output: std::vector<MapItem*> - stack of map item pointers
 */
std::vector<MapItem*> Tile::getItems() const
{
  return items;
}

/*
 * Description: Gets the lower sprite(s) and returns it(them), if set.
 *
 * Inputs: none
 * Output: std::vector<Sprite*> - the lower sprites, in a vector
 */
std::vector<Sprite*> Tile::getLower() const
{
  return lower;
}

/*
 * Description: Gets the passability for the entire lower portion of the tile.
 *              Only returns for set lower sprites. Otherwise, it returns
 *              true.
 *
 * Inputs: Direction dir - the direction to get
 * Output: bool - the lower passability directional status
 */
bool Tile::getLowerPassability(Direction dir) const
{
  bool passability = true;

  if(dir == Direction::DIRECTIONLESS)
  {
    passability = false;
  }
  else
  {
    for(uint8_t i = 0; i < lower_passability.size(); i++)
      passability &= getLowerPassability(i, dir);
  }

  return passability;
}

/*
 * Description: Gets the passability for the lower specific index of the stack.
 *              Only returns an actual value for set sprites. If it's
 *              not set or if it's out of range, it returns true.
 *
 * Inputs: int index - the lower sprite stack index
 *         Direction dir - the direction to get
 * Output: bool - the lower passability directional status
 */
bool Tile::getLowerPassability(uint8_t index, Direction dir) const
{
  if(index < lower_passability.size() && lower[index] != NULL)
  {
    if(dir == Direction::DIRECTIONLESS)
      return false;
    return ((lower_passability[index] & static_cast<uint8_t>(dir)) > 0);
  }
  return true;
}

/*
 * Description: Returns the max render level indicator. This allows for
 *              simplifying the render calculations.
 *
 * Inputs: none
 * Output: uint16_t - the render depth indicator
 */
uint16_t Tile::getMaxRenderLevel() const
{
  uint16_t depth = 0;

  /* Things v. Persons for depth */
  if(things.size() > persons_main.size())
    depth = things.size();
  else
    depth = persons_main.size();

  /* IOs vs current depth */
  if(ios.size() > depth)
    depth = ios.size();

  /* Empty check v. items */
  if(depth == 0 && items.size() > 0)
    depth = 1;

  return depth;
}

/*
 * Description: Gets if the tile is passable entering from the given direction.
 *              This does not take into account the status of the thing(s) and
 *              if it's passable.
 *
 * Inputs: Direction dir - the direction enumerated for passability
 * Output: bool - status if the tile passability is possible.
 */
bool Tile::getPassabilityEntering(Direction dir) const
{
  if(dir == Direction::NORTH)
    return getPassabilityExiting(Direction::SOUTH);
  else if(dir == Direction::EAST)
    return getPassabilityExiting(Direction::WEST);
  else if(dir == Direction::SOUTH)
    return getPassabilityExiting(Direction::NORTH);
  else if(dir == Direction::WEST)
    return getPassabilityExiting(Direction::EAST);
  return getPassabilityExiting(dir);
}

/*
 * Description: Gets if the tile is passable leaving the given direction.
 *              This does not take into account the status of the thing(s)
 *              and if it's passable.
 *
 * Inputs: Direction dir - the direction enumerated for passability
 * Output: bool - status if the tile passability is possible.
 */
bool Tile::getPassabilityExiting(Direction dir) const
{
  if(status != OFF)
  {
    if(dir == Direction::DIRECTIONLESS)
      return (getBasePassability(dir) || getLowerPassability(dir));
    return (getBasePassability(dir) && getLowerPassability(dir));
  }
  return false;
}

/*
 * Description: Returns the person stored on this tile at the indicated render
 *              level as a main or previous (first non-null one).
 *
 * Inputs: uint8_t render_level - integer of the render level on tile
 * Output: MapPerson* - person pointer on tile. NULL if none set
 */
MapPerson* Tile::getPerson(uint8_t render_level) const
{
  MapPerson* selected_person = getPersonMain(render_level);
  if(selected_person == NULL)
    selected_person = getPersonPrev(render_level);
  return selected_person;
}

/*
 * Description: Returns the person stored on this tile at the indicated render
 *              level as a main (moving to or not moving).
 *
 * Inputs: uint8_t render_level - integer of the render level on tile
 * Output: MapPerson* - person pointer on tile. NULL if none set
 */
MapPerson* Tile::getPersonMain(uint8_t render_level) const
{
  MapPerson* selected_person = NULL;

  if(persons_main.size() > render_level)
    selected_person = persons_main[render_level];

  return selected_person;
}

/*
 * Description: Returns the person stored on this tile at the indicated render
 *              level as a previous (moving from).
 *
 * Inputs: uint8_t render_level - integer of the render level on tile
 * Output: MapPerson* - person pointer on tile. NULL if none set
 */
MapPerson* Tile::getPersonPrev(uint8_t render_level) const
{
  MapPerson* selected_person = NULL;

  if(persons_prev.size() > render_level)
    selected_person = persons_prev[render_level];

  return selected_person;
}

/*
 * Description: Returns all persons stored on the tile. This is ordered based
 *              on render level (moving to or not moving). I.e., index 4 will
 *              correspond to render level 4. If the render level is not set in
 *              the vector, it is NULL and not used.
 *
 * Inputs: none
 * Output: std::vector<MapPerson*> - stack of main persons stored in tile
 */
std::vector<MapPerson*> Tile::getPersonsMain() const
{
  return persons_main;
}

/*
 * Description: Returns all persons stored on the tile. This is ordered based
 *              on render level (moving from). I.e., index 4 will correspond to
 *              render level 4. If the render level is not set in the vector,
 *              it is NULL and not used.
 *
 * Inputs: none
 * Output: std::vector<MapPerson*> - stack of previous persons stored in tile
 */
std::vector<MapPerson*> Tile::getPersonsPrev() const
{
  return persons_prev;
}

/*
 * Description: Returns the X coordinates of the top left of the tile, in
 *              pixels.
 *
 * Inputs: none
 * Output: int - the X coordinate, in pixels
 */
uint32_t Tile::getPixelX() const
{
  return (x * width);
}

/*
 * Description: Returns the Y coordinates of the top left of the tile, in
 *              pixels.
 *
 * Inputs: none
 * Output: int - the Y coordinate, in pixels
 */
uint32_t Tile::getPixelY() const
{
  return (y * height);
}

/*
 * Description: Gets the render things based on the render level. This will
 *              return a pointer for item, person, and thing, if applicable.
 *              Pointers that are NULL have nothing set to render on that
 *              level.
 *
 * Inputs: uint8_t render_level - the render depth indicator
 *         MapPerson*& person - ref of person pointer to place render object
 *         MapThing*& thing - ref of thing pointer to place render object
 *         MapInteractiveObject*& io - ref of IO pointer to place render obj
 * Output: status if the objects are valid for rendering
 */
bool Tile::getRenderThings(uint8_t render_level, MapPerson*& person,
                           MapThing*& thing, MapInteractiveObject*& io) const
{
  bool valid_pointer = false;

  if(status == ACTIVE && render_level < Helpers::getRenderDepth())
  {
    /* Only parse persons if there is valid area */
    if(persons_main.size() > render_level)
    {
      /* If main is not NULL, send it. Otherwise, send previous */
      if(persons_main[render_level] != NULL)
      {
        person = persons_main[render_level];
        valid_pointer = true;
      }
      else if(persons_prev[render_level] != NULL)
      {
        person = persons_prev[render_level];
        valid_pointer = true;
      }
    }

    /* Check for valid things */
    if(things.size() > render_level && things[render_level] != NULL)
    {
      thing = things[render_level];
      valid_pointer = true;
    }

    /* Check for valid IOs */
    if(ios.size() > render_level && ios[render_level] != NULL)
    {
      io = ios[render_level];
      valid_pointer = true;
    }
  }

  return valid_pointer;
}

/*
 * Description: Returns the sound ID of the first valid sprite with valid sound
 *              ID on the tile. If none, will return -1. The order is:
 *              LOWER5-LOWER1, BASE. Only checks the sprites and not the things.
 *
 * Inputs: none
 * Output: int32_t - the found sound ID
 */
int32_t Tile::getSoundID()
{
  int found_id = -1;

  /* Loop through lowers first */
  for(int i = lower.size() - 1; found_id < 0 && i >= 0; i--)
  {
    if(lower[i] != nullptr && lower[i]->getSize() > 0)
      found_id = lower[i]->getSoundID();
  }

  /* If still none found, proceed to check base */
  if(found_id < 0 && base != nullptr && base->getSize() > 0)
    found_id = base->getSoundID();

  return found_id;
}

/*
 * Description: Returns the status the tile is currently classified in. Uses
 *              the enum from Layer. See the corresponding setStatus()
 *
 * Inputs: none
 * Output: Status - public enum from layer identifying state
 */
Tile::TileStatus Tile::getStatus() const
{
  return status;
}

/*
 * Description: Returns the thing stored on this tile at the indicated render
 *              level.
 *
 * Inputs: uint8_t render_level - integer of the render level on tile
 * Output: MapThing* - thing reference stored. NULL if not set
 */
MapThing* Tile::getThing(uint8_t render_level) const
{
  MapThing* selected_thing = NULL;

  if(things.size() > render_level)
    selected_thing = things[render_level];

  return selected_thing;

}

/*
 * Description: Returns all things stored on the tile. This is ordered based
 *              on render level. I.e., index 4 will correspond to render level
 *              4. If the render level is not set in the vector, it is NULL
 *              and not used.
 *
 * Inputs: none
 * Output: std::vector<MapThing*> - stack of things stored in tile
 */
std::vector<MapThing*> Tile::getThings() const
{
  return things;
}

/*
 * Description: Gets the upper sprite(s) and returns it(them), if set.
 *
 * Inputs: none
 * Output: std::vector<Sprite*> - the upper sprites, in a QList
 */
std::vector<Sprite*> Tile::getUpper() const
{
  return upper;
}

/*
 * Description: Gets the width stored within the tile (and all the layers).
 *
 * Inputs: none
 * Output: uint16_t - the width in pixels
 */
uint16_t Tile::getWidth() const
{
  return width;
}

/*
 * Description: Gets the X coordinate stored within the tile. This is free of
 *              width and height changes and only is a reference.
 *
 * Inputs: none
 * Output: uint16_t - the x coordinate, in tile count
 */
uint16_t Tile::getX() const
{
  return x;
}

/*
 * Description: Gets the Y coordinate stored within the tile. This is free of
 *              width and height changes and only is a reference.
 *
 * Inputs: none
 * Output: uint16_t - the y coordinate, in tile count
 */
uint16_t Tile::getY() const
{
  return y;
}

/*
 * Description: Inserts the new Sprite frame data onto the created lower
 *              portion for this tile.
 *
 * Inputs: Sprite* lower - the lower tile data, as a Sprite set
 *         uint8_t index - the index to where the lower sprite is inserted
 * Output: bool - if the insert call succeeded and the data was used
 */
bool Tile::insertLower(Sprite* lower, uint8_t index)
{
  if(lower != NULL && index < kLOWER_COUNT_MAX)
  {
    /* Increase the size of lower if it isn't big enough */
    Sprite* null_sprite = NULL;
    if(index >= this->lower.size())
    {
      for(int i = this->lower.size(); i <= index; i++)
      {
        this->lower.push_back(null_sprite);
        lower_passability.push_back(
                                static_cast<uint8_t>(Direction::DIRECTIONLESS));
      }
    }

    /* Actually set the lower layer now */
    this->lower[index] = lower;
    lower_passability[index] = static_cast<uint8_t>(Direction::DIRECTIONLESS);

    return true;
  }

  return false;
}

/*
 * Description: Inserts the new Sprite frame data onto the created upper
 *              portion for this tile.
 *
 * Inputs: Sprite* upper - the upper tile data, as a Sprite set
 *         uint8_t index - the index to where the upper sprite is inserted
 * Output: bool - if the insert call succeeded and the data was used
 */
bool Tile::insertUpper(Sprite* upper, uint8_t index)
{
  if(upper != 0 && index < kUPPER_COUNT_MAX)
  {
    /* Increase the size of upper if it isn't big enough */
    Sprite* null_sprite = NULL;
    if(index >= this->upper.size())
      for(int i = this->upper.size(); i <= index; i++)
        this->upper.push_back(null_sprite);

    /* Actually set the upper layer now */
    this->upper[index] = upper;

    return true;
  }

  return false;
}

/*
 * Description: Returns if the Base Sprite is set
 *
 * Inputs: none
 * Output: bool - status if the base sprite is set
 */
bool Tile::isBaseSet() const
{
  return (base != NULL);
}

/*
 * Description: Returns if the Enhancer Sprite is set
 *
 * Inputs: none
 * Output: bool - status if the enhancer sprite is set
 */
bool Tile::isEnhancerSet() const
{
  return (enhancer != NULL);
}

/*
 * Description: Returns if there is a IO on the indicated render level.
 *
 * Inputs: uint8_t render_level - the level of rendering on tile.
 * Output: bool - true if the IO is set on that render level
 */
bool Tile::isIOSet(uint8_t render_level) const
{
  return (ios.size() > render_level && ios[render_level] != NULL);
}

/*
 * Description: Returns if at least one IO is on the tile.
 *
 * Inputs: none
 * Output: bool - true if there is at least one IO on the tile.
 */
bool Tile::isIOsSet() const
{
  for(uint16_t i = 0; i < ios.size(); i++)
    if(ios[i] != NULL)
      return true;
  return false;
}

/*
 * Description: Returns if the number of items on the stack is below the limit.
 *
 * Inputs: none
 * Output: bool - true if there is room for more map items on the tile
 */
bool Tile::isItemsAtLimit() const
{
  return (items.size() >= kMAX_ITEMS);
}

/*
 * Description: Returns if at least one item is set and stored in the thing.
 *
 * Inputs: none
 * Output: bool - true if there is at least one item
 */
bool Tile::isItemsSet() const
{
  return (items.size() > 0);
}

/*
 * Description: Returns if the Lower Sprite(s) is(are) set
 *
 * Inputs: none
 * Output: bool - status if there is at least one lower sprite
 */
bool Tile::isLowerSet() const
{
  return (lower.size() > 0);
}

/*
 * Description: Returns if the person at the indicated render level is set for
 *              the main (moving to or on the tile).
 *
 * Inputs: uint8_t render_level - the rendering level, based on the person frame
 * Output: bool - is this person main frame set?
 */
bool Tile::isPersonMain(uint8_t render_level) const
{
  return (persons_main.size() > render_level &&
          persons_main[render_level] != NULL);
}

/*
 * Description: Returns if the person at the indicated render level is set for
 *              the previous (moving from the tile).
 *
 * Inputs: uint8_t render_level - the rendering level, based on the person frame
 * Output: bool - is this person previous frame set?
 */
bool Tile::isPersonPrevious(uint8_t render_level) const
{
  return (persons_prev.size() > render_level &&
          persons_prev[render_level] != NULL);
}

/*
 * Description: Returns if there is a person on the indicated render level.
 *
 * Inputs: uint8_t render_level - the level of rendering on tile.
 * Output: bool - true if the person is set on that render level
 */
bool Tile::isPersonSet(uint8_t render_level) const
{
  return (isPersonMain(render_level) || isPersonPrevious(render_level));
}

/*
 * Description: Returns if at least one person is on the tile.
 *
 * Inputs: none
 * Output: bool - true if there is at least one person on the tile.
 */
bool Tile::isPersonsSet() const
{
  for(uint16_t i = 0; i < persons_main.size(); i++)
    if(persons_main[i] != NULL)
      return true;
  return false;
}

/*
 * Description: Returns if there is a thing on the indicated render level.
 *
 * Inputs: uint8_t render_level - the level of rendering on tile.
 * Output: bool - true if the thing is set on that render level
 */
bool Tile::isThingSet(uint8_t render_level) const
{
  return (things.size() > render_level && things[render_level] != NULL);
}

/*
 * Description: Returns if at least one thing is on the tile.
 *
 * Inputs: none
 * Output: bool - true if there is at least one thing on the tile.
 */
bool Tile::isThingsSet() const
{
  for(uint16_t i = 0; i < things.size(); i++)
    if(things[i] != NULL)
      return true;
  return false;
}

/*
 * Description: Returns if the Upper Sprite(s) is(are) set
 *
 * Inputs: none
 * Output: bool - status if there is at least one upper sprite
 */
bool Tile::isUpperSet() const
{
  return (upper.size() > 0);
}

/*
 * Description: Finishes the move process for the person. This triggers the
 *              previous pointer to be set back to NULL, as the person has left
 *              the tile.
 *
 * Inputs: uint8_t render_level - the rendering level of the thing
 *         bool no_events - should events not trigger on tile exit?
 *         bool reverse_last - reverse last person move start command
 * Output: bool - status if move finish was successful
 */
bool Tile::personMoveFinish(uint8_t render_level, bool no_events,
                            bool reverse_last)
{
  if(persons_prev.size() > render_level && persons_prev[render_level] != NULL)
  {
    /* If reverse last, put it back as main */
    if(reverse_last)
    {
      persons_main[render_level] = persons_prev[render_level];
    }
    else
    {
      /* Event is only applicable if it's a render level 0 on person */
      if(render_level == 0 && !no_events)
      {
        /* Execute exit event, if applicable */
        if(event_handler != NULL && !event_exit.isEmpty())
          event_handler->executeEventSet(getEventExit(),
                                         persons_prev[render_level]);
      }
    }

    persons_prev[render_level] = NULL;
    return true;
  }
  return false;
}

/*
 * Description: Initiates the move process for the person. This triggers the
 *              previous pointer being set to the main and the main back to
 *              NULL, which effectively means the person is leaving.
 *
 * Inputs: uint8_t render_level - the rendering level of the thing
 * Output: bool - status if set was successful
 */
bool Tile::personMoveStart(uint8_t render_level)
{
  if(persons_main.size() > render_level && persons_main[render_level] != NULL
                                        && persons_prev[render_level] == NULL)
  {
    persons_prev[render_level] = persons_main[render_level];
    persons_main[render_level] = NULL;
    return true;
  }
  return false;
}

/*
 * Description: Renders the lower sprites in the tile using SDL calls. The
 *              sprite initialization must have occurred before any rendering.
 *
 * Inputs: SDL_Renderer* renderer - the sdl graphical rendering context
 *         int offset_x - the offset in X off of base coordinates
 *         int offset_y - the offset in Y off of base coordinates
 * Output: bool - status if the sprite was painted.
 */
bool Tile::renderLower(SDL_Renderer* renderer, int offset_x, int offset_y)
{
  bool success = true;
  int pixel_x = getPixelX() - offset_x;
  int pixel_y = getPixelY() - offset_y;

  /* Only proceed if the status isn't off */
  if(status != OFF)
  {
    if(status == ACTIVE)
    {
      /* Paint the base first */
      if(base != NULL)
        success &= base->render(renderer, pixel_x, pixel_y, width, height);

      /* Then the enhancer sprite */
      if(enhancer != NULL)
        success &= enhancer->render(renderer, pixel_x, pixel_y, width, height);

      /* Then Paint the set of lower layers */
      for(uint8_t i = 0; i < lower.size(); i++)
        if(lower[i] != NULL)
          success &= lower[i]->render(renderer, pixel_x, pixel_y,
                                                width, height);
    }
    else if(status == BLANKED)
    {
      SDL_Rect tile_rect = {pixel_x, pixel_y, width, height};
      SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
      SDL_RenderFillRect(renderer, &tile_rect);
    }
  }

  return success;
}

/*
 * Description: Renders the upper sprites in the tile using SDL calls. The
 *              sprite initialization must have occurred before any rendering.
 *
 * Inputs: SDL_Renderer* renderer - the sdl graphical rendering context
 *         int offset_x - the offset in X off of base coordinates
 *         int offset_y - the offset in Y off of base coordinates
 * Output: bool - status if the sprite(s) were painted. If failed, make sure
 *         there is an image in the sprite.
 */
bool Tile::renderUpper(SDL_Renderer* renderer, int offset_x, int offset_y)
{
  bool success = true;

  /* Only paint if the status is enabled */
  if(status == ACTIVE)
  {
    int pixel_x = getPixelX() - offset_x;
    int pixel_y = getPixelY() - offset_y;

    /* Paint the upper set, if set */
    for(uint8_t i = 0; i < upper.size(); i++)
      if(upper[i] != NULL)
        success &= upper[i]->render(renderer, pixel_x, pixel_y,
                                              width, height);
  }
  return success;
}

/*
 * Description: Sets the base sprite stored within the tile. Only sets it
 *              if the pointer is valid and the number of frames is greater
 *              than 0. The old base is set if the new pointer is valid.
 *
 * Inputs: Sprite* base - the new base sprite to attempt to set
 * Output: bool - status if the set procedure succeeded
 */
bool Tile::setBase(Sprite* base)
{
  if(base != NULL)
  {
    unsetBase();
    this->base = base;
    setBasePassability(Direction::NORTH, false);
    setBasePassability(Direction::EAST, false);
    setBasePassability(Direction::SOUTH, false);
    setBasePassability(Direction::WEST, false);

    return true;
  }
  return false;
}

/*
 * Description: Sets the base passability using a given direction and the
 *              set boolean value. Only fails if the base internal to the
 *              tile is not set.
 *
 * Inputs: Direction dir - the direction to set
 *         bool set_value - the value to set it to.
 * Output: bool - the status if the set was successful.
 */
bool Tile::setBasePassability(Direction dir, bool set_value)
{
  /* Only set if the base is set */
  if(base != NULL)
  {
    if(dir == Direction::DIRECTIONLESS && set_value)
      base_passability = static_cast<uint8_t>(Direction::DIRECTIONLESS);
    else
      (set_value) ? (base_passability |= static_cast<uint8_t>(dir))
                  : (base_passability &= ~static_cast<uint8_t>(dir));

    return true;
  }
  return false;
}

/*
 * Description: Sets the enhancer sprite stored within the tile. Only sets it
 *              if the pointer is valid and the number of frames is greater
 *              than 0. The old enhancer is set if the new pointer is valid.
 *
 * Inputs: Sprite* enhancer - the new enhancer sprite to attempt to set
 * Output: bool - status if the insertion succeeded
 */
bool Tile::setEnhancer(Sprite* enhancer)
{
  if(enhancer != NULL)
  {
    unsetEnhancer();
    this->enhancer = enhancer;

    return true;
  }
  return false;
}

/*
 * Description: Sets the event handler to create and manage all existing events
 *              that get fired throughout interaction with the class. This is
 *              necessary to ensure that any events work.
 *
 * Inputs: EventHandler* event_handler - the new handler pointer (must not be 0)
 * Output: none
 */
void Tile::setEventHandler(EventHandler* event_handler)
{
  this->event_handler = event_handler;
}

/*
 * Description: Sets the height of the tile.
 *
 * Inputs: uint16_t height - the tile height, in pixels
 * Output: none
 */
void Tile::setHeight(uint16_t height)
{
  this->height = height;
}

/*
 * Description: Sets an IO on the tile with the designated render level.
 *              This does not take into account if the tile is passable and
 *              will set regardless. No events boolean allows for events to
 *              be used (or not). This will unset the IO if one already
 *              exists in the same place.
 *
 * Inputs: MapInteractiveObject* io - the IO to set on the tile
 *         uint8_t render_level - the render order for placing the IO
 * Output: bool - if the IO was successfully set.
 */
bool Tile::setIO(MapInteractiveObject* io, uint8_t render_level)
{
  if(io != NULL)
  {
    /* First, unset the existing IO */
    unsetIO(render_level);

    /* Set the new IO */
    if(growIOStack(render_level))
    {
      ios[render_level] = io;
      return true;
    }
  }

  return false;
}

/*
 * Description: Sets the lower sprite stored within the tile. Only sets it if
 *              the pointer is valid and the number of frames is greater than
 *              0. Since lower is a stack, it unsets the stack if the sprite
 *              is valid and puts this new lower at the front of the list.
 *
 * Inputs: Sprite* lower - the new lower sprite to attempt to set
 * Output: bool - status if the insertion succeeded
 */
bool Tile::setLower(Sprite* lower)
{
  if(lower != NULL)
  {
    Sprite* null_sprite = NULL;

    unsetLower();
    this->lower.push_back(null_sprite);
    this->lower[0] = lower;
    lower_passability[0] = static_cast<uint8_t>(Direction::DIRECTIONLESS);

    return true;
  }
  return false;
}

/*
 * Description: Sets the lower passability for a sprite based on an index,
 *              a direction enumerator, and the value it should be. Only
 *              fails if the index is out of range of the allowable bounds
 *              or the lower isn't set at that index.
 *
 * Inputs: uint8_t index - the lower layer index
 *         Direction dir - the direction passability to set
 *         bool set_value - the value to set that direction to
 * Output: bool - if the set was successful
 */
bool Tile::setLowerPassability(uint8_t index, Direction dir, bool set_value)
{
  /* Only set if the lower layer is valid and in the right range */
  if(index < kLOWER_COUNT_MAX && index < lower.size() && lower[index] != NULL)
  {
    if(dir == Direction::DIRECTIONLESS && set_value)
      lower_passability[index] = static_cast<uint8_t>(Direction::DIRECTIONLESS);
    else
      (set_value) ? (lower_passability[index] |= static_cast<uint8_t>(dir))
                  : (lower_passability[index] &= ~static_cast<uint8_t>(dir));

    return true;
  }
  return false;
}

/*
 * Description: Sets a person on the tile with the designated render level.
 *              This does not take into account if the tile is passable and
 *              will set regardless. No events boolean allows for events to
 *              be used (or not). This will unset the person if one already
 *              exists in the same place.
 *
 * Inputs: MapPerson* person - the person to set on the tile
 *         uint8_t render_level - the render order for placing the person
 *         bool no_events - if events should be triggered on set
 * Output: bool - if the person was successfully set.
 */
bool Tile::setPerson(MapPerson* person, uint8_t render_level, bool no_events)
{
  if(person != NULL)
  {
    /* First, unset the existing person */
    unsetPerson(render_level, no_events);

    /* Set the new person */
    if(growPersonStack(render_level))
    {
      persons_main[render_level] = person;

      /* Execute enter event, if applicable */
      if(!no_events && render_level == 0 && event_handler != NULL)
      {
        /* Pickup the item, if applicable */
        for(uint16_t i = 0; i < items.size(); i++)
          if(items[i] != NULL)
            event_handler->executePickup(items[i], true);

        /* Execute the enter event, if applicable */
        if(!event_enter.isEmpty())
          event_handler->executeEventSet(getEventEnter(), person);
      }

      return true;
    }
  }

  return false;
}

/*
 * Description: Sets the tile status. This allows of 3 possible states that
 *              the tile can be in. This affects the visibility and painting \
 *              of the tile.
 *
 * Inputs: TileStatus status - the new status to update the tile to
 * Output: none
 */
void Tile::setStatus(TileStatus status)
{
  this->status = status;
}

/*
 * Description: Sets a thing on the tile with the designated render level.
 *              This does not take into account if the tile is passable and
 *              will set regardless. No events boolean allows for events to
 *              be used (or not). This will unset the thing if one already
 *              exists in the same place.
 *
 * Inputs: MapThing* thing - the thing to set on the tile
 *         uint8_t render_level - the render order for placing the thing
 * Output: bool - if the thing was successfully set.
 */
bool Tile::setThing(MapThing* thing, uint8_t render_level)
{
  if(thing != NULL)
  {
    /* First, unset the existing thing */
    unsetThing(render_level);

    /* Set the new thing */
    if(growThingStack(render_level))
    {
      things[render_level] = thing;
      return true;
    }
  }

  return false;
}

/*
 * Description: Sets the upper sprite stored within the tile. Only sets it if
 *              the pointer is valid and the number of frames is greater than
 *              0. Since upper is a stack, it unsets the stack if the sprite
 *              is valid and puts this new upper at the front of the list.
 *
 * Inputs: Sprite* upper - the new upper layer to attempt to set
 * Output: bool - status if the insertion succeeded
 */
bool Tile::setUpper(Sprite* upper)
{
  if(upper != NULL)
  {
    unsetUpper();
    this->upper[0] = upper;

    return true;
  }
  return false;
}

/*
 * Description: Sets the width of the tile.
 *
 * Inputs: int width - the tile width, in pixels
 * Output: none
 */
void Tile::setWidth(uint16_t width)
{
  this->width = width;
}

/*
 * Description: Sets the X coordinate of the tile.
 *
 * Inputs: uint16_t x - the tile x coordinate, in tile count
 * Output: none
 */
void Tile::setX(uint16_t x)
{
  this->x = x;
}

/*
 * Description: Sets the Y coordinate of the tile.
 *
 * Inputs: uint16_t y - the tile y coordinate, in tile count
 * Output: none
 */
void Tile::setY(uint16_t y)
{
  this->y = y;
}

/*
 * Description: Updates the enter event, from the relevant file data. This then
 *              passes the call to the event handler which parses the file data.
 *
 * Inputs: XmlData data - the data calculated from the file
 *         int file_index - the offset index where the event is
 *         int section_index - the relevant map section index
 * Output: bool - returns if the call was successful
 */
bool Tile::updateEventEnter(XmlData data, int file_index,
                                          uint16_t section_index)
{
  /* Parse depending on the key value */
  if(data.getElement(file_index) == "tileevent")
  {
    if(data.getKeyValue(file_index) == "enter")
      return event_enter.loadData(data, file_index + 2, section_index);
    else if(data.getKeyValue(file_index) == "enterset")
      return event_enter.loadData(data, file_index + 3, section_index);
  }
  return false;
}

/*
 * Description: Updates the exit event, from the relevant file data. This then
 *              passes the call to the event handler which parses the file data.
 *
 * Inputs: XmlData data - the data calculated from the file
 *         int file_index - the offset index where the event is
 *         int section_index - the relevant map section index
 * Output: bool - returns if the call was successful
 */
bool Tile::updateEventExit(XmlData data, int file_index, uint16_t section_index)
{
    /* Parse depending on the key value */
  if(data.getElement(file_index) == "tileevent")
  {
    if(data.getKeyValue(file_index) == "exit")
      return event_exit.loadData(data, file_index + 2, section_index);
    else if(data.getKeyValue(file_index) == "exitset")
      return event_exit.loadData(data, file_index + 3, section_index);
  }
  return false;
}

/*
 * Description: Unsets the base sprite in the tile.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetBase()
{
  base = NULL;
  base_passability = static_cast<uint8_t>(Direction::DIRECTIONLESS);
}

/*
 * Description: Unsets the enhancer sprite in the tile.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetEnhancer()
{
  enhancer = NULL;
}

/*
 * Description: Unsets the IO that matches the indicated IO pointer from
 *              the stack. It will re-arrange to necessitate the removal.
 *
 * Inputs: MapInteractiveObject* io - the IO pointer to remove
 * Output: bool - true if the IO was removed.
 */
bool Tile::unsetIO(MapInteractiveObject* io)
{
  for(uint16_t i = 0; i < ios.size(); i++)
  {
    if(ios[i] == io)
    {
      ios[i] = NULL;
      return true;
    }
  }

  return false;
}

/*
 * Description: Removes the IO at the indicated render level.
 *
 * Inputs: uint8_t render_level - the render depth indicator
 *         bool no_events - if events should trigger upon removal
 * Output: bool - if an IO was removed
 */
bool Tile::unsetIO(uint8_t render_level)
{
  if(ios.size() > render_level)
  {
    if(ios[render_level] != NULL)
    {
      ios[render_level] = NULL;
      return true;
    }
  }

  return false;

}

/*
 * Description: Unsets all IOs from the tile.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetIOs()
{
  ios.clear();
}

/*
 * Description: Unsets the item that matches the indicated item pointer from
 *              the stack. It will re-arrange to necessitate the removal.
 *
 * Inputs: MapItem* item - the item pointer to remove
 * Output: bool - true if the item was removed.
 */
bool Tile::unsetItem(MapItem* item)
{
  for(uint16_t i = 0; i < items.size(); i++)
  {
    if(items[i] == item)
    {
      items[i] = nullptr;
      items.erase(items.begin()+i);
      return true;
    }
  }

  return false;
}

/*
 * Description: Unsets all items from the tile.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetItems()
{
  items.clear();
}

/*
 * Description: Unsets the lower sprite(s) in the tile.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetLower()
{
  /* Disconnect all references */
  for(uint8_t i = 0; i < lower.size(); i++)
  {
    lower[i] = NULL;
    lower_passability[i] = static_cast<uint8_t>(Direction::DIRECTIONLESS);
  }

  /* Clean out the array */
  lower.clear();
  lower_passability.clear();
}

/*
 * Description: Unsets the lower sprite in the tile based on the index, only
 *              if it exists.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: uint8_t index - the index of the lower to unset
 * Output: bool - status if the unset occurred
 */
bool Tile::unsetLower(uint8_t index)
{
  if(index < lower.size())
  {
    lower[index] = NULL;
    lower_passability[index] = static_cast<uint8_t>(Direction::DIRECTIONLESS);
    return true;
  }
  return false;
}

/*
 * Description: Unsets the person that matches the indicated person pointer from
 *              the stack. It will re-arrange to necessitate the removal.
 *
 * Inputs: MapPerson* person - the person pointer to remove
 *         bool no_events - if events should be triggered on removal
 * Output: bool - true if the person was removed.
 */
bool Tile::unsetPerson(MapPerson* person, bool no_events)
{
  for(uint16_t i = 0; i < persons_main.size(); i++)
    if(persons_main[i] == person)
      return unsetPerson(i, no_events);
  return false;
}

/*
 * Description: Removes the person at the indicated render level. This allows
 *              for choosing if events will trigger upon removal.
 *
 * Inputs: uint8_t render_level - the render depth indicator
 *         bool no_events - if events should trigger upon removal
 * Output: bool - if a person was removed
 */
bool Tile::unsetPerson(uint8_t render_level, bool no_events)
{
  if(persons_main.size() > render_level)
  {
    /* Finish the move, if applicable */
    if(!personMoveFinish(render_level, no_events) &&
       persons_main[render_level] != nullptr)
    {
      /* Event is only applicable if it's a render level 0 on person */
      if(render_level == 0 && !no_events)
      {
        /* Execute exit event, if applicable */
        if(event_handler != nullptr && !event_exit.isEmpty())
          event_handler->executeEventSet(getEventExit(),
                                         persons_main[render_level]);
      }
    }

    persons_main[render_level] = nullptr;
    persons_prev[render_level] = nullptr;
    return true;
  }

  return false;
}

/*
 * Description: Unsets all persons from the tile. Allows for disabling or
 *              enabling events upon removal.
 *
 * Inputs: bool no_events - if events should be triggered upon removal
 * Output: none
 */
void Tile::unsetPersons(bool no_events)
{
  uint8_t event_render_level = 0;

  /* Event is only applicable if it's a render level 0 on person */
  if(persons_main.size() > event_render_level)
    unsetPerson(event_render_level, no_events);

  persons_main.clear();
  persons_prev.clear();
}

/*
 * Description: Unsets the thing that matches the indicated thing pointer from
 *              the stack. It will re-arrange to necessitate the removal.
 *
 * Inputs: MapThing* thing - the thing pointer to remove
 * Output: bool - true if the thing was removed.
 */
bool Tile::unsetThing(MapThing* thing)
{
  for(uint16_t i = 0; i < things.size(); i++)
  {
    if(things[i] == thing)
    {
      things[i] = NULL;
      return true;
    }
  }

  return false;
}

/*
 * Description: Removes the thing at the indicated render level.
 *
 * Inputs: uint8_t render_level - the render depth indicator
 *         bool no_events - if events should trigger upon removal
 * Output: bool - if a thing was removed
 */
bool Tile::unsetThing(uint8_t render_level)
{
  if(things.size() > render_level)
  {
    if(things[render_level] != NULL)
    {
      things[render_level] = NULL;
      return true;
    }
  }

  return false;

}

/*
 * Description: Unsets all things from the tile.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetThings()
{
  things.clear();
}

/*
 * Description: Unsets the upper sprite(s) in the tile.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: none
 * Output: none
 */
void Tile::unsetUpper()
{
  /* Disconnect all references */
  for(uint8_t i = 0; i < upper.size(); i++)
    upper[i] = NULL;

  /* Clear out the array */
  upper.clear();
}

/*
 * Description: Unsets the upper sprite in the tile based on the index, only
 *              if it exists.
 * Note: this class does NOT delete the pointer, only releases it.
 *
 * Inputs: uint8_t index - the index of the upper to unset
 * Output: bool - status if the unset occurred
 */
bool Tile::unsetUpper(uint8_t index)
{
  if(index < upper.size())
  {
    upper[index] = NULL;
    return true;
  }
  return false;
}
