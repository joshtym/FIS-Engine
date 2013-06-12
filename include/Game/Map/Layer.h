/******************************************************************************
* Class Name: Layer
* Date Created: May 10, 2013
* Inheritance: QGraphicsItem
* Description: This class becomes the middle interface in between a sprite 
*              and converting it into the data needed to add it to the
*              QGraphicsScene. Essentially it just offers the re-implemented
*              functions needed to define a sprite.
******************************************************************************/
#ifndef LAYER_H
#define LAYER_H

#include <QGraphicsItem>
#include <QPainter>

#include "Game/Sprite.h"

class Layer : public QGraphicsItem
{
public:
  /* Constructor functions */
  Layer();
  Layer(Sprite* item, int width, int height, 
        int x = 0, int y = 0, int z = 0);

  /* Destructor function */
  ~Layer();

private:
  /* The information that defines the item */
  Sprite* item;
  int width;
  int height;
  bool visible;

  /* Temporary, DELETE when done */
  int paint_count;

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Virtual bounding rectangle - The rectangle that encapsulates the item */
  QRectF boundingRect() const;

  /* Clear out the layer definition */
  void clear();

  /* Returns the height of the layer */
  int getHeight();

  /* Returns the sprite stored in the layer for control */
  Sprite* getItem();

  /* Returns the number of time the layer has painted */
  int getPaintCount();

  /* Returns the width of the layer */
  int getWidth();

  /* Returns if the painted information in the tile is visible */
  bool isVisible();

  /* Virtual painter reimplementation - for painting the item */
  void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, 
             QWidget* widget);

  /* Sets the height, stored in the layer */
  bool setHeight(int height);

  /* Sets the item within the class */
  bool setItem(Sprite* item, bool unset_old = true);

  /* Sets if the tile is visible or not (black or painted) */
  void setVisible(bool status);

  /* Sets the width, stored in the layer */
  bool setWidth(int width);

  /* Unsets the item within the class */
  void unsetItem(bool deleteItem = true);
};

#endif // LAYER_H
