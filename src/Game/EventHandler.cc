/******************************************************************************
* Class Name: EventHandler
* Date Created: September 12, 2013
* Inheritance: none
* Description: This event handler contains all events between the multiple
*              classes and manages data transfer between from calls internally.
*              It is merely meant as a facilitator of actions that need to
*              happen deep inside of the nested structure and then need to
*              fire from the top of the chain.
******************************************************************************/
#include "Game/EventHandler.h"

/* Constant Implementation - see header file for descriptions */
//const uint8_t EventHandler::kGIVE_ITEM_COUNT = 1;

/*============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *===========================================================================*/

/* Constructor function */
EventHandler::EventHandler()
{
  event_queue.clear();
  queue_index = 0;
  sound_handler = nullptr;
}

/* Destructor function */
EventHandler::~EventHandler()
{
  event_queue.clear();
  queue_index = 0;
  sound_handler = nullptr;
}

/*============================================================================
 * PRIVATE FUNCTIONS
 *===========================================================================*/

/* Returns the event in the queue: either from the set or event pointer */
bool EventHandler::getEvent(Event& event, bool trigger)
{
  if(pollEventAvailable())
  {
    if(event_queue[queue_index].event_set != nullptr)
    {
      event = event_queue[queue_index].event_set->getEvent(trigger);
    }
    else if(event_queue[queue_index].event_ref != nullptr)
    {
      event = *(event_queue[queue_index].event_ref);
      if(trigger)
        event_queue[queue_index].event_ref->has_exec = true;
    }
    else
    {
      event = event_queue[queue_index].event;
    }
    return true;
  }
  return false;
}

/* Returns the event ref in the queue: either from the set or event pointer */
bool EventHandler::getEventRef(Event*& event_ref, bool trigger)
{
  if(pollEventAvailable())
  {
    /* Find event references */
    if(event_queue[queue_index].event_set != nullptr)
    {
      event_ref = event_queue[queue_index].event_set->getEventRef(trigger);
      return true;
    }
    else if(event_queue[queue_index].event_ref != nullptr)
    {
      event_ref = event_queue[queue_index].event_ref;
      if(trigger)
        event_queue[queue_index].event_ref->has_exec = true;
      return true;
    }
  }
  return false;
}

/* Trigger queue sound */
void EventHandler::triggerQueueSound(Event event)
{
  if(event.sound_id >= 0)
    triggerSound(event.sound_id, SoundChannels::TRIGGERS);
}

/*============================================================================
 * PUBLIC FUNCTIONS
 *===========================================================================*/

/* Execute the given event - done through signal emits */
void EventHandler::executeEvent(Event event, MapPerson* initiator,
                                             MapThing* source)
{
  if(event.classification != EventClassifier::NOEVENT)
  {
    /* Create the executed event queue entry */
    EventExecution executed_event;
    executed_event.event = event;
    executed_event.event_ref = nullptr;
    executed_event.event_set = nullptr;
    executed_event.item = nullptr;
    executed_event.initiator = initiator;
    executed_event.source = source;

    /* Push the event to the back of the queue */
    event_queue.push_back(executed_event);
  }
}

/* Execute the given event reference */
void EventHandler::executeEventRef(Event* event, MapPerson* initiator,
                                   MapThing* source)
{
  if(event != nullptr && event->classification != EventClassifier::NOEVENT &&
     (!event->one_shot || !event->has_exec))
  {
    /* Create the executed eevent queue entry */
    EventExecution executed_event;
    executed_event.event = EventSet::createBlankEvent();
    executed_event.event_ref = event;
    executed_event.event_set = nullptr;
    executed_event.item = nullptr;
    executed_event.initiator = initiator;
    executed_event.source = source;

    /* Push the event to the back of the queue */
    event_queue.push_back(executed_event);
  }
}

/* Execute the given event set */
void EventHandler::executeEventSet(EventSet* set, MapPerson* initiator,
                                   MapThing* source)
{
  if(set != nullptr && !set->isEmpty())
  {
    /* Create the executed event queue entry */
    EventExecution executed_event;
    executed_event.event = EventSet::createBlankEvent();
    executed_event.event_ref = nullptr;
    executed_event.event_set = set;
    executed_event.item = nullptr;
    executed_event.initiator = initiator;
    executed_event.source = source;

    /* Push the event to the back of the queue */
    event_queue.push_back(executed_event);
  }
}

/* Executes an MIO trigger event */
void EventHandler::executeIOTrigger(MapInteractiveObject* io,
                                    int interaction_state,
                                    MapPerson* initiator)
{
  /* Create the executed event queue entry */
  if(io != nullptr && interaction_state >= 0)
  {
    /* Create the executed event queue entry */
    EventExecution executed_event;
    executed_event.event = EventSet::createBlankEvent();
    executed_event.event_ref = nullptr;
    executed_event.event_set = nullptr;
    executed_event.item = nullptr;
    executed_event.initiator = initiator;
    executed_event.source = (MapThing*)io;

    /* Specific event properties */
    executed_event.event.classification = EventClassifier::TRIGGERIO;
    executed_event.event.ints.push_back(interaction_state);

    /* Push the event to the back of the queue */
    event_queue.push_back(executed_event);
  }
}

/* Executes a pickup item event */
void EventHandler::executePickup(MapItem* item, bool walkover)
{
  /* Create the event and identify */
  Event new_event = EventSet::createBlankEvent();
  new_event.classification = EventClassifier::PICKUPITEM;
  new_event.ints.push_back(walkover);

  /* Now execute the pickup event (throw it on the queue) */
  executeEvent(new_event, NULL);
  event_queue.back().item = item;
}

/* Returns a reference to the key handler */
KeyHandler& EventHandler::getKeyHandler()
{
  return key_handler;
}

/* Returns a reference to the sound handler */
SoundHandler* EventHandler::getSoundHandler()
{
  return sound_handler;
}

/* Clears the event handling poll queue */
void EventHandler::pollClear()
{
  event_queue.clear();
  queue_index = 0;
}

/* Poll a conversation event. Only true if this event is next on queue */
bool EventHandler::pollConversation(Conversation** convo, MapThing** source)
{
  if(pollEventType() == EventClassifier::STARTCONVO && convo != NULL &&
     source != NULL)
  {
    Event event;
    if(getEvent(event, true))
    {
      *convo = event.convo;
      *source = event_queue[queue_index].source;
      triggerQueueSound(event);
      return true;
    }
  }

  return false;
}

/* Poll a give item event */
bool EventHandler::pollGiveItem(int* id, int* count)
{
  if(id != nullptr && count != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventGiveItem(event, *id, *count))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* It goes to the next event in the poll sequence. Returns true if there is
 * one */
bool EventHandler::pollEvent()
{
  /* Update the index if it's still under the queue size */
  if(queue_index < event_queue.size())
    queue_index++;

  /* Check if the poll is complete */
  if(queue_index >= event_queue.size())
    return false;
  return true;
}

/* Checks if there is any valid event available - usually called first */
bool EventHandler::pollEventAvailable()
{
  return (event_queue.size() > 0 && queue_index < event_queue.size());
}

/* Returns the current event type, to be polled by the management class */
EventClassifier EventHandler::pollEventType()
{
  Event event;
  if(getEvent(event, false))
  {
    return event.classification;
  }
  return EventClassifier::NOEVENT;
}

/* Polls to see if the current event is locked and can be unlocked (such as
 * with a have item call */
bool EventHandler::pollLock(LockedState& state)
{
  if(pollLockAvail())
  {
    state = event_queue[queue_index].event_set->getLockedState().state;
    return true;
  }
  return false;
}

/* Is a lock avaiable? */
bool EventHandler::pollLockAvail()
{
  if(queue_index < event_queue.size() &&
     event_queue[queue_index].event_set != nullptr)
  {
    EventSet* set = event_queue[queue_index].event_set;

    /* Determine if the current set is locked and if its a locked state that
     * needs to be reviewed at call time */
    if(set->isLocked() &&
       (set->getLockedState().state == LockedState::ITEM))
    {
      return true;
    }
  }
  return false;
}

/* Get the locked struct, only if data is available */
bool EventHandler::pollLockGetData(Locked& lock)
{
  if(pollLockAvail())
  {
    lock = event_queue[queue_index].event_set->getLockedState();
    return true;
  }
  return false;
}

/* Sets the locked struct, only if data is available */
bool EventHandler::pollLockSetData(Locked lock)
{
  if(pollLockAvail())
  {
    EventSet* set = event_queue[queue_index].event_set;
    set->setLocked(lock);
    return true;
  }
  return false;
}

/* Poll the empty event */
bool EventHandler::pollNone()
{
  if(pollEventType() == EventClassifier::NOEVENT)
  {
    Event event;
    getEvent(event, true);
    return true;
  }
  return false;
}

/* Poll a notification event */
bool EventHandler::pollNotification(std::string* notification)
{
  if(notification != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventNotification(event, *notification))
    {

      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll a pickup item event */
bool EventHandler::pollPickupItem(MapItem** item, bool* walkover)
{
  if(pollEventType() == EventClassifier::PICKUPITEM && item != NULL &&
     walkover != NULL && event_queue[queue_index].event.ints.size() == 1)
  {
    *item = event_queue[queue_index].item;
    *walkover = event_queue[queue_index].event.ints[0];

    return true;
  }

  return false;
}

/* Poll a sound event */
bool EventHandler::pollSound()
{
  Event event;
  if(pollEventType() == EventClassifier::JUSTSOUND &&
     getEvent(event, true))
  {
    triggerQueueSound(event);
    return true;
  }
  return false;
}

/* Poll a start battle event */
bool EventHandler::pollStartBattle(MapPerson** person, MapThing** source,
                                   BattleFlags& flags, Event*& event_win,
                                   Event*& event_lose)
{
  if(pollEventType() == EventClassifier::RUNBATTLE &&
     person != nullptr && source != nullptr)
  {
    Event* event;
    if(getEventRef(event, true) &&
       EventSet::dataEventStartBattle(event, flags, event_win, event_lose))
    {
      *person = event_queue[queue_index].initiator;
      *source = event_queue[queue_index].source;
      triggerQueueSound(*event);
      return true;
    }
  }
  return false;
}

/* Poll a start map event */
bool EventHandler::pollStartMap(int* id)
{
  if(id != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventStartMap(event, *id))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll a take item event */
bool EventHandler::pollTakeItem(int* id, int* count)
{
  if(id != nullptr && count != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventTakeItem(event, *id, *count))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll a teleport thing event */
bool EventHandler::pollTeleportThing(int* thing_id, int* x, int* y,
                                                    int* section_id)
{
  if(thing_id != nullptr && x != nullptr && y != nullptr &&
     section_id != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventTeleport(event, *thing_id, *x, *y, *section_id))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll a trigger IO event */
bool EventHandler::pollTriggerIO(MapInteractiveObject** io, int* state,
                                 MapPerson** initiator)
{
  if(pollEventType() == EventClassifier::TRIGGERIO && io != nullptr &&
     state != nullptr && initiator != nullptr &&
     event_queue[queue_index].event.ints.size() == 1)
  {
    *io = (MapInteractiveObject*)event_queue[queue_index].source;
    *state = event_queue[queue_index].event.ints.front();
    *initiator = event_queue[queue_index].initiator;

    return true;
  }
  return false;
}

/* Poll the unlock event(s) */
bool EventHandler::pollUnlockIO(int* io_id, UnlockIOMode* mode, int* state_num,
                                UnlockIOEvent* mode_events,
                                UnlockView* mode_view, int* view_time)
{
  if(io_id != nullptr && mode != nullptr && state_num != nullptr &&
     mode_events != nullptr && mode_view != nullptr && view_time != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventUnlockIO(event, *io_id, *mode, *state_num,
                                   *mode_events, *mode_view, *view_time))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll the unlock event(s) */
bool EventHandler::pollUnlockThing(int* thing_id, UnlockView* mode_view,
                                   int* view_time)
{
  if(thing_id != nullptr && mode_view != nullptr && view_time != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventUnlockThing(event, *thing_id, *mode_view, *view_time))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Poll the unlock event(s) */
bool EventHandler::pollUnlockTile(int* section_id, int* tile_x, int* tile_y,
                                  UnlockTileMode* mode, UnlockView* mode_view,
                                  int* view_time)
{
  if(section_id != nullptr && tile_x != nullptr && tile_y != nullptr &&
     mode != nullptr && mode_view != nullptr && view_time != nullptr)
  {
    Event event;
    if(getEvent(event, true) &&
       EventSet::dataEventUnlockTile(event, *section_id, *tile_x, *tile_y,
                                     *mode, *mode_view, *view_time))
    {
      triggerQueueSound(event);
      return true;
    }
  }
  return false;
}

/* Sets the sound handler used. If unset, no sounds will play */
void EventHandler::setSoundHandler(SoundHandler* new_handler)
{
  sound_handler = new_handler;
}

/* Trigger Audio Stop - on channel */
void EventHandler::triggerAudioStop(SoundChannels channel)
{
  if(sound_handler != nullptr)
  {
    sound_handler->addStopToQueue(channel);
  }
}

/* Trigger music file */
void EventHandler::triggerMusic(uint32_t id)
{
  if(sound_handler != nullptr)
  {
    sound_handler->addPlayToQueue(id, SoundChannels::MUSIC1);
  }
}

/* Trigger sound file */
void EventHandler::triggerSound(uint32_t id, SoundChannels channel)
{
  if(sound_handler != nullptr)
  {
    if(channel != SoundChannels::MUSIC1 || channel != SoundChannels::MUSIC2 ||
       channel != SoundChannels::UNASSIGNED)
    {
      sound_handler->addPlayToQueue(id, channel);
    }
  }
}

/* Trigger weather file */
void EventHandler::triggerWeather(uint32_t id)
{
  if(sound_handler != nullptr)
  {
    sound_handler->addPlayToQueue(id, SoundChannels::WEATHER1);
  }
}
