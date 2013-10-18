/******************************************************************************
* Class Name: Game
* Date Created: Dec 2 2012
* Inheritance: QStackedWidget
* Description: This class is the top layer that handles all interclass
*              communication between map, player, and battle as well as all
*              setup and overall insight and control. The painting control
*              won't be handled here and this will just act as an intermediate
*              data highway / event handler.
*
* TODO:
*  1. Add Event Handler. Integrate in to handle all exchanges between class
*  2. Talk to Mike about Battle and how we should keep it constructed and then
*     just change the player and foes when it switches. This is primarily
*     for dealing with the stacked widget.
*  3. Add victory screen logic
*  4. Add speed up button to allow the game to accelerate movement and
*     everything else. Do it by multiplying the time elapsed.
******************************************************************************/
#include "Game/Game.h"

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* Constructor function */
Game::Game(QWidget* parent)
{
  setParent(parent);
  setupGame();
}

/* Constructor function */
Game::Game(Options running_config, QWidget* parent)
{
  game_config = running_config;
  setParent(parent);
  setupGame();
}

/* Destructor function */
Game::~Game()
{
  /* Clear out the widgets in the stack */
  while(count() > 0)
    removeWidget(widget(0));

  /* Delete battle */
  if(game_battle != 0)
  {
    delete game_battle;
    game_battle = 0;
  }

  /* Delete map */
  if(game_map != 0)
  {
    delete game_map;
    game_map = 0;
  }
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/
  
/* Connect the event handler to the game */
void Game::connectEvents()
{
  QObject::connect(&event_handler, SIGNAL(startBattle()), 
                   this,           SLOT(startBattle()));

  QObject::connect(&event_handler, 
                   SIGNAL(teleportThing(MapPerson*, int, int, int)), 
                   this,           
                   SLOT(teleportThing(MapPerson*, int, int, int)));

  QObject::connect(
              &event_handler, 
              SIGNAL(initConversation(Conversation*, MapPerson*, MapThing*)), 
              this, 
              SLOT(initConversation(Conversation*, MapPerson*, MapThing*)));
}

/* Set up the battle - old battle needs to be deleted prior to calling */
void Game::setupBattle()
{
  // Begin Action Builds
  Action action_0001("1,RAISE,PHYSICAL AGGRESSION,1.1,0,,0,,5,2");
  Action action_0002("2,RAISE,PHYSICAL FORTITUDE,1.1,0,,0,,10,3");
  Action action_0003("3,GIVE,Poison,2.5,0,,0,,,");

  // End Action Builds

  // Begin Skill Builds

  QVector<Action*> effect_list;
  effect_list.push_back(&action_0001);
  effect_list.push_back(&action_0002);
  effect_list.push_back(&action_0003);

  QVector<float> chance_list;
  chance_list.push_back(1.00);
  chance_list.push_back(0.95);
  chance_list.push_back(0.90);

  Skill* poison_skill = new Skill("Posion Attack", effect_list, chance_list);
  poison_skill->setFlag(Skill::OFFENSIVE, true);
  poison_skill->setFlag(Skill::PHYSICAL, true);

  // End Skill Builds

  // Testing

  //action_0003.printAll();

  // End Testing

  QList<uint> stats1;
  QList<uint> stats2;
  QList<uint> stats3;

  for (int i = 0; i < 19; i++)
  {
    stats1.append(5 + i);
    stats2.append(500 + i);
    stats3.append(15 + i);
  }

  AttributeSet race_set(stats1);
  AttributeSet cate_set(stats1);
  AttributeSet race_max(stats3);
  AttributeSet cate_max(stats2);

  Race* base_race = new Race("Fiends");
  base_race->setAttrSet(race_set);
  base_race->setMaxSet(race_max);
  Category* base_category = new Category("Battle Class");
  base_category->setAttrSet(cate_set);
  base_category->setMaxSet(cate_max);
  Person* main_character
          = new Person("Malgidus", base_race, base_category, "PHA", "CYB");
  Person* secd_character
          = new Person("Arcadius", base_race, base_category, "PHA", "CYA");

  /* Level up Tests */
  main_character->setPersonFlag(Person::CAN_LEVEL_UP, true);

  Party* friends = new Party(main_character);
  Party* foes = new Party(secd_character);

  /*
  BubbyFlavour* spark_flavour = new BubbyFlavour(0, "Spark");
  spark_flavour->setAttr(&base_set);
  Bubby* first_bubby = new Bubby(spark_flavour);
  first_bubby->setLevel(5);
  first_bubby->setTier(3);
  Bubby* second_bubby = new Bubby(spark_flavour);
  second_bubby->setTier(1);
  Bubby* third_bubby = new Bubby(spark_flavour);
  std::vector<std::pair<ushort, ushort> > list;
  list.push_back(std::make_pair(0, 0));
  list.push_back(std::make_pair(1, 1));
  list.push_back(std::make_pair(2, 2));
  list.push_back(std::make_pair(3, 3));
  list.push_back(std::make_pair(4, 4));
  Signature* equip_signature = new Signature(6, 6, list);
  equip_signature->attach(0, 1, second_bubby);
  equip_signature->attach(0, 3, first_bubby);
  equip_signature->attach(0, 0, third_bubby);
  QList<BubbyFlavour*> flavours = equip_signature->getUniqueFlavours();
  equip_signature->unattach(0, 3);
  equip_signature->unattach(0, 1);
  */

  game_battle = new Battle(friends, foes, this);
}

// TODO: Add victory screen
void Game::setupGame()
{
  game_battle = 0;
  game_map = 0;

  /* Set up the internal widgets */
  setupBattle();
  setupMap();

  /* Set up the blank disabled widget */
  QPalette palette;
  palette.setColor(QPalette::Background, Qt::black);
  blank_widget.setAutoFillBackground(true);
  blank_widget.setBackgroundRole(QPalette::Window);
  blank_widget.setPalette(palette);

  /* Add widgets to the stack */
  addWidget(&blank_widget);
  if(game_map != 0)
    addWidget(game_map);
  else
    qDebug() << "[ERROR] Failed to initialize map in game.";
  if(game_battle != 0)
    addWidget(game_battle);
  else
    qDebug() << "[ERROR] Failed to initialize battle in game.";
  setCurrentIndex(DISABLED);

  /* Connections relevant to the game */
  QObject::connect(game_map, SIGNAL(closeMap()), 
                   this,     SIGNAL(closeGame()));
  QObject::connect(game_battle, SIGNAL(closeBattle()), 
                   this,        SIGNAL(closeGame()));

  /* Set up events */
  connectEvents();
}

/* Set up the map - old map needs to be deleted prior to calling */
void Game::setupMap()
{
  /* Sets up the map format and map with vsync and double buffering forced on */
  /* TODO: add checking if OpenGL is not enabled */
  QGLFormat gl_format(QGL::SampleBuffers);
  gl_format.setDoubleBuffer(true);
  if(game_config.isVsyncEnabled())
    gl_format.setSwapInterval(1);
  else
    gl_format.setSwapInterval(0);
  game_map = new Map(gl_format, game_config, &event_handler);

  /* Load the map - temporary location */
  game_map->loadMap("maps/test_04");
}

/*============================================================================
 * PUBLIC SLOTS
 *===========================================================================*/

void Game::initConversation(Conversation* convo, MapPerson* initiator, 
                                                 MapThing* source)
{
  if(game_map != 0)
    game_map->initConversation(convo, initiator, source);
}

void Game::startBattle()
{
  switchGameMode(BATTLE);
}

void Game::teleportThing(MapPerson* target, int x, int y, int section_id)
{
  if(game_map != 0 && target != 0)
  {
    if(section_id < 0)
      game_map->teleportThing(target->getID(), x, y);
    else
      game_map->teleportThing(target->getID(), x, y, section_id);
  }
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

// TODO: How to do map initialization with GL if options change in the middle
void Game::setConfiguration(Options running_config)
{
  game_config = running_config;

  // TODO: Add reset map and battle parameters when this is changed */
}

void Game::switchGameMode(GameMode mode)
{
  setCurrentIndex(mode);
  currentWidget()->setFocus();
}

/* Updates the game state */
void Game::updateGame(int cycle_time)
{
  if(currentIndex() == MAP && game_map != 0)
    game_map->updateMap(cycle_time);

  // if (currentIndex() == BATTLE && game_battle != 0)
    //game_battle->updateBattle(cycle_time);
}
