/*******************************************************************************
* Class Name: Player [Implementation]
* Date Created: January 18th, 2014
* Inheritance: none
* Description: A Player contains the information of the Parties that are usable
*              by the player and represents pertinent information to the
*              progression of the game.
*
* Notes
* -----
*
* [1]:
*
* See .h file for TODOs
*******************************************************************************/

#include "Game/Player/Player.h"

/*=============================================================================
 * CONSTANTS - See header declaration for details
 *============================================================================*/

const uint32_t Player::kDEFAULT_GRAVITY{981};
const uint32_t Player::kSTARTING_CREDITS{100};
const uint32_t Player::kMAX_CREDITS{3141592654};

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description: Player construction! Constructs a player with a Sleuth and
 *              Bearacks party pointers.
 *
 * Inputs: Party* sleuth - pointer to the sleuth party for the player.
 *         Party* bearacks - pointer to the bearacks party for the player.
 */
Player::Player(Party* sleuth, Party* bearacks)
    : sleuth{sleuth},
      bearacks{bearacks},
      player_sex{Sex::FEMALE},
      player_name{StringDb::kDEFAULT_PLAYER},
      credits{kSTARTING_CREDITS},
      gravity{kDEFAULT_GRAVITY},
      steps{0}
{
  play_time = {0, 0, 0};
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Attempts to add an amount of credits to the player's total.
 *
 * Inputs: uint32_t value - the number of credits to add
 * Output: bool - true if credits added (even if max limit reached)
 */
bool Player::addCredits(const uint32_t& value)
{
  if(value < kMAX_CREDITS)
  {
    if(kMAX_CREDITS - value >= credits)
      credits += value;
    else
      credits = kMAX_CREDITS;

    return true;
  }

  return false;
}

/*
 * Description: Attempts to add a learned skill to the given party type with a
 *              required level. The party types are "sleuth" and "bearacks".
 *
 * Inputs: std::string party_type - the party type: sleuth or bearacks
 *         Skill* skill - the skill to attempt to load
 *         uint32_t person_index - the person index within the party. Default 0
 *         uint32_t req_level - the level required for the person to use
 * Output: bool - true if the skill was added to the learned set
 */
bool Player::addLearnedSkill(std::string party_type, Skill* skill,
                             const uint32_t& person_index,
                             const uint32_t& req_level)
{
  if(!skill)
  {
    Party* ref_party = nullptr;
    if(party_type == "sleuth")
      ref_party = sleuth;
    else if(party_type == "bearacks")
      ref_party = bearacks;

    if(ref_party && !ref_party->getMember(person_index))
    {
      return ref_party->getMember(person_index)
                 ->getLearnedSet(true)
                 ->addSkill(skill, req_level);
    }
  }

  return false;
}

/*
 * Description: Adds play time in milliseconds to the total time played of the
 *              player.
 *
 * Inputs: const uint32_t &milliseconds - the delta time in milliseconds
 * Output: none
 */
void Player::addPlayTime(const uint32_t& milliseconds)
{
  play_time.milliseconds += milliseconds;

  if(play_time.milliseconds >= 60000)
  {
    play_time.minutes += (play_time.milliseconds / 60000);
    play_time.milliseconds = (play_time.milliseconds % 60000);

    if(play_time.minutes >= 60)
    {
      play_time.hours += (play_time.minutes / 60);
      play_time.minutes = (play_time.minutes % 60);
    }
  }
}

/*
 * Description: Adds steps onto the total that the player has traversed in
 *              number of tiles. Triggered on save or changed map.
 *
 * Inputs: const uint32_t &value - the value of steps in tiles to add
 * Output: none
 */
void Player::addSteps(const uint32_t& value)
{
  steps += value;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
/* Returns the amount of credits (money) the player has */
uint32_t Player::getCredits()
{
  return credits;
}

/*
 * Description:
 *
 * Inputs:
 * Output:
 */
/* Gets the bearacks party */
Party* Player::getBearacks()
{
  return bearacks;
}

/*
 * Description: REturns the cu
 *
 * Inputs:
 * Output:
 */
/* Returns the carry weight */
double Player::getCarryWeight()
{
  if(sleuth->getInventory())
    return gravity * sleuth->getInventory()->getMass();

  return 0.0;
}

/*
 * Description: Returns the gravity the player is currently experiencing.
 *
 * Inputs: none
 * Output: uint32_t - the gravity the player is experiencing
 */
uint32_t Player::getGravity()
{
  return gravity;
}

/*
 * Description: Returns the play time structure of hours, minutes, and
 *              milliseconds of the player.
 *
 * Inputs: none
 * Output: TimeStore - the time storage structure
 */
TimeStore Player::getPlayTime()
{
  return play_time;
}

/*
 * Description: Returns the play time string formatted in Days:Hours:Minutes.
 *
 * Inputs: none
 * Output: std::string - the string output of time for printing, etc
 */
std::string Player::getPlayTimeStr()
{
  std::string time_str = "";

  return time_str;

  if(play_time.hours >= 24)
  {
    time_str += std::to_string(play_time.hours / 24) + "d:";
    time_str += std::to_string(play_time.hours % 24) + "h:";
  }
  else
  {
    time_str += std::to_string(play_time.hours) + "h:";
  }

  time_str += std::to_string(play_time.minutes) + "m";
}

/*
 * Description: Returns the pointer to the sleuth party.
 *
 * Inputs: none
 * Output: Party* - pointer to the sleuth party.
 */
Party* Player::getSleuth()
{
  return sleuth;
}

/*
 * Description: Returns the number of tile steps the player has executed on all
 *              maps.
 *
 * Inputs: none
 * Output: uint32_t - the number of tile steps
 */
uint32_t Player::getSteps()
{
  return steps;
}

/*
 * Description: Loads the data from file associated with the player.
 *
 * Inputs: XmlData data - the xml data structure
 *         int index - the element reference index
 *         SDL_Renderer* renderer - the rendering engine
 *         std::string base_path - the base path for file handling
 * Output: bool - true if load was successful
 */
bool Player::loadData(XmlData data, int index, SDL_Renderer* renderer,
                      std::string base_path)
{
  bool success = true;

  if(data.getElement(index) == "bearacks")
  {
    if(bearacks)
      success &= bearacks->loadData(data, index + 1, renderer, base_path);
    else
      success = false;
  }
  else if(data.getElement(index) == "credits")
  {
    success &= setCredits(data.getDataInteger(&success));
  }
  else if(data.getElement(index) == "gravity")
  {
    success &= setGravity(data.getDataInteger(&success));
  }
  else if(data.getElement(index) == "playtime")
  {
    if(data.getElement(index + 1) == "hours")
    {
      setPlayTime(data.getDataInteger(&success), play_time.minutes,
                  play_time.milliseconds);
    }
    else if(data.getElement(index + 1) == "minutes")
    {
      setPlayTime(play_time.hours, data.getDataInteger(&success),
                  play_time.milliseconds);
    }
    else if(data.getElement(index + 1) == "milliseconds")
    {
      setPlayTime(play_time.hours, play_time.minutes,
                  data.getDataInteger(&success));
    }
  }
  else if(data.getElement(index) == "sex")
  {
    // TODO: Set player sex [08-21-16]
    // success &= player_sex =
    // Helpers::sexFromStr(data.getDataString(&success));
  }
  else if(data.getElement(index) == "sleuth")
  {
    if(sleuth != nullptr)
      success &= sleuth->loadData(data, index + 1, renderer, base_path);
    else
      success = false;
  }
  else if(data.getElement(index) == "steps")
  {
    setSteps(data.getDataInteger(&success));
  }

  return success;
}

/*
 * Description: Removes a bearacks member by index by calling the Paty's
 *              remove function.
 *
 * Inputs: uint32_t index the index of the bearacks member to be removed.
 * Output: bool - true if the removal occurs.
 */
bool Player::removeBearacksMember(const uint32_t& index)
{
  return bearacks->removeMember(index);
}

/*
 * Description: Removes a bearacks member by name by calling Party's remove
 *              function.
 *
 * Inputs: const std::string name - the name of the bearacks member to be rem.
 * Output: bool - true if the member was removed.
 */
bool Player::removeBearacksMember(const std::string& name)
{
  return bearacks->removeMember(name);
}

/*
 * Description: Attempts to remove an amount of credits to the player's total.
 *
 * Inputs: uint32_t value - the number of credits to remove
 * Output: bool - true if credits removed
 */
bool Player::removeCredits(const uint32_t& value)
{
  if(value <= credits)
  {
    credits -= value;

    return true;
  }

  return false;
}

/*
 * Description: Removes a sleuth member by index by calling its party's removal
 *              function.
 *
 * Inputs: uint32_t index - the index of sleuth member to be removed.
 * Output: bool - true if the member was removed successfully.
 */
bool Player::removeSleuthMember(const uint32_t& index)
{
  return sleuth->removeMember(index);
}

/*
 * Description: Removes a sleuth member by name by calling Party's remove
 *              function.
 *
 * Inputs: const std::string name - the name of the sleuth member to remo.
 * Output: bool - true if the member was removed.
 */
bool Player::removeSleuthMember(const std::string& name)
{
  return sleuth->removeMember(name);
}

/*
 * Description: Resets the play time of the player back to 0 on all fronts
 *
 * Inputs: none
 * Output: none
 */
void Player::resetPlayTime()
{
  play_time = {0, 0, 0};
}

/*
 * Description: Saves the data of this player to the file handler pointer.
 *
 * Inputs: FileHandler* fh - the saving file handler
 * Output: bool - true if successful
 */
bool Player::saveData(FileHandler* fh)
{
  if(fh)
  {
    fh->writeXmlElement("player");

    /* Write class data */
    fh->writeXmlData("credits", credits);
    if(gravity != kDEFAULT_GRAVITY)
      fh->writeXmlData("gravity", gravity);
    fh->writeXmlElement("playtime");
    fh->writeXmlData("hours", play_time.hours);
    // fh->writeXmlData("sex", Helpers::sexToStr(player_sex));
    // fh->writeXmlData("name", player_name);
    fh->writeXmlData("minutes", play_time.minutes);
    fh->writeXmlData("milliseconds", play_time.milliseconds);
    fh->writeXmlElementEnd();
    fh->writeXmlData("steps", steps);

    /* Write parties */
    if(sleuth)
      sleuth->saveData(fh, "sleuth");
    if(bearacks)
      bearacks->saveData(fh, "bearacks");

    fh->writeXmlElementEnd();

    return true;
  }

  return false;
}

/*
 * Description: Assigns a pointer to the Bearacks party.
 *
 * Inputs: Party* pointer to the bearacks party.
 * Output: none
 */
void Player::setBearacks(Party* const new_bearacks)
{
  bearacks = new_bearacks;
}

/*
 * Description: Assigns the amount of money the player has.
 *
 * Inputs: uint32_t value - the amoutn of credtis the player has.
 * Output: bool - true if the credit assignment was kosher.
 */
bool Player::setCredits(const uint32_t& value)
{
  if(value < kMAX_CREDITS)
  {
    credits = value;

    return true;
  }

  return false;
}

/*
 * Description: Assigns a new value to gravity for the gravity that the player
 *              is experiencing.
 *
 * Inputs: uint32_t new_value - new value for gravity for the player.
 * Output: bool - true if the assignment was successful.
 */
bool Player::setGravity(const uint32_t& new_value)
{
  gravity = new_value;

  return true;
}

/*
 * Description: Assigns the string name to be assigned for the player
 *
 * Inputs: std::string player_name - assigns the player name
 * Output: none
 */
void Player::setName(std::string player_name)
{
  this->player_name = player_name;

  if(sleuth && sleuth->getMember(0))
    sleuth->getMember(0)->setName(this->player_name);
}

/*
 * Description: Assigns the enumerated Sex of the Player, for rendering of the
 *              Sprites.
 *
 * Inputs: Sex player_sex - enumerated sexual genital formations owned by Player
 * Output: none
 */
void Player::setSex(Sex player_sex)
{
  this->player_sex = player_sex;
}

/*
 * Description: Sets the total time played by the Player. This overwrites all
 *              existing information.
 *
 * Inputs: uint32_t hours - the hours played
 *         uint32_t minutes - the minutes played
 *         uint32_t milliseconds - the milliseconds played
 * Output: none
 */
void Player::setPlayTime(const uint32_t& hours, const uint32_t& minutes,
                         const uint32_t& milliseconds)
{
  play_time.hours = hours;
  play_time.minutes = minutes;
  play_time.milliseconds = milliseconds;
}

/*
 * Description: Assigns the Sleuth party.
 *
 * Inputs: Party* new_slueth - new sleuth party.
 * Output: none
 */
void Player::setSleuth(Party* const new_sleuth)
{
  sleuth = new_sleuth;
}

/*
 * Description: Sets the number of steps in tiles the player has traversed.
 *
 * Inputs: const uint32_t &new_value - the complete count of steps for player
 * Output: none
 */
void Player::setSteps(const uint32_t& new_value)
{
  steps = new_value;
}

/*=============================================================================
 * PUBLIC STATIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Returns the default value for gravity.
 *
 * Inputs: none
 * Output: uint32_t - the default level of gravity.
 */
uint32_t Player::getDefaultGravity()
{
  return kDEFAULT_GRAVITY;
}

/*
 * Description: Returns the starting credits for the Player.
 *
 * Inputs: none.
 * Output: uint32_t - the starting amount of credits for the player
 */
uint32_t Player::getStartingCredits()
{
  return kSTARTING_CREDITS;
}

/*
 * Description: Returns the maximum possible credits that a Player may have.
 *
 * Inputs: none
 * Output: uint32_t - the maximum possible value for credits
 */
uint32_t Player::getMaxCredits()
{
  return kMAX_CREDITS;
}
