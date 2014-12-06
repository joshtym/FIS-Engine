/******************************************************************************
 * Class Name: Map
 * Date Created: Dec 2 2012
 * Inheritance: none
 * Description: The map class, this is the top level with regard to an actual
 *              in-game map. This contains all the tiles that a map is composed
 *              of, it also holds pointers to all of the NPC's contained in the
 *              maps tiles. This also handles the viewport for the map.
 *
 * TODO: 
 *  1. If the tile image does not exist, disable the painting sequence
 *    - There seems to be a small glitch in this where it paints a white tile
 *  2. If mode is switched, end all notification animations and such. -> battle
 *  3. Animation event for teleport requires fade out -> teleport -> fade in
 *  4. If tile set fails, don't quit the map. Useful only multi tile setups.
 *     Maybe only pass if it is a multi tile setup ??
 *
 * Extra:
 *  1. As the player is teleported to a new map, maybe have the option to have
 *     it at a tile and then walking in. More true animation. How to?
 ******************************************************************************/
#include "Game/Map/Map.h"

/* Constant Implementation - see header file for descriptions */
// const int Map::kDOUBLE_DIGITS = 10;
// const int Map::kELEMENT_DATA = 0;
const uint8_t Map::kFILE_CLASSIFIER = 3;
const uint8_t Map::kFILE_GAME_TYPE = 1;
const uint8_t Map::kFILE_SECTION_ID = 2;
const uint8_t Map::kFILE_TILE_COLUMN = 5;
const uint8_t Map::kFILE_TILE_ROW = 4;
const uint8_t Map::kPLAYER_ID = 0;
const uint16_t Map::kZOOM_TILE_SIZE = 16;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* Constructor function */
Map::Map(Options* running_config, EventHandler* event_handler)
{
  /* Set initial variables */
  base_path = "";
  this->event_handler = event_handler;
  loaded = false;
  map_encrypted = false;
  map_index = 0;
  map_path = "";
  player = NULL;
  running = false;
  system_options = NULL;
  
  /* Configure the width / height of tiles and sets default zooming */
  tile_height = Helpers::getTileSize();
  tile_width = tile_height;
  zoom_in = false;
  zoom_out = false;
  
  /* Set options configuration */
  viewport.setTileSize(tile_width, tile_height);
  setConfiguration(running_config);

  // /* Configure the scene */
  // items.clear();
  // loaded = false;
  // map_index = 0;
  // persons.clear();
  // player = 0;
  // things.clear();
  
  // /* Configure the FPS animation and time elapsed, and reset to 0 */
  // paint_animation = 0;
  // paint_frames = 0;
  // paint_time = 0;
  // time_elapsed = 0;

  /* Set up the map displays */
  item_menu.setEventHandler(event_handler);
  map_dialog.setEventHandler(event_handler);
}

/* Destructor function */
Map::~Map()
{
  unloadMap();
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

bool Map::addSpriteData(XmlData data, std::string id, 
                        int file_index, SDL_Renderer* renderer)
{
  uint16_t access_id = 0;
  Sprite* access_sprite = NULL;
  uint16_t copy_id = 0;
  Sprite* copy_sprite = NULL;
  
  /* Get the ID information */
  std::vector<std::string> id_set = Helpers::split(id, '>');
  if(id_set.size() == 1)
    access_id = std::stoi(id);
  else if(id_set.size() == 2)
  {
    access_id = std::stoi(id_set.back());
    copy_id = std::stoi(id_set.front());
  }

  /* Only move forward if the access id is valid */
  if(access_id > 0)
  {
    /* Search for the sprite in the existing stack */
    for(uint16_t i = 0; i < tile_sprites.size(); i++)
    {
      if(tile_sprites[i]->getId() == access_id)
        access_sprite = tile_sprites[i];
      else if(tile_sprites[i]->getId() == copy_id)
        copy_sprite = tile_sprites[i];
    }

    /* If the sprite doesn't exist, create it */
    if(access_sprite == NULL)
    {
      access_sprite = new Sprite();
      access_sprite->setId(access_id);
      tile_sprites.push_back(access_sprite);
      
      /* If the copy sprite isn't null, copy the data into the new sprite */
      if(copy_sprite != NULL)
        *access_sprite = *copy_sprite;
    }

    return access_sprite->addFileInformation(data, file_index, 
                                             renderer, base_path);
  }
  
  return false;
}

bool Map::addTileData(XmlData data, uint16_t section_index)
{
  std::string classifier = data.getElement(kFILE_CLASSIFIER);
  std::string element = data.getAllElements().back();
  bool success = true;
  
  /* Split based on the element information if it's for a path */
  if(element == "sprite_id" || element == "sprite_matrix")
  {
    std::vector< std::vector<uint16_t> > id_matrix;
    if(element == "sprite_id")
      id_matrix = splitIdString(data.getDataString());
    else
      id_matrix = splitIdString(data.getDataString(), true);
    
    /* Go through all the available IDs, in the matrix */
    for(uint16_t i = 0; i < id_matrix.size(); i++)
    {
      for(uint16_t j = 0; j < id_matrix[i].size(); j++)
      {
        Sprite* found_sprite = NULL;
        
        /* Try and find the id */
        if(id_matrix[i][j] != 0)
        {
          for(uint32_t k = 0; k < tile_sprites.size(); k++)
          {
            if(tile_sprites[k]->getId() == id_matrix[i][j])
              found_sprite = tile_sprites[k];
          }
        }
        
        /* If the sprite is found, add it to the tile */
        // TODO: Review revising to incorporate new helper function.
        if(found_sprite != NULL)
        {
          std::vector<std::string> col_list =
                       Helpers::split(data.getKeyValue(kFILE_TILE_COLUMN), ',');
          uint16_t index = 0;
          std::vector<std::string> row_list = 
                          Helpers::split(data.getKeyValue(kFILE_TILE_ROW), ',');
          
          /* Loop through all the column and rows of the indexes */
          while(index < row_list.size() && index < col_list.size())
          {
            uint16_t col_end = i;
            uint16_t col_start = i;
            uint16_t row_end = j;
            uint16_t row_start = j;
            parseCoordinateInfo(row_list[index], col_list[index], section_index,
                                &row_start, &row_end, &col_start, &col_end);
            
            /* Add the sprite to all the applicable tiles */
            for(int r = row_start; r < row_end; r++)
              for(int c = col_start; c < col_end; c++)
                success &= geography[section_index][r][c]->
                                  addSprite(found_sprite, 
                                            data.getElement(kFILE_CLASSIFIER), 
                                            data.getKeyValue(kFILE_CLASSIFIER));
            index++;
          }
        }
      }
    }

    return success;
  }
  /* Otherwise, access the passability information for the tile */
  else if(element == "passability" || 
          classifier == "tileevent")
  {
    std::vector<std::string> col_list =
                       Helpers::split(data.getKeyValue(kFILE_TILE_COLUMN), ',');
    uint16_t index = 0;
    std::vector<std::string> row_list = 
                          Helpers::split(data.getKeyValue(kFILE_TILE_ROW), ',');
    
    /* Loop through all the column and rows of the indexes */
    while(index < row_list.size() && index < col_list.size())
    {
      uint16_t col_end = 0;
      uint16_t col_start = 0;
      uint16_t row_end = 0;
      uint16_t row_start = 0;
      parseCoordinateInfo(row_list[index], col_list[index], section_index,
                          &row_start, &row_end, &col_start, &col_end);
      
      /* Add the sprite to all the applicable tiles */
      for(int r = row_start; r < row_end; r++)
      {
        for(int c = col_start; c < col_end; c++)
        {
          /* Set the passability */
          if(element == "passability")
          {
            success &= geography[section_index][r][c]->
                    addPassability(data.getDataString(), 
                                   data.getElement(kFILE_CLASSIFIER), 
                                   data.getKeyValue(kFILE_CLASSIFIER));
          }
          /* Otherwise, it's a tile event */
          else
          {
            /* Classify between enter and exit events */
            if(data.getKeyValue(kFILE_CLASSIFIER) == "enter")
              success &= geography[section_index][r][c]->
                    updateEnterEvent(data, kFILE_CLASSIFIER + 3, section_index);
            else if(data.getKeyValue(kFILE_CLASSIFIER) == "exit")
              success &= geography[section_index][r][c]->
                     updateExitEvent(data, kFILE_CLASSIFIER + 3, section_index);
          }
        }
      }
      index++;
    }
    return success;
  }
  return false;
}

bool Map::addThingData(XmlData data, uint16_t section_index, 
                                     SDL_Renderer* renderer)
{
  std::string identifier = data.getElement(kFILE_CLASSIFIER);
  uint16_t id = std::stoul(data.getKeyValue(kFILE_CLASSIFIER));
  uint16_t index = 0;
  MapThing* modified_thing = NULL;

  /* Identify which thing to be created */
  if(identifier == "mapthing" || identifier == "mapio")
  {
    /* Search for the existing map object */
    while(modified_thing == NULL && index < things.size())
    {
      if(things[index]->getID() == id)
        modified_thing = things[index];
      index++;
    }

    /* Create a new thing if one does not exist */
    if(modified_thing == NULL)
    {
      if(identifier == "mapthing")
        modified_thing = new MapThing();
      else
        modified_thing = new MapInteractiveObject();
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);

      /* Append the new one */
      things.push_back(modified_thing);
    }
  }
  else if(identifier == "mapperson" || identifier == "mapnpc")
  {
    /* Search for the existing map object */
    while(modified_thing == NULL && index < persons.size())
    {
      if(persons[index]->getID() == id)
        modified_thing = persons[index];
      index++;
    }

    /* Create a new person if one does not exist */
    if(modified_thing == NULL)
    {
      if(identifier == "mapperson")
        modified_thing = new MapPerson();
      else
        modified_thing = new MapNPC();
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);
      
      /* If the ID is the player ID, tie to the player */
      if(id == kPLAYER_ID)
        player = static_cast<MapPerson*>(modified_thing);
      
      /* Append the new one */
      persons.push_back(static_cast<MapPerson*>(modified_thing));
    }
  }
  else if(identifier == "mapitem")
  {
    /* Search for the existing map object */
    while(modified_thing == NULL && index < items.size())
    {
      if(items[index]->getID() == id)
        modified_thing = items[index];
      index++;
    }
    
    /* If no item found, create one */
    if(modified_thing == NULL)
    {
      /* Create the new thing */
      modified_thing = new MapItem();
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);
      
      /* Append the new one */
      items.push_back(static_cast<MapItem*>(modified_thing));
    }
  }

  /* Proceed to update the thing information from the XML data */
  if(modified_thing != NULL)
    return modified_thing->addThingInformation(data, kFILE_CLASSIFIER + 1, 
                                               section_index, renderer, 
                                               base_path);

  return false;
}

std::vector<MapThing*> Map::getThingData(std::vector<int> thing_ids)
{
  std::vector<MapThing*> used_things;

  /* Loop through all the ids to find the associated things */
  for(auto i = thing_ids.begin(); i != thing_ids.end(); i++)
  {
    /* Only continue if the ID is valid and >= 0 */
    if(*i >= 0)
    {
      bool found = false;

      /* Loop through things */
      for(auto j = things.begin(); j != things.end(); j++)
      {
        if(!found && *i == (*j)->getID())
        {
          used_things.push_back(*j);
          found = true;
        }
      }

      /* Loop through persons */
      if(!found)
      {
        for(auto j = persons.begin(); j != persons.end(); j++)
        {
          if(!found && *i == (*j)->getID())
          {
            used_things.push_back(static_cast<MapThing*>(*j));
            found = true;
          }
        }
      }

      /* Loop through items */
      if(!found)
      {
        for(auto j = items.begin(); j != items.end(); j++)
        {
          if(!found && *i == (*j)->getID())
          {
            used_things.push_back(static_cast<MapThing*>(*j));
            found = true;
          }
        }
      }
    }
  }

  return used_things;
}
  
/* Returns a matrix of tiles that match the frames in the thing */
// TODO: Comment
std::vector<std::vector<Tile*>> Map::getTileMatrix(MapThing* thing, 
                                                   Direction direction)
{
  std::vector<std::vector<Tile*>> tile_set;

  if(thing != NULL)
  {
    SDL_Rect render_box = thing->getBoundingBox();

    /* Check the direction to modify the range if needed */
    if(direction == Direction::NORTH)
      render_box.y--;
    else if(direction == Direction::EAST)
      render_box.x++;
    else if(direction == Direction::SOUTH)
      render_box.y++;
    else if(direction == Direction::WEST)
      render_box.x--;

    return getTileMatrix(thing->getMapSection(), render_box.x, render_box.y, 
                         render_box.w, render_box.h);
  }

  return tile_set;
}

// TODO: Comment
std::vector<std::vector<Tile*>> Map::getTileMatrix(uint16_t section, 
                                                   uint16_t x, uint16_t y, 
                                                   uint16_t width, 
                                                   uint16_t height)
{
  std::vector<std::vector<Tile*>> tile_set;
  uint16_t range_x = x + width;
  uint16_t range_y = y + height;

  /* Confirm range is within valid parameters of map */
  if(geography.size() > section && geography[section].size() > range_x && 
     geography[section][range_x].size() > range_y)
  {
    /* Load the tiles that correspond to the thing */
    for(uint16_t i = x; i < range_x; i++)
    {
      std::vector<Tile*> tile_col;
          
      for(uint16_t j = y; j < range_y; j++)
        tile_col.push_back(geography[section][i][j]);
      tile_set.push_back(tile_col);
    }
  }

  return tile_set;
}

bool Map::initiateMapSection(uint16_t section_index, int width, int height)
{
  /* Check if the initiation will be viable */
  if(width > 0 && height > 0 && (geography.size() <= section_index || 
                                 geography[section_index].size() == 0))
  {
    /* Get the geography subset filled up past the point of being set */
    while(geography.size() <= section_index)
    {
      std::vector< std::vector<Tile*> > map_section;
      geography.push_back(map_section);
    }

    /* Fill the section */
    for(int i = 0; i < width; i++)
    {
      std::vector<Tile*> col;
      
      for(int j = 0; j < height; j++)
      {
        Tile* t = new Tile(event_handler, tile_width, tile_height, i, j);
        col.push_back(t);
      }
      
      geography[section_index].push_back(col);
    }

    return true;    
  }
  
  return false;
}

/* Initiates a thing action, based on the action key being hit */
// TODO: Fix for generic things
void Map::initiateThingInteraction(MapPerson* initiator)
{
  // FIND EACH AT 0. IF 0, check direction facing and tile adjacent for
  // if it's 0. If not, proceed. If found, interact

  if(initiator != NULL)
  {
    std::vector<std::vector<Tile*>> thing_tiles = initiator->getTileRender(0);
    Direction direction = initiator->getDirection();
    bool finished = false;
    bool invalid = false;
    int16_t starting_x = initiator->getTileX();
    int16_t starting_y = initiator->getTileY();

    /* Things to look for */
    std::vector<MapItem*> items_found;
    MapPerson* person_found = NULL;
    MapThing* thing_found = NULL;

    /* Check direction for tile that needs to be determined */
    if(direction == Direction::NORTH)
      starting_y--;
    else if(direction == Direction::EAST)
      starting_x++;
    else if(direction == Direction::SOUTH)
      starting_y++;
    else if(direction == Direction::WEST)
      starting_x--;
    else
      invalid = true;

    if(!invalid)
    {
      /* Loop through the tile set, looking for valid tiles on the render lvl */
      for(uint16_t i = 0; !finished && i < thing_tiles.size(); i++)
      {
        for(uint16_t j = 0; !finished && j < thing_tiles[i].size(); j++)
        {
          /* If thing is not NULL, this is rendering depth 0 tile to check */
          if(thing_tiles[i][j] != NULL && (starting_x + i) >= 0 
                                       && (starting_y + j) >= 0)
          {
            /* Get the x and y of tile to check and confirm validity */
            uint16_t x = starting_x + i;
            uint16_t y = starting_y + j;

            if(x < geography[map_index].size() && 
               y < geography[map_index][x].size())
            {
              /* Check for person */
              person_found = geography[map_index][x][y]->getPerson(0);
              if(person_found != NULL)
              {
                if(geography[map_index][x][y]->isPersonMain(0))
                  finished = true;
                else
                  person_found = NULL;
              }

              /* Check for thing */
              if(!finished && thing_found == NULL)
                thing_found = geography[map_index][x][y]->getThing(0);

              /* Check for item(s) */
              if(!finished && items_found.size() == 0)
                items_found = thing_tiles[i][j]->getItems();
            }
          }
        }
      }

      /* Interact with the object found (in the order listed below) */
      if(person_found != NULL)
      {
        person_found->interact(initiator);
      }
      else if(thing_found != NULL)
      {
        thing_found->interact(initiator);
      }
      else if(items_found.size() > 0)
      {
        for(uint16_t i = 0; i < items_found.size(); i++)
          event_handler->executePickup(items_found[i]);
      }
    }
  }
}

/* Parse coordinate info from file to give the designated tile coordinates
 * to update */
bool Map::parseCoordinateInfo(std::string row, std::string col, uint16_t index, 
                              uint16_t* r_start, uint16_t* r_end, 
                              uint16_t* c_start, uint16_t* c_end)
{
  std::vector<std::string> row_list = Helpers::split(row, '-');
  std::vector<std::string> col_list = Helpers::split(col, '-');
  if(row_list.size() > 0 && col_list.size() > 0)
  {
    /* Determine the row of parsing - limit to map size */
    *r_start += std::stoul(row_list.front());
    *r_end += std::stoul(row_list.back()) + 1;
    if(*r_start > geography[index].size())
      *r_start = geography[index].size();
    if(*r_end > geography[index].size())
      *r_end = geography[index].size();
    
    /* Determine the column of parsing - limit to map size */
    *c_start += std::stoul(col_list.front());
    *c_end += std::stoul(col_list.back()) + 1;
    if(*c_start > geography[index][*r_start].size())
      *c_start = geography[index][*r_start].size();
    if(*c_end > geography[index][*r_start].size())
      *c_end = geography[index][*r_start].size();
      
    return true;
  }
  
  return false;
}

/* Changes the map section index - what is displayed */
bool Map::setSectionIndex(uint16_t index)
{
  if(index < geography.size() && geography[index].size() > 0)
  {
    map_index = index;
    viewport.setMapSize(geography[index].size(), geography[index][0].size());
    
    return true;
  }
  
  return false;
}

/* Splits the ID into a vector of IDs */
std::vector< std::vector<uint16_t> > Map::splitIdString(std::string id, 
                                                        bool matrix)
{
  std::vector< std::vector<uint16_t> > id_stack;
  std::vector<std::string> xy_split = Helpers::split(id, ',');

  /* Only proceed if the string isn't empty and a split occurred */
  if(xy_split.size() > 0)
  {
    /* If it is a non-matrix split, handle accordingly */
    if(!matrix)
    {
      uint16_t id_max = 0;
      uint16_t id_min = 0;
      uint16_t multiplier_max = 0;
      uint16_t multiplier_min = 0;
    
      /* Work with the first element to determine the first and last row id */
      std::vector<std::string> x_split = Helpers::split(xy_split.front(), '-');
      if(x_split.size() > 0)
      {
        id_min = std::stoul(x_split.front());
        id_max = std::stoul(x_split.back());
      }
      
      /* Check if there is a multiplier element to work with */
      if(xy_split.size() > 1)
      {
        std::vector<std::string> y_split = Helpers::split(xy_split.back(), '-');
        if(y_split.size() > 0)
        {
          multiplier_min = std::stoul(y_split.front());
          multiplier_max = std::stoul(y_split.back());
        }
      }
      
      /* Loop through multipliers and id min and max to construct ID matrix */
      for(uint16_t mult = multiplier_min; mult <= multiplier_max; mult++)
      {
        uint16_t addition = (id_max - id_min + 1) * mult;
        std::vector<uint16_t> id_row;
        
        for(uint16_t id_x = id_min; id_x <= id_max; id_x++)
          id_row.push_back(id_x + addition);
        id_stack.push_back(id_row);
      }
    }
    /* Otherwise, it's an explicitely defined matrix */
    else
    {
      for(uint16_t i = 0; i < xy_split.size(); i++)
      {
        std::vector<uint16_t> id_row;
        
        std::vector<std::string> row_split = Helpers::split(xy_split[i], '.');
        for(uint16_t j = 0; j < row_split.size(); j++)
          id_row.push_back(std::stoul(row_split[j]));
          
        id_stack.push_back(id_row);
      }
    }
  }
  
  return id_stack;
}

/* Updates the height and width, based on zoom factors */
void Map::updateTileSize()
{
  bool updated = false;
  
  /* Try and zoom out the map */
  if(zoom_out)
  {
    /* Modify the tile height and width, limited by the constants */
    tile_height--;
    tile_width--;
    if(tile_height < kZOOM_TILE_SIZE || tile_width < kZOOM_TILE_SIZE)
    {
      tile_height = kZOOM_TILE_SIZE;
      tile_width = kZOOM_TILE_SIZE;
      zoom_out = false;
    }
    
    updated = true;
  }
  /* Otherwise, try and zoom back in */
  else if(zoom_in)
  {
    /* Modify the tile height and width, limited by the constants */
    tile_height++;
    tile_width++;
    if(tile_height > Helpers::getTileSize() || 
       tile_width > Helpers::getTileSize())
    {
      tile_height = Helpers::getTileSize();
      tile_width = Helpers::getTileSize();
      zoom_in = false;
    }
    
    updated = true;
  }
  
  /* If updated, update the height and width everywhere */
  if(updated)
  {
    /* Update map tiles */
    for(uint16_t i = 0; i < geography.size(); i++)
      for(uint16_t j = 0; j < geography[i].size(); j++)
        for(uint16_t k = 0; k < geography[i][j].size(); k++)
        {
          geography[i][j][k]->setHeight(tile_height);
          geography[i][j][k]->setWidth(tile_width);
        }
        
    /* Update viewport */
    viewport.setTileSize(tile_width, tile_height);
  }
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

// MapPerson* Map::getPlayer()
// {
  // return player;
// }

// /* Returns the map viewport, for scrolling through the scene */
// MapViewport* Map::getViewport()
// {
  // return viewport;
// }

// void Map::initialization()
// {
  // player->setFocus();
// }

bool Map::initConversation(Conversation* convo, MapThing* source)
{
  if(player != NULL && player->getTarget() == NULL 
                    && map_dialog.initConversation(convo, player))
  {
    /* Finalize conversation setup */
    std::vector<int> list = map_dialog.getConversationIDs();
    map_dialog.setConversationThings(getThingData(list));
    
    /* Flush player keys */
    player->keyFlush();

    /* Set the targets */
    player->setTarget(source);
    if(source != NULL)
      source->setTarget(player);

    return true;
  }

  return false;
}

bool Map::initNotification(std::string notification)
{
  return map_dialog.initNotification(notification);
}

bool Map::initNotification(Frame* image, int count)
{
  return map_dialog.initPickup(image, count);
}

/* Initializes item store display, within the map */
bool Map::initStore(ItemStore::StoreMode mode, std::vector<Item*> items,
                    std::vector<uint32_t> counts,
                    std::vector<int32_t> cost_modifiers,
                    std::string name, bool show_empty)
{
  (void)show_empty;

  //TODO: Fix
  bool status = item_menu.initDisplay(mode, items, counts, 
                                      cost_modifiers, name);//, show_empty);
  
  /* If successful, flush player keys */
  if(status)
    player->keyFlush();
  
  return true;
}

// /* Checks whether the viewport contains any tiles with the given sector */
// bool Map::isInSector(int index)
// {
  // (void)index;//warning
  // return true;
// }

bool Map::isLoaded()
{
  return loaded;
}

/* The key up and down events to be handled by the class */
bool Map::keyDownEvent(SDL_KeyboardEvent event)
{
  if(event.keysym.sym == SDLK_1)
  {
    if(geography.size() > 0)
    {
      player->keyFlush();
      map_index = 0;
      viewport.setMapSize(geography[map_index].size(), 
                          geography[map_index][0].size());
      player = persons.front();
    }
  }
  else if(event.keysym.sym == SDLK_2)
  {
    if(geography.size() > 1)
    {
      player->keyFlush();
      map_index = 1;
      viewport.setMapSize(geography[map_index].size(), 
                          geography[map_index][0].size());
      for(uint16_t i = 0; i < persons.size(); i++)
        if(persons[i]->getID() == 24)
          player = persons[i];
    }
  }
  else if(event.keysym.sym == SDLK_g)
  {
    bool enable = !tile_sprites[0]->isGreyScale();
    for(auto i = tile_sprites.begin(); i != tile_sprites.end(); i++)
      (*i)->useGreyScale(enable);
  }
  /* Exit the map, map has finished processing */
  else if(event.keysym.sym == SDLK_ESCAPE)
  {
    if(item_menu.isActive())
      item_menu.close();
    else
    {
      unfocus();
      return true;
    }
  }
  else if(item_menu.isActive())
    item_menu.keyDownEvent(event);
  else if(event.keysym.sym == SDLK_p)
    map_dialog.setPaused(!map_dialog.isPaused());
  else if(event.keysym.sym == SDLK_r)
    player->resetPosition();
  else if(event.keysym.sym == SDLK_6) // TODO: SEG
    map_dialog.initPickup(items[1]->getDialogImage(), 15, 2500);
  else if(event.keysym.sym == SDLK_7) // TODO: SEG
    map_dialog.initPickup(items.front()->getDialogImage(), 5);
  else if(event.keysym.sym == SDLK_8)
    map_dialog.initNotification("Hello sunshine, what a glorious day and I'll keep writing forever and forever and forever and forever and forever and forever and forFU.", true, 0);
  else if(event.keysym.sym == SDLK_9)
    map_dialog.initNotification("Hello sunshine, what a glorious day and I'll keep writing forever and forever and forever and forever and forever and forever and forFU.");
  else if(event.keysym.sym == SDLK_0)
  {
    Event blank_event = event_handler->createBlankEvent();
    
    Conversation* convo = new Conversation;
    convo->category = DialogCategory::TEXT;
    convo->action_event = blank_event;
    convo->text = "This is the initial conversation point that will start it. ";
    convo->text += "How can this continue? It must pursue to complete ";
    convo->text += "embodiment. Ok, maybe I'll just keep typing until I break ";
    convo->text += "the entire compiler.";
    convo->thing_id = 0;
    Conversation convo2;
    convo2.category = DialogCategory::TEXT;
    convo2.action_event = blank_event;
    convo2.text = "Pass the chips please.";
    convo2.thing_id = 24;
    Conversation test1, test2, test3, test4, test5;
    test1.category = DialogCategory::TEXT;
    test1.action_event = blank_event;
    test1.text = "This is a test to see how data runs. The line will split ";
    test1.text += "once unless it is an option based selection in which case ";
    test1.text += "it will restrict."; 
    test1.thing_id = 3;
    test2.category = DialogCategory::TEXT;
    test2.action_event = blank_event;
    test2.text = "This is a no man case. See what happens!! Ok, this is the ";
    test2.text += "too long case where the lines never cease to exist and the ";
    test2.text += "points aren't for real. I'm feeling a bit hungry though ";
    test2.text += "so I don't know if I'll have the stamina to clean up this ";
    test2.text += "case in all it's glory. Repeat: ";
    test2.text += test2.text;
    test2.text += test2.text;
    test2.thing_id = 2;
    test2.next.push_back(test1);
    test3.category = DialogCategory::TEXT;
    test3.action_event = event_handler->createStartBattleEvent();
    test3.text = "Back to finish off with a clean case with a couple of lines.";
    test3.text += " So this requires me to write a bunch of BS to try and fill";
    test3.text += " these lines.";
    test3.text += test3.text;
    test3.text += test3.text;
    test3.thing_id = 1003;
    test3.next.push_back(test2);
    test4.category = DialogCategory::TEXT;
    test4.action_event = blank_event;
    test4.text = "Option 1 - This goes on and on and on and on and on and ";
    test4.text += "lorem ipsum. This is way too long to be an option. Loser";
    test4.thing_id = -1;
    test4.next.push_back(test2);
    test5.category = DialogCategory::TEXT;
    test5.action_event = blank_event;
    test5.text = "Option 2";
    test5.thing_id = -1;
    test5.next.push_back(test3);
    test1.next.push_back(test4);
    test1.next.push_back(test5);
    test4.text = "Option 3";
    test1.next.push_back(test4);
    test5.text = "Option 4";
    test1.next.push_back(test5);
    test4.text = "Option 5";
    test1.next.push_back(test4);
    test5.text = "Option 6";
    test1.next.push_back(test5);
    convo2.next.push_back(test1);
    convo->next.push_back(convo2);

    /* Run the conversation and then delete */
    if(map_dialog.initConversation(convo, player))
    {
      std::vector<int> list = map_dialog.getConversationIDs();
      map_dialog.setConversationThings(getThingData(list));
    }

    delete convo;
  }
  else if(map_dialog.isConversationActive())
    map_dialog.keyDownEvent(event);
  else if(event.keysym.sym == SDLK_SPACE)
    initiateThingInteraction(player);
  else if(event.keysym.sym == SDLK_z)
    zoom_out = true;
  else if(event.keysym.sym == SDLK_x)
    zoom_in = true;
  else if(event.keysym.sym == SDLK_t)
  {
    if(persons[1]->getTarget() == NULL)
      persons[1]->setTarget(player);
    else
      persons[1]->clearTarget();
  }
  else if(event.keysym.sym == SDLK_y)
  {
    MapItem* item = NULL;
    MapThing* thing = NULL;
    MapPerson* person = NULL;
    geography[0][7][8]->getRenderThings(0, item, person, thing);

    std::cout << geography[0][7][8]->isItemsSet() << ","
              << geography[0][8][8]->isItemsSet() << std::endl;
    std::cout << item << " " << thing << " " << person << std::endl;
  }
  else if(player != NULL)
  {
    if((event.keysym.sym == SDLK_LSHIFT || event.keysym.sym == SDLK_RSHIFT) && 
       !running)
    {
      running = true;
      player->setSpeed(player->getSpeed() * 2);
    }
    else if(event.keysym.sym == SDLK_3)
    {
      viewport.lockOn(player);
    }
    else if(event.keysym.sym == SDLK_4)
    {
      if(persons.size() >= 1)
        viewport.lockOn(persons[1]);
    }
    else
      player->keyDownEvent(event);
  }
  
  return false;
}

void Map::keyUpEvent(SDL_KeyboardEvent event)
{
  if(item_menu.isActive())
    item_menu.keyUpEvent(event);
  else if(map_dialog.isConversationActive())
    map_dialog.keyUpEvent(event);
  else if(player != NULL)
  {
    if((event.keysym.sym == SDLK_LSHIFT || event.keysym.sym == SDLK_RSHIFT) && 
       running)
    {
      running = false;
      player->setSpeed(player->getSpeed() / 2);
    }
    else
      player->keyUpEvent(event);
  }
}

// TODO: Separate file add success and XML read success to parse error
bool Map::loadMap(std::string file, SDL_Renderer* renderer, bool encryption)
{
  bool done = false;
  bool read_success = true;
  bool success = true;
  FileHandler fh(file, false, true, encryption);
  XmlData data;

  /* Start the map read */
  success &= fh.start();
  std::cout << "Date: " << fh.getDate() << std::endl; // TODO: Remove
  
  /* If file open was successful, move forward */
  if(success)
  {
    /* Run through the map components and add them to the map */
    int height = -1;
    int index = -1;
    int width = -1;
    
    do
    {
      /* Read set of XML data */
      data = fh.readXmlData(&done, &read_success);
      success &= read_success;

      /* Parse map data */
      if(data.getElement(kFILE_GAME_TYPE) == "map")
      {
        if(data.getElement(kFILE_SECTION_ID) == "sprite" &&
           !data.getKeyValue(kFILE_SECTION_ID).empty())
        {
          success &= addSpriteData(data, data.getKeyValue(kFILE_SECTION_ID), 
                                   kFILE_SECTION_ID + 1, renderer);
        }
        else
        {
          int temp_index = -1;
          
          /* Determine if it's a new section */
          if(data.getElement(kFILE_SECTION_ID) == "main" && index != 0)
            temp_index = 0;
          else if(data.getElement(kFILE_SECTION_ID) == "section" && 
                  index != std::stoi(data.getKeyValue(kFILE_SECTION_ID)))
            temp_index = std::stoi(data.getKeyValue(kFILE_SECTION_ID));
          
          /* Update the index information accordingly */
          if(temp_index >= 0)
          {
            index = temp_index;
            if(geography.size() > static_cast<uint16_t>(index) && 
               geography[index].size() > 0)
            {
              height = geography[index].front().size();
              width = geography[index].size();
            }
            else
            {
              height = -1;
              width = -1;
            }
          }

          /* Parse the data, if it is relevant to the map */
          if(height == -1 && data.getElement(kFILE_CLASSIFIER) == "height")
          {
            height = data.getDataInteger();
            if(width > 0 && height > 0)
              initiateMapSection(index, width, height);
          }
          else if(width == -1 && data.getElement(kFILE_CLASSIFIER) == "width")
          {
            width = data.getDataInteger();
            if(width > 0 && height > 0)
              initiateMapSection(index, width, height);
          }
          else if(index >= 0 && height > 0 && width > 0)
          {
            /* Tile data */
            if(data.getElement(kFILE_CLASSIFIER) == "base" ||
               data.getElement(kFILE_CLASSIFIER) == "enhancer" ||
               data.getElement(kFILE_CLASSIFIER) == "lower" ||
               data.getElement(kFILE_CLASSIFIER) == "upper" ||
               data.getElement(kFILE_CLASSIFIER) == "tileevent")
            {
              success &= addTileData(data, index);
            }
            /* Thing data */
            else if(data.getElement(kFILE_CLASSIFIER) == "mapthing" || 
                    data.getElement(kFILE_CLASSIFIER) == "mapperson" ||
                    data.getElement(kFILE_CLASSIFIER) == "mapnpc" ||
                    data.getElement(kFILE_CLASSIFIER) == "mapitem" || 
                    data.getElement(kFILE_CLASSIFIER) == "mapio")
            {
              success &= addThingData(data, index, renderer);
            }
          }
        }
      }
    } while(!done);// && success); // TODO: Success in loop??
  }

  success &= fh.stop();

  /* If the map load was successful, proceed to clean-up */
  if(success)
  {
    if(geography[map_index].size() > 0)
    {
      viewport.setMapSize(geography[map_index].size(), 
                          geography[map_index][0].size());

      if(player != NULL)
        viewport.lockOn(player);
    }
    
    /* Load the item menu sprites - TODO: In file? */
    item_menu.loadImageBackend("sprites/Overlay/item_store_left.png", 
                               "sprites/Overlay/item_store_right.png", 
                               renderer);

    /* Load map dialog sprites - TODO: In file? */
    map_dialog.loadImageConversation("sprites/Overlay/dialog.png", renderer);
    map_dialog.loadImageDialogShifts("sprites/Overlay/dialog_next.png", 
                                     "sprites/Overlay/dialog_extender.png", 
                                     renderer);
    map_dialog.loadImageNameLeftRight(
                                 "sprites/Overlay/dialog_corner.png", renderer);
    map_dialog.loadImageOptions("sprites/Overlay/option_circle.png", 
                                "sprites/Overlay/option_triangle.png", 
                                renderer);
    map_dialog.loadImagePickupTopBottom(
                           "sprites/Overlay/notification_corner.png", renderer);

    /* Thing clean-up and tile set-up */
    for(uint16_t i = 0; i < things.size(); i++)
    {
      /* Clean the matrix - fixes up the rendering box */
      things[i]->cleanMatrix();

      /* Get the tile matrix to match the frames and set */
      std::vector<std::vector<Tile*>> tile_set = getTileMatrix(things[i]);
      if(tile_set.size() > 0)
        things[i]->setStartingTiles(tile_set, true);
      else
        things[i]->unsetFrames(true);
    }

    /* Person clean-up and tile set-up */
    for(uint16_t i = 0; i < persons.size(); i++)
    {
      if(persons[i]->cleanMatrix())
      {
        std::vector<std::vector<Tile*>> tile_set = getTileMatrix(persons[i]);
        if(tile_set.size() > 0)
          persons[i]->setStartingTiles(tile_set, true);
        else
          persons[i]->unsetStates(true);
      }
      else
      {
        persons[i]->unsetStates(true);
      }
    }

    /* Items clean-up and tile set-up */
    for(uint16_t i = 0; i < items.size(); i++)
    {
      /* Clean the matrix - fixes up the rendering box */
      if(items[i]->cleanMatrix())
      {
        /* Get the tile matrix to match the frames and set */
        std::vector<std::vector<Tile*>> tile_set = getTileMatrix(items[i]);
        if(tile_set.size() > 0)
          items[i]->setStartingTiles(tile_set, true);
        else
          items[i]->unsetFrames(true);
      }
    }

    /* TODO: Testing - Remove */
    if(geography.size() > 0 && geography[0].size() > 3 
                            && geography[0][3].size() > 3)
      geography[0][3][3]->setStatus(Tile::BLANKED);
      
    /* TODO: Remove - testing */
    std::cout << "--" << std::endl;
    if(things.size() > 0)
    {
      std::vector<std::vector<TileSprite*>> set = things.front()->getFrames();
      for(uint32_t i = 0; i < set.size(); i++)
        for(uint32_t j = 0; j < set[i].size(); j++)
          if(set[i][j] != NULL)
            std::cout << i << "," << j << ": " 
                      << set[i][j]->getPassability(Direction::NORTH) << " " 
                      << set[i][j]->getPassability(Direction::EAST) << " " 
                      << set[i][j]->getPassability(Direction::SOUTH) << " " 
                      << set[i][j]->getPassability(Direction::WEST) << " "
                      << (int)set[i][j]->getRenderDepth() << std::endl;
                
      std::cout << "Bounding Box: " 
                << things.front()->getBoundingBox().x << " "
                << things.front()->getBoundingBox().y << " "
                << things.front()->getBoundingBox().w << " "
                << things.front()->getBoundingBox().h << std::endl;
      std::cout << "Bounding Box in Pixels: " 
                << things.front()->getBoundingPixels().x << " "
                << things.front()->getBoundingPixels().y << " "
                << things.front()->getBoundingPixels().w << " "
                << things.front()->getBoundingPixels().h << std::endl;
      std::cout << "Coord: " << things.front()->getX() << "," 
                             << things.front()->getY() << std::endl;
    }
    std::cout << "--" << std::endl;
  }
  
  /* Save file path, or unload if the load sequence failed*/
  if(success)
  {
    map_path = file;
    map_encrypted = encryption;
  }
  else
  {
    unloadMap();
  }
  
  loaded = success;

  return success;
}

/* Proceeds to pickup the total number of this marked item */
bool Map::pickupItem(MapItem* item)
{
  if(item != NULL)
  {
    /* Set the on map count to 0 */
    item->setCount(0);
    
    return true;
  }
  
  return false;
}

// TODO: Comment
bool Map::reloadMap(SDL_Renderer* renderer)
{
  if(isLoaded())
  {
    /* Get the map file */
    std::string path = map_path;
    bool encrypted = map_encrypted;
    
    /* Unload the map */
    unloadMap();
    
    /* Re-load the same map */
    return loadMap(path, renderer, encrypted);
  }
  
  return false;
}

/* Renders the title screen */
bool Map::render(SDL_Renderer* renderer)
{
  bool success = true;
  
  if(geography.size() > map_index)
  {
    /* Grab the variables for viewport */
    uint16_t tile_x_start = viewport.getXTileStart();
    uint16_t tile_x_end = viewport.getXTileEnd();
    uint16_t tile_y_start = viewport.getYTileStart();
    uint16_t tile_y_end = viewport.getYTileEnd();
    float x_offset = viewport.getX();
    //int x_start = viewport.getXStart(); // TODO: Possible remove?
    //int x_end = viewport.getXEnd();
    float y_offset = viewport.getY();
    //int y_start = viewport.getYStart();
    //int y_end = viewport.getYEnd();

    /* Render the lower tiles within the range of the viewport */
    for(uint16_t i = tile_x_start; i < tile_x_end; i++)
      for(uint16_t j = tile_y_start; j < tile_y_end; j++)
        geography[map_index][i][j]->renderLower(renderer, x_offset, y_offset);

    /* Render the map things within the range of the viewport */
    for(uint8_t index = 0; index < Helpers::getRenderDepth(); index++)
    {
      for(uint16_t i = tile_x_start; i < tile_x_end; i++)
      {
        for(uint16_t j = tile_y_start; j < tile_y_end; j++)
        {
          MapItem* render_item = NULL;
          MapPerson* render_person = NULL;
          MapThing* render_thing = NULL;
         
          if(geography[map_index][i][j]->getRenderThings(index, render_item,
                                                         render_person, 
                                                         render_thing))
          {
            /* Different indexes result in different rendering procedures
             * If base index, render order is top item, thing, then person */
            if(index == 0)
            {
              if(render_item != NULL)
                render_item->renderMain(renderer, geography[map_index][i][j], 
                                        index, x_offset, y_offset);
              if(render_thing != NULL)
                render_thing->renderMain(renderer, geography[map_index][i][j], 
                                         index, x_offset, y_offset);
              if(render_person != NULL)
              {
                if(render_person->getMovement() == Direction::EAST || 
                   render_person->getMovement() == Direction::SOUTH)
                  render_person->renderPrevious(renderer, 
                          geography[map_index][i][j],index, x_offset, y_offset);
                else
                  render_person->renderMain(renderer, 
                         geography[map_index][i][j], index, x_offset, y_offset);
              }
            }
            /* Otherwise, render order is person, then thing */
            else
            {
              if(render_person != NULL)
              {
                if(render_person->getMovement() == Direction::EAST || 
                   render_person->getMovement() == Direction::SOUTH)
                  render_person->renderPrevious(renderer, 
                         geography[map_index][i][j], index, x_offset, y_offset);
                else
                  render_person->renderMain(renderer, 
                         geography[map_index][i][j], index, x_offset, y_offset);
              }

              if(render_thing != NULL)
                render_thing->renderMain(renderer, geography[map_index][i][j], 
                                         index, x_offset, y_offset);
            }
          }
        }
      }
    }

    /* Render the upper tiles within the range of the viewport */
    for(uint16_t i = tile_x_start; i < tile_x_end; i++)
      for(uint16_t j = tile_y_start; j < tile_y_end; j++)    
        geography[map_index][i][j]->renderUpper(renderer, x_offset, y_offset);
    
    /* Render the map dialogs / pop-ups */
    item_menu.render(renderer);
    map_dialog.render(renderer);
  }
  
  return success;
}

/* Sets the running configuration, from the options class */
bool Map::setConfiguration(Options* running_config)
{
  if(running_config != NULL)
  {
    system_options = running_config;
    base_path = system_options->getBasePath();
    
    /* Update the viewport information */
    viewport.setSize(running_config->getScreenWidth(), 
                     running_config->getScreenHeight());

    /* Update the dialogs with options information */
    item_menu.setConfiguration(running_config);
    map_dialog.setConfiguration(running_config);

    return true;
  }
  
  return false;
}
  
// Possibly make the teleport add the ability of shifting map thing
void Map::teleportThing(int id, int tile_x, int tile_y, int section_id)
{
  /* If the section id is below 0, then set to internal map index */
  if(section_id < 0)
    section_id = map_index;
  
  if(tile_x >= 0 && tile_y >= 0 && section_id >= 0)
  {
    uint16_t x = tile_x;
    uint16_t y = tile_y;
    uint16_t section = section_id;
    
    /* Ensure that the tile x and y is within the range */
    if(section < geography.size() && geography[section].size() > x 
                                  && geography[section][x].size() > y)
    {
      /* Change the starting tile for the thing */
      for(auto i = persons.begin(); i != persons.end(); i++)
      {
        if((*i)->getID() == id)
        {
          // TODO: Fix
          //if((*i)->setStartingTile(section, geography[section][x][y]))
          //{
          //  //map_dialog.endConversation();
          //  if(map_index != section)
          //    setSectionIndex(section);
          //  (*i)->clearAllMovement();
          //}
        }
      }
    }
  }
}

void Map::unfocus()
{
  /* If player is set, clear movement */
  if(player != NULL)
  {
    player->keyFlush();
    
    /* If player was running, reset it */
    if(running)
      player->setSpeed(player->getSpeed() / 2);
    running = false;
  } 
}

void Map::unloadMap()
{
  /* Reset the index and applicable parameters */
  map_index = 0;
  tile_height = Helpers::getTileSize();
  tile_width = tile_height;
  
  /* Delete the items */
  for(uint16_t i = 0; i < items.size(); i++)
  {
    if(items[i] != NULL)
      delete items[i];
    items[i] = NULL;
  }
  items.clear();

  /* Delete the persons */
  for(uint16_t i = 0; i < persons.size(); i++)
  {
    if(persons[i] != NULL)
      delete persons[i];
    persons[i] = NULL;
  }
  persons.clear();
  
  /* Delete the things */
  for(uint16_t i = 0; i < things.size(); i++)
  {
    if(things[i] != NULL)
      delete things[i];
    things[i] = NULL;
  }
  things.clear();
  
  /* Delete all the tiles that have been set */
  for(uint16_t i = 0; i < geography.size(); i++)
  {
    for(uint16_t j = 0; j < geography[i].size(); j++)
    {
      for(uint16_t k = 0; k < geography[i][j].size(); k++)
      {
        delete geography[i][j][k];
        geography[i][j][k] = NULL;
      }
      geography[i][j].clear();
    }
    geography[i].clear();
  }
  geography.clear();

  /* Deletes the sprite data stored */
  for(uint16_t i = 0; i < tile_sprites.size(); i++)
  {
    delete tile_sprites[i];
    tile_sprites[i] = 0;
  }
  tile_sprites.clear();
  
  /* Reset the viewport */
  viewport.setMapSize(0, 0);
  viewport.lockOn(0, 0);

  /* Clear the remaining and disable the loading */
  // clear();
  loaded = false;
  map_encrypted = false;
  map_path = "";
}

/* Updates the game state */
bool Map::update(int cycle_time)
{
  std::vector<std::vector<Tile*>> tile_set;
  
  /* Check on player interaction */
  if(player != NULL && player->getTarget() != NULL 
                    && !map_dialog.isConversationActive() 
                    && !map_dialog.isConversationReady() 
                    && !map_dialog.isConversationWaiting())
  {
    player->getTarget()->clearTarget();
    player->clearTarget();
  }
  
  /* Update the sprite animation */
  for(uint16_t i = 0; i < tile_sprites.size(); i++)
    tile_sprites[i]->update(cycle_time);

  /* Update map items */
  for(uint16_t i = 0; i < items.size(); i++)
    items[i]->update(cycle_time, tile_set);
    
  /* Update persons for movement and animation */
  for(uint16_t i = 0; i < persons.size(); i++)
  {
    if(persons[i] != NULL && persons[i]->getMapSection() == map_index &&
       persons[i]->isTilesSet())
    {
      if(persons[i]->isMoving() || persons[i]->isMoveRequested())
        tile_set = getTileMatrix(persons[i], 
                                 persons[i]->getPredictedMoveRequest());

      /* Proceed to update the person */
      persons[i]->update(cycle_time, tile_set);
    }
  }
  
  /* Update map things */
  for(uint16_t i = 0; i < things.size(); i++)
    things[i]->update(cycle_time, tile_set);
  
  /* Finally, update the viewport and dialogs */
  item_menu.update(cycle_time);
  map_dialog.update(cycle_time);
  updateTileSize();
  viewport.update();
  
  return false;
}
