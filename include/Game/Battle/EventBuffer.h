/*******************************************************************************
* Class Name: EventBuffer [Declaration]
* Date Created: January 11, 2014
* Inheritance: None
* Description: The EventBuffer is a Buffer structure for Battle which creates,
*              stores, and clears events for both rendering and performing
*              during the processing action phase of battle.
*
* Notes
* -----
*
* [1]:
*
* TODO
* ----
*****************************************************************************/
#ifndef EVENTBUFFER_H
#define EVENTBUFFER_H

#include <algorithm>

#include "EnumDb.h"
#include "Game/Player/Item.h"
#include "Game/Player/Skill.h"
#include "Game/Player/Person.h"

/* A BattleEvent - an event during a battle which requires rendering and
 * performing */
struct BattleEvent
{
  EventType type;
  Action* action_use;
  Item*  item_use;
  Skill* skill_use;
  Person* user;
  std::vector<Person*> targets;
  int32_t amount;

  bool happens;
  bool rendered;
  bool performed;
};

class EventBuffer
{
public:
  /* Generic EventBuffer instantiation function */
  EventBuffer();

  /* Annihilates an EventBuffer */
  ~EventBuffer();

private:
  /* Vector of pointers to BattleEvent objects */
  std::vector<BattleEvent*> events;
  
  /* Is the EventBuffer active? */
  bool active;

  /* Current processing index */
  int32_t curr_index;

  /* ------------ Constants --------------- */

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* Is a given index null? */
  bool isNullptr(BattleEvent* check_event);

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/
public:
  /* Clears and frees all BattleEvents */
  void clearAll();
  void clearCurrent();
  void clearRendered();

  /* Creates a new blank event and returns the constructed event */
  BattleEvent* createDamageEvent(EventType damage_type, Person* target,
      int32_t amount);

  /* Creates a death event and returns the constructd object */
  BattleEvent* createDeathEvent(EventType death_type, Person* target);

  /* Creates a defend (begin/persist/break) event and returns a pointer */
  BattleEvent* createDefendEvent(EventType defend_type, Person* user);

  /* Creates a guard event and returns a pointer to the event */
  BattleEvent* createGuardEvent(EventType guard_type, Person* user,
      Person* target);

  /* Creates a miss event and returns a pointer */
  BattleEvent* createMissEvent(EventType miss_type, Person* user,
      std::vector<Person*> targets);

  /* Creates and returns an empty BattleEvent */
  BattleEvent* createNewEvent();

  /* Creates an action event */
  BattleEvent* createActionEvent(Action* action_use, Skill* skill_use,
      Person* user, Person* target, bool happens);

  /* Creaes a Skill Use event with given information */
  BattleEvent* createSkillEvent(Skill* skill_use, Person* user,
      std::vector<Person*> targets, bool happens);

  /* Creates a Skill Use Fizzle event */
  BattleEvent* createFizzleEvent(EventType fizzle_type, Person* user,
      std::vector<Person*> targets);

  /* Returns whether the EventBuffer is active */
  bool isActive();

  /* Prints out all the current events on the Buffer */
  void print(bool only_current);

  /* Prints an event at a given index */
  bool printEvent(uint32_t index);

  /* Returns a pointer to the most current battle event or nullptr */
  BattleEvent* getCurrentEvent();

  /* Returns the current index (first non-performed index) */
  int32_t getCurrentIndex();

  /* Returns the size of the event buffer */
  uint32_t getCurrentSize();

  /* Returns a pointer to the event at a given index */
  BattleEvent* getEvent(int32_t index);

  /* Sets the EventBuffer to active */
  void setActive();

  /* Sets the index to the oldest non-performed index */
  bool setCurrentIndex();

  /* Sets the EventBuffer to inactive */
  void setInactive();

  /* Assigns a new index for event performing/rendering */
  bool setIndex(int32_t new_index);

  /* Attempts to set the next index, returns true if successful */
  bool setNextIndex();

  /* Sets the given index to performed */
  bool setPerformed(int32_t index);

  /* Sets the index to the oldest non-rendered index */
  bool setRenderIndex();

  /* Sets the given index to rendered */
  bool setRendered(int32_t index);
};

#endif //EVENTBUFFER_H