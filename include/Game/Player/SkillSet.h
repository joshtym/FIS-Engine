/*******************************************************************************
* Class Name: SkillSet [Declaration]
* Date Created: November 23rd, 2013
* Inheritance: None
* Description: A SkillSet is a container for a list of skills and the level
*              required to be usable. A SkillSet is contained in a BubbyFlavour,
*              Race, Category, Person, Equipment, Item. A SkillList can be
*              sorted by level, by cost of the skill, by the assigned point
*              value (for AI), by the elements the skill uses, etc.
*
* Notes
* -----
*
* [1]: A SkillSet may also have individually enabled/disabled Skills.
*
* TODO
* ----
* [08-13-14] SkillSet construction from XML Data
*******************************************************************************/
#ifndef SKILLSET_H
#define SKILLSET_H

#include "Game/Player/Skill.h"
#include "EnumDb.h"
#include "FileHandler.h"

/* A SkillSet is a vector of these SetElements */
struct SetElement
{
  /* Pointer to the skill in the skill element */

  Skill* skill;

  /* The level the skill becomes available */
  size_t level_available;

  /* Whether the skill is currently enabled or disabled */
  bool enabled;

  /* Whether the skill is unusable because of silence */
  bool silenced;
};

class SkillSet
{
public:
  /* Default constructor - default */
  SkillSet(int new_id = 0);

  /* Single skill constructor */
  SkillSet(Skill* skill, const uint32_t &level);

  /* Multi-skill constructor */
  SkillSet(const std::vector<Skill*> &skills,
           const std::vector<uint32_t> &levels);

  /* Annihilates a SkillSet object - default destructor */
  ~SkillSet() = default;

private:
  /* The ID of the set */
  int id;

  /* The vector of SetElements */
  std::vector<SetElement> skill_elements;

  /* ------------ Constants --------------- */
public:
  static const bool   kADDITION_BYPASS;  /* Does the + operator bypass max? */
  static const bool   kENABLED_DEFAULT;  /* Are skills enabled by default? */
  static const uint32_t kID_BUBBIFIED;   /* The bubbified skill set ID */
  static const size_t kMAX_SKILLS;       /* Problems with adding mass skills */
  static const size_t kMIN_UNLOCK_LEVEL; /* Min. lvl a Skill can be unlocked */
  static const size_t kMAX_UNLOCK_LEVEL; /* Max. lvl a Skill can be unlocked */
  static const int    kUNSET_ID;         /* ID for an unset Skill Set */

  /*======================== PRIVATE FUNCTIONS ===============================*/
 private:
  /* Returns the lowest level required of a given Skill ID */
  uint32_t calcLowestLevel(const uint32_t &skill_id);

  /* Returns unique set elements (by unique Skill IDs) */
  static std::vector<SetElement>
                calcUniques(const std::vector<SetElement> &check_elements);

  /* Cleans up the skill set */
  void cleanUp();

  /*========================= PUBLIC FUNCTIONS ===============================*/
public:
  /* Attempts to add a single skill */
  bool addSkill(Skill* skill, const uint32_t &req_level = 200,
  	            const bool enabled = true);

  /* Attempts to add a vector of skills */
  bool addSkills(const std::vector<Skill*> skills,
  	             const std::vector<uint32_t> &req_levels,
  	             const std::vector<bool> &enabled);

  /* Attempts to add a vector of skill set elements */
  bool addSkills(const std::vector<SetElement> &new_elements,
                 const bool &bypass = false);

  /* Clears all skills */
  void clear();

  /* Prints out the state of the object */
  void print(const bool &simple = false);

  /* Removes a SetElement by a given index */
  bool removeIndex(const uint32_t &index);

  /* Removes a SetElement by a given ID */
  bool removeID(const uint32_t &id);

  /* Save data to file */
  bool saveData(FileHandler* fh, std::string set_text = "skill",
                std::string wrap_text = "");

  /* Sorts the SetElements by a given sort type */
  bool sort(const SkillSorts &sort_type, bool ascending = true);

  /* Returns the vector of the enabled values */
  std::vector<bool> getAllEnabled();

  /* Returns a SetElement at a given index */
  SetElement getElement(const uint32_t &index);

  /* Returns the vector of SetElements */
  std::vector<SetElement> getElements(const uint32_t &at_level);

  /* Returns the enabled value at a given index */
  bool getEnabled(const uint32_t &index);

  /* Returns the silenced value at a given index */
  bool getSilenced(const uint32_t &index);

  /* Returns the useable value at a given index */
  bool getUseable(const uint32_t &index);

  /* Returns the ID of the Skill Set */
  int getID();

  /* Returns the index of a given Skill ID (if found) */
  int32_t getIndexOfID(const uint32_t &id);

  /* Returns the first index of a name beginning with the desird char */
  int32_t getIndexOfAlpha(const char &alpha);

  /* Returns the level req. of a Skill at a given index */
  uint32_t getLevel(const uint32_t &index);

  /* Returns the vector of all level required values */
  std::vector<uint32_t> getLevels();

  /* Returns the string name of a Skill at a given index */
  std::string getName(const uint32_t &index);

  /* Returns a compiled list of string names for the Skills */
  std::vector<std::string> getNames();

  /* Returns the number of skill elements */
  uint32_t getSize();

  /* Assigns an enabled state to a SetElement at a given index */
  bool setEnabled(const uint32_t &index, const bool &state = true);

  /* Assigns an ID to the Skill Set */
  void setID(const int &new_id);

  /* Assigns a silenced state to a SetElement at a given index */
  bool setSilenced(const uint32_t &index, const bool &state = true);

  /* Builds the vector of values for each skill index */
  std::vector<uint32_t> getValues();

  /*======================== OPERATOR FUNCTIONS ==============================*/
public:
  /* Compound assignment - adds a SkillSet to another SkillSet */
  SkillSet& operator+=(const SkillSet &rhs);

  /* Compound subtraction - subtracts a SkillSet from another */
  SkillSet& operator-=(const SkillSet &rhs);

  /* Addition - adds two SkillSets together (uses compound assignment) */
  SkillSet& operator+(const SkillSet &rhs);

  /* Subtraction - subtracts a SkillSet (uses compound subtraction) */
  SkillSet& operator-(const SkillSet &rhs);
};

#endif //SKILLSET_H
