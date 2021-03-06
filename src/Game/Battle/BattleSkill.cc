#include "Game/Battle/BattleSkill.h"
#include "Game/Battle/BattleActor.h"

BattleSkill::BattleSkill()
  : valid_status{ValidStatus::INVALID}
  , skill{nullptr}
  , true_cost{0}
  , targets{}
{

}

bool BattleSkill::isDefensive()
{
  if(skill)
    return skill->getFlag(SkillFlags::DEFENSIVE);

  return false;
}

bool BattleSkill::isOffensive()
{
  if(skill)
    return skill->getFlag(SkillFlags::OFFENSIVE);

  return false;
}

bool BattleSkill::loadData(SDL_Renderer* renderer)
{
  if(renderer && skill && skill->getAnimation())
  {
    skill->getAnimation()->loadData(renderer);

    return true;
  }

  return false;
}

void BattleSkill::print()
{
  std::cout << "-- Battle Skill --" << std::endl;
  std::cout << "ValidStatus: " << Helpers::validStatusToStr(valid_status);

  if(skill)
    std::cout << "\nSkill: " << skill->getName() << std::endl;;
  std::cout << "True Cost: " << true_cost << std::endl;

  std::cout << "Targets: ";

  for(auto& target : targets)
    if(target && target->getBasePerson())
      std::cout << target->getBasePerson()->getName() << ", ";

  std::cout << std::endl;
}
