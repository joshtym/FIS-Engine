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
#include "Game/Map/MapNPC.h"
#include "Game/Map/MapPerson.h"
// #include "Game/Map/MapStatusBar.h"
#include "Game/Map/MapThing.h"
#include "Game/Map/MapViewport.h"
// #include "Game/Map/Sector.h"
#include "Game/Map/Tile.h"
#include "Game/Lay.h"
// #include "Helpers.h"
// #include "Options.h"
// #include "Sprite.h"

// TODO: FOR OVERLAY/UNDERLAY - TESTING ONLY - UNCOMMENT TO ENABLE
//#define MAP_LAY

/* Sub map structure - contains all data related only to each sub */
// TODO: Move thing instances into this struct?
struct SubMap
{
  /* Tile data */
  std::vector<std::vector<Tile*>> tiles;

  /* Lay data */
  std::vector<LayOver> underlays;
  std::vector<LayOver> overlays;

  /* Sound data */
  std::vector<uint32_t> battles;
  std::vector<uint32_t> music;
  int32_t weather;
};

/* Class for map control */
class Map
{
public:
  /* Constructor function */
  Map(Options* running_config = NULL, EventHandler* event_handler = NULL);

  /* Destructor function */
  ~Map();

  /* The map mode operator, for controlling the visible widget */
  enum MapMode
  {
    DISABLED = 0,
    NORMAL = 1,
    SWITCHSUB = 2,
    VIEW = 3,
    NONE = 4
  };

private:
  /* Base things */
  std::vector<MapInteractiveObject*> base_ios;
  std::vector<MapItem*> base_items;
  std::vector<MapPerson*> base_persons;
  std::vector<MapThing*> base_things;

  /* The base system path to the resources */
  std::string base_path;

  /* Battling things/information */
  EventPair battle_eventlose;
  EventPair battle_eventwin;
  BattleFlags battle_flags;
  MapPerson* battle_person;
  std::vector<uint32_t> battle_scenes;
  MapThing* battle_thing;
  bool battle_trigger;

  /* A reference blank event for setting events in the game */
  EventHandler* event_handler;

  /* Fade status */
  uint8_t fade_alpha;
  MapFade fade_status;

  /* Images used in rendering */
  Frame img_spotted;

  /* The item store menu */
  ItemStore item_menu;

  /* Map lays */
  std::vector<Lay*> lay_overs;
  std::vector<Lay*> lay_unders;

  /* Indication if the map has been loaded */
  bool loaded;

  /* The menus and dialogs on top of the map */
  MapDialog map_dialog;
  // MapMenu map_menu;

  /* Map reference index for current sub-map */
  uint16_t map_index;
  int map_index_next;

  // /* The status bar on the map */
  // MapStatusBar map_status_bar; // TODO: Remove

  /* Map parsing modes */
  MapMode mode_curr;
  MapMode mode_next;

  /* Music ID Stack */
  int music_id;
  int music_runtime;

  // /* The players info on the map */
  std::vector<MapInteractiveObject*> ios;
  std::vector<MapItem*> items;
  std::vector<MapPerson*> persons;
  MapPerson* player; /* The actively controlled player */
  std::vector<MapThing*> things;

  // /* The sectors on the map (for rooms, caves, houses etc) */
  // QList<Sector> sectors;

  /* The set of map data */
  std::vector<SubMap> sub_map;

  /* The system options, used for rendering, settings, etc. */
  Options* system_options;

  /* The number of pixels for each tile in the class */
  uint16_t tile_height;
  uint16_t tile_width;

  /* Tile Sprites */
  std::vector<Sprite*> tile_sprites;

  // /* The time that has elapsed for each draw cycle */
  // int time_elapsed;

  /* View mode data */
  int view_acc;
  uint16_t view_section;
  bool view_start;
  MapThing* view_thing;
  Tile* view_tile;
  int view_time;
  bool view_travel;

  /* The viewport for the map, controls positioning */
  MapViewport viewport;

  // /* Weather effect on the overall map (May be pushed to the sector level at
  // * a later time) */
  // Weather* weather_effect;

  /* Status of the zoom on the map */
  bool zoom_in;
  bool zoom_out;

  /*------------------- Constants -----------------------*/
  const static uint8_t kFADE_BLACK;  /* The fully faded out screen alpha */
  const static uint8_t kFADE_FACTOR; /* 1/x fade factor for ms cycle time */
  const static uint8_t kFADE_MAX;    /* The max differential to fade delta */
  const static uint8_t kFADE_VIS; /* The fully visible faded in screen alpha */
  const static uint8_t kFILE_CLASSIFIER; /* The file tile classification text */
  const static uint8_t kFILE_GAME_TYPE;  /* The game type classifier */
  const static uint8_t kFILE_SECTION_ID; /* The section identifier, for file */
  const static uint8_t kFILE_TILE_COLUMN; /* The tile depth in XML of column */
  const static uint8_t kFILE_TILE_ROW;    /* The tile depth in XML of row */
  const static uint32_t kMUSIC_REPEAT;    /* Milliseconds each song repeats */
  const static uint8_t kPLAYER_ID; /* The player ID for computer control */
  const static uint16_t kZOOM_TILE_SIZE; /* The tile size, when zoomed out */

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/
private:
  /* Adds sprite data, as per data from the file */
  bool addSpriteData(XmlData data, std::string id, int file_index,
                     SDL_Renderer* renderer);

  /* Adds tile data, as per data from the file */
  bool addTileData(XmlData data, uint16_t section_index);

  /* Adds thing data, as per data from the file */
  bool addThingBaseData(XmlData data, int file_index, SDL_Renderer* renderer);
  bool addThingData(XmlData data, uint16_t section_index,
                    SDL_Renderer* renderer);

  /* Audio start/stop triggers */
  void audioStart();
  void audioStop();
  void audioUpdate(bool sub_change = false);

  /* Change the mode that the game is running */
  bool changeMode(MapMode mode);

  /* Returns the item, based on the ID */
  MapItem* getItem(uint16_t id);
  MapItem* getItemBase(uint16_t id);

  /* Returns the interactive object, based on the ID */
  MapInteractiveObject* getIO(uint16_t id);
  MapInteractiveObject* getIOBase(uint16_t id);

  /* Returns the person, based on the ID */
  MapPerson* getPersonBase(uint16_t id);

  /* Returns the thing, based on the ID */
  MapThing* getThing(uint16_t id);
  MapThing* getThing(uint16_t id, ThingBase type);
  MapThing* getThingBase(uint16_t id);

  /* Returns a stack of map things that correspond to the ID stack */
  std::vector<MapThing*> getThingData(std::vector<int> thing_ids);

  /* Returns a matrix of tiles that match the frames in the thing */
  std::vector<std::vector<Tile*>>
  getTileMatrix(MapThing* thing,
                Direction direction = Direction::DIRECTIONLESS);
  std::vector<std::vector<Tile*>> getTileMatrix(uint16_t section, uint16_t x,
                                                uint16_t y, uint16_t width,
                                                uint16_t height);

  /* Initiates a section block of map. Triggered from the file data */
  bool initiateMapSection(uint16_t section_index, int width, int height);

  /* Initiates a check for NPC if a forced interaction will occur */
  bool initiateNPCInteraction();

  /* Initiates a thing action, based on the action key being hit */
  void initiateThingInteraction(MapPerson* initiator);

  /* Mode view updates */
  bool modeViewStart(int cycle_time, bool travel);
  bool modeViewStop(int cycle_time, bool travel);

  /* Parse coordinate info from file to give the designated tile coordinates
   * to update */
  bool parseCoordinateInfo(std::string row, std::string col, uint16_t index,
                           uint16_t* r_start, uint16_t* r_end,
                           uint16_t* c_start, uint16_t* c_end);

  /* Changes the map section index - what is displayed */
  bool setSectionIndex(uint16_t index);
  bool setSectionIndexMode(int index_next = -1);

  /* Splits the ID into a vector of IDs */
  std::vector<std::vector<int32_t>> splitIdString(std::string id,
                                                  bool matrix = false);

  /* Triggers a view of the passed in data */
  bool triggerViewThing(MapThing* view_thing, UnlockView view_mode,
                        int view_time);
  bool triggerViewTile(Tile* view_tile, uint16_t view_section,
                       UnlockView view_mode, int view_time);

  /* Updates the map mode - lots of logic here */
  bool updateFade(int cycle_time);
  void updateMode(int cycle_time);

  /* Updates the height and width, based on zoom factors */
  void updateTileSize();

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Battle won/loss/end trigger for map */
  void battleLose();
  void battleRun();
  void battleWon();

  /* Enable view trigger */
  void enableView(bool enable);

  /* Returns the battle information */
  EventPair getBattleEventLose();
  EventPair getBattleEventWin();
  BattleFlags getBattleFlags();
  int getBattlePersonID();
  int getBattleScene();
  int getBattleThingID();

  /* Returns the current enumerated WindowState of the Map dialog */
  WindowStatus getDialogStatus();

  /* The fade status of the map */
  MapFade getFadeStatus();

  /* Return a MapPerson */
  MapPerson* getPerson(uint16_t id);

  /* Returns the number of steps the player has used on map */
  uint32_t getPlayerSteps();

  /* Initiates a battle, within the map */
  bool initBattle(MapPerson* person, MapThing* source, BattleFlags flags,
                  EventPair event_win, EventPair event_lose);

  /* Initiates a conversation, within the map. */
  bool initConversation(ConvoPair convo_pair, MapThing* source);

  /* Initiates a notification, within the map (either string or image based) */
  bool initNotification(std::string notification);
  bool initNotification(Frame* image, int count);

  /* Initializes item store display, within the map */
  bool initStore(ItemStore::StoreMode mode, std::vector<Item*> items,
                 std::vector<uint32_t> counts,
                 std::vector<int32_t> cost_modifiers, std::string name = "",
                 bool show_empty = false);

  /* Returns battle flags and properties */
  bool isBattleLoseGameOver();
  bool isBattleReady();
  bool isBattleRestoreHealth();
  bool isBattleRestoreQD();
  bool isBattleWinDisappear();

  /* Returns if the map has been currently loaded with data */
  bool isLoaded();

  /* Mode checks - only returns true for normal if normal and visible, reverse
   * for disabled */
  bool isModeDisabled();
  bool isModeNormal();

  /* The key up and down events to be handled by the class */
  bool keyDownEvent(SDL_KeyboardEvent event);
  void keyUpEvent(SDL_KeyboardEvent event);

  /* Loads the map data */
  bool loadData(XmlData data, int index, SDL_Renderer* renderer,
                std::string base_path);
  void loadDataFinish(SDL_Renderer* renderer);

  /* Modify thing properties based on passed in properties */
  void modifyThing(MapThing* source, ThingBase type, int id,
                   ThingProperty props, ThingProperty bools, int respawn_int,
                   int speed_int, TrackingState track_enum, int inactive_int);

  /* Picks up the total number of the item */
  bool pickupItem(MapItem* item, int count = -1);

  /* Renders the map */
  bool render(SDL_Renderer* renderer);

  /* Resets the player steps */
  void resetPlayerSteps();

  /* Sets the running configuration, from the options class */
  bool setConfiguration(Options* running_config);

  /* Sets the operational event handler */
  void setEventHandler(EventHandler* event_handler);

  /* Teleport a thing, based on the given coordinates */
  void teleportThing(int id, int tile_x, int tile_y, int section_id);

  /* Handles all the necessary clean up when map focus is lost */
  void unfocus();

  /* Unload the map, if there is one loaded */
  void unloadMap();

  /* Unlock triggers, based on parameter information */
  void unlockIO(MapThing* source, int io_id, UnlockIOMode mode, int state_num,
                UnlockIOEvent mode_events, UnlockView mode_view, int view_time);
  void unlockThing(MapThing* source, int thing_id, UnlockView mode_view,
                   int view_time);
  void unlockTile(int section_id, int tile_x, int tile_y, UnlockTileMode mode,
                  UnlockView mode_view, int view_time);

  /* Updates the game state */
  bool update(int cycle_time);
};

#endif // MAP_H
