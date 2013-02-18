/******************************************************************************
* Class Name: Skill
* Date Created: February 17th, 2013
* Inheritance: QWidget
* Description: Skill class is essentially a container for actions. Persons and
*              items should have vectors of skills, where a skill can be
*              one or more actions, such as a posion action or a damage action
*              for use on map, in battle, or in menu etc. The action class will
*              parse individual actions, and skills will be tied to each of
*              these actions. This way there is no need for skills with
*              different names but similar purposes to create more than one
*              action that does the same thing.
******************************************************************************/


#include "Game/Player/Skill.h"

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/*
 * Description: Constructs a default skill object
 *
 * Inputs: QString - name of the skill
 */
Skill::Skill(QString name)
{
  setName(name);
  qd_cost = 0;
  description = "";
  using_message = "";
}

/*
 * Description: Constructs a skill object based off a list of actions
 *
 * Inputs: QString - name of the skill
 *         QVector<Action> - vector of actions to base the skill off
 */
Skill::Skill(QString name, QVector<Action*> effect_list, QVector<float> effect_chance_list)
{
    setName(name);
    setQdCost(0);
    setDescription("");
    setUsingMessage("");
    for (int i = 0; i < effect_list.size(); i++)
    {
        addEffect(effect_list[i]);
        addEffectChance(effect_chance_list[i]);
    }
}

/*
 * Description: Annihilates a Skill object
 */
Skill::~Skill()
{
  delete animation;
  delete sound_effect;
  for (int i = 0; i < effects.size(); i++)
      effects.remove(i);
  animation = NULL;
  sound_effect = NULL;
}

/*============================================================================
 * FUNCTIONS
 *===========================================================================*/

/*
 * Description: Appends an effect chance to the vector
 *
 * Inputs: float - new value of chance (1 = 100%)
 * Output: none
 */
const bool Skill::addEffectChance(float new_value)
{
  if (effect_chance.size() + 1 < kMAX_ACTIONS)
  {
    effect_chance.push_back(new_value);
    return TRUE;
  }
  return FALSE;
}

/*
 * Description: Removes an effect chance from the vector
 *
 * Notes: The effect at the same index should also be removed!
 * Inputs: uint - index of effect to be removed.
 * Output: none
 */
const bool Skill::removeEffectChance(uint index)
{
  if (effect_chance.at(index))
  {
    effect_chance.remove(index);
    return TRUE;
  }
  return FALSE;
}

/*
 * Description: Adds a skill effect to the vector
 *
 * Inputs: Action* - action to be added
 * Output: none
 */
const bool Skill::addEffect(Action* new_action)
{
  if (effects.size() + 1 < kMAX_ACTIONS)
  {
    effects.push_back(new_action);
    return TRUE;
  }
  return FALSE;
}

/*
 * Description: Removes an effect chance from the vector
 *
 * Inputs: uint - index of the effect to be removed
 * Output: none
 */
const bool Skill::removeEffect(uint index)
{
  if (effects.at(index))
  {
    effects.remove(index);
    return TRUE;
  }
  return FALSE;
}

/*
 * Description: Toggles a SkillType flag
 *
 * Inputs: SkillType - flag to be toggled
 * Output: none
 */
void Skill::toggleFlag(SkillType flags)
{
  setFlag(flags, !getFlag(flags));
}

/*
 * Description: Gets a SkillType flag
 *
 * Inputs: SkillType - flag to be returned
 * Output: Boolean evaluation of the flag
 */
const bool Skill::getFlag(SkillType flags)
{
  return skill_flags.testFlag(flags);
}

/*
 * Description: Returns the description of the string.
 *
 * Inputs: None
 * Output: QString - string of the description
 */
QString Skill::getDescription()
{
  return description;
}

/*
 * Description: Returns the name of the skill.
 *
 * Inputs: None
 * Output: QString - name of the skill
 */
QString Skill::getName()
{
  return name;
}

/*
 * Description: Returns the using message of the skill.
 *
 * Inputs: none
 * Output: QString - the using message
 */
QString Skill::getUsingMessage()
{
  return using_message;
}

/*
 * Description: Returns the effects vector of the skill
 *
 * Inputs: none
 * Output: Vector of action pointers - the actions of the skill
 */
QVector<Action*> Skill::getEffects()
{
  return effects;
}

/*
 * Description: Returns the effect chance vector of the skill
 *
 * Inputs: none
 * Output: Vector of float pointers - the chances of the skill
 */
QVector<float> Skill::getEffectChance()
{
  return effect_chance;
}

/*
 * Description: Returns a specific action pointer to an effect
 *
 * Inputs: uint - index of action to be returned
 * Output: none
 */
Action* Skill::getEffectIndex(uint index)
{
    return effects[index];
}

/*
 * Description: Returns a specific float to an effect chance
 *
 * Inputs: uint - index of the effect chance to be returned
 * Output: none
 */
float Skill::getEffectChance(uint index)
{
  if (effect_chance.at(index))
    return effect_chance[index];
}

/*
 * Description: Returns the QD cost of the skill
 *
 * Inputs: none
 * Output: uint - QD cost of the skill
 */
uint Skill::getQdValue()
{
  return qd_cost;
}

/*
 * Description: Sets the animation sprite pointer
 *
 * Inputs: Sprite* - pointer to the new sprite
 * Output: none
 */
void Skill::setAnimation(Sprite* new_sprite)
{
  animation = new_sprite;
}

/*
 * Description: Sets the effect chance vector
 *
 * Inputs: new_value - new vector of values.
 * Output: none
 */
void Skill::setEffectChances(QVector<float> new_value)
{
  effect_chance.clear();
  for (uint i = 0; i < effect_chance.size(); i++)
    effect_chance.push_back(new_value[i]);
}

/*
 * Description: Sets the description of the skill
 *
 * Inputs: new_value - new description
 * Output: none
 */
void Skill::setDescription(QString new_value)
{
  description = new_value;
}

/*
 * Description: Sets a given skill flag
 *
 * Inputs: SkillFlag - flag to be set to
 *         set_value - value flag is to be set to
 * Output: none
 */
void Skill::setFlag(SkillFlags flags, const bool set_value)
{
  (set_value) ? (skill_flags |= flags) : (skill_flags ^= flags);
}

/*
 * Description: Sets the name of the skill
 *
 * Inputs: new_value - value to set the name to
 * Output: none
 */
void Skill::setName(QString new_value)
{
  if (new_value.length() <= kMAX_NAME_LENGTH)
    name = new_value;
  else
    name = "INVALID NAME LENGTH";
}

/*
 * Description: Sets the QD cost value
 *
 * Inputs: new_value - qd cost value to be set
 * Output: none
 */
void Skill::setQdCost(uint new_value)
{
  if (qd_cost <= kMAX_QD_COST)
    qd_cost = new_value;
  else
    qd_cost = kMAX_QD_COST;
}

/*
 * Description: Sets the sound effect pointer
 *
 * Inputs: Sound* - new sound pointer
 * Output: none
 */
void Skill::setSoundEffect(Sound* new_sound)
{
  sound_effect = new_sound;
}

/*
 * Description: Sets the using message of the skill
 *
 * Inputs: new_value - using message of the skill
 * Output: none
 */
void Skill::setUsingMessage(QString new_value)
{
  using_message = new_value;
}

