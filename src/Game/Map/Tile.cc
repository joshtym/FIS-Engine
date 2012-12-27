/******************************************************************************
* Class Name: Tile
* Date Created: Dec 2 2012
* Inheritance: QWidget
* Description: The Tile class
******************************************************************************/
#include "Game/Map/Tile.h"

/* Constructor function */
Tile::Tile(int width, int height, int x, int y, QWidget* parent)
{
  base_set = FALSE;
  enhancer_set = FALSE;
  lower_set = FALSE;
  thing_set = FALSE;
  upper_set = FALSE;
  walkover_set = FALSE;

  setPassibility(TRUE);

  setParent(parent);
  setGeometry(x, y, width, height);
  show();
}

/* Destructor function */
Tile::~Tile()
{
  unsetBase();
  unsetEnhancer();
}

/* Animates all sprites on tile (Including thing and walkover sprites) */
void Tile::animate()
{
  //update();
}

/* Paint event for the tile */
void Tile::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);

  /* Print the base if it exists */
  if(base_set)
    painter.drawPixmap(0, 0, base->getCurrent());

  /* Print the enhancer if it exists */
  if(enhancer_set)
  {
    if(enhancer.size() == 1)
    {
      painter.drawPixmap(0,0,enhancer[0]->getCurrent());
    }
    else
    {
      painter.drawPixmap(0,0,enhancer[0]->getCurrent());
      painter.drawPixmap(32,0,enhancer[1]->getCurrent());
      painter.drawPixmap(0,32,enhancer[2]->getCurrent());
      painter.drawPixmap(32,32,enhancer[3]->getCurrent());
    }
  }
}

/* gets east passiblity */
bool Tile::getPassibilityEast()
{
    return east_passibility;
}

/* gets north passiblity */
bool Tile::getPassibilityNorth()
{
    return north_passibility;
}

/* gets south passiblity */
bool Tile::getPassibilitySouth()
{
    return south_passibility;
}

/* gets west passiblity */
bool Tile::getPassibilityWest()
{
    return west_passibility;
}

/* Returns if the Base Sprite is set */
bool Tile::isBaseSet()
{
  return base_set;
}

/* Returns if the Enhancer Sprite(s) is set */
bool Tile::isEnhancerSet()
{
  return enhancer_set;
}

/* Returns if the Lower Sprite is set */
bool Tile::isLowerSet()
{
  return lower_set;
}

/* Returns if the Thing Sprite is set */
bool Tile::isThingSet()
{
  return thing_set;
}

/* Returns if the Upper Sprite is set */
bool Tile::isUpperSet()
{
  return upper_set;
}

/* Returns if the Walkover Sprite is set */
bool isWalkoverSet();

/* 
 * Description: Sets the base tile using a path to the sprite image file.
 *
 * Inputs: QString path - the path to the image to load in
 * Output: bool - returns TRUE if the base was set before
 */
bool Tile::setBase(QString path)
{
  bool was_base_set = unsetBase();

  /* Set the new base */
  base = new Sprite(path);
  base_set = TRUE;

  return was_base_set;
}

/*
 * Description: Sets the enhancer tile using a path to a single sprite image
 *              file to cover the tile.
 *
 * Inputs: QString path - the path to the image to load in
 * Output: bool - returns TRUE if the enhancer was set before
 */
bool Tile::setEnhancer(QString path)
{
  bool was_enhancer_set = unsetEnhancer();

  /* Sets the new enhancer tile */
  Sprite* new_enhancer = new Sprite(path);
  enhancer.append(new_enhancer);
  enhancer_set = TRUE;

  return was_enhancer_set;
}

/*
 * Description: Sets the enhancer tile using a path to 4 sprites that cover
 *              the 4 corners of the standard tile size. The corners are: 
 *              NW NE
 *              SW SE
 *
 * Inputs: QString nw_path - the NW corner path for the sprite
 *         QString ne_path - the NE corner path for the sprite
 *         QString sw_path - the SW corner path for the sprite
 *         QString se_path - the SE corner path for the sprite
 * Output: bool - returns TRUE if the enhancer was set before
 */
bool Tile::setEnhancer(QString nw_path, QString ne_path, 
                       QString sw_path, QString se_path)
{
  bool was_enhancer_set = unsetEnhancer();

  /* Sets the new enhancer tile with 4 1/4 portions of a tile */
  Sprite* nw_enhancer = new Sprite(nw_path);
  Sprite* ne_enhancer = new Sprite(ne_path);
  Sprite* sw_enhancer = new Sprite(sw_path);
  Sprite* se_enhancer = new Sprite(se_path);
  enhancer.append(nw_enhancer);
  enhancer.append(ne_enhancer);
  enhancer.append(sw_enhancer);
  enhancer.append(se_enhancer);
  enhancer_set = TRUE;

  return was_enhancer_set;
}

/* Sets all passibility */
void Tile::setPassibility(bool is_passable)
{
  north_passibility = is_passable;
  east_passibility = is_passable;
  south_passibility = is_passable;
  west_passibility = is_passable;
}

/* Sets east passiblity */
void Tile::setPassibilityEast(bool is_passable)
{
  east_passibility = is_passable;
}

/* Sets north passiblity */
void Tile::setPassibilityNorth(bool is_passable)
{
  north_passibility = is_passable;
}

/* Sets south passiblity */
void Tile::setPassibilitySouth(bool is_passable)
{
  south_passibility = is_passable;
}

/* Sets west passiblity */
void Tile::setPassibilityWest(bool is_passable)
{
  west_passibility = is_passable;
}

/* 
 * Description: Unsets the base in the tile. Deletes the pointer, if applicable
 *              and sets the internal variable to notify the class so the base
 *              isn't repainted.
 *
 * Inputs: none
 * Output: bool - returns TRUE if the base was set before being unset
 */
bool Tile::unsetBase()
{
  if(base_set)
  {
    delete base;
    base_set = FALSE;
    return TRUE;
  }
  return FALSE;
}

/* 
 * Description: Unsets the enhancer in the tile. Deletes the pointer(s), 
 *              clears out the QVector if applicable, and sets the internal
 *              variable to notify the class so the enhancer isn't repainted.
 *
 * Inputs: none
 * Output: bool - returns TRUE if the enhancer was set before being unset
 */
bool Tile::unsetEnhancer()
{
  if(enhancer_set)
  {
    for(int i = 0; i < enhancer.size(); i++)
      delete enhancer[i];

    while(enhancer.size() > 0)
      enhancer.remove(0);
    
    enhancer_set = FALSE;
    return TRUE;
  }
  return FALSE;
}
