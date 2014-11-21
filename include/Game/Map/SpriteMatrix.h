/******************************************************************************
 * Class Name: SpriteMatrix
 * Date Created: November 20, 2014
 * Inheritance: none
 * Description: This class contains the logic for matrix of sprites that will
 *              be rendered as a group. Primarily utilized by MapThing and 
 *              children.
 *****************************************************************************/
#ifndef SPRITEMATRIX_H
#define SPRITEMATRIX_H

#include "Game/Map/TileSprite.h"

class SpriteMatrix
{
public:
  /* Constructor functions */
  SpriteMatrix();
  SpriteMatrix(std::vector<std::vector<TileSprite*>> sprites);

  /* Copy constructor */
  SpriteMatrix(const SpriteMatrix &source);

  /* Destructor function */
  ~SpriteMatrix();
  
private:
  /* Matrix of sprites */
  std::vector<std::vector<TileSprite*>> sprite_matrix;

  /* -------------------------- Constants ------------------------- */
  //const static uint8_t kMAX_RENDER_DEPTH; /* Max render depth */

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/
private:
  /* Copy function, to be called by a copy or equal operator constructor */
  void copySelf(const SpriteMatrix &source);

  /* Counts the valid sprites */
  uint16_t countValidSprites();

  /* Gets a valid sprite from the frame matrix. If one doesn't exist, one is
   * made */
  TileSprite* getValidSprite();

  /* Grow the sprite matrix to the indicated size */
  void growMatrix(uint16_t x, uint16_t y);

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/
public:
  /* Adds the matrix information from the XML data classifier from file */
  bool addFileInformation(XmlData data, int index, SDL_Renderer* renderer, 
                          std::string base_path = "");

  /* Returns the sprite at a given coordinate */
  TileSprite* at(uint16_t x, uint16_t y);

  /* Shrinks the sprite matrix to the valid size and removes all null and
   * void pointers */
  void cleanMatrix();

  /* Returns the height of the sprite matrix */
  uint16_t getHeight() const;
 
  /* Returns the stored sprite matrix */
  std::vector<std::vector<TileSprite*>> getMatrix() const;
  
  /* Returns the width of the sprite matrix */
  uint16_t getWidth() const;

  /* Rendering function, for the entire matrix */
  bool render(SDL_Renderer* renderer, int start_x, int start_y, 
              int offset_x, int offset_y);

  /* Sets an individual sprite in the matrix */
  bool setSprite(TileSprite* sprite, uint16_t x, uint16_t y, 
                 bool delete_old = true);

  /* Sets the entire set of sprites in the matrix */
  bool setSprites(std::vector<std::vector<TileSprite*>> sprites, 
                  bool delete_old = false);

  /* Sets all tiles for all entities in the sprite matrix */
  bool setTiles(std::vector<std::vector<Tile*>> tiles, bool fresh_start = true);

  /* Unsets an individual sprite in the matrix */
  void unsetSprite(uint16_t x, uint16_t y, bool delete_sprite = true);

  /* Unsets the entire matrix of sprites in the matrix */
  void unsetSprites(bool delete_sprites = true);

  /* Unsets all tiles for all entities in the sprite matrix */
  void unsetTiles();

/*============================================================================
 * OPERATOR FUNCTIONS
 *===========================================================================*/
public:
  SpriteMatrix& operator= (const SpriteMatrix &source);
};

#endif // SPRITE_MATRIX_H
