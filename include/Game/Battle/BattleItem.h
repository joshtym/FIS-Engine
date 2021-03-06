/*******************************************************************************
* Class Name: BattleItem
* Date Created:
* Inheritance: None
*
* Description
* -----------

*
* Notes
* -----
*
* [1]:
*
* TODO
* ----
*******************************************************************************/
#ifndef BATTLE_ITEM_H
#define BATTLE_ITEM_H

class BattleActor;

#include "Game/Battle/BattleActor.h"
#include "Game/Player/Item.h"

/* Structure representing an item and its amt and possible targets */
class BattleItem
{
public:
  /* Construct a BattleItem object */
  BattleItem();

  /* The validity of this BattleItem choice */
  ValidStatus valid_status;

  /* Pointer to the item which would be chosen */
  Item* item;

  /* The # of items of this type available */
  uint32_t amount;

  /* The valid targets for the valid item */
  std::vector<BattleActor*> targets;

public:
  /* Is the BattleItem defensive? */
  bool isDefensive();

  /* Is the BattleItem offensvie? */
  bool isOffensive();

  /* Load the animation data for the Skill */
  bool loadData(SDL_Renderer* renderer);

  /* Print out the BattleItem information */
  void print();
};

#endif
