/*******************************************************************************
* Class Name: Bubby
* Date Created: November 4th, 2012 (Rewritten March 6th, 2013)
* Inheritance: Item
* Description: The Bubby specification under Item that adds the extra
*              details to define experience, level cap, BubbyFlavour, etc.
*******************************************************************************/
#ifndef BUBBY_H
#define BUBBY_H

#include <QtGui/QWidget>
#include "Game/Player/BubbyFlavour.h"
#include "Game/Player/SkillSet.h"
#include "Game/Player/Item.h"

class Bubby : public Item
{
public:
  /* Constructor function */
  Bubby(BubbyFlavour* type);

  /* Destructor function */
  ~Bubby();

private:
  /* ------------ Constants --------------- */
  const static ushort kTIER_CAP  =  3;
  const static ushort kTIER1_LVL =  9;
  const static ushort kTIER2_LVL = 19;
  const static ushort kLEVEL_CAP = 20;
  const static uint kMIN_LVL_EXP =       75;
  const static uint kMAX_LVL_EXP =   450000;
  const static uint kMAX_EXPERIENCE = 1000000;

  /* Calculate the experience table for Bubbies */
  static void calcExpTable();

  /* Updates the Bubby to the appropriate sprite (on tier level up) */
  const bool setSprite();

  /* Bubby Type */
  BubbyFlavour* type;

  /* Static ID data, and current object's ID */
  static int id;
  int my_id;

  /* The experience table for Bubbies */
  static QVector<uint> exp_table;

  /* Bubby's Experience, Level, and Tier */
  uint total_exp;
  ushort level;
  ushort tier;

  /* Pointer to the current sprite */
  Sprite* current_sprite;

public:
  /* Adds experience to the Bubby */
  void addExperience(uint amount);

  /* Gets the bubbies Id */
  int getId();

  /* Gets the bubbies exp */
  uint getExp();

  /* Gets the exp required at a given Bubby level */
  uint getExpAt(ushort level);

  /* Gets the bubbies level */
  ushort getLevel();

  /* Gets the current sprite for the tier */
  Sprite* getSprite();

  /* Gets the bubbies tier */
  ushort getTier();

  /* Gets the type of the Bubby */
  BubbyFlavour* getType();

  /* Increments the bubbie's Id */
  static int setId();

  /* Sets the exp of the bubby based on use in battle */
  void setExperience(uint new_experience);

  /* Sets the level of the bubby based on exp amounts */
  void setLevel(ushort new_level);

  /* Sets the tier of the bubby based on leveling */
  void setTier(ushort new_tier);

  /* Sets the type of the Bubby */
  void setType(BubbyFlavour* new_type);
};

#endif // BUBBY_H
