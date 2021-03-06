/*******************************************************************************
* Class Name: Menu [Declaration]
* Date Created: February 20, 2016
* Inheritance: None
* Description:
*
*
* TODO
* ----
*
* See .cc file for NOTEs
*****************************************************************************/
#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

#include "Game/Battle/BattleActor.h"
#include "Game/Battle/BattleDisplayData.h"
#include "Game/EventHandler.h"
#include "Game/KeyHandler.h"
#include "Game/Map/Map.h"
#include "Game/Player/Player.h"
#include "Game/Save.h"
#include "AnalogOption.h"
#include "DigitalOption.h"
#include "Options.h"
#include "Sprite.h"
#include "Window.h"

enum class InventoryIndex : uint8_t
{
  NONE = 0,
  ITEMS = 1,
  EQUIPMENT = 2,
  BUBBIES = 3,
  KEY_ITEMS = 4
};

enum class QuitIndex : uint8_t
{
  NONE = 0,
  NO = 1,
  YES = 2
};

ENUM_FLAGS(MenuState)
enum class MenuState
{
  CALL_SAVE = 1 << 1, /* Save the current menu */
  SHOWING = 1 << 2,   /* Is the menu currently rendering */
  QUITTING = 1 << 3   /* Is the menu declaring to quit the game? */
};

class Menu
{
public:
  /* BattleMenu Default Initialization Function */
  Menu();

  /* Annihilates a BattleMenu object */
  ~Menu();

private:
  /* Battle Display Data */
  BattleDisplayData* battle_display_data;

  /* Current Map the Menu is Running On */
  Map* curr_map;

  /* The current player for the menu */
  Player* curr_player;

  /* Assigned EventHandler */
  EventHandler* event_handler;

  /* Assigned Configuration */
  Options* config;

  /* Current working coordinate */
  Coordinate current;

  /* Enumerated flags for the Menu */
  MenuState flags;

  /* Frames to be built */
  Frame* frame_bubbies;
  Frame* frame_checkbox;
  Frame* frame_equipment;
  Frame* frame_footsteps;
  Frame* frame_items;
  Frame* frame_key_items;
  Frame* frame_location;
  Frame* frame_money;
  Frame* frame_exp_full;
  Frame* frame_exp_middle;
  Frame* frame_exp_empty;

  /* Inventory / Face Titles (Icon Boxes) */
  std::vector<Box> titles;

  /* Inventory Screen Boxes */
  Box inventory_top_box;
  Box inventory_scroll_box;
  Box inventory_bottom_box;
  Box inventory_icon_box;

  /* Current menu layer */
  MenuLayer layer;

  /* Main Section (Centre) Window */
  Window main_section;

  /* Options for the Option Menu */
  AnalogOption option_audio_level;
  AnalogOption option_music_level;
  AnalogOption option_scaling_ui_level;
  AnalogOption option_scaling_text_level;
  DigitalOption option_auto_run;
  DigitalOption option_mute;
  DigitalOption option_fast_battle;

  /* The Battle Actors for the sleuth */
  std::vector<BattleActor*> actors;

  /* Inventory */
  Inventory* player_inventory;

  /* Assigned Renderer */
  SDL_Renderer* renderer;

  /* Sleuth Screen Boxes */
  Box s_top_box;
  Box s_sprite_box;
  Box s_rank_box;
  Box s_exp_box;
  Box s_details_box;
  Box s_top_stats_box;
  Box s_attributes_box;
  Box s_vita_bar;
  Box s_qtdr_bar;

  /* Save file data */
  std::vector<Save> save_data;

  /* Save screen scroll box */
  Box save_scroll_box;
  MenuSaveState save_state;

  /* Skill Screen Boxes */
  Box skills_top_box;
  Box skills_name_box;
  Box skills_icon_box;
  Box skills_bot_box;
  Box skills_scroll_box;

  /* Sleuth Frames */
  std::vector<Sprite*> sleuth_faces;
  std::vector<Frame*> sleuth_attr_frames;
  std::vector<Frame*> sleuth_stat_frames;

  /* Vector of title elements */
  std::vector<TitleElement> title_elements;

  /* Vector of  */
  std::vector<TitleElement> person_title_elements;

  /* Element indexes */
  int32_t inventory_element_index;
  int32_t option_element_index;
  int32_t person_element_index;
  int32_t skills_element_index;
  int32_t save_element_index;
  int32_t sleuth_element_index;
  int32_t title_element_index;

  /* Inventory Index Values */
  InventoryIndex inventory_title_index;
  QuitIndex quit_index;

  /* Title Section (Left) Window */
  Window title_section;

  /* Options Elements */
  AnalogOption audio_level;
  AnalogOption music_level;

  /* ------- Constants ------- */

  /* Bars */
  static const float kBAR_VITA_WIDTH;
  static const float kBAR_QTDR_WIDTH;

  /* Title Section */
  static const uint8_t kTITLE_ALPHA;
  static const float kTITLE_HEIGHT;
  static const float kTITLE_WIDTH;
  static const float kTITLE_X_OFFSET;
  static const float kTITLE_ELEMENT_X_OFFSET;
  static const float kTITLE_Y_OFFSET;
  static const float kTITLE_ELEMENT_GAP;
  static const float kTITLE_CORNER_LENGTH;
  static const float kTITLE_SLIDE_RATE;
  static const float kTITLE_LOCATION_Y_OFFSET;
  static const float kTITLE_ICONS_Y_GAP;
  static const float kTITLE_ICON_TEXT_X;
  static const float kTITLE_ICON_TEXT_Y;
  static const float kTITLE_HOVER_OFFSET_X;
  static const float kTITLE_HOVER_WIDTH;
  static const float kTITLE_HOVER_RATE;
  static const float kTITLE_HOVER_MIN;
  static const float kTITLE_HOVER_MAX;

  /* Main Section General */
  static const uint8_t kMAIN_ALPHA;
  static const float kMAIN_SLIDE_RATE;
  static const float kMAIN_CORNER_LENGTH;
  static const float kINV_WIDTH;
  static const float kOPTIONS_WIDTH;
  static const float kQUIT_WIDTH;
  static const float kSAVE_WIDTH;
  static const float kSLEUTH_WIDTH;

  /* Save Section */
  static const float kSAVE_GAP;
  static const float kSAVE_ELEMENT_WIDTH;
  static const float kSAVE_ELEMENT_HEIGHT;

  /* Sleuth Section */
  static const float kSLEUTH_GAP;
  static const float kSLEUTH_SPRITE_WIDTH;
  static const float kSLEUTH_ATTRIBUTE_HEIGHT;
  static const float kSLEUTH_ELEMENT_HEIGHT;
  static const float kSLEUTH_EQUIP_ICON_SIZE;
  static const float kSLEUTH_ATTRIBUTE_INSET;

  /* Skill Screen (In Sleuth) */
  static const float kSKILL_ELEMENT_WIDTH;
  static const float kSKILL_ELEMENT_HEIGHT;
  static const float kSKILL_ELEMENT_INSET;

  /* Inventory Section */
  static const float kINV_GAP;
  static const float kINV_MASS_TEXT_Y;
  static const float kINV_MASS_VALUE_Y;
  static const float kINV_THUMB_GAP;
  static const float kINV_ITEM_NAME_X;
  static const float kINV_ITEM_NAME_Y;
  static const float kINV_ITEM_ELEMENT_WIDTH;
  static const float kINV_ITEM_ELEMENT_HEIGHT;
  static const float kINV_ITEM_ELEMENT_INSET;
  static const float kINV_ITEM_MASS_Y;
  static const float kINV_ITEM_DESC_Y;

  /* Options Section */
  static const uint32_t kNUM_OPTIONS;
  static const float kOPTIONS_X;
  static const float kOPTIONS_Y;
  static const float kOPTIONS_Y_BAR_GAP;
  static const float kOPTIONS_Y_GAP;
  static const float kOPTIONS_DIGITAL_TEXT_GAP;
  static const float kOPTIONS_BOX_SIZE;

  /* Popup Sections */
  static const float kSAVE_POPUP_HEIGHT;
  static const float kSAVE_POPUP_WIDTH;
  static const float kSAVE_POPUP_GAP;

  /* Colors */
  static const SDL_Color kCOLOR_TITLE_BG;
  static const SDL_Color kCOLOR_TITLE_BORDER;
  static const SDL_Color kCOLOR_TITLE_HOVER;
  static const SDL_Color kCOLOR_MAIN_BORDER;
  static const SDL_Color kCOLOR_TEXT;
  static const SDL_Color kCOLOR_OPTION_FILL;
  static const SDL_Color kCOLOR_OPTION_FILL_SELECTED;
  static const SDL_Color kCOLOR_INVENTORY_ICON_FILL;
  static const SDL_Color kCOLOR_BORDER_UNSELECTED;
  static const SDL_Color kCOLOR_ICON_UNSELECTED_FILL;
  static const SDL_Color kCOLOR_ELEMENTAL_CURVE;

  /*=============================================================================
   * PRIVATE FUNCTIONS
   *============================================================================*/
private:
  /* Constructs icon frames */
  void buildIconFrames();
  void buildIconTitles(int32_t number);

  /* Construct the inventory display screen */
  void buildInventoryBubbies();
  void buildInventoryEquips();
  void buildInventoryElements();
  void buildInventoryItems();
  void buildInventoryKeyItems();

  /* Texture Creations for Attributes, Elements */
  SDL_Texture* buildAttributeFrame(Attribute attr, uint32_t width,
                                   uint32_t height);
  SDL_Texture* buildElementFrame(ElementType element_type, uint32_t width,
                                 uint32_t height);

  /* Item Frame */
  SDL_Texture* buildItemListFrame(Item* build_item, int32_t count,
                                  uint32_t width, uint32_t height);

  /* Texture Creation for Skills (List and Detail Frames) */
  SDL_Texture* buildSkillListFrame(Skill* build_skill, uint32_t width,
                                   uint32_t height);

  /* Construct the main section backdrop */
  void buildMainSection(MenuType menu_type);

  /* Constructs the Options Data */
  void buildOptions();

  /* Builds the quit screen */
  void buildQuit();

  /* Construct the Save box and screen */
  void buildSave();

  /* Construct the Sleuth overview screen */
  bool buildSleuthScreen();

  /* Construct the skill frames for the current person */
  void buildSkillFrames();

  /* Constructs the signature screen */
  void buildSignature();

  /* Construct a vector of TitleElements for the Title Section */
  void buildTitleElements();

  /* Construct person title elements for selection */
  void buildPersonTitleElements();

  /* Construct the TitleSection (Main Selection) of the Menu */
  void buildTitleSection();
  int32_t calcMainCornerInset();
  int32_t calcSleuthAttributeHeight();
  int32_t calcSleuthElementHeight();
  int32_t calcSleuthTileSize();

  /* Calculate the required string for Item Details */
  std::string calcItemDetailsString(Item* item);

  /* Calculate dimensions of the Item Title frames */
  int32_t calcItemTitleWidth();
  int32_t calcItemTitleHeight();

  /* Calculate dimensions of the Skill Title frames */
  int32_t calcSkillTitleWidth();
  int32_t calcSkillTitleHeight();

  /* Can the sleuth element index be incremented? */
  bool canIncrementSleuth();

  /* Clear the battle actors */
  void clearActors();

  /* Clears the extra attribute frames */
  void clearAttributeFrames();

  /* Clear the element frames */
  void clearElementFrames();

  /* Clear out the Icon Frames */
  void clearIconFrames();

  /* Decrement index functions */
  void decrementInventoryIndex();
  void decrementOptionIndex();
  void decrementQuitIndex();
  void decrementSaveIndex();
  void decrementSleuthIndex();

  /* Increment index functions */
  void incrementInventoryIndex();
  void incrementOptionIndex();
  void incrementSaveIndex();
  void incrementQuitIndex();
  void incrementSleuthIndex();

  /* Key Down Methods */
  void keyDownLeft();
  void keyDownRight();
  void keyDownUp();
  void keyDownDown();
  void keyDownAction();
  void keyDownCancel();

  /* Selection methods */
  void selectInventoryIndex();
  void selectOptionIndex();
  void selectSleuthIndex();

  /* Render the attribute frames */
  void renderAttributes(Coordinate start, int32_t gap);

  /* Render Bubbies */
  void renderBubbies();

  /* Render Equipment */
  void renderEquipment();

  /* Render the Inventory Screen */
  void renderInventory();

  /* Render Items */
  void renderItem(Coordinate start, int32_t icon_w, int32_t gap,
                  int32_t bot_height);

  /* Render Key Items */
  void renderKeyItems();

  /* Render the main section */
  void renderMainSection();

  /* Render the title section */
  void renderTitleSection();

  /* Render the title triangle on a given icon box */
  void renderTitleTriangle(Box& icon_box);

  /* Render options functions */
  UCoordinate renderOptionAnalog(AnalogOption& option, UCoordinate point);
  UCoordinate renderOptionDigital(DigitalOption& option, UCoordinate point);
  void renderOptions();

  /* Render the person element titles */
  void renderPersonElementTitles(int32_t gap);
  void renderPersonElementTitle(TitleElement& element);

  /* Render the Sleuth Screen */
  void renderSleuth();

  /* Render the sleuth details sections */
  void renderSleuthDetails();
  void renderSleuthDetailsRank();
  void renderSleuthDetailsExp();
  void renderSleuthDetailsStats();

  /* Render the other sleuth information */
  void renderSleuthOverview();
  void renderSleuthEquipment();
  void renderSleuthSkills();
  void renderSleuthSkillDetail(Coordinate start, int32_t icon_w, int32_t gap);

  /* Render the Save Screen */
  void renderSave();

  /* Render the Quit Screen */
  void renderQuit();

  /* Setup for the standard box used throughout the menu */
  void setupDefaultBox(Box& setup_box);

  /* Unselecting index functions */
  void unselectInventoryIndex();
  void unselectOptionIndex();
  void unselectSleuthIndex();

  /* Update the UI for the current scaling factor */
  void updateScalingFactor();

  /* Obtain a pointer to the currently selected person */
  BattleActor* getCurrentActor();
  Skill* getCurrentSkill();
  Person* getCurrentPerson();

  /* Returns the requested menu font */
  TTF_Font* getFont(FontName font_name);

  /* Returns the MenuType currently rendering */
  MenuType getMainMenuType();

  /* Returns the MenuType of the Sleuth Screen */
  MenuType getSleuthMenuType();

  /*=============================================================================
   * PUBLIC FUNCTIONS
   *============================================================================*/
public:
  /* Clear the Menu State */
  void clear();

  /* Returns an evaluated MenuState flag */
  bool getFlag(const MenuState& test_flag);

  /* Enumerated menu layer */
  MenuLayer getMenuLayer();

  /* Return the current save index, or -1 if invalid index */
  int32_t getSaveIndex();

  /* Return the SaveState */
  MenuSaveState getMenuSaveState();

  /* Hide the Menu */
  void hide();

  /* Returns true if the main section is sliding */
  bool isMainSliding();

  /* Key down event */
  bool keyDownEvent(KeyHandler& key_handler);

  /* Show the Menu */
  void show();

  /* Render the Menu in its current state */
  void render();

  /* Assign the Battle Display Data */
  void setBattleDisplayData(BattleDisplayData* battle_display_data);

  /* Assign Configuration */
  void setConfig(Options* config);

  /* Assign EventHandler */
  void setEventHandler(EventHandler* event_handler);

  /* Assign a given MenuState flag a given value */
  void setFlag(MenuState set_flags, const bool& set_value = true);

  /* Assign the inventory */
  void setInventory(Inventory* player_inventory);

  /* Assign a map the menu is running on */
  void setMap(Map* new_map);

  /* Assign a new player to the menu */
  void setPlayer(Player* new_player);

  /* Assign Renderer */
  void setRenderer(SDL_Renderer* renderer);

  /* Save data */
  void setSaveData(std::vector<Save> saves);

  /* Assigns the enumerated save state */
  void setMenuSaveState(MenuSaveState save_state);

  /* Update the menu with the current cycle time */
  bool update(int32_t cycle_time);

  /* Update the SaveTitles */
  void updateSaveTitles();
};

#endif // MENU_H
