/******************************************************************************
 * Class Name: Map
 * Date Created: Oct 28 2012
 * Inheritance: none
 * Description: The map class, this is the top level with regard to an actual
 *              in-game map. This contains all the tiles that a map is composed
 *              of, it also holds pointers to all of the NPC's contained in the
 *              maps tiles. This also handles the viewport for the map.
 ******************************************************************************/
#ifndef MAP_H
#define MAP_H

#include <vector>

#include "FileHandler.h"
#include "Game/EventHandler.h"
#include "Game/Map/ItemStore.h"
#include "Game/Map/MapDialog.h"
#include "Game/Map/MapInteractiveObject.h"
#include "Game/Map/MapItem.h"
// #include "Game/Map/MapMenu.h"
#include "Game/Map/MapNPC.h"
#include "Game/Map/MapPerson.h"
// #include "Game/Map/MapStatusBar.h"
#include "Game/Map/MapThing.h"
#include "Game/Map/MapViewport.h"
// #include "Game/Map/Sector.h"
#include "Game/Map/Tile.h"
// #include "Game/Weather.h"
#include "Helpers.h"
#include "Options.h"
#include "Sprite.h"

class Map
{
public:
  /* Constructor function */
  Map(Options* running_config = NULL, EventHandler* event_handler = NULL);

  /* Destructor function */
  ~Map();

private:
  /* The base system path to the resources */
  std::string base_path;
  
  /* A reference blank event for setting events in the game */
  EventHandler* event_handler;
  
  /* The item store menu */
  ItemStore item_menu;

  /* The actual tiles that comprise a map, dynamically sized */
  std::vector<std::vector<std::vector<Tile*>>> geography;
  uint16_t map_index;
  std::vector<Sprite*> tile_sprites;

  /* Indication if the map has been loaded */
  bool loaded;
  
  /* The menus and dialogs on top of the map */
  MapDialog map_dialog;
  // MapMenu map_menu;

  /* The path to the loaded map file */
  bool map_encrypted;
  std::string map_path;
  
  // /* The status bar on the map */
  // MapStatusBar map_status_bar; // TODO: Remove

  // /* The players info on the map */
  std::vector<MapItem*> items;
  std::vector<MapPerson*> persons;
  MapPerson* player; /* The actively controlled player */
  std::vector<MapThing*> things;

  // /* The sectors on the map (for rooms, caves, houses etc) */
  // QList<Sector> sectors;

  /* The system options, used for rendering, settings, etc. */
  Options* system_options;
  
  /* The number of pixels for each tile in the class */
  uint16_t tile_height;
  uint16_t tile_width;
  
  // /* The time that has elapsed for each draw cycle */
  // int time_elapsed;

  /* The viewport for the map, controls positioning */
  MapViewport viewport;

  // /* Weather effect on the overall map (May be pushed to the sector level at 
   // * a later time) */
  // Weather* weather_effect;

  /* Status of the zoom on the map */
  bool zoom_in;
  bool zoom_out;
  
  // /* The painting monitoring parameters */
  // QString frames_per_second;
  // int paint_animation;
  // int paint_frames;
  // int paint_time;

  // /*------------------- Constants -----------------------*/
  // const static int kDOUBLE_DIGITS;    /* The point when integers are more
  //                                      * than a single digit */
  // const static int kELEMENT_DATA;     /* Element data type for sprite */
  const static uint8_t kFILE_CLASSIFIER; /* The file tile classification text */
  const static uint8_t kFILE_GAME_TYPE; /* The game type classifier */
  const static uint8_t kFILE_SECTION_ID; /* The section identifier, for file */
  const static uint8_t kFILE_TILE_COLUMN; /* The tile depth in XML of column */
  const static uint8_t kFILE_TILE_ROW; /* The tile depth in XML of row */
  const static uint8_t kPLAYER_ID;     /* The player ID for computer control */
  const static uint16_t kZOOM_TILE_SIZE;  /* The tile size, when zoomed out */

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/
private:
  /* Adds sprite data, as per data from the file */
  bool addSpriteData(XmlData data, std::string id, 
                     int file_index, SDL_Renderer* renderer);
  
  /* Adds tile data, as per data from the file */
  bool addTileData(XmlData data, uint16_t section_index);

  // /* Adds a tile sprite, based on the path and some XMLData */
  // bool addTileSprite(QString path, int x_diff, int y_diff, 
                     // QList<QString> adjustments, int section_index, 
                     // XmlData data);

  /* Adds thing data, as per data from the file */
  bool addThingData(XmlData data, uint16_t section_index, 
                                  SDL_Renderer* renderer);

  /* Returns a stack of map things that correspond to the ID stack */
  std::vector<MapThing*> getThingData(std::vector<int> thing_ids);

  /* Returns a matrix of tiles that match the frames in the thing */
  std::vector<std::vector<Tile*>> getTileMatrix(
               MapThing* thing, Direction direction = Direction::DIRECTIONLESS);
  std::vector<std::vector<Tile*>> getTileMatrix(uint16_t section,
                                                uint16_t x, uint16_t y, 
                                                uint16_t width, 
                                                uint16_t height);

  /* Initiates a section block of map. Triggered from the file data */
  bool initiateMapSection(uint16_t section_index, int width, int height);
  
  /* Initiates a thing action, based on the action key being hit */
  void initiateThingInteraction(MapPerson* initiator);
 
  /* Parse coordinate info from file to give the designated tile coordinates
   * to update */
  bool parseCoordinateInfo(std::string row, std::string col, uint16_t index,
                           uint16_t* r_start, uint16_t* r_end, 
                           uint16_t* c_start, uint16_t* c_end);
                           
  /* Changes the map section index - what is displayed */
  bool setSectionIndex(uint16_t index);
  
  /* Splits the ID into a vector of IDs */
  std::vector< std::vector<uint16_t> > splitIdString(std::string id, 
                                                     bool matrix = false);

  /* Updates the height and width, based on zoom factors */
  void updateTileSize();

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  // /* Returns the map person, for access */
  // MapPerson* getPlayer();

  // /* Returns the map viewport, for displaying the scene to the screen */
  // MapViewport* getViewport();

  // /* Initial call when map is displayed */
  // void initialization();

  /* Initiates a conversation, within the map. */
  bool initConversation(Conversation* convo, MapThing* source);

  /* Initiates a notification, within the map (either string or image based) */
  bool initNotification(std::string notification);
  bool initNotification(Frame* image, int count);

  /* Initializes item store display, within the map */
  bool initStore(ItemStore::StoreMode mode, std::vector<Item*> items,
                 std::vector<uint32_t> counts,
                 std::vector<int32_t> cost_modifiers,
                 std::string name = "", bool show_empty = false);
  
  // /* Checks whether the viewport contains any tiles with the given sector */
  // bool isInSector(int index);

  /* Returns if the map has been currently loaded with data */
  bool isLoaded();

  /* The key up and down events to be handled by the class */
  bool keyDownEvent(SDL_KeyboardEvent event);
  void keyUpEvent(SDL_KeyboardEvent event);
  
  /* Loads the map */
  bool loadMap(std::string file, SDL_Renderer* renderer, 
                                 bool encryption = false);

  /* Picks up the total number of the item */
  bool pickupItem(MapItem* item);
  
  /* Reloads the existing map */
  bool reloadMap(SDL_Renderer* renderer);
  
  /* Renders the map */
  bool render(SDL_Renderer* renderer);

  /* Sets the running configuration, from the options class */
  bool setConfiguration(Options* running_config);
  
  /* Teleport a thing, based on the given coordinates */
  void teleportThing(int id, int tile_x, int tile_y, int section_id);

  /* Handles all the necessary clean up when map focus is lost */
  void unfocus();
  
  /* Unload the map, if there is one loaded */
  void unloadMap();

  /* Updates the game state */
  bool update(int cycle_time);
};

#endif // MAP_H
