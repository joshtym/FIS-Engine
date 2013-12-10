/*******************************************************************************
* Class Name: Action [Implementation]
* Date Created: November 19th, 2013
* Inheritance: None
* Description: An action is an element of a skill used buy a person in a Battle.
*              A skill may have a number of actions which have various effects:
*              altering stats, flipping flags, inflict ailments/buffs. An action
*              is constructed from parsing a string from a file. Actions have
*              unique IDs.
*
* Notes
* -----
*
* [1]: The Action is parsed from a string with convention as follows:
*
* [ID],[ALTER/INFLICT/RELIEVE/ASSIGN],[ATTRIBUTE/AILMENT],[MIN].[MAX],
* [IGNORE ATK ELEMENT 1].[IGNORE ATK ELEMENT 2]...,
* [IGNORE DEF ELEMENT 1].[IGNORE DEF ELEMENT 2]...,
* [AMOUNT/PC].[BASE],[AMOUNT/PC].[VARIANCE]
*
* Where:
*   - ID - the unique ID that represents the action
*   - ALTER/INFLICT/RELIEVE/ASSIGN/REVIVE keywords - see ActionFlags enum
*   - ATTRIBUTE/AILMENT - the affected attribute or ailment by the key words
*   - MIN.MAX - the duration an inflicted ailment will persist
*   - IGNORE ATK ELMENTs - a list of valid elements which when set, will not
*                          include the action user's corresponding offensive
*                          elemental statistics into battle calculations.
*                         - can use ALL for all elements or ELEMENTAL for
*                           everything non-physical
*   - IGNORE DEF ELEMENTs - a list of valid elements which when set, will not
*                           include the action target's corresponding defensive
*                           elemental statistics into battle calculations.
*                         - can use ALL for all elements or ELEMENTAL for
*                           everything non-physical
*   - BASE - the base power of the action (amount an attribute is affected)\
*          - negative values will be used only when the ALTER key word is set
*   - AMOUNT/PC - decides between utilizing base and variance as an amount 
*                 (+ or -) value or as a factor (%) value
*   - VARIANCE - the variance (even distribution) which base may change by
*              - may be set to -1 for the highest possible variance
* -----
*
* [2]: Example actions and their intended effects:
*
* 1,ALTER,THAG,,,,AMOUNT.50,AMOUNT.15
* will alter Thermal Aggression of target by 50 +/- 15
*
* 1,ALTER,VITA,,PHYSICAL,PHYSICAL.THERMAL,AMOUNT.50,AMOUNT.10
* will damage target ignoring user's phys. atk and target's phys. and ther. def
* at 50 +/- 15 points
*
* 1,INFLICT,POISON,2.7,,,,,
* will inflict poison lasting 2-7 turns
*
* 1,RELIEVE,CURSE,,,,,,
* will relieve curse
*
* 1,REVIVE,,,,,PC.25,AMOUNT.50
* will revive a KO'd target with 25% +/- 15 VITA
*
* See .h file for TODOs
*******************************************************************************/

#include "Game/Player/Action.h"

/*=============================================================================
 * CONSTANTS - See implementation for details
 *============================================================================*/

const bool         Action::kDEBUG_ENABLED = true;
const int          Action::kDEFAULT_ID  = INT_MAX;
const int          Action::kDEFAULT_MIN = 1;
const int          Action::kDEFAULT_MAX = 2;
const char         Action::kDELIMITER   = ',';
const char         Action::kDELIMITER_2 = '.';
const int          Action::kMAX_BASE_PC = 100;
const uint32_t     Action::kMAX_VARIANCE_PC = 1000;

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description: Constructs a default (invalid) Action object
 *
 * Inputs: none
 */
Action::Action()
    : action_flags(static_cast<ActionFlags>(0)),
      attribute(Attribute::NONE),
      ailment(Infliction::INVALID),
      base(0),
      id(kDEFAULT_ID),
      ignore_atk(static_cast<IgnoreFlags>(0)),
      ignore_def(static_cast<IgnoreFlags>(0)),
      min_duration(0),
      max_duration(0),
      variance(0)
{
  action_flags &= static_cast<ActionFlags>(0);
  ignore_atk   &= static_cast<IgnoreFlags>(0);
  ignore_def   &= static_cast<IgnoreFlags>(0);
}

/*
 * Description: Constructs a default Action object given a raw string to parse
 *
 * Inputs: raw - the raw string to be parsed
 */
Action::Action(const std::string &raw)
    : action_flags(static_cast<ActionFlags>(0)),
      attribute(Attribute::NONE),
      ailment(Infliction::INVALID),
      base(0),
      id(kDEFAULT_ID),
      ignore_atk(static_cast<IgnoreFlags>(0)),
      ignore_def(static_cast<IgnoreFlags>(0)),
      min_duration(0),
      max_duration(0),
      variance(0)
{
  parse(raw);
}

/*
 * Description: Annihilates an action object
 */
Action::~Action() {}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/*
 * Description: Primary method for parsing the raw string. Divides the string up
 *              into workable sections, calling sub-methods on the sub-string
 *              portions--checking for errors and along the way.
 *
 * Inputs: raw - the raw string to be parsed into an action
 * Output: bool - the final validity of the Action
 */
bool Action::parse(const std::string &raw)
{
  action_flags |= ActionFlags::VALID;
  std::vector<std::string> sub_strings = Helpers::split(raw, kDELIMITER);

  if (sub_strings.size() == 7 || sub_strings.size() == 8)
  {
  	/* Parse the ID */
    id = std::stoi(sub_strings.at(0));

  	/* Parse the initial action keyword */
    parseActionKeyword(sub_strings.at(1));

    /* ALTER and ASSIGN keywords relate to attributes */
    if (actionFlag(ActionFlags::ALTER) || actionFlag(ActionFlags::ASSIGN))
      parseAttribute(sub_strings.at(2));

    /* INFLICT and RELIEVE keywords relate to ailments */
    else if(actionFlag(ActionFlags::INFLICT)||actionFlag(ActionFlags::RELIEVE))
      parseAilment(sub_strings.at(2));
  
    /* Parse min & max durations */
    if (sub_strings.at(3) != "")
    {
      std::vector<std::string> turns =
          Helpers::split(sub_strings.at(3), kDELIMITER_2);

      if (turns.size() == 2)
        setDuration(std::stoi(turns.at(0)), std::stoi(turns.at(1)));
      else
        parseWarning("invalid duration size", raw);
    }
    else if (sub_strings.at(3) == "")
      if (actionFlag(ActionFlags::INFLICT) || actionFlag(ActionFlags::RELIEVE))
        setDuration(kDEFAULT_MIN, kDEFAULT_MAX);

    /* Parse ignore_atk flags */
    if (sub_strings.at(4) != "")
      parseIgnoreFlags(ignore_atk, sub_strings.at(4));

    /* Parse ignore def flags */
    if (sub_strings.at(5) != "")
      parseIgnoreFlags(ignore_def, sub_strings.at(5));

    /* Parse base change */
    if (sub_strings.at(6) != "")
    {
      std::vector<std::string> base_values = 
          Helpers::split(sub_strings.at(6), kDELIMITER_2);
      
      if (base_values.size() == 2)
      {
        if (base_values.at(0) == "PC")
          action_flags |= ActionFlags::BASE_PC;
        else if (base_values.at(0) != "AMOUNT")
          parseWarning("invalid base keyword", raw);
      
        base = std::stoi(base_values.at(1));
      }
      else
        parseWarning("wrong # arguments in base parse", raw);
    }
 
    /* Parse variance [check if size is right - no empty end on split() ] */
    if (sub_strings.size() == 8 && sub_strings.at(7) != "")
    {
      std::vector<std::string> variance_values = 
          Helpers::split(sub_strings.at(7), kDELIMITER_2);

      if (variance_values.size() == 2)
      {
        if (variance_values.at(0) == "PC")
          action_flags |= ActionFlags::VARI_PC;
        else if (variance_values.at(0) != "AMOUNT")
          parseWarning("invalid variance keyword", raw);

        variance = std::stoi(variance_values.at(1));
      }
      else
        parseWarning("wrong # arguments in variance parse", raw);
    }

    /* Warning Checking */
    if (actionFlag(ActionFlags::BASE_PC) && variance > kMAX_BASE_PC)
      parseWarning("base percent value higher than permitted", raw);

    if (actionFlag(ActionFlags::VARI_PC) && variance > kMAX_VARIANCE_PC)
      parseWarning("variance percent value higher than permitted", raw);
  }
  else
    parseWarning("invalid sub string size", raw);
  
  return actionFlag(ActionFlags::VALID);
}

/*
 * Description: Sub-method for parsing the sub-string containing the infliction
 *              the action may inflict or relieve. Uses string compare to assign
 *              an enumerated value. If the infliction was not found, a warning
 *              is emitted and the parse fails.
 *
 * Inputs: ailm - string containing the ailment inflicted/relieved
 * Output: bool - validity of the ailment parse
 */
bool Action::parseAilment(const std::string &ailm)
{
  if      (ailm == "POISON")      ailment = Infliction::POISON;
  else if (ailm == "BURN")        ailment = Infliction::BURN;
  else if (ailm == "SCALD")       ailment = Infliction::SCALD;
  else if (ailm == "CHARR")       ailment = Infliction::CHARR;
  else if (ailm == "BERSERK")     ailment = Infliction::BERSERK;
  else if (ailm == "CONFUSE")     ailment = Infliction::CONFUSE;
  else if (ailm == "SILENCE")     ailment = Infliction::SILENCE;
  else if (ailm == "BUBBIFY")     ailment = Infliction::BUBBIFY;
  else if (ailm == "DEATHTIMER")  ailment = Infliction::DEATHTIMER;
  else if (ailm == "PARALYSIS")   ailment = Infliction::PARALYSIS;
  else if (ailm == "BLINDNESS")   ailment = Infliction::BLINDNESS;
  else if (ailm == "DREADSTRUCK") ailment = Infliction::DREADSTRUCK;
  else if (ailm == "DREAMSNARE")  ailment = Infliction::DREAMSNARE;
  else if (ailm == "HELLBOUND")   ailment = Infliction::HELLBOUND;
  else if (ailm == "BOND")        ailment = Infliction::BOND;
  else if (ailm == "BONDED")      ailment = Infliction::BONDED;
  else if (ailm == "ALLATKBUFF")  ailment = Infliction::ALLATKBUFF;
  else if (ailm == "ALLDEFBUFF")  ailment = Infliction::ALLDEFBUFF;
  else if (ailm == "PHYBUFF")     ailment = Infliction::PHYBUFF;
  else if (ailm == "THRBUFF")     ailment = Infliction::THRBUFF;
  else if (ailm == "POLBUFF")     ailment = Infliction::POLBUFF;
  else if (ailm == "PRIBUFF")     ailment = Infliction::PRIBUFF;
  else if (ailm == "CHGBUFF")     ailment = Infliction::CHGBUFF;
  else if (ailm == "CYBBUFF")     ailment = Infliction::CYBBUFF;
  else if (ailm == "NIHBUFF")     ailment = Infliction::NIHBUFF;
  else if (ailm == "LIMBUFF")     ailment = Infliction::LIMBUFF;
  else if (ailm == "UNBBUFF")     ailment = Infliction::UNBBUFF;
  else if (ailm == "MOMBUFF")     ailment = Infliction::MOMBUFF; 
  else if (ailm == "VITBUFF")     ailment = Infliction::VITBUFF; 
  else if (ailm == "QDBUFF")      ailment = Infliction::QDBUFF;
  else if (ailm == "ROOTBOUND")   ailment = Infliction::ROOTBOUND;
  else if (ailm == "DOUBLECAST")  ailment = Infliction::DOUBLECAST;
  else if (ailm == "TRIPLECAST")  ailment = Infliction::TRIPLECAST;
  else if (ailm == "HALFCOST")    ailment = Infliction::HALFCOST;
  else if (ailm == "REFLECT")     ailment = Infliction::REFLECT;
  else if (ailm == "HIBERNATION") ailment = Infliction::HIBERNATION;
  else if (ailm == "CURSE")       ailment = Infliction::CURSE;
  else if (ailm == "METATETHER")  ailment = Infliction::METATETHER;
  else if (ailm == "STUBULATE")   ailment = Infliction::STUBULATE;

  if (ailment != Infliction::INVALID)
    return true;

  parseWarning("attempting to parse ailment", ailm);
  return false;
}

/*
 * Description: Sub-method for parsing the sub-string containing the action
 *              keyword. Uses a string compare to determine the keyword, else
 *              a warning is emitted and the parse fails.
 *
 * Inputs: action_keyword - string containing the action keyword
 * Output: bool - validity of the keyword parse
 */
bool Action::parseActionKeyword(const std::string &action_keyword)
{
  if (action_keyword == "ALTER")
    action_flags |= ActionFlags::ALTER;
  else if (action_keyword == "INFLICT")
    action_flags |= ActionFlags::INFLICT;
  else if (action_keyword == "RELIEVE")
    action_flags |= ActionFlags::RELIEVE;
  else if (action_keyword == "ASSIGN")
    action_flags |= ActionFlags::ASSIGN;
  else if (action_keyword == "REVIVE")
    action_flags |= ActionFlags::REVIVE;
  else
  {
    parseWarning("attempting to parse keyword", action_keyword);
    return false;
  }

  return true;
}

/*
 * Description: Sub-method for parsing the sub-string containing the attribute
 *              which the action may affect (via alteration or assigmment).
 *              Determination via string compare, else the method emits a
 *              warning and the parse fails.
 *
 * Inputs: attr_parse - string containing the affected attribute
 * Output: bool - validity o the attribute parse
 */
bool Action::parseAttribute(const std::string &attr_parse)
{
  if (attr_parse == "VITA") attribute = Attribute::VITA;
  if (attr_parse == "QTDR") attribute = Attribute::QTDR;
  if (attr_parse == "PHAG") attribute = Attribute::PHAG;
  if (attr_parse == "PHFD") attribute = Attribute::PHFD;
  if (attr_parse == "THAG") attribute = Attribute::THAG;
  if (attr_parse == "THFD") attribute = Attribute::THFD;
  if (attr_parse == "PRAG") attribute = Attribute::PRAG;
  if (attr_parse == "PRFD") attribute = Attribute::PRFD;
  if (attr_parse == "POAG") attribute = Attribute::POAG;
  if (attr_parse == "POFD") attribute = Attribute::POFD;
  if (attr_parse == "CHAG") attribute = Attribute::CHAG;
  if (attr_parse == "CHFD") attribute = Attribute::CHFD;
  if (attr_parse == "CYAG") attribute = Attribute::CYAG;
  if (attr_parse == "CYFD") attribute = Attribute::CYFD;
  if (attr_parse == "NIAG") attribute = Attribute::NIAG;
  if (attr_parse == "NIFD") attribute = Attribute::NIFD;
  if (attr_parse == "MMNT") attribute = Attribute::MMNT;
  if (attr_parse == "LIMB") attribute = Attribute::LIMB;
  if (attr_parse == "UNBR") attribute = Attribute::UNBR;
  if (attr_parse == "MANN") attribute = Attribute::MANN;

  if (attribute != Attribute::NONE)
    return true;

  parseWarning("attempting to parse attribute", attr_parse);
  return false;
}

/*
 * Description: Sub-method for parsing the sub-string containing the ignore
 *              flags. Passes in an IgnoreFlags parameter to be able to
 *              use the function for both atk and def flag sets.
 *
 * Inputs: flag_sets - the particular set of flags (atk or def) to assign to
 *         flags - the sub-string containing the elemental ignore flags
 * Output: none
 */
void Action::parseIgnoreFlags(IgnoreFlags& flag_set, const std::string &flags)
{
  std::vector<std::string> sub_strings = Helpers::split(flags, kDELIMITER_2);

  for (std::string s : sub_strings)
  {
    if (s == "ALL" || s == "ELEMENTAL")
    {
      flag_set |= IgnoreFlags::PHYSICAL | IgnoreFlags::THERMAL    | 
                  IgnoreFlags::POLAR    | IgnoreFlags::PRIMAL     |
                  IgnoreFlags::CHARGED  | IgnoreFlags::CYBERNETIC |
                  IgnoreFlags::NIHIL;
    }
    if (s == "ELEMENTAL")
      flag_set &= ~IgnoreFlags::PHYSICAL;
    else if (s == "PHYSICAL")
      flag_set |= IgnoreFlags::PHYSICAL;
    else if (s == "THERMAL")
      flag_set |= IgnoreFlags::THERMAL;
    else if (s == "POLAR")
      flag_set |= IgnoreFlags::POLAR;
    else if (s == "PRIMAL")
      flag_set |= IgnoreFlags::PRIMAL;
    else if (s == "CHARGED")
      flag_set |= IgnoreFlags::CHARGED;
    else if (s == "CYBERNETIC")
      flag_set |= IgnoreFlags::CYBERNETIC;
    else if (s == "NIHIL")
      flag_set |= IgnoreFlags::NIHIL;
  }

  if (static_cast<int>(ignore_atk) == 0)
    parseWarning("attempting to parse ignore flags", flags);
}

/*
 * Description: Method for printing out a standardized warning emitted by
 *              the parsing functions.  The warning is only emitted if the
 *              debug mode for attribute (constant) is enabled. Also turns off
 *              the flag representing the validity of the action.
 *
 * Inputs: warning - string containing the specific warning text
 *         raw - string containing the raw info dump related to the warning
 * Output: none
 */
void Action::parseWarning(const std::string &warning, const std::string &raw)
{
  if (kDEBUG_ENABLED)
    std::cerr << "Action Error: " << warning << " on: " << raw << std::endl;

  action_flags &= ~ActionFlags::VALID;
}

/*
 * Description: Method for assigning the duration the infliction of the Action
 *              may last for given min and max values. Fails if the values are
 *              not valid.
 *
 * Inputs: min_value - minimum # of turns the infliction can last for
 *         max_value - maximum # of turns the infliction can last for
 * Output: bool - the validity of the duration assignment
 */
bool Action::setDuration(const int &min_value, const int &max_value)
{
  if (max_value >= min_value && min_value >= 0 && max_value >= 0)
  {
    min_duration = min_value;
    max_duration = max_value;
    return true;
  }

  min_duration = -1;
  max_duration = -1;

  parseWarning("attempting to parse min, max value", "");
  return false;
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Method for printing out all the info describing the state of the
 *              current action in an easy to view format. [std::cout display]
 *
 * Inputs: none
 * Output: none
 *
 * TODO [12-08-13] Print the string of the attribute the action alters or assigns
 * TODO [12-08-13] Print the string of the ailment the action inflicts or relieves 
 */
void Action::print()
{
  std::cout << "Action: " << id << std::endl;

  std::cout << "--- Action Flags --- " << std::endl;
  std::cout << "ALTER: "   << actionFlag(ActionFlags::ALTER) << std::endl;
  std::cout << "INFLICT: " << actionFlag(ActionFlags::INFLICT) << std::endl;
  std::cout << "RELIEVE: " << actionFlag(ActionFlags::RELIEVE) << std::endl;
  std::cout << "ASSIGN: "  << actionFlag(ActionFlags::ASSIGN) << std::endl;
  std::cout << "REVIVE: "  << actionFlag(ActionFlags::REVIVE) << std::endl;
  std::cout << "BASE_PC: " << actionFlag(ActionFlags::BASE_PC) << std::endl;
  std::cout << "VARI_PC: " << actionFlag(ActionFlags::VARI_PC) << std::endl;
  std::cout << "VALID: "   << actionFlag(ActionFlags::VALID) << std::endl;

  std::cout << "--- Igore Atk Flags --- " << std::endl;
  std::cout << "PHYS: " << atkFlag(IgnoreFlags::PHYSICAL) << std::endl;
  std::cout << "THER: " << atkFlag(IgnoreFlags::THERMAL) << std::endl;
  std::cout << "POLA: " << atkFlag(IgnoreFlags::POLAR) << std::endl;
  std::cout << "PRIM: " << atkFlag(IgnoreFlags::PRIMAL) << std::endl;
  std::cout << "CHAR: " << atkFlag(IgnoreFlags::CHARGED) << std::endl;
  std::cout << "CYBR: " << atkFlag(IgnoreFlags::CYBERNETIC) << std::endl;
  std::cout << "NIHI: " << atkFlag(IgnoreFlags::NIHIL) << std::endl;

  std::cout << "--- Igore Def Flags --- " << std::endl;
  std::cout << "PHYS: " << defFlag(IgnoreFlags::PHYSICAL) << std::endl;
  std::cout << "THER: " << defFlag(IgnoreFlags::THERMAL) << std::endl;
  std::cout << "POLA: " << defFlag(IgnoreFlags::POLAR) << std::endl;
  std::cout << "PRIM: " << defFlag(IgnoreFlags::PRIMAL) << std::endl;
  std::cout << "CHAR: " << defFlag(IgnoreFlags::CHARGED) << std::endl;
  std::cout << "CYBR: " << defFlag(IgnoreFlags::CYBERNETIC) << std::endl;
  std::cout << "NIHI: " << defFlag(IgnoreFlags::NIHIL) << std::endl;

  std::cout << "Min Duration: " << min_duration << std::endl;
  std::cout << "Max Duration: " << max_duration << std::endl;
  std::cout << "Variance: "     << variance << std::endl;
  std::cout << "Base Change: "  << base << std::endl;
  std::cout << std::endl;
}

/*
 * Description: Returns the evaluation of a given ActionFlag flag or set of 
 *              flags by comparing them after a bit-wise and to the current set.
 *
 * Inputs: none
 * Output: bool - evaluation of the given flag
 */
bool Action::actionFlag(ActionFlags test_flag)
{
  return static_cast<bool>((action_flags & test_flag) == test_flag);
}

/*
 * Description: Returns the evaluation of a given ignore atk flag or set of 
 *              flags by comparing them after a bit-wise and to the current set.
 *
 * Inputs: none
 * Output: bool - evaluation of the given flag
 */
bool Action::atkFlag(IgnoreFlags test_flag)
{
  return static_cast<bool>((ignore_atk & test_flag) == test_flag);
}

/*
 * Description: Returns the evaluation of a given ignore def flag or set of 
 *              flags by comparing them after a bit-wise and to the current set.
 *
 * Inputs: none
 * Output: bool - evaluation of the givenflag
 */
bool Action::defFlag(IgnoreFlags test_flag)
{
  return static_cast<bool>((ignore_def & test_flag) == test_flag);
}

/*
 * Description: Returns the enumerated value of the attribute the action affects
 *
 * Inputs: none
 * Output: Attribute - enumerated atribute the action affects
 */
Attribute Action::getAttribute()
{
  return attribute;
}

/*
 * Description: Returns the enumerated infliction the action affects
 *
 * Inputs: none
 * Output: Infliction - enumerated attribute the action affects
 */
Infliction Action::getAilment()
{
  return ailment;
}

/*
 * Description: Returns the base value by which the Action alters an attribute
 *
 * Inputs: none
 * Output: int - the base value of the Action
 */
int Action::getBase()
{
  return base;
}

/*
 * Description: Returns the ID of the Action
 *
 * Inputs: none
 * Output: int - the ID of the Action
 */
int Action::getID()
{
  return id;
}

/*
 * Description: Returns the minimum duration of the Action
 *
 * Inputs: none
 * Output: int - the minimum duration of the Action
 */
int Action::getMin()
{
  return min_duration;
}

/*
 * Description: Returns the maximum duration of the Action
 *
 * Inputs: none
 * Output: int - the maximum duration of the Action
 */
int Action::getMax()
{
  return max_duration;
}

/*
 * Description: Returns the variance of the Action
 *
 * Inputs: none
 * Output: uint32_t - the variance on the Base value
 */
uint32_t Action::getVariance()
{
  return variance;
}