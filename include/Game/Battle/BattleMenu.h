/*******************************************************************************
* Class Name: BattleMenu [Declaration]
* Date Created: April 13th, 2013
* Date Redesigned: September 14th, 2015
*
* Inheritance: None
* Description:
*
* Notes
* -----
*
* [1]:
*
* TODO
* ----
*****************************************************************************/
#ifndef BATTLEMENU_H
#define BATTLEMENU_H

#include <SDL2/SDL.h>

#include "Game/KeyHandler.h"
#include "Game/EventHandler.h"
#include "Game/Battle/BattleActor.h"
#include "Game/Battle/BattleDisplayData.h"

enum BattleMenuLayer
{
  ZEROTH_LAYER = 0,
  TYPE_SELECTION = 1,
  ACTION_SELECTION = 2,
  TARGET_SELECTION = 3
};

ENUM_FLAGS(BattleMenuState)
enum class BattleMenuState
{
  READY = 1 << 0,
  SELECTION_COMPLETE = 1 << 1,
  SKILL_FRAMES_BUILT = 1 << 2,
  ITEM_FRAMES_BUILT = 1 << 3
};

class BattleMenu
{
public:
  /* BattleMenu Default Initialization Function */
  BattleMenu();

  /* Annihilates a BattleMenu object */
  ~BattleMenu();

private:
  /* The BattleActor which is using the Menu */
  BattleActor* actor;

  /* The display data for the Battle */
  BattleDisplayData* battle_display_data;

  /* Configuration pointer for the BattleMenu */
  Options* config;

  /* EventHandler */
  EventHandler* event_handler;

  /* Frames of the item description/information */
  std::vector<Frame*> frames_item_info;

  /* Frames of the item name and quantity */
  std::vector<Frame*> frames_item_name;

  /* Vector of frames for skill infos */
  std::vector<Frame*> frames_skill_info;

  /* The frames for skill names */
  std::vector<Frame*> frames_skill_name;

  /* The flags for the class */
  BattleMenuState flags;

  /* The current menu layer */
  BattleMenuLayer menu_layer;

  /* The assigned Renderer */
  SDL_Renderer* renderer;

  /* The selected action_scope */
  ActionScope selected_action_scope;

  /* The selected action type */
  ActionType selected_action_type;

  /* The selected battle skill */
  BattleSkill* selected_battle_skill;

  /* The selected battle item */
  BattleItem* selected_battle_item;

  /* Targets already selected */
  std::vector<BattleActor*> selected_targets;

  /* The window status of the GUI */
  WindowStatus status_window;

  /* Vector of valid action types */
  std::vector<ActionType> valid_action_types;

  /* Selectable battle items */
  std::vector<BattleItem*> valid_battle_items;

  /* Selectable battle */
  std::vector<BattleSkill*> valid_battle_skills;

  /* The current element index for the menu */
  int32_t element_index;
  int32_t index_actions;
  int32_t index_types;

  /* ------------ Constants --------------- */
  const static uint16_t kALLY_HEIGHT; /* Ally display section height */

  const static uint16_t kBIGBAR_OFFSET; /* Offset of bar off bottom */
  const static float kBIGBAR_L;         /* The percentage of the left section */
  const static float kBIGBAR_M1; /* The percentage of the middle section */
  const static float kBIGBAR_M2; /* The percentage of the second middle */

  const static uint16_t kBIGBAR_CHOOSE;   /* Additional offset for choice */
  const static uint16_t kBIGBAR_R_OFFSET; /* Offset off end for right section */

  const static uint8_t kMENU_SEPARATOR_B; /* Separator gap off bottom */
  const static uint8_t kMENU_SEPARATOR_T; /* Separator gap off top */

  const static uint8_t kSCROLL_R; /* Radius on scroll renders */

  const static uint8_t kSKILL_BORDER; /* Border around edge and elements */
  const static uint8_t kSKILL_BORDER_WIDTH; /* Width of border around element */
  const static uint8_t kSKILL_DESC_GAP;   /* Gap between name and description */
  const static uint8_t kSKILL_DESC_LINES; /* Max number of description lines */
  const static uint8_t kSKILL_DESC_SEP;   /* Gap between lines in description */
  const static uint8_t kSKILL_FRAME_S;    /* Small frame size on skill info */
  const static uint8_t kSKILL_FRAME_L;    /* Large frame size on skill info */
  const static uint8_t kSKILL_QD_GAP;     /* Gap between top edge and QD icon */
  const static uint8_t kSKILL_SEP;        /* Separator between image and text */
  const static uint8_t kSKILL_SUCCESS;    /* Gap between success and cooldown */
  const static uint8_t kSKILL_TIME_GAP;   /* Gap between cooldown and bottom */

  const static uint8_t kTYPE_MARGIN; /* Margin around text options in type */
  const static uint8_t kTYPE_MAX;    /* Max number of action types to render */
  const static uint8_t kTYPE_SELECT; /* Margin to spread select around type */
  const static uint16_t kINFO_W;     /* Width of enemy info bar */

  /* Text Colors */
  const static SDL_Color kTEXT_STANDARD;
  const static SDL_Color kTEXT_INVALID;
  const static SDL_Color kTEXT_PRICEY;
  const static SDL_Color kTEXT_NO_TARGETS;

  /*======================== PRIVATE FUNCTIONS ===============================*/
private:
  void setRectBot(SDL_Rect& srect, uint32_t height);
  void setRectTop(SDL_Rect& rect);

  /* Returns the actor of a given element index */
  BattleActor* actorOfElementIndex(int32_t index);

  /* Determines whether the current selection is off. (default enemy targ) */
  bool isActionOffensive();

  /* Does the given BattleActor match? */
  bool isActorMatch(BattleActor* target, bool same_party);

  /* Determines whether a given element index is valid for selection purp. */
  bool isIndexValid(int32_t index);

  /* Checks to make sure the target is valid before allowing a selection */
  bool isTargetValid(BattleActor* check_target);

  /* Get element index of a given BattleActor */
  int32_t elementIndexOfActor(BattleActor* check_actor);

  /* Methods for containing code for each key addition */
  void keyDownAlpha(const char& c);
  void keyDownCancel();
  void keyDownDecrement();
  void keyDownIncrement();
  void keyDownSelect();

  /* Returns actor among actors which corresponds to the sp. ordered index */
  BattleActor* targetOfOrderedIndex(std::vector<BattleActor*> actors,
                                    int32_t ordered_index);

  /* Methods for determining valid indexes on various circumstances */
  int32_t validFirst();
  int32_t validLast();
  int32_t validNext();
  int32_t validPrevious();

  void unsetHoverTargets();

  /* Vector of Party targets */
  std::vector<BattleActor*> getPartyTargets(std::vector<BattleActor*> actors,
                                            bool allies);

  /* Targets which are currently possible to select */
  std::vector<BattleActor*> getSelectableTargets();

  /* Obtains the maximum index value for the current issue */
  int32_t getMaxIndex();

  /* Print out the selectable targets */
  void printSelectableTargets();

  /*========================= PUBLIC FUNCTIONS ===============================*/
public:
  /* Clears the Item frames */
  void clearItemFrames();

  /* Clears the Skill Frames */
  void clearSkillFrames();

  /* Creates the Frames for a given BattleItem */
  SDL_Texture* createItemFrame(BattleItem* battle_item, uint32_t width,
                               uint32_t height);

  /* Creates a Frame for a given BattleSkill */
  SDL_Texture* createSkillFrame(BattleSkill* battle_skill, uint32_t width,
                                uint32_t height);

  /* Rendering functions */
  bool renderActionTypes(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  bool renderItems(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
  bool renderSkills(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

  /* Clears the information in the menu - for a new person/turn */
  void clear();

  /* Constructs other battle menu data (like the QD frame) */
  bool buildData();

  /* Sets the battle menu ready for use for a new person */
  void ready();

  /* Key press evnet for menu operation */
  bool keyDownEvent(KeyHandler& key_handler);

  /* Returns the current actor to the menu */
  BattleActor* getActor();

  /* Return the value of a given BattleMenuState flag */
  bool getFlag(const BattleMenuState& test_flag);

  /* Current enumerated menu layer */
  BattleMenuLayer getMenuLayer();

  /* Ptr to selected BattleSkill */
  BattleSkill* getSelectedBattleSkill();

  /* Ptr to selected battle item */
  BattleItem* getSelectedBattleItem();

  /* Returns the type of action selected */
  ActionType getSelectedType();

  /* Returns a vector of returning hovered/selected targets */
  std::vector<BattleActor*> getTargetsHovered();
  std::vector<BattleActor*> getTargetsSelected();

  /* Assign a new BattleActor for the BattleMenu */
  bool setActor(BattleActor* actor);

  /* Assigns the Renderer */
  bool setConfig(Options* config);

  /* Assigns the DisplayData object which holds various constructed frames */
  bool setDisplayData(BattleDisplayData* battle_display_data);

  /* The event handler */
  bool setEventHandler(EventHandler* event_handler);

  /* Assigns a BattleMenuState flag a given value */
  void setFlag(BattleMenuState flags, const bool& set_value = true);

  /* Set up the hover targets */
  void setHoverTargets();

  /* Assigns the Renderer of BattleMenu elements */
  bool setRenderer(SDL_Renderer* renderer);

  /* Assigns selectable action types */
  void setSelectableTypes(std::vector<ActionType> valid_action_types);

  /* Assigns selectable skills */
  void setSelectableSkills(std::vector<BattleSkill*> menu_skills);

  /* Assigns selectable items */
  void setSelectableItems(std::vector<BattleItem*> menu_items);

  /* Assigns an enumerated window status value to the BattleMenu */
  void setWindowStatus(WindowStatus status_window);

  /* Constructs all ItemFrames for the current BattleItems */
  bool createItemFrames(uint32_t width_left, uint32_t width_right);

  /* Constructs all SkillFrames for the current BattleSkills */
  bool createSkillFrames(uint32_t width_left, uint32_t width_right);

  /* Render the battle menu */
  bool render();

  /*===================== PUBLIC STATIC FUNCTIONS ============================*/
private:
  static BattleActor* nextMenuIndex(int32_t curr,
                                    std::vector<BattleActor*> selectable);
  static BattleActor* prevMenuIndex(int32_t curr,
                                    std::vector<BattleActor*> selectable);
};

#endif // BATTLEMENU_H
