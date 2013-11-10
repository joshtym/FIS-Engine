/******************************************************************************
* Class Name: Map
* Date Created: Dec 2 2012
* Inheritance: QGLWidget
* Description: The map class, this is the top level with regard to an actual
*              in-game map. This contains all the tiles that a map is composed
*              of, it also holds pointers to all of the NPC's contained in the
*              maps tiles. This also handles the viewport for the map.
*
* TODO: 
*  1. If the tile image does not exist, disable the painting sequence
*    - There seems to be a small glitch in this where it paints a white tile
*  2. Allow for map to run with a parameterless constructor. Is that possible
*     with the QGLFormat in the mix that is required for setting up the 
*     QGLWidget?
*  3. If mode is switched, end all notification animations and such. -> battle
*  4. Animation event for teleport requires fade out -> teleport -> fade in
*  5. If tile set fails, don't quit the map. Useful only multi tile setups.
*     Maybe only pass if it is a multi tile setup ??
*
* Extra:
*  1. As the player is teleported to a new map, maybe have the option to have
*     it at a tile and then walking in. More true animation. How to?
******************************************************************************/
#include "Game/Map/Map.h"

/* Constant Implementation - see header file for descriptions */
const int Map::kDOUBLE_DIGITS = 10;
const int Map::kELEMENT_DATA = 0;
const short Map::kFILE_CLASSIFIER = 3;
const short Map::kFILE_GAME_TYPE = 1;
const short Map::kFILE_SECTION_ID = 2;
const short Map::kFILE_TILE_COLUMN = 5;
const short Map::kFILE_TILE_ROW = 4;
const short Map::kPLAYER_ID = 0;
const int Map::kTILE_HEIGHT = 64;
const int Map::kTILE_WIDTH = 64;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* Constructor function */
Map::Map(const QGLFormat & format, Options running_config, 
         EventHandler* event_handler) : QGLWidget(format)
{
  /* Set some initial class flags */
  //setAttribute(Qt::WA_PaintOnScreen);
  //setAttribute(Qt::WA_NoSystemBackground);
  setAutoBufferSwap(false);
  setAutoFillBackground(false);
  
  /* Configure the scene */
  items.clear();
  loaded = false;
  map_index = 0;
  persons.clear();
  player = 0;
  things.clear();

  /* Configure the FPS animation and time elapsed, and reset to 0 */
  paint_animation = 0;
  paint_frames = 0;
  paint_time = 0;
  time_elapsed = 0;

  //setMinimumSize(2000, 2000);

  /* Set up the map displays */
  //map_dialog.setFont(QFont("Times", 10));
  map_dialog.setDialogImage("sprites/Map/Overlay/dialog.png");
  map_dialog.setEventHandler(event_handler);
  map_dialog.setViewportDimension(running_config.getScreenWidth(), 
                                  running_config.getScreenHeight());

  /* Necessary connections for child functions */
  connect(&map_dialog, SIGNAL(setThingData(QList<int>)), 
          this, SLOT(getThingData(QList<int>)));
  connect(&map_dialog, SIGNAL(finishThingTarget()), 
          this, SLOT(finishThingTarget()));

  /* Setup the viewport */
  viewport = new MapViewport(running_config.getScreenWidth(), 
                             running_config.getScreenHeight(), 
                             kTILE_WIDTH, kTILE_HEIGHT);

  /* Initialize the event handler, for map creation */
  this->event_handler = event_handler;
}

/* Destructor function */
Map::~Map()
{
  unloadMap();
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

bool Map::addTileData(XmlData data, int section_index)
{
  QList<QString> adjustments;
  bool success = true;

  /* Get the element information */
  QStringList element = data.getElement(data.getNumElements()-1).split("_");
  QString classifier = data.getElement(kFILE_CLASSIFIER);
  QString descriptor = element[0].toLower().trimmed();

  /* Split based on the element information if it's for a path */
  if(descriptor == "path")
  {
    /* If there is adjustment information available, use it */
    adjustments = element.mid(1);

    /* Get the actual set of paths and add them */
    QList< QList<QString> > path_stack = splitTilePath(data.getDataString());
    for(int y = 0; y < path_stack.size(); y++)
      for(int x = 0; x < path_stack[y].size(); x++)
        success &= addTileSprite(path_stack[y][x], x, y, 
                                 adjustments, section_index, data);

    return success;
  }
  /* Otherwise, access the passability information for the tile */
  else if(descriptor == "passability" || 
          classifier == "tileevent")
  {
    QStringList row_list = data.getKeyValue(kFILE_TILE_ROW).split(",");
    QStringList col_list = data.getKeyValue(kFILE_TILE_COLUMN).split(",");
    QString type = data.getKeyValue(kFILE_CLASSIFIER);
    
    for(int i = 0; i < row_list.size(); i++) /* Coordinate set index */
    {
      QStringList rows = row_list[i].split("-"); /* x range for coordinate */
      QStringList cols = col_list[i].split("-"); /* y range for coordinate */
      
      /* Determine the row of parsing - limit to map size */
      int r_start = rows[0].toInt();
      int r_end = rows[rows.size() - 1].toInt();
      if(r_start >= geography[section_index].size())
        r_start = geography[section_index].size() - 1;
      if(r_end >= geography[section_index].size())
        r_end = geography[section_index].size() - 1;
      
      /* Determine the column of parsing - limit to map size */
      int c_start = cols[0].toInt();
      int c_end = cols[cols.size() - 1].toInt();
      if(c_start >= geography[section_index][r_start].size())
        c_start = geography[section_index][r_start].size() - 1;
      if(c_end >= geography[section_index][r_start].size())
        c_end = geography[section_index][r_start].size() - 1;
      
      /* Shift through all the rows and column pairs of the coordinate */
      for(int r = r_start; r <= r_end; r++)
      {
        for(int c = c_start; c <= c_end; c++)
        {
          /* Set the passability */
          if(descriptor == "passability")
            success &= geography[section_index][r][c]->
                    addPassability(data.getDataString(), 
                                   data.getElement(kFILE_CLASSIFIER), 
                                   data.getKeyValue(kFILE_CLASSIFIER));
          /* Otherwise, it's a tile event */
          else
          {
            /* Classify between enter and exit events */
            if(type == "enter")
              success &= geography[section_index][r][c]->
                    updateEnterEvent(data, kFILE_CLASSIFIER + 3, section_index);
            else if(type == "exit")
              success &= geography[section_index][r][c]->
                    updateExitEvent(data, kFILE_CLASSIFIER + 3, section_index);
          }
        }
      }
    }
    
    return success;
  }
  /* Otherwise, if animation, it is assumed that this is tied with the last
   * sprite added and sets the animation accordingly */
  else if(descriptor == "animation")
  {
    Sprite* last_sprite = 0;
    if(!tile_sprites.isEmpty())
      last_sprite = tile_sprites.last();

    /* Access the tile if it's not 0 */
    if(last_sprite != 0)
      last_sprite->setAnimationTime(data.getDataInteger());
  }

  return false;
}

/* Adds a tile sprite, based on the path and some XMLData */
// TODO: Add search existing sprites to see if it exists
bool Map::addTileSprite(QString path, int x_diff, int y_diff, 
                        QList<QString> adjustments, int section_index, 
                        XmlData data)
{
  bool success = false;

  /* Run through this list, checking ranges and add the corresponding
   * tiles, only if the sprite data is legitimate */
  Sprite* tile_frames = new Sprite(path);
  tile_frames->execImageAdjustments(adjustments);
  if(tile_frames->getSize() > 0)
  {
    /* Split up the coordinates for the tile sprite */
    QStringList row_list = data.getKeyValue(kFILE_TILE_ROW).split(",");
    QStringList col_list = data.getKeyValue(kFILE_TILE_COLUMN).split(",");
    for(int i = 0; i < row_list.size(); i++) /* Coordinate set index */
    {
      QStringList rows = row_list[i].split("-"); /* x range for coordinate */
      QStringList cols = col_list[i].split("-"); /* y range for coordinate */

      /* Shift through all the rows and column pairs of the coordinate */
      for(int r = rows[0].toInt(); r <= rows[rows.size() - 1].toInt(); r++)
      {
        for(int c = cols[0].toInt(); c <= cols[cols.size() - 1].toInt(); c++)
        {
          int x = r + x_diff;
          int y = c + y_diff;

          if(section_index >= 0 && section_index < geography.size() && 
             x >= 0 && x < geography[section_index].size() && 
             y >= 0 && y < geography[section_index][x].size())
          {
            success |= geography[section_index][x][y]->
                  addSprite(tile_frames, data.getElement(kFILE_CLASSIFIER), 
                                         data.getKeyValue(kFILE_CLASSIFIER));
          }
        }
      }
    }
  }

  /* If successful, add the data to the stack. Else, delete it */
  if(success)
    tile_sprites.append(tile_frames);
  else
  {
    delete tile_frames;
    qDebug() << "[ERROR] Map sprite add fail with path: " + path;
  }

  return success;
}

bool Map::addThingData(XmlData data, int section_index)
{
  QString identifier = data.getElement(kFILE_CLASSIFIER);
  int id = data.getKeyValue(kFILE_CLASSIFIER).toInt();
  int index = 0;
  MapThing* modified_thing = 0;
  
  /* Identify which thing to be created */
  if(identifier == "mapthing" || identifier == "mapio")
  {
    /* Search for the existing map object */
    while(modified_thing == 0 && index < things.size())
    {
      if(things[index]->getID() == id)
        modified_thing = things[index];
      index++;
    }

    /* Create a new thing if one does not exist */
    if(modified_thing == 0)
    {
      if(identifier == "mapthing")
        modified_thing = new MapThing(0, kTILE_WIDTH, kTILE_HEIGHT);
      else
        modified_thing = new MapInteractiveObject(kTILE_WIDTH, kTILE_HEIGHT);
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);
      
      /* Append the new one */
      things.append(modified_thing);
    }
    
    //qDebug() << "1: " << identifier << " " << modified_thing;
  }
  else if(identifier == "mapperson" || identifier == "mapnpc")
  {
    /* Search for the existing map object */
    while(modified_thing == 0 && index < persons.size())
    {
      if(persons[index]->getID() == id)
        modified_thing = persons[index];
      index++;
    }

    /* Create a new person if one does not exist */
    if(modified_thing == 0)
    {
      if(identifier == "mapperson")
        modified_thing = new MapPerson(kTILE_WIDTH, kTILE_HEIGHT);
      else
        modified_thing = new MapNPC(kTILE_WIDTH, kTILE_HEIGHT);
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);
      
      /* If the ID is the player ID, tie to the player */
      if(id == kPLAYER_ID)
        player = (MapPerson*)modified_thing;
      
      /* Append the new one */
      persons.append((MapPerson*)modified_thing);
    }
  }
  else if(identifier == "mapitem")
  {
    /* Search for the existing map object */
    while(modified_thing == 0 && index < items.size())
    {
      if(items[index]->getID() == id)
        modified_thing = items[index];
      index++;
    }
    
    /* If no item found, create one */
    if(modified_thing == 0)
    {
      /* Create the new thing */
      modified_thing = new MapItem(0, kTILE_WIDTH, kTILE_HEIGHT);
      modified_thing->setEventHandler(event_handler);
      modified_thing->setID(id);
      
      /* Append the new one */
      items.append((MapItem*)modified_thing);
    }
  }
  
  /* Proceed to update the thing information from the XML data */
  if(modified_thing != 0)
  {
    /* Handle the startpoint case if it's the identifying element */
    if(data.getElement(kFILE_CLASSIFIER + 1) == "startpoint")
    {
      QStringList points = data.getDataString().split(",");
      if(points.size() == 2) /* There needs to be an x and y point */
      {
        int x = points[0].toInt();
        int y = points[1].toInt();

        /* Check if the tile data is relevant */
        if(geography.size() > section_index && 
           geography[section_index].size() > x && 
           geography[section_index][x].size() > y)
        {
          return modified_thing->setStartingTile(section_index, 
                                          geography[section_index][x][y], true);
        }
      }      
      
      return false;
    }
   
    /* Otherwise, add the thing information (virtual function) */
    return modified_thing->addThingInformation(data, kFILE_CLASSIFIER, 
                                                     section_index);
  }

  return false;
}

bool Map::initiateMapSection(int section_index, int width, int height)
{
  /* Check if the initiation will be viable */
  if(section_index >= 0 && width > 0 && height > 0 && 
    (geography.size() <= section_index || 
     geography[section_index].size() == 0))
  {
    /* Get the geography subset filled up past the point of being set */
    while(geography.size() <= section_index)
    {
      QList< QList<Tile*> > map_section;
      geography.append(map_section);
    }

    /* Fill the section */
    for(int i = 0; i < width; i++)
    {
      QList<Tile*> col;
      
      for(int j = 0; j < height; j++)
      {
        Tile* t = new Tile(event_handler, kTILE_WIDTH, kTILE_HEIGHT, i, j);
        col.append(t);
      }
      
      geography[section_index].append(col);
    }

    return true;    
  }
  
  return false;
}

/* Initiates a thing action, based on the action key being hit */
void Map::initiateThingInteraction()
{
  if(player != 0)
  {
    bool interacted = false;
    int x = player->getTile()->getX();
    int y = player->getTile()->getY();

    /* Determine the direction and offset coordinate tile selection */
    EnumDb::Direction direction = player->getDirection();
    if(direction == EnumDb::NORTH)
      y--;
    else if(direction == EnumDb::EAST)
      x++;
    else if(direction == EnumDb::SOUTH)
      y++;
    else if(direction == EnumDb::WEST)
      x--;
    /* Throw X out of bounds if no direction */
    else
      x = -1;

    /* Aquire the thing, that's being pointed at and try to interact */
    int index = map_index;
    if(x >= 0 && x < geography[index].size() && 
       y >= 0 && y < geography[index][0].size())
    {
      if(geography[index][x][y]->isPersonSet() &&
         geography[index][x][y]->getPerson()->getTile()->getX() == x &&
         geography[index][x][y]->getPerson()->getTile()->getY() == y)
      {
        geography[index][x][y]->getPerson()->interact(player);
        interacted = true;
      }
      else if(geography[index][x][y]->isThingSet())
      {
        geography[index][x][y]->getThing()->interact(player);
        interacted = true;
      }
    }
    
    /* If there was no thing to interact with, proceed to try and pickup the
     * tile item. */
    if(!interacted && player->getTile()->getItem() != 0 && 
       player->getTile()->getItem()->getCount() > 0 && event_handler != 0)
    {
      event_handler->executePickup(player->getTile()->getItem());
    }
  }
}

/* Splites the tile path, to determine if numerous tiles are needed */
QList< QList<QString> > Map::splitTilePath(QString path)
{
  QList< QList<QString> > path_matrix;

  /* First split, to pull out the range */
  QStringList split_1 = path.split("{");
  if(split_1.size() == 2)
  {
    /* Second split, to pull out the range */
    QStringList split_2 = split_1[1].split("}");
    if(split_2.size() == 2)
    {
      /* Split the two directional coordinates */
      QStringList range = split_2[0].split("x");
      if(range.size() == 2)
      {
        /* Split the range */
        QStringList first_digit = range[0].split("-");
        int first_max = first_digit[first_digit.size()-1].toInt();
        int first_start = first_digit[0].toInt();
        QStringList second_digit = range[1].split("-");
        int second_max = second_digit[second_digit.size()-1].toInt();
        int second_start = second_digit[0].toInt();

        /* Loop through and create all the paths */
        for(int i = first_start; i <= first_max; i++)
        {
          QList<QString> x_paths;

          for(int j = second_start; j <= second_max; j++)
            x_paths.append(split_1[0] + QChar(64+i) + QChar(64+j) + split_2[1]);

          path_matrix.append(x_paths);
        }

        return path_matrix;
      }
    }
  }

  /* If here, the split was unsuccessful and therefore only one path */
  QList<QString> path_stack;
  path_stack.append(path);
  path_matrix.append(path_stack);
  return path_matrix;
}

/*============================================================================
 * PROTECTED FUNCTIONS
 *===========================================================================*/

/* TODO: Possibly add it so that any map can keep updating? This might get too
 *       heavy. 
 *       Should updates be restricted to a larger space around the viewport
 *       and no more? */
void Map::animate(short time_since_last)
{
  /* Only proceed if the animation time is positive */
  if(time_since_last > 0)
  {
    /* The movement handling for persons */
    for(int i = 0; i < persons.size(); i++)
    {
      if(persons[i] != 0)
      {
        Tile* next_tile = 0;

        if(persons[i]->getMapSection() == map_index && 
           persons[i]->getTile() != 0)
        {
          int tile_x = persons[i]->getTile()->getX();
          int tile_y = persons[i]->getTile()->getY();

          /* Based on the move request, provide the next tile in line using the
           * current centered tile and move request */
          switch(persons[i]->getMoveRequest())
          {
            case EnumDb::NORTH:
              if(--tile_y >= 0)
                next_tile = geography[map_index][tile_x][tile_y];
              break;
            case EnumDb::EAST:
              if(++tile_x < geography[map_index].size())
                next_tile = geography[map_index][tile_x][tile_y];
              break;
            case EnumDb::SOUTH:
              if(++tile_y < geography[map_index][tile_x].size())
                next_tile = geography[map_index][tile_x][tile_y];
              break;
            case EnumDb::WEST:
              if(--tile_x >= 0)
                next_tile = geography[map_index][tile_x][tile_y];
              break;
            case EnumDb::DIRECTIONLESS:
              next_tile = 0;
          }
        }

        /* Proceed to update the thing */
        persons[i]->updateThing(time_since_last, next_tile);
      }
    }

    /* Update the sprites on the map (tiles) */
    for(int i = 0; i < tile_sprites.size(); i++)
    {
      if(tile_sprites[i] != 0)
        tile_sprites[i]->updateSprite(time_since_last);
    }
    
    /* Update items on the map */
    for(int i = 0; i < items.size(); i++)
    {
      if(items[i] != 0)
        items[i]->updateThing(time_since_last, 0);
    }

    /* Update things on the map */
    for(int i = 0; i < things.size(); i++)
    {
      if(things[i] != 0)
        things[i]->updateThing(time_since_last, 0);
    }

    /* Animate the displays on the screen */
    map_dialog.update(time_since_last);
  }
}

void Map::initializeGL()
{
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_COLOR_MATERIAL);
  glEnable(GL_BLEND);
  //glEnable(GL_CULL_FACE); // Performance Add? Only for 3d
  //glEnable(GL_POLYGON_SMOOTH); // Causes strange lines drawn between tiles
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0, 0, 0, 0);

  /* Initialize Gl in other classes */
  map_dialog.initializeGl();
}

void Map::keyPressEvent(QKeyEvent* key_event)
{
  if(key_event->key() == Qt::Key_Escape)
  {
    if(player != 0)
    {
      player->clearAllMovement();
      player->setSpeed(150);
    }
    emit closeMap();
  }
  else if(key_event->key() == Qt::Key_P)
    map_dialog.setPaused(!map_dialog.isPaused());
  else if(key_event->key() == Qt::Key_1 && player != 0)
    player->resetPosition();
  else if(key_event->key() == Qt::Key_4)
    map_dialog.initNotification("Testing", 1000, true);
  else if(key_event->key() == Qt::Key_5)
    map_dialog.initNotification("This is a really long message. It goes on and on without end. Who makes notifications this long except for crazy deranged eutherlytes. Yes, I made a new word. You want to fight about it?");//map_dialog.haltDialog();
  else if(key_event->key() == Qt::Key_6)
    map_dialog.initNotification("This is a really long message. It goes on and on without end. Who makes notifications this long except for crazy deranged eutherlytes. Yes, I made a new word. You want to fight about it?", 5000, true);
  else if(map_dialog.isInConversation())
    map_dialog.keyPress(key_event);
  else if(key_event->key() == Qt::Key_Shift && !key_event->isAutoRepeat())
  {
    if(player != 0)
      player->setSpeed(300);
  }
  else if(key_event->key() == Qt::Key_Space)
    initiateThingInteraction();
  else if(key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Up ||
          key_event->key() == Qt::Key_Right || key_event->key() == Qt::Key_Left)
  {
    if(player != 0)
      player->keyPress(key_event);
  }
  else if(key_event->key() == Qt::Key_F4)
    player->setSpeed(player->getSpeed() - 1);
  else if(key_event->key() == Qt::Key_F5)
    player->setSpeed(player->getSpeed() + 1);
  else if(key_event->key() == Qt::Key_0)
  {
    Event blank_event = event_handler->createBlankEvent();
    
    Conversation* convo = new Conversation;
    convo->category = EnumDb::TEXT;
    convo->action_event = blank_event;
    convo->text = "This is the initial conversation point that will start it. ";
    convo->text += "How can this continue? It must pursue to complete ";
    convo->text += "embodiment. Ok, maybe I'll just keep typing until I break ";
    convo->text += "the entire compiler.";
    convo->thing_id = 0;
    Conversation test1, test2, test3, test4, test5;
    test1.category = EnumDb::TEXT;
    test1.action_event = blank_event;
    test1.text = "This is a test to see how the data performs. The line will split once ";
    test1.text += "unless it is an option based selection in which case it will restrict."; 
    test1.thing_id = 3;
    test2.category = EnumDb::TEXT;
    test2.action_event = blank_event;
    test2.text = "This is a no man case. See what happens!! Ok, this is the ";
    test2.text += "too long case where the lines never cease to exist and the ";
    test2.text += "points aren't for real. I'm feeling a bit hungry though ";
    test2.text += "so I don't know if I'll have the stamina to clean up this ";
    test2.text += "case in all it's glory. Repeat: ";
    test2.text += test2.text;
    test2.text += test2.text;
    test2.thing_id = 2;
    test2.next.append(test1);
    test3.category = EnumDb::TEXT;
    test3.action_event = blank_event;
    test3.text = "Back to finish off with a clean case with a couple of lines.";
    test3.text += " So this requires me to write a bunch of BS to try and fill";
    test3.text += " these lines.";
    test3.text += test3.text;
    //test3.text += test3.text;
    test3.thing_id = 24;
    test3.next.append(test2);
    test4.category = EnumDb::TEXT;
    test4.action_event = blank_event;
    test4.text = "Option 1 - This goes on and on and on and on and on and lorem ipsum. This is way too long to be an option. Loser";
    test4.thing_id = -1;
    test4.next.append(test2);
    test5.category = EnumDb::TEXT;
    test5.action_event = blank_event;
    test5.text = "Option 2";
    test5.thing_id = -1;
    test5.next.append(test3);
    test1.next.append(test4);
    test1.next.append(test5);
    test4.text = "Option 3";
    test1.next.append(test4);
    test5.text = "Option 4";
    test1.next.append(test5);
    test4.text = "Option 5";
    test1.next.append(test4);
    test5.text = "Option 6";
    test1.next.append(test5);
    convo->next.append(test1);
    event_handler->executeEvent(event_handler->createConversationEvent(convo),
                                player);
    
    delete convo;
  }
}

void Map::keyReleaseEvent(QKeyEvent* key_event)
{
  if(map_dialog.isInConversation())
    map_dialog.keyRelease(key_event);
  else if(key_event->key() == Qt::Key_Down || key_event->key() == Qt::Key_Up ||
          key_event->key() == Qt::Key_Right || key_event->key() == Qt::Key_Left)
  {
    if(player != 0)
      player->keyRelease(key_event);
  }
  else if(key_event->key() == Qt::Key_Shift && !key_event->isAutoRepeat())
  {
    if(player != 0)
      player->setSpeed(150);
  }
}

/* TODO: might need to restrict animation for things out of the display
 *       due to lag (high number of Things). Further testing required.
 *
 * Seem to be getting more consistent results for displaying, and it seems
 * better with vsync disabled */
void Map::paintGL()
{
  /* Start a QTimer to determine time elapsed for painting */
  QTime time;
  time.start();
  
  /* Swap the buffers from the last call - put it on top or bottom? */
  //swapBuffers();
  //qDebug() << "1: " << time.elapsed();

  /* Start by setting the context and clearing the screen buffers */
  //makeCurrent();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glPushMatrix();

  /* Only proceed if the map is loaded */
  if(loaded)
  {
    /* Animate the map based on the elapsed time and then update
     * the viewport due to the animate */
    animate(time_elapsed);
    viewport->updateView();

    /* Grab the variables for viewport */
    int tile_x_start = viewport->getXTileStart();
    int tile_x_end = viewport->getXTileEnd();
    int tile_y_start = viewport->getYTileStart();
    int tile_y_end = viewport->getYTileEnd();
    float x_offset = viewport->getX();
    int x_start = viewport->getXStart();
    int x_end = viewport->getXEnd();
    float y_offset = viewport->getY();
    int y_start = viewport->getYStart();
    int y_end = viewport->getYEnd();
    
    /* Paint the lower half */
    for(int i = tile_x_start; i < tile_x_end; i++)
      for(int j = tile_y_start; j < tile_y_end; j++)
        geography[map_index][i][j]->paintLower(x_offset, y_offset);

    /* Paint the walkover objects */
    for(int i = tile_x_start; i < tile_x_end; i++)
    {
      for(int j = tile_y_start; j < tile_y_end; j++)
      {
        if(geography[map_index][i][j]->isItemSet() && 
           geography[map_index][i][j]->getItem()->getCount() > 0)
        {
          geography[map_index][i][j]->getItem()->
                                                  paintGl(x_offset, y_offset);
        }
      }
    }

    /* Paint all things on the map */
    for(int i = 0; i < things.size(); i++)
    {
      if(things[i] != 0 && things[i]->getMapSection() == map_index && 
         things[i]->getX() >= x_start && things[i]->getX() <= x_end && 
         things[i]->getY() >= y_start && things[i]->getY() <= y_end)
      {
        things[i]->paintGl(x_offset, y_offset);
      }
    }

    /* Paint all persons on the map */
    for(int i = 0; i < persons.size(); i++)
    {
      if(persons[i] != 0 && persons[i]->getMapSection() == map_index && 
         persons[i]->getX() >= x_start && persons[i]->getX() <= x_end && 
         persons[i]->getY() >= y_start && persons[i]->getY() <= y_end)
      {
        persons[i]->paintGl(x_offset, y_offset);
      }
    }

    /* Paint the upper half */
    for(int i = tile_x_start; i < tile_x_end; i++)
      for(int j = tile_y_start; j < tile_y_end; j++)
        geography[map_index][i][j]->paintUpper(x_offset, y_offset);

    /* Paint the dialog */
    map_dialog.paintGl(this);
  }

  /* Paint the frame rate */
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0, 0.0, 0.0, 0.5);
  glBegin(GL_QUADS);
    glVertex3f(7, 40, 0);//7,40
    glVertex3f(7, 10, 0);//7,10
    glVertex3f(64, 10, 0);//64,10
    glVertex3f(64, 40, 0);//64,40
  glEnd();
  glColor4f(1.0, 1.0, 1.0, 1.0);
  renderText(20, 30, frames_per_second);

  /* Clean up the drawing procedure */
  //glFlush();
  //glPopMatrix();
  //glFinish();
  
  /* Determine the FPS sample rate */
  if(paint_animation <= 0)
  {
    frames_per_second.setNum(paint_frames /(paint_time / 1000.0), 'f', 2);
    paint_animation = 20;
  }
  paint_animation--;

  /* Check the FPS monitor to see if it needs to be reset */
  if((paint_frames % 100) == 0)
  {
    paint_frames = 0;
    paint_time = 0;
  }
  paint_frames++;
  paint_time += time_elapsed;
  
  /* Finish by swapping the buffers and then restarting the timer to update 
   * the map again */
  swapBuffers();
}

void Map::resizeGL(int width, int height) 
{
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, height, 0); // set origin to bottom left corner
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

/*============================================================================
 * PUBLIC SLOTS
 *===========================================================================*/

void Map::finishThingTarget()
{
  if(player != 0)
  {
    if(player->getTarget() != 0)
      player->getTarget()->clearTarget();
    player->clearTarget();
  }
}

void Map::getThingData(QList<int> thing_ids)
{
  QList<MapThing*> used_things;

  /* Loop through all the ids to find the associated things */
  for(int i = 0; i < thing_ids.size(); i++)
  {
    /* Only continue if the ID is valid and >= than 0 */
    if(thing_ids[i] >= 0)
    {
      /* Compile a list of all MapThings */
      QList<MapThing*> combined = things;
      for(int k = 0; k < items.size(); k++)
        combined.append((MapThing*)items[k]);
      for(int k = 0; k < persons.size(); k++)
        combined.append((MapThing*)persons[k]);

      bool found = false;
      int j = 0;

      /* Loop through the stack of things to find the associated ID */
      while(!found && j < combined.size())
      {
        if(combined[j]->getID() == thing_ids[i])
        {
          used_things.append(combined[j]);
          found = true;
        }
        j++;
      }
    }
  }

  /* Set the dialog with the new stack of things */
  map_dialog.setThingData(used_things);
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

MapPerson* Map::getPlayer()
{
  return player;
}

/* Returns the map viewport, for scrolling through the scene */
MapViewport* Map::getViewport()
{
  return viewport;
}

void Map::initialization()
{
  //player->setFocus();
}

bool Map::initConversation(Conversation* convo, MapPerson* initiator, 
                                                MapThing* source)
{
  if(player != 0 && map_dialog.initConversation(convo, initiator))
  {
    player->keyFlush();

    /* Set the targets */
    if(initiator != 0)
      initiator->setTarget(source);
    if(source != 0)
      source->setTarget(initiator);

    return true;
  }

  return false;
}

bool Map::initNotification(QString notification)
{
  return map_dialog.initNotification(notification);
}

/* Checks whether the viewport contains any tiles with the given sector */
bool Map::isInSector(int index)
{
  (void)index;//warning
  return true;
}

bool Map::isLoaded()
{
  return loaded;
}

// TODO: Separate file add success and XML read success to parse error
bool Map::loadMap(QString file)
{
  bool done = false;
  bool success = true;
  FileHandler fh(file, false, true);
  XmlData data;

  /* Set up for gl initialization */
  glInit();

  /* Start the map read */
  success &= fh.start();

  /* If file open was successful, move forward */
  if(success)
  {
    /* Run through the map components and add them to the map */
    int height = -1;
    int index = -1;
    int width = -1;
    data = fh.readXmlData(&done, &success);
    do
    {
      /* Temporary variables */
      int temp_index = -1;
      
      if(data.getElement(kFILE_GAME_TYPE) == "map")
      {
        /* Determine if it's a new section */
        if(index != 0 && data.getElement(kFILE_SECTION_ID) == "main")
          temp_index = 0;
        else if(index != data.getKeyValue(kFILE_SECTION_ID).toInt() && 
                data.getElement(kFILE_SECTION_ID) == "section")
          temp_index = data.getKeyValue(kFILE_SECTION_ID).toInt();
        
        /* Update the index information accordingly */
        if(temp_index >= 0)
        {
          index = temp_index;
          if(geography.size() > index && geography[index].size() > 0)
          {
            height = geography[index][0].size();
            width = geography[index].size();
          }
          else
          {
            height = -1;
            width = -1;
          }
        }
        
        /* Parse the data, if it is relevant to the map */
        if(height == -1 && data.getElement(kFILE_CLASSIFIER) == "height")
        {
          height = data.getDataInteger();
          if(width > 0 && height > 0)
            initiateMapSection(index, width, height);
        }
        else if(width == -1 && data.getElement(kFILE_CLASSIFIER) == "width")
        {
          width = data.getDataInteger();
          if(width > 0 && height > 0)
            initiateMapSection(index, width, height);
        }
        else if(index >= 0 && height > 0 && width > 0)
        {
          /* Tile data */
          if(data.getElement(kFILE_CLASSIFIER) == "base" ||
             data.getElement(kFILE_CLASSIFIER) == "enhancer" ||
             data.getElement(kFILE_CLASSIFIER) == "lower" ||
             data.getElement(kFILE_CLASSIFIER) == "upper" ||
             data.getElement(kFILE_CLASSIFIER) == "tileevent")
          {
            success &= addTileData(data, index);
          }
          /* Thing data */
          else if(data.getElement(kFILE_CLASSIFIER) == "mapthing" || 
                  data.getElement(kFILE_CLASSIFIER) == "mapperson" ||
                  data.getElement(kFILE_CLASSIFIER) == "mapnpc" ||
                  data.getElement(kFILE_CLASSIFIER) == "mapitem" || 
                  data.getElement(kFILE_CLASSIFIER) == "mapio")
          {
            success &= addThingData(data, index);
          }
        }
      }
      
      /* Get the next element */
      data = fh.readXmlData(&done, &success);
    } while(!done && success);
  }
  success &= fh.stop();

  /* If the map load failed, unload the map */
  if(!success)
  {
    unloadMap();
  }
  else
  {
    if(geography[map_index].size() > 0)
    {
      viewport->setMapSize(geography[map_index].size(), 
                           geography[map_index][0].size());
      if(player != 0)
        viewport->lockOn(player);
    }
    
    for(int i = 0; i < geography.size(); i++)
      for(int j = 0; j < geography[i].size(); j++)
        for(int k = 0; k < geography[i][j].size(); k++) 
          geography[i][j][k]->initializeGl();
   
    for(int i = 0; i < items.size(); i++)
      items[i]->initializeGl();
    for(int i = 0; i < persons.size(); i++)
      persons[i]->initializeGl();
    for(int i = 0; i < things.size(); i++)
      things[i]->initializeGl();
  }
  loaded = success;

  return success;
}

/* Checks the tile you are attempting to enter for passibility of the given
 * direction */
bool Map::passible(EnumDb::Direction dir, int x, int y)
{
  (void)dir;//warning
  (void)x;//warning
  (void)y;//warning
  return true;
}

/* Causes the thing you are moving into to start its interactive action */
void Map::passOver()
{
}

/* Proceeds to pickup the total number of this marked item */
bool Map::pickupItem(MapItem* item)
{
  if(item != 0)
  {
    Frame* dialog_image = item->getDialogImage();
    Sprite* map_image = item->getFrames();
    
    /* Show pickup dialog */
    if(dialog_image != 0 && dialog_image->isImageSet())
      map_dialog.initPickup(dialog_image, item->getCount());
    else if(map_image != 0 && map_image->getSize() > 0)
      map_dialog.initPickup(map_image->getFirstFrame(), item->getCount());
      
    /* Finally, set the on map count to 0 */
    item->setCount(0);
    
    return true;
  }
  
  return false;
}

/* Changes the map section index - what is displayed */
bool Map::setSectionIndex(int index)
{
  if(index >= 0 && index < geography.size() && geography[index].size() > 0)
  {
    map_index = index;
    viewport->setMapSize(geography[index].size(), geography[index][0].size());
    
    return true;
  }
  
  return false;
}
  
// Possibly make the teleport add the ability of shifting map thing
void Map::teleportThing(int id, int tile_x, int tile_y, int section_id)
{
  /* If the section id is below 0, then set to internal map index */
  if(section_id < 0)
    section_id = map_index;
  
  /* Ensure that the tile x and y is within the range */
  if(section_id < geography.size() && 
     geography[section_id].size() > tile_x && tile_x >= 0 && 
     geography[section_id][tile_x].size() > tile_y && tile_y >= 0)
  {
    /* Change the starting tile for the thing */
    for(int i = 0; i < persons.size(); i++)
    {
      if(persons[i]->getID() == id)
      {
        if(persons[i]->setStartingTile(section_id, 
                                       geography[section_id][tile_x][tile_y]))
        {
          map_dialog.endConversation();
          if(map_index != section_id)
            setSectionIndex(section_id);
          persons[i]->clearAllMovement();
        }
      }
    }
  }
}

void Map::unloadMap()
{
  /* Delete the items */
  for(int i = 0; i < items.size(); i++)
  {
    if(items[i] != 0)
      delete items[i];
    items[i] = 0;
  }
  items.clear();

  /* Delete all persons */
  for(int i = 0; i < persons.size(); i++)
  {
    if(persons[i] != 0)
      delete persons[i];
    persons[i] = 0;
  }
  persons.clear();

  /* Delete the things */
  for(int i = 0; i < things.size(); i++)
  {
    if(things[i] != 0)
      delete things[i];
    things[i] = 0;
  }
  things.clear();

    /* Delete all the tiles that have been set */
  for(int i = 0; i < geography.size(); i++)
  {
    for(int j = 0; j < geography[i].size(); j++)
    {
      for(int k = 0; k < geography[i][j].size(); k++)
      {
        delete geography[i][j][k];
        geography[i][j][k] = 0;
      }
      geography[i][j].clear();
    }
    geography[i].clear();
  }
  geography.clear();

  /* Deletes the sprite data stored */
  for(int i = 0; i < tile_sprites.size(); i++)
  {
    delete tile_sprites[i];
    tile_sprites[i] = 0;
  }
  tile_sprites.clear();
  
  /* Reset the viewport */
  viewport->setMapSize(0, 0);
  viewport->lockOn(0, 0);

  /* Clear the remaining and disable the loading */
  //clear();
  loaded = false;
}

/* Updates the map - called by the cycle timer call from game */
void Map::updateMap(int cycle_time)
{
  time_elapsed = cycle_time;
  updateGL();
}
