/*******************************************************************************
 * Class Name: MapInteractiveObject
 * Date Created: Dec 2 2012
 * Inheritance: MapThing
 * Description: This is the object is a specialization of the stock map thing
 *              that allows interaction from key presses by players in 
 *              nearby slots. Pressing the key will toggle the object and allow
 *              it to change states. These objects are unmovable and are of the
 *              typical interactive objects such as doors, chests, etc.
 *
 * TODO:
 * 1. Locked status of MIO??
 * 2. Possibly optimize the walkon/walkoff in update. Seems inefficient to
 *    constantly check when it could be triggered by a walk on/walk off event
 *    (Possibly built into the thing itself?)
 ******************************************************************************/
#include "Game/Map/MapInteractiveObject.h"

// TODO: Comment All

/* Constant Implementation - see header file for descriptions */
const short MapInteractiveObject::kRETURN_TIME_UNUSED = -1;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/*
 * Description: Base constructor function. No parameters. Initializes all 
 *              parameters blank.
 *
 * Inputs: none
 */
MapInteractiveObject::MapInteractiveObject() : MapThing()
{
  action_initiator = NULL;
  node_current = NULL;
  node_head = NULL;
  person_on = NULL;
  shifting_forward = true;
  time_elapsed = 0;
  time_return = kRETURN_TIME_UNUSED;
}

/* 
 * Description: ID based constructor function. Sets up an interactive object 
 *              with the base introduction parameters. 
 *
 * Inputs: int id - the id of the MIO
 *         std::string name - the name of the MIO
 *         std::string description - the description of the MIO
 */
MapInteractiveObject::MapInteractiveObject(int id, std::string name,
                                           std::string description)
                    : MapThing(id, name, description)
{
  action_initiator = NULL;
  node_current = NULL;
  node_head = NULL;
  person_on = NULL;
  shifting_forward = true;
  time_elapsed = 0;
  time_return = kRETURN_TIME_UNUSED;
}

/*
 * Description: Destructor function
 */
MapInteractiveObject::~MapInteractiveObject()
{
  unsetMatrix();
  clear();
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

/*
 * Description: Appends an empty node onto the back of the stack of states.
 *
 * Inputs: none
 * Output: none
 */
void MapInteractiveObject::appendEmptyNode()
{
  StateNode* node = new StateNode;

  /* Set the state parameters */
  node->state = NULL;
  node->transition = NULL;
  node->passable = false;
  node->previous = NULL;
  node->next = NULL;

  /* Assign the node to the tail of the sequence */
  appendNode(node);
}

/*
 * Description: Appends a node passed in to the end of the list of states
 *
 * Inputs: StateNode* node - node pointer to append
 * Output: none
 */
void MapInteractiveObject::appendNode(StateNode* node)
{
  StateNode* tail_node = getTailNode();

  /* Assign the node to the tail of the sequence */
  node->previous = tail_node;
  if(tail_node == NULL)
  {
    node_head = node;
    node_current = node_head;
    shifting_forward = true;
    setParentFrames();
  }
  else
    tail_node->next = node;
}

/*
 * Description: Removes a node at a given ID and clears the memory for said
 *              node, which represents a state in the stack.
 *
 * Inputs: int id - id corresponding to a state in the stack
 * Output: bool - true if the node was successfully cleared
 */
bool MapInteractiveObject::clearNode(int id)
{
  StateNode* node_to_clear = getNode(id);

  /* Check if the node is valid */
  if(node_to_clear != NULL)
  {
    /* Delete state pointer, if relevant */
    if(node_to_clear->state != NULL)
      delete node_to_clear->state;
    node_to_clear->state = NULL;

    /* Delete transition pointer, if relevant */
    if(node_to_clear->transition != NULL)
      delete node_to_clear->transition;
    node_to_clear->transition = NULL;

    return true;
  }

  return false;
}

/*
 * Description: Returns the node based on the ID. NULL if doesn't exist.
 *
 * Inputs: int id - id corresponding to a state in the stack
 * Output: StateNode* - pointer to the node at the ID. NULL if none
 */
StateNode* MapInteractiveObject::getNode(int id)
{
  StateNode* selected = NULL;

  if(id >= 0 && id < getNodeLength())
  {
    selected = node_head;

    while(id > 0)
    {
      selected = selected->next;
      id--;
    }
  }

  return selected;
}

/*
 * Description: Returns the length of the stack of states, as part of the MIO.
 *
 * Inputs: none
 * Output: int - integer representing the number of states
 */
int MapInteractiveObject::getNodeLength()
{
  StateNode* parser = node_head;
  int length = 0;

  /* Determine the length of the node sequence by parsing through it */
  while(parser != NULL)
  {
    length++;
    parser = parser->next;
  }

  return length;
}

/*
 * Description: Returns the state node at the tail of the stack (last ID)
 * 
 * Inputs: none
 * Output: StateNode* - the node pointer for the tail state
 */
StateNode* MapInteractiveObject::getTailNode()
{
  StateNode* tail = node_head;

  /* If no nodes have been set, return null */
  if(tail == NULL)
    return tail;

  /* Otherwise, find the tail */
  while(tail->next != NULL)
    tail = tail->next;

  return tail;
}

/*
 * Description: Handles the connection to the parent class (thing matrix) and
 *              connecting it to the correct state, based on the current ptr.
 *              It will also set the animation back to the main frames.
 *
 * Inputs: none
 * Output: none
 */
void MapInteractiveObject::setParentFrames()
{
  /* Only proceed if the current node is non-NULL */
  if(node_current != NULL)
  {
    /* Determine if this is a transition sequence or a state sequence */
    if(node_current->state != NULL)
    {
      node_current->state->getMatrix()->setAtFirst();
      setMatrix(node_current->state->getMatrix());
      animate(0, true, false);
    }
    else if(node_current->transition != NULL)
    {
      node_current->transition->setAtFirst();
      node_current->transition->setDirectionForward();
      setMatrix(node_current->transition);
      animate(0, true, false);

      /* Treat the sprite sequence according to the order, if it's in reverse
       * or not */
      if(shifting_forward)
      {
        node_current->transition->setDirectionForward();
      }
      else
      {
        node_current->transition->setDirectionReverse();
        node_current->transition->shiftNext();
      }
    }
  }
}

/*
 * Description: Shifts the current state node to the next one, based on the 
 *              direction designated in the class. Passes the call to
 *              shiftNext() or shiftPrevious(), as applicable.
 *
 * Inputs: none
 * Output: bool - true if the shift was successful
 */
bool MapInteractiveObject::shift()
{
  bool status = false;

  /* Do the interaction based on the direction travelling */
  if(shifting_forward)
  {
    if(!shiftNext())
    {
      shifting_forward = false;
      status |= shiftPrevious();
    }
    else
      status = true;
  }
  else
  {
    if(!shiftPrevious())
    {
      shifting_forward = true;
      status |= shiftNext();
    }
    else
      status = true;
  }

  return status;
}

/*
 * Description: Shifts to the next state in the linear linked list. This shifts
 *              to a node with a greater ID (right). This also handles event
 *              triggers and animations.
 *
 * Inputs: none
 * Output: bool - true if the state has been shifted
 */
bool MapInteractiveObject::shiftNext()
{
  if(node_current != NULL && node_current->next != NULL)
  {
    /* Fire exit event */
    if(node_current->state != NULL)
      node_current->state->triggerExitEvent(action_initiator);

    /* Shift the pointer and update the frames */
    node_current = node_current->next;
    time_elapsed = 0;
    setParentFrames();

    /* Fire enter event */
    if(node_current->state != NULL)
      node_current->state->triggerEnterEvent(action_initiator);

    return true;
  }

  return false;
}

/*
 * Description: Shifts to the previous state in the linear linked list. This 
 *              shifts to a node with a lesser ID (left). This also handles 
 *              event triggers and animations.
 *
 * Inputs: none
 * Output: bool - true if the state has been shifted
 */
bool MapInteractiveObject::shiftPrevious()
{
  if(node_current != NULL && node_current->previous != NULL)
  {
    /* Fire exit event */
    if(node_current->state != NULL)
      node_current->state->triggerExitEvent(action_initiator);

    node_current = node_current->previous;
    time_elapsed = 0;
    setParentFrames();

    /* Fire enter event */
    if(node_current->state != NULL)
      node_current->state->triggerEnterEvent(action_initiator);

    return true;
  }

  return false;
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

/*
 * Description: Adds IO information from the XML file. This will be
 *              virtually called by children where the child will deal with
 *              additional sets needed and then pass call to this. Works off
 *              reading the XML file in a strict manner. Passes call to parent
 *              after it is complete.
 *
 * Inputs: XmlData data - the read XML data
 *         int file_index - the index in the xml data where this detail begins
 *         int section_index - the map section index of the npc
 *         SDL_Renderer* renderer - the graphical rendering engine pointer
 *         std::string base_path - the base path for resources
 * Output: bool - status if successful
 */
bool MapInteractiveObject::addThingInformation(XmlData data, int file_index, 
                                               int section_index, 
                                               SDL_Renderer* renderer, 
                                               std::string base_path)
{
  std::vector<std::string> elements = data.getTailElements(file_index);
  std::string identifier = data.getElement(file_index);
  bool success = true;

  /* Ensure there is at least one valid node */
  if(node_head == NULL)
  {
    appendEmptyNode();
    node_head->state = new MapState();
    node_head->state->setMatrix(new SpriteMatrix());
    setParentFrames();
  }

  /* Parse the identifier for setting the IO information */
  /*--------------- INACTIVITY TIME -----------------*/
  if(identifier == "inactive" && elements.size() == 1)
  {
    setInactiveTime(data.getDataInteger(&success));
  }
  /*----------------- RENDER MATRIX -----------------*/
  else if(elements.size() == 1 && elements.front() == "rendermatrix")
  {
    StateNode* node = node_head;
    while(node != NULL)
    {
      /* Set the render matrix */
      if(node->state != NULL && node->state->getMatrix() != NULL)
        node->state->getMatrix()->setRenderMatrix(data.getDataString(&success));
      else if(node->transition != NULL)
        node->transition->setRenderMatrix(data.getDataString(&success));

      /* Bump the node */
      node = node->next;
    }
  }
  /*--------------- STATES FOR MIO -----------------*/
  else if(identifier == "states" && elements.size() > 2 && 
          (elements[1] == "state" || elements[1] == "transition") && 
          data.getKey(file_index + 1) == "id")
  {
    int index = std::stoi(data.getKeyValue(file_index + 1));
    int length = getNodeLength();

    /* Fill up the node length to suffice for the given index */
    while(length <= index)
    {
      appendEmptyNode();
      length++;
    }

    /* Proceed to determine if it was changed */
    StateNode* modified_node = getNode(index);
    if(modified_node != NULL)
    {
      /* Get the render matrix of the head node */
      std::string render_matrix = "";
      if(node_head->state != NULL)
        render_matrix = node_head->state->getMatrix()->getRenderMatrix();
      else
        render_matrix = node_head->transition->getRenderMatrix();

      /* Clear the node if it has the wrong data in it */
      if((elements[1] == "state" && modified_node->transition != NULL) || 
         (elements[1] == "transition" && modified_node->state != NULL))
        clearNode(index);

      /* Move forward with updating the given state/transiton sequence */
      if(elements[1] == "state")
      {
        /* Make the state if it doesn't exist */
        if(modified_node->state == NULL)
        {
          modified_node->state = new MapState(event_handler);
          modified_node->state->getMatrix()->setRenderMatrix(render_matrix);
          if(modified_node == node_current)
            setParentFrames();
        }

        /* Update the state */
        success &= modified_node->state->addFileInformation(data, 
                            file_index + 2, section_index, renderer, base_path);
      }
      /*--------------------- TRANSITION FRAMES -----------------*/
      else if(elements[1] == "transition")
      {
        /* Make the transition if it doesn't exist */
        if(modified_node->transition == NULL)
        {
          modified_node->transition = new SpriteMatrix();
          modified_node->transition->setRenderMatrix(render_matrix);
          if(modified_node == node_current)
            setParentFrames();
        }

        /* Update the transition */
        success &= modified_node->transition->addFileInformation(data, 
                                           file_index + 3, renderer, base_path);
      }
    }
  }
  else
  {
    /* Proceed to parent */
    success &= MapThing::addThingInformation(data, file_index, 
                                             section_index, renderer, 
                                             base_path);
  }

  return success;
}

/*
 * Description: This is the class descriptor. Primarily used for encapsulation
 *              to determine which class to cast it to for specific parameters.
 *
 * Inputs: none
 * Output: std::string - the string descriptor, it will be the same as the class
 *                       name. For example, "MapThing", "MapPerson", etc.
 */
std::string MapInteractiveObject::classDescriptor()
{
  return "MapInteractiveObject";
}

/*
 * Description: Takes the frame matrix, as it's been set up and removes any 
 *              rows or columns at the end that have no valid frames set. A
 *              frame is classified as valid if it's not NULL and has renderable
 *              frames stored within it.
 *
 * Inputs: none
 * Output: bool - true if clean validated frame data
 */
bool MapInteractiveObject::cleanMatrix()
{
  bool equal_size = true;
  uint16_t height = 0;
  uint16_t width = 0;
  StateNode* node = node_head;
  
  /* Parse through each node and check/clean the data involved */
  while(node != NULL)
  {
    SpriteMatrix* matrix = NULL;

    /* Get the matrix corresponding to the node */
    if(node->state != NULL)
      matrix = node->state->getMatrix();
    else
      matrix = node->transition;

    if(matrix != NULL)
    {
      /* Clean the matrix */
      matrix->cleanMatrix();

      /* Get the first state width and height */
      if(height == 0 || width == 0)
      {
        height = matrix->height();
        width = matrix->width();
      }
      else if(height != matrix->height() || width != matrix->width())
      {
        equal_size = false;
      }
    }

    /* Shift to the next */
    node = node->next;
  }

  return equal_size;
}

/*
 * Description: Clears all variables in the MIO, back to blank default state.
 *
 * Inputs: none
 * Output: none
 */
void MapInteractiveObject::clear()
{
  /* Clears action initiator pointer */
  action_initiator = NULL;
  person_on = NULL;
  shifting_forward = true;
  time_elapsed = 0;
  time_return = kRETURN_TIME_UNUSED;
  
  /* Clear states */
  unsetFrames();

  /* Parent cleanup */
  MapThing::clear();
}

/*
 * Description: Returns the inactive time before returning back down the state
 *              path.
 *
 * Inputs: none
 * Output: int - the integer inactive time. -1 if inactive time not used
 */
int MapInteractiveObject::getInactiveTime()
{
  return time_return;
}

/*
 * Description: Returns the current state node.
 *
 * Inputs: none
 * Output: StateNode* - state node pointer. DO NOT DELETE. NULL if unset
 */
StateNode* MapInteractiveObject::getStateCurrent()
{
  return node_current;
}

/*
 * Description: Returns the head state node.
 *
 * Inputs: none
 * Output: StateNode* - state node pointer. DO NOT DELETE. NULL if unset
 */
StateNode* MapInteractiveObject::getStateHead()
{
  return node_head;
}

/*
 * Description: The generic interact call. This is what fires when a player
 *              presses a use key and then this searches for if an event is
 *              available and starts it.
 *
 * Inputs: MapPerson* initiator - the pointer to the person that initiated it
 * Output: bool - if the call can occur (Event handler needs to be set)
 */
bool MapInteractiveObject::interact(MapPerson* initiator)
{
  bool status = false;

  /* Only proceed if the node is available */
  if(node_current != NULL && node_current->state != NULL)
  {
    /* Set the action initiator for the state */
    action_initiator = initiator;

    /* Fire the use event, if it exists */
    status |= node_current->state->triggerUseEvent(initiator);

    /* Proceed to execute a use event, if it exists */
    if(node_current->state->getInteraction() == MapState::USE)
      status |= shift();

    /* Reset the time elapsed */
    time_elapsed = 0;
  }

  return status;
}

/*
 * Description: Resets the MIO back to the head state.
 * 
 * Inputs: none
 * Output: none
 */
void MapInteractiveObject::reset()
{
  node_current = node_head;
  shifting_forward = true;
  setParentFrames();
}

/*
 * Description: Sets the inactive time before the node sequence begins reverting
 *              to prior states.
 *
 * Inputs: int time - the time in ms before returning down the state stack.
 * Output: none
 */
void MapInteractiveObject::setInactiveTime(int time)
{
  if(time <= 0)
    time_return = kRETURN_TIME_UNUSED;
  else
    time_return = time;

  time_elapsed = 0;
}

/*
 * Description: Sets the passed in main state at the tail end of the MIO stack.
 *
 * Inputs: MapState* state - new state to append to the stack
 * Output: bool - true if the node was added
 */
bool MapInteractiveObject::setState(MapState* state)
{
  if(state != NULL && state->getMatrix() != NULL)
  {
    StateNode* node = new StateNode;

    /* Set the state parameters */
    node->state = state;
    node->transition = NULL;
    node->previous = NULL;
    node->next = NULL;

    /* Assign the node to the tail of the sequence */
    appendNode(node);

    return true;
  }

  return false;
}

/*
 * Description: Sets the passed in transition state at the tail end of the MIO 
 *              stack.
 *
 * Inputs: MapState* state - new transition state to append to the stack
 * Output: bool - true if the node was added
 */
bool MapInteractiveObject::setState(SpriteMatrix* transition)
{
  if(transition != NULL)
  {
    StateNode* node = new StateNode;

    /* Set the state parameters */
    node->state = NULL;
    node->transition = transition;
    node->passable = passable;
    node->previous = NULL;
    node->next = NULL;

    /* Assign the node to the tail of the sequence */
    appendNode(node);

    return true;
  }

  return false;
}

/*
 * Description: Triggers the walk off event for the passed of person. Pertinent
 *              for triggering events. Handled by the movement of thing.
 *
 * Inputs: MapPerson* trigger - the triggering person pointer
 * Output: none
 */
void MapInteractiveObject::triggerWalkOff(MapPerson* trigger)
{
  if(trigger != NULL && person_on == trigger)
  {
    /* Trigger walk off interaction */
    if(node_current != NULL && node_current->state != NULL && 
       node_current->state->getInteraction() == MapState::WALKOFF)
      shift();

    person_on = NULL;
  }
}

/*
 * Description: Triggers the walk on event for the passed of person. Pertinent
 *              for triggering events. Handled by the movement of thing.
 *
 * Inputs: MapPerson* trigger - the triggering person pointer
 * Output: none
 */
void MapInteractiveObject::triggerWalkOn(MapPerson* trigger)
{
  if(trigger != NULL && person_on == NULL)
  {
    person_on = trigger;

    if(node_current != NULL && node_current->state != NULL)
    {
      /* Trigger walkover event, if valid */
      node_current->state->triggerWalkoverEvent(person_on);

      /* Trigger walk on interaction */
      if(node_current->state->getInteraction() == MapState::WALKON)
        shift();
    }
  }
}

/*
 * Description: Updates the frames of the MIO. This can include animation
 *              sequencing or movement and such. Called on the tick.
 *
 * Inputs: int cycle_time - the ms time to update the movement/animation
 *         std::vector<std::vector<Tile*>> tile_set - the next tiles to move to
 * Output: none 
 */
void MapInteractiveObject::update(int cycle_time, 
                                  std::vector<std::vector<Tile*>> tile_set)
{
  (void)tile_set;

  if(isTilesSet())
  {
    /* Animate the frames and determine if the frame has changed */
    bool frames_changed = animate(cycle_time, false, false);

    /* Only proceed if frames are changed and a transitional sequence  */
    if(frames_changed && node_current != NULL && 
       node_current->transition != NULL)
    {
      if(node_current->transition->isDirectionForward() && 
         node_current->transition->isAtFirst())
      {
        /* Try and shift to the next state. If fails, re-animate transition */
        if(!shiftNext())
        {
          node_current->transition->setDirectionReverse();
          shifting_forward = false;
          time_elapsed = 0;
        }
      }
      else if(!node_current->transition->isDirectionForward() && 
              node_current->transition->isAtEnd())
      {
        /* Try and shift to the previous state. If fails, re-animate 
         * transition */
        if(!shiftPrevious())
        {
          node_current->transition->setDirectionForward();
          shifting_forward = true;
          time_elapsed = 0;
        }
      }
    }
  
    /* Determine if the cycle time has passed on activity response */
    if(time_return != kRETURN_TIME_UNUSED && node_current != node_head)
    {
      time_elapsed += cycle_time;
      if(time_elapsed > time_return)
      {
        shifting_forward = false;
        shiftPrevious();
      }
    }
  }
}

/*
 * Description: Unsets the frames that are embedded in all state nodes in the
 *              MIO.
 *
 * Inputs: bool delete_state - should the old frames be deleted?
 * Output: none 
 */
void MapInteractiveObject::unsetFrames(bool delete_frames)
{
  StateNode* node = node_head;
  unsetMatrix();
  
  /* Proceed to delete all nodes */
  while(node != NULL)
  {
    StateNode* temp_node = node;
    node = node->next;
    
    /* Proceed to delete all relevant data */
    if(delete_frames)
    {
      if(temp_node->state != NULL)
        delete temp_node->state;
      else if(temp_node->transition != NULL)
        delete temp_node->transition;
      delete temp_node;
    }
  }

  /* Clear the class data */
  node_current = NULL;
  node_head = NULL;
}
