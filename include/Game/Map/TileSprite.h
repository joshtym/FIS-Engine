/******************************************************************************
 * Class Name: TileSprite
 * Date Created: October 28, 2014
 * Inheritance: Sprite
 * Description: This class is for sprites that exist within the tile structure
 *              of the map. It adds additional functionality for dealing with
 *              passability and any other map desired functionality.
 *****************************************************************************/
#ifndef TILESPRITE_H
#define TILESPRITE_H

class Tile;

#include "Sprite.h"

class TileSprite : public Sprite
{
public:
  /* Constructor functions */
  TileSprite();
  TileSprite(std::string path, SDL_Renderer* renderer);
  TileSprite(std::string head_path, int num_frames, 
             std::string tail_path, SDL_Renderer* renderer);

  /* Copy constructor */
  TileSprite(const TileSprite &source);

  /* Destructor function */
  ~TileSprite();
  
private:
  /* Passability of the tile sprite */
  uint8_t passability;

  /* Render depth - 'z' coordinate */
  uint8_t render_depth;

  /* Tile pointers, for the connection of the sprite to the rendering tile */
  Tile* tile_main;
  Tile* tile_previous;
  
  /* -------------------------- Constants ------------------------- */
  //const static uint8_t kMAX_RENDER_DEPTH; /* Max render depth */

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* Copy function, to be called by a copy or equal operator constructor */
  void copySelf(const TileSprite &source);

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Adds sprite information from the XML data classifier from the file */
  bool addFileInformation(XmlData data, int index, SDL_Renderer* renderer, 
                          std::string base_path = "");

  /* Call to add passability, as extracted from file data */
  void addPassability(std::string data);
  
  /* Const get functions for internal variables */
  bool getPassability(Direction dir) const;
  uint8_t getRenderDepth() const;

  /* Returns the tiles, as stored in the class */
  Tile* getTileMain() const;
  Tile* getTilePrevious() const;
  
  /* Resets the passability to default */
  void resetPassability();

  /* Resets the tile that the sprite is rendering on */
  void resetTile();
  
  /* Sets the passability, for a particular direction */
  void setPassability(Direction dir, bool set_value);

  /* Sets the render depth. 0 is the base up to max */
  bool setRenderDepth(uint8_t depth);
 
  /* Sets the starting tile. */
  bool setStartingTile(Tile* starting_tile);

  /* Starts and stops tile move. Relies on underlying logic for occurance */
  void tileMoveFinish();
  bool tileMoveStart(Tile* next_tile);

/*============================================================================
 * OPERATOR FUNCTIONS
 *===========================================================================*/
public:
  TileSprite& operator= (const TileSprite &source);
};

#endif // TILESPRITE_H