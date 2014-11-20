/*******************************************************************************
 * Class Name: Game
 * Date Created: Dec 2 2012
 * Inheritance: none
 * Description: This class is the top layer that handles all interclass
 *              communication between map, player, and battle as well as all
 *              setup and overall insight and control. The painting control
 *              won't be handled here and this will just act as an intermediate
 *              data highway / event handler.
 *
 *
 * //TODO: Starting inventory items [01-11-14]
 ******************************************************************************/
#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <chrono>

#include "Game/Battle/AIModuleTester.h"
#include "Game/Battle/Battle.h"
#include "Game/EventHandler.h"
#include "Game/Map/Map.h"
#include "Game/Player/Player.h"
#include "Game/Player/Bubby.h"
//#include "Game/VictoryScreen.h"
#include "Game/Player/Inventory.h" //TODO
#include "Options.h"

using std::begin;
using std::end;

class Game
{
public:
  /* Constructor function */
  Game(Options* running_config = nullptr);

  /* Destructor function */
  ~Game();

  /* The game mode operator, for controlling the visible widget */
  enum GameMode 
  {
    DISABLED       = 0, 
    MAP            = 1, 
    BATTLE         = 2, 
    VICTORY_SCREEN = 3 
  };

private:
  /* The active rendering engine */
  SDL_Renderer* active_renderer;
  
  /* The computed base path for resources in the application */
  std::string base_path;
  
  /* Handles all events throughout the game. */
  EventHandler event_handler;
  
  /* A current battle pointer */
  Battle* game_battle;
  
  /* The configuration for the display of the game */
  Options* game_config;

  /* The current loaded map */
  Map* game_map; // TODO: Make non-pointer?

  /* The game starting inventory */
  Inventory* game_inventory; //TODO: Make part of player?

  /* List of all actions */
  std::vector<Action*> action_list;

  /* List of all Battle Class categories */
  std::vector<Category*> battle_class_list;

  /* List of all Race categories */
  std::vector<Category*> race_list;

  /* List of all flavours */
  std::vector<Flavour*> flavour_list;

  /* List of all skills */
  std::vector<Skill*> skill_list;

  /* List of all base persons */
  std::vector<Person*> base_person_list;

  /* List of all available items in the game */
  std::vector<Item*> base_item_list;
  
  /* The maps in the game */
  // [09-07-14] TODO: Recommend Map*, std::string pair ?
  //std::vector<Map*> levels;
  //std::vector<std::string> level_list;

  /* The level number currently on */
  //int level_num;

  /* The player */
  //Player* main_player;

  /* The mode that the game is currently running at */
  GameMode mode;
  
  /* A current victory screen pointer */
  //VictoryScreen* victory_screen;

  /* ------------ Constants --------------- */
  static const uint32_t kMONEY_ITEM_ID;

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* A give item event, based on an ID and count (triggered from stored event */
  bool eventGiveItem(int id, int count);
  
  /* Initiates a conversation event */
  void eventInitConversation(Conversation* convo, MapThing* source);

  /* Initiates a notification event (in map) */
  void eventInitNotification(std::string notification);
  
  /* The pickup item event - from walking over or triggering from action key */
  void eventPickupItem(MapItem* item, bool walkover);
  
  /* Starts a battle event. Using the given information - TODO */
  void eventStartBattle();
  
  /* Teleport thing event, based on ID and coordinates */
  void eventTeleportThing(int thing_id, int x, int y, int section_id);
  
  /* Polls events running from the event handler. Triggered from other places
   * in the game */
  void pollEvents();

  /* Set up the battle */
  void setupBattle();

  /* Set up the map */
  void setupMap();

  /* Sets up the default player inventory */
  void setupPlayerInventory();

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Returns a pointer to an action by index or by ID */
  Action* getAction(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a battle class by index or by ID */
  Category* getBattleClass(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a race category by index or by ID */
  Category* getCategory(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a flavour by index or by ID */
  Flavour* getFlavour(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a skill by index or by ID */
  Skill* getSkill(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a person by index or by ID */
  Person* getPerson(const int32_t &index, const bool &by_id = true);

  /* Returns a pointer to a person by index or by ID */
  Item* getItem(const int32_t &index, const bool &by_id = true);
  
  /* The key up and down events to be handled by the class */
  bool keyDownEvent(SDL_KeyboardEvent event);
  void keyUpEvent(SDL_KeyboardEvent event);

  /* Renders the title screen */
  bool render(SDL_Renderer* renderer);
  
  /* Runs the test battle */
  void runTestBattle();

  /* Set the running configuration, from the options class */
  bool setConfiguration(Options* running_config);

  /* Updates the game state */
  bool update(int cycle_time);
};

#endif // GAME_H
