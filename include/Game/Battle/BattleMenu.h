/*******************************************************************************
* Class Name: BattleMenu [Declaration]
* Date Created: April 13th, 2018
* Inheritance: None
* Description: 
*
* Notes
* -----
*
* [1]: [1: Action Type Layer] - [2: Selection Layer] -    [3: Target Layer]
*      -------------------     ------------------------   -----------------
*      <Skill>                 --- <Select skill> ------  <Skill dependable>
*      <Item>                  --- <Select items> ------  <Item dependable>
*      <Defend>                -------------------------  <Self target>
*      <Guard>                 -------------------------  <Ally Non-self target>
*      <Implode>               -------------------------  <Self>
*      <Run>                   -------------------------  ----------------- 
*      <Pass>                  -------------------------  -----------------
*
*      [4: Verification Layer]
*      -----------------------
*      All -> Confirm [Y/N]
*******************************************************************************/
#ifndef BATTLEMENU_H
#define BATTLEMENU_H

#include "EnumFlags.h"
#include "EnumDb.h"
#include "Options.h"
#include "Game/Player/Person.h"
#include "Game/Player/Item.h"
#include "Game/Player/SkillSet.h"

#include <vector>

using std::begin;
using std::end;

ENUM_FLAGS(BattleMenuState)
enum class BattleMenuState
{
  ACTION_SELECTED    = 1 << 0,
  TARGETS_ASSIGNED   = 1 << 1,
  SCOPE_ASSIGNED     = 1 << 2,
  SELECTION_VERIFIED = 1 << 3
};

class BattleMenu
{
public:
  /* BattleMenu */
  BattleMenu(Options* running_config);

private:
  /* The QTDR cost paid for a selected action */
  int32_t qtdr_cost_paid;

  /* Valid action types for the current person index */
  std::vector<ActionType> valid_actions;

  /* The current selectable items on the menu */
  std::vector<std::pair<Item*, uint16_t>> menu_items;

  /* Current skill set for Skills to be chosen from */
  SkillSet* menu_skills;

  /* Remaining valid targets that can be chosen and already chosen targets */
  std::vector<int32_t> valid_targets;
  std::vector<int32_t> selected_targets;

  /* The type of the action that was chosen (if selection_completed) */ 
  ActionType action_type;

  /* The scope of the action */
  ActionScope action_scope;

  /* Currently assigned options */
  Options* config;

  /* Potentially selected object pointers */
  Skill* selected_skill;
  Item*   selected_item;

  /* Set of BattleMenuState flags */
  BattleMenuState flags;

  /* The window status of the BattleMenu */
  WindowStatus window_status;
  
  /* Pointer to the current user of the Battle menu */
  Person* current_user;

  /* Menu indexes */
  int32_t person_index;
  int32_t layer_index;
  int32_t element_index;

  /* ------------ Constants --------------- */

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* Decrement the current menu layer */
  bool decrementLayer(const int32_t &new_layer_index);

  /* Increment the current menu layer */
  bool incrementLayer(const int32_t &new_layer_index);

  /* Adding and removing target selections */
  bool addTarget(const int32_t &new_target);
  bool addPartyTargets(const int32_t &party_index);
  bool removeLastTarget(const bool &clear_all = false);

  /* Methods for containing code for each key addition */
  void keyDownAlpha(const char &c);
  void keyDownCancel();
  void keyDownDecrement();
  void keyDownIncrement();
  void keyDownSelect();

  /* Compiles and returns a vector of valid actions for the current user */
  std::vector<ActionType> getValidActions();

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/
public:
  /* Unset all BattleMenu information (for end of selection, etc) */
  void unsetAll();

  /* Returns the state of a chosen type of action */
  bool isActionTypeSelected();

  /* Returns the state of the current menu selection */
  bool isActionSelected();

  /* Resets the menu data to be used for a new Person */
  void reset(Person* const new_user, const uint32_t &new_person_index);

  /* Layer 2 printing of items to choose from */
  void printItems();

  /* Prints out the state of the menu */
  void printMenuState();

  /* Layer 1 printing of skills to choose from */
  void printSkills();

  /* Prints out the list of (selected and) valid targets to choose from */
  void printTargets(const bool &print_selected = false);

  /* Layer 0 printing of actions to choose from */
  void printValidActions();

  /* Key press evnet for menu operation */
  bool keyDownEvent(SDL_KeyboardEvent event);

  /* Obtains the selected enumerated ActionType */
  ActionType getActionType();
  
  /* Obtains the index of action (Skill or Item lists) chosen */
  int32_t getActionIndex();

  /* Obtains user selected targets for the action from the Menu */
  std::vector<int32_t> getActionTargets();

  /* Finds the maximum index for the current layer */
  int32_t getMaxIndex();

  /* Return the value of a given BattleMenuState flag */
  bool getMenuFlag(const BattleMenuState &test_flag);

  /* Returns the current selectable skills on the menu */
  SkillSet* getMenuSkills();

  /* Returns the current selectable items on the menu */
  std::vector<std::pair<Item*, uint16_t>> getMenuItems();

  /* The currently selected Skill (if set) */
  Skill* getSelectedSkill();

  /* The currently selected Item (if set) */
  Item* getSelectedItem();

  /* Returns the window status of the BattleMenu */
  WindowStatus getWindowStatus();

  /* Assigns the scope of the skill when a skill has been chosen */
  void setActionScope(const ActionScope &new_action_scope);

  /* Assigns a new selectable list of items for the menu */
  bool setSelectableItems(std::vector<std::pair<Item*, uint16_t>> new_menu_items);

  /* Assigns valid targets for the menu */
  bool setSelectableTargets(std::vector<int32_t> valid_targets);
 
  /* Assigns the running configuration of the menu */
  bool setConfiguration(Options* new_config);

  /* Assigns a BattleMenuState flag a given value */
  void setMenuFlag(BattleMenuState flags, const bool &set_value = true);
};

#endif //BATTLEMENU_H