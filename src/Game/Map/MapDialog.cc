/*******************************************************************************
 * Class Name: MapDialog
 * Date Created: August 7, 2013
 * Inheritance: none
 * Description: The dialog display at the bottom of the map. Offers either
 *              conversation options and display or notifications. This requires
 *              that system options have been set via setConfiguration(). If
 *              this is not done, it will seg fault. This is intentional as an
 *              optimization.
 *
 * TODO:
 *  - Number entry, text entry. Shop mode? Also, built into conversation
 *
 * Text HTML like response for manipulation description:
 *  - [b] - bold
 *  - [u] - underline
 *  - [i] - italic
 *  - [ff0000] - color change in hex rrggbb
 *  - {12} - id to text name reference change
 ******************************************************************************/
#include "Game/Map/MapDialog.h"

/* Constant Implementation - see header file for descriptions */
const uint8_t MapDialog::kBORDER_WIDTH = 1;
const float MapDialog::kBUBBLES_ANIMATE = 277.67;
const uint8_t MapDialog::kBUBBLES_COUNT = 3;
const uint8_t MapDialog::kBUBBLES_OFFSET = 16;
const uint8_t MapDialog::kBUBBLES_SPACING = 7;
const SDL_Color MapDialog::kCOLOR_ADD = {0, 210, 13, 255};
const SDL_Color MapDialog::kCOLOR_REM = {232, 0, 13, 255};
const uint16_t MapDialog::kCURSOR_ANIMATE = 300;
const uint8_t MapDialog::kCURSOR_HEIGHT = 8;
const uint8_t MapDialog::kHIGHLIGHT_MARGIN = 5;
const uint16_t MapDialog::kHOLD_DELAY = 300;
const uint8_t MapDialog::kHOLD_OFFSET = 10;
const std::string MapDialog::kITEM_COLOR = "ffc400";
const uint8_t MapDialog::kLINE_SPACING = 12;
const uint8_t MapDialog::kMARGIN_SIDES = 50;
const uint8_t MapDialog::kMARGIN_TOP = 35;
const uint16_t MapDialog::kMSEC_PER_WORD = 333;
const uint8_t MapDialog::kNAME_BOX_OFFSET = 45;
const uint8_t MapDialog::kNAME_MARGIN = 13;
const uint8_t MapDialog::kNOTIFY_MAX_LINES = 4;
const float MapDialog::kOPACITY_BACKEND = 0.65;
const uint8_t MapDialog::kOPACITY_MAX = 255;
const uint8_t MapDialog::kOPTION_OFFSET = 50;
const uint16_t MapDialog::kPAUSE_TIME = 750;
const uint16_t MapDialog::kPICKUP_DISPLAY_TIME = 5000;
const uint16_t MapDialog::kPICKUP_IMG_SIZE = 64;
const uint8_t MapDialog::kPICKUP_TEXT_MARGIN = 10;
const uint16_t MapDialog::kPICKUP_Y = 50;
const float MapDialog::kSHIFT_TIME = 3.0; //.704;
const uint8_t MapDialog::kTEXT_LINES = 4;
const uint8_t MapDialog::kTEXT_OPTIONS = 3;
const float MapDialog::kTEXT_DISPLAY_SPEED = 33.33;
const float MapDialog::kTEXT_SHIFT = 5.704;
const std::string MapDialog::kTHING_COLOR = "00eaff";

/*=============================================================================
 * CONSTRUCTORS / DESTRUCTORS
 *============================================================================*/

/*
 * Description: Constructs the default map dialog. This clears out all starting
 *              variables and sets up the running config right away, if passed
 *              in.
 *
 * Inputs: Options* running_config - master system config pointer
 */
MapDialog::MapDialog(Options* running_config)
{
  /* Set class parameters */
  clearData();

  setConfiguration(running_config);
}

/*
 * Description: The destructor function. Deletes all dynamic memory and cleans
 *              up the class.
 */
MapDialog::~MapDialog()
{
  renderOptions(NULL);
  setupRenderText();
  deleteFonts();

  clearData();
}

/*=============================================================================
 * PRIVATE FUNCTIONS
 *============================================================================*/

/*
 * Description: Takes a single conversation and parses all required thing IDs
 *              that are stored within it. It creates an array of these IDs.
 *              This function does NOT gaurantee no duplicates.
 *
 * Inputs: Conversation convo - the conversation to parse for IDs
 * Output: vector<int> - vector array of all thing IDs
 */
vector<int> MapDialog::calculateThingList(Conversation* convo)
{
  vector<int> list;

  if(convo != nullptr)
  {
    /* Recursively get all other IDs from embedded conversations */
    for(uint32_t i = 0; i < convo->next.size(); i++)
    {
      vector<int> computed_list = calculateThingList(&(convo->next[i]));
      list.insert(list.end(), computed_list.begin(), computed_list.end());
    }

    /* Determine if any IDs within the text */
    vector<int> text_list = calculateThingList(convo->text);
    if(text_list.size() > 0)
      list.insert(list.end(), text_list.begin(), text_list.end());

    /* Append final ID for the current conversation */
    list.push_back(convo->thing_id);
  }

  return list;
}

/*
 * Description: Determines the thing references stored within the set of text.
 *              These are delimited by {ID} within. If none are found, returns
 *              empty vector.
 *
 * Inputs: string text - the text to search for ID delimiters
 * Output: vector<int> - vector array of all thing IDs within text
 */
vector<int> MapDialog::calculateThingList(string text)
{
  vector<int> ids;

  /* Perform first split on '{' delimiter */
  size_t found = text.find('{');
  if(found != string::npos)
  {
    vector<string> first_split = Helpers::split(text, '{');
    for(uint32_t i = 0; i < first_split.size(); i++)
    {
      /* Perform second split on '}' delimiter */
      size_t found2 = first_split[i].find('}');
      if(found2 != string::npos)
      {
        vector<string> second_split = Helpers::split(first_split[i], '}');
        if(second_split.size() > 0)
        {
          try
          {
            int id = stoi(second_split.front());
            ids.push_back(id);
          }
          catch(std::exception)
          {
          }
        }
      }
    }
  }

  return ids;
}

/*
 * Description: Clears all data that is stored in the class. Primarily for
 *              initialization and final destruction.
 *
 * Inputs: none
 * Output: none
 */
void MapDialog::clearData()
{
  clearAll(true);

  animation_cursor = 0.0;
  animation_cursor_up = true;
  animation_shifter = 0.0;
  conversation_delay = 0;
  conversation_info = nullptr;
  conversation_inst = nullptr;
  conversation_ready = false;
  conversation_update = false;
  conversation_waiting = false;
  dialog_alpha = kOPACITY_MAX;
  dialog_mode = DialogMode::DISABLED;
  dialog_status = WindowStatus::OFF;
  dialog_offset = 0.0;
  dialog_option = 0;
  dialog_option_top = 0;
  event_handler = nullptr;
  font_normal = nullptr;
  font_title = nullptr;
  notification_queue.clear();
  notification_time = 0;
  notification_waiting.clear();
  paused = false;
  pickup_offset = 0.0;
  pickup_status = WindowStatus::OFF;
  pickup_time = 0;
  pickup_update = false;
  source = nullptr;
  system_options = nullptr;
  target = nullptr;
  text_index = 0.0;
  text_index_max = 0;
  text_lines.clear();
  text_offset = 0.0;
  text_offset_max = 0;
  text_options.clear();
  text_strings.clear();
  text_top = 0;
  text_update = false;
  thing_active = nullptr;
  thing_data.clear();

  // img_convo->unsetTexture();
  // img_convo_m->unsetTexture();
  // img_convo_n->unsetTexture();
  // img_name_l->unsetTexture();
  // img_name_r->unsetTexture();
  // img_opt_c->unsetTexture();
  // img_opt_d->unsetTexture();
  // img_opt_u->unsetTexture();
  // img_pick_b->unsetTexture();
  // img_pick_t->unsetTexture();
  frame_bottom.unsetTexture();
  frame_right.unsetTexture();
}

/*
 * Description: Attempts to create the main font and the title font that is used
 *              throughout dialog representation. This is acquired from the
 *              configuration of the application. If successful, set the new
 *              fonts.
 *
 * Inputs: none
 * Output: bool - status if creation was successful
 */
bool MapDialog::createFonts()
{
  bool success = false;

  if(system_options != nullptr)
  {
    /* Try and create the new fonts */
    TTF_Font* regular_font = Text::createFont(
        system_options->getBasePath() + system_options->getFont(3), 20);
    TTF_Font* title_font = Text::createFont(system_options->getBasePath() +
                                                system_options->getFont(3),
                                            20, TTF_STYLE_BOLD);

    /* If successful, insert the new fonts. Otherwise, delete if any were
     * created */
    if(regular_font != nullptr && title_font != nullptr)
    {
      deleteFonts();
      font_normal = regular_font;
      font_title = title_font;
      success = true;
    }
    else
    {
      TTF_CloseFont(regular_font);
      regular_font = nullptr;

      TTF_CloseFont(title_font);
      title_font = nullptr;
    }
  }

  return success;
}

/*
 * Description: Delete the rendering fonts stored in the class. Do not try and
 *              render again since it will try to use the NULL fonts and fail.
 *
 * Inputs: none
 * Ouptut: none
 */
void MapDialog::deleteFonts()
{
  TTF_CloseFont(font_normal);
  font_normal = nullptr;

  TTF_CloseFont(font_title);
  font_title = nullptr;
}

/*
 * Description: Executes an event stored within the current conversation, if
 *              there is one. This gets stacked on the event queue, to be parsed
 *              by the upper level game.
 *
 * Inputs: none
 * Output: none
 */
void MapDialog::executeEvent()
{
  if(event_handler != nullptr && conversation_info != nullptr &&
     conversation_inst != nullptr &&
     conversation_info->action_event.classification != EventClassifier::NOEVENT)
  {
    EventPair pair;
    pair.base = &(conversation_info->action_event);
    pair.inst = &(conversation_inst->action_event);
    event_handler->executeEventRef(pair.base, pair.inst, target, source);
    pair.inst->has_exec = true;
  }
}

/*
 * Description: Returns the item name correlating with the passed in ID. If no
 *              ID found, returns the blank string.
 *
 * Inputs: int id - the item ID to find
 * Output: std::string - the name of the item
 */
std::string MapDialog::getItemName(int id)
{
  if(id >= 0)
  {
    for(uint32_t i = 0; i < item_data.size(); i++)
      if(item_data[i].id == id)
        return item_data[i].name;
  }
  return "";
}

/*
 * Description: Returns the item name correlating with the passed in string ID.
 *              This can handle the ID straight from the conversation with I
 *              in the front to tag item. If not found or fails, returns the
 *              blank string.
 *
 * Inputs: std::string id - the item string ID to find
 * Output: std::string - the name of the item
 */
std::string MapDialog::getItemName(std::string id)
{
  std::string name = "";

  try
  {
    if(id.front() == 'I')
      id.erase(id.begin());
    name = getItemName(std::stoi(id));
  }
  catch(std::exception)
  {
    name = "";
  }

  return name;
}

/*
 * Description: This takes a thing ID and back checks it with stored stack of
 *              thing pointers that were acquired when the conversation was
 *              created in the attempt to find a name of the ID. Returns "" if
 *              not found.
 *
 * Inputs: int id - the reference id (-1 is source, < -1 is invalid)
 *         vector<MapThing*>* things - set of things to search. If null, uses
 *                                     stored conversation things
 * Output: string - the string name of the ID
 */
string MapDialog::getThingName(int id, vector<MapThing*>* things)
{
  string name = "";
  MapThing* thing_reference = getThingReference(id, things);
  if(thing_reference != nullptr)
    name = thing_reference->getName();
  return name;
}

/*
 * Description: This takes a thing ID and back checks it with stored stack of
 *              thing pointers that were acquired when the conversation was
 *              created. Returns NULL if not found.
 *
 * Inputs: int id - the reference id (<0  invalid)
 *         vector<MapThing*>* things - set of things to search. If null, uses
 *                                     stored conversation things
 * Output: MapThing* - map thing reference pointer
 */
MapThing* MapDialog::getThingReference(int id, vector<MapThing*>* things)
{
  MapThing* thing_reference = NULL;
  if(things == nullptr)
    things = &thing_data;

  if(id == -1)
  {
    thing_reference = source;
  }
  else if(id >= 0)
  {
    for(uint32_t i = 0; i < things->size(); i++)
    {
      if(things->at(i)->getID() == id)
        thing_reference = things->at(i);
    }
  }

  return thing_reference;
}

/*
 * Description: Takes a list of options and creates the internal class stack
 *              of options in the Text container, rendered.
 *
 * Inputs: SDL_Renderer* renderer - the graphical engine rendering pointer
 *         vector<vector<vector<pair<string, TextProperty>>>> options - vector
 *                                                     array of string options
 * Output: none
 */
void MapDialog::renderOptions(
    SDL_Renderer* renderer,
    vector<vector<vector<pair<string, TextProperty>>>> options)
{
  /* Clear the options stack */
  for(auto i = text_options.begin(); i != text_options.end(); i++)
    delete *i;
  text_options.clear();

  /* Render and create new options */
  for(auto i = options.begin(); i != options.end(); i++)
  {
    Text* t = new Text(font_normal);
    t->setFont(font_normal);
    t->setText(renderer, *i);
    t->setAlpha(dialog_alpha);
    text_options.push_back(t);
  }
}

/*
 * Description: Takes the text and replaces {ID} references with name if found.
 *              If not found, blanks out.
 *
 * Inputs: string text - the text to process
 *         vector<MapThing*>* things - set of things to search. If null, uses
 *                                     stored conversation things
 * Output: string - the processed text
 */
string MapDialog::replaceIDReferences(string text, vector<MapThing*>* things)
{
  string new_text = "";

  /* Perform first split on '{' delimiter */
  size_t found = text.find('{');
  if(found != string::npos)
  {
    vector<string> first_split = Helpers::split(text, '{');
    for(uint32_t i = 0; i < first_split.size(); i++)
    {
      /* Perform second split */
      size_t found2 = first_split[i].find('}');
      if(found2 != string::npos && (i > 0 || text.front() == '{'))
      {
        vector<string> second_split = Helpers::split(first_split[i], '}');
        if(second_split.size() > 0)
        {
          /* Process the first which would contain the relevant ID */
          try
          {
            std::string name = "";

            /* If item, find item name */
            if(second_split.front().front() == 'I')
            {
              name = getItemName(second_split.front());
              new_text +=
                  "[" + kITEM_COLOR + "]" + name + "[/" + kITEM_COLOR + "]";
            }
            /* Otherwise get thing name */
            else
            {
              name = getThingName(std::stoi(second_split.front()), things);
              new_text +=
                  "[" + kTHING_COLOR + "]" + name + "[/" + kTHING_COLOR + "]";
            }
          }
          catch(std::exception)
          {
            new_text += second_split.front();
          }

          /* Append remaining */
          for(uint32_t j = 1; j < second_split.size(); j++)
            new_text += second_split[j];
        }
      }
      else
      {
        string to_add = first_split[i];
        to_add.erase(remove(to_add.begin(), to_add.end(), '}'), to_add.end());
        new_text += to_add;
      }
    }
  }
  else
  {
    new_text = text;
    new_text.erase(remove(new_text.begin(), new_text.end(), '}'),
                   new_text.end());
  }

  return new_text;
}

/*
 * Description: Sets all the alpha ratings for applicable painted frames and
 *              text data. This also updates the internal alpha reference. This
 *              is primarily used for pausing when the program shifts it from
 *              visible to invisibible.
 *
 * Inputs: uint8_t alpha - alpha opacity rating. 0 invisible - 255 fully opaque
 * Output: none
 */
void MapDialog::setAlpha(uint8_t alpha)
{
  dialog_alpha = alpha;

  /* Sets the frame alpha ratings */
  frame_bottom.setAlpha(alpha);
  frame_right.setAlpha(alpha);
  img_convo_c.setOpacity(alpha);
  img_convo_m.setAlpha(alpha);
  img_convo_n.setAlpha(alpha);
  img_opt_c.setAlpha(alpha);
  img_opt_d.setAlpha(alpha);
  img_opt_u.setAlpha(alpha);

  /* Set the text alpha */
  for(auto i = text_lines.begin(); i != text_lines.end(); i++)
    (*i)->setAlpha(alpha);
  for(auto i = text_options.begin(); i != text_options.end(); i++)
    (*i)->setAlpha(alpha);
}

/*
 * Description: Sets up the current conversation snapshot by rendering the main
 *              frame which holds the name, thing image, and main dialog wrapper
 *              and then gets all the text data ready for rendering. This is
 *              called each time the conversation snapshot is finished and
 *              shifted to the next one.
 *
 * Inputs: SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: none
 */
void MapDialog::setupConversation(SDL_Renderer* renderer)
{
  if(conversation_info != nullptr)
  {
    Frame* dialog_frame = nullptr;
    int render_height = img_convo.getHeight();
    int render_width = img_convo.getWidth();
    int txt_length = img_convo.getWidth() - (kMARGIN_SIDES << 1);

    /* Set the active thing data plus grab image frame */
    thing_active = getThingReference(conversation_info->thing_id);
    if(thing_active != nullptr)
    {
      dialog_frame = thing_active->getDialogImage();

      /* Modify render height/width for new frame */
      if(dialog_frame->getHeight() > render_height)
        render_height = dialog_frame->getHeight();
      render_width += dialog_frame->getWidth() / 2;

      /* Modify text length limiters */
      int width_modifier = (dialog_frame->getWidth() >> 1);
      if(width_modifier > kMARGIN_SIDES)
        txt_length -= (width_modifier - kMARGIN_SIDES);
    }

    /* Create the name information */
    string name = "";
    Text name_text;
    if(thing_active != nullptr)
      name = thing_active->getName();
    if(!name.empty())
    {
      SDL_Color text_color = {255, 255, 255, kOPACITY_MAX};
      name_text.setFont(font_title);
      name_text.setText(renderer, name, text_color);
      if((img_convo.getHeight() + img_name_l.getHeight()) > render_height)
        render_height = img_convo.getHeight() + img_name_l.getHeight();
    }

    /* Render texture creation and setup */
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_TARGET,
                                             render_width, render_height);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(renderer, texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    /* Render images onto texture */
    int convo_y = render_height - img_convo.getHeight();
    img_convo.render(renderer, 0, convo_y);
    if(dialog_frame != nullptr)
      dialog_frame->render(renderer, render_width - dialog_frame->getWidth(),
                           render_height - dialog_frame->getHeight());

    /* Handle name rendering, if it exists */
    if(!name.empty())
    {
      /* Draw corners to name box */
      int textbox_height = img_name_l.getHeight();
      img_name_l.render(renderer, kNAME_BOX_OFFSET, convo_y - textbox_height);
      img_name_r.render(renderer,
                        kNAME_BOX_OFFSET + img_name_l.getWidth() +
                            name_text.getWidth() + kNAME_MARGIN + kNAME_MARGIN,
                        convo_y - textbox_height);

      /* Draw top white bar encapsulating text */
      SDL_SetRenderDrawColor(renderer, 255, 255, 255, kOPACITY_MAX);
      SDL_Rect src_rect;
      src_rect.x = kNAME_BOX_OFFSET + img_name_l.getWidth();
      src_rect.y = convo_y - textbox_height;
      src_rect.w = name_text.getWidth() + kNAME_MARGIN + kNAME_MARGIN;
      src_rect.h = kBORDER_WIDTH;
      SDL_RenderFillRect(renderer, &src_rect);

      /* Draw opaque fill under text */
      SDL_SetRenderDrawColor(renderer, 0, 0, 0,
                             kOPACITY_MAX * kOPACITY_BACKEND);
      src_rect.y += src_rect.h;
      src_rect.h = textbox_height - src_rect.h;
      SDL_RenderFillRect(renderer, &src_rect);

      /* Draw text */
      name_text.render(renderer,
                       src_rect.x + (src_rect.w - name_text.getWidth()) / 2,
                       src_rect.y + (src_rect.h - name_text.getHeight()) / 2);
    }
    SDL_SetRenderTarget(renderer, nullptr);

    /* Create the base frame display texture */
    frame_bottom.setTexture(texture);

    /* Determine the length of the viewing area and split text lines */
    string txt_line = replaceIDReferences(conversation_info->text);
    if(font_normal != nullptr)
    {
      if(conversation_info->next.size() > 1)
      {
        text_strings =
            Text::splitLineProperty(font_normal, txt_line, txt_length, true);

        /* Fill the options */
        int options_length = txt_length - kOPTION_OFFSET;
        vector<vector<vector<pair<string, TextProperty>>>> options_text;

        for(auto i = conversation_info->next.begin();
            i != conversation_info->next.end(); i++)
        {
          options_text.push_back(Text::splitLineProperty(font_normal, (*i).text,
                                                         options_length,
                                                         true).front());
        }
        renderOptions(renderer, options_text);
      }
      else
      {
        text_strings =
            Text::splitLineProperty(font_normal, txt_line, txt_length);
        renderOptions(nullptr);
      }

      setupRenderText(text_strings, true);
    }

    conversation_delay = 0;

    /* Modify the offset if it's above the new limits */
    dialog_mode = DialogMode::CONVERSATION;
    if(dialog_offset > 0.0)
      dialog_offset = frame_bottom.getHeight();
    dialog_option = 0;
    dialog_option_top = 0;
  }
}

/*
 * Description: Sets up the notification on top of the queue to be shown on the
 *              screen. This will render all the data onto a texture which
 *              then will be shifted into view. Warning, this call will seg
 *              fault if there is nothing in the notification stack.
 *
 * Inputs: SDL_Renderer* renderer - the graphical engine rendering reference
 * Output: none
 */
void MapDialog::setupNotification(SDL_Renderer* renderer)
{
  int line_width = img_convo.getWidth() - (kMARGIN_SIDES << 1);
  int render_height = kLINE_SPACING + kLINE_SPACING;
  int render_width = img_convo.getWidth();
  Notification to_display = notification_queue.front();

  /* Split text and create text */
  vector<vector<vector<pair<string, TextProperty>>>> line_set =
      Text::splitLineProperty(font_normal, to_display.text, line_width,
                              to_display.text_lines);
  vector<Text*> rendered_lines;
  for(auto i = line_set.begin(); i != line_set.end(); i++)
  {
    Text* single_line = new Text(font_normal);
    single_line->setText(renderer, *i); //, {255, 255, 255, kOPACITY_MAX});
    rendered_lines.push_back(single_line);

    render_height += single_line->getHeight() + kLINE_SPACING;
  }

  /* Create rendering texture */
  SDL_Texture* texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, render_width, render_height);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  /* Render main frame */
  int y_index = 0;
  img_convo.render(renderer, 0, y_index);
  y_index += kLINE_SPACING + 0.5 * kLINE_SPACING;

  /* Render text */
  for(auto i = rendered_lines.begin(); i != rendered_lines.end(); i++)
  {
    int x_index = (render_width - (*i)->getWidth()) / 2;
    (*i)->render(renderer, x_index, y_index);
    y_index += (*i)->getHeight() + kLINE_SPACING;

    delete(*i);
  }
  SDL_SetRenderTarget(renderer, nullptr);

  /* Create the base frame display texture and set the mode */
  frame_bottom.setTexture(texture);
  dialog_mode = DialogMode::NOTIFICATION;
  dialog_offset = 0.0;
  dialog_status = WindowStatus::SHOWING;
  notification_time = to_display.time_visible;
}

/*
 * Description: Sets up the pickup on top of the queue to be shown on the
 *              screen. This will render all the data onto a texture which
 *              then will be shifted into view. Warning, this call will seg
 *              fault if there is nothing in the notification stack.
 *
 * Inputs: SDL_Renderer* renderer - the graphical engine rendering reference
 * Output: none
 */
void MapDialog::setupPickup(SDL_Renderer* renderer, bool update)
{
  Notification pickup = pickup_queue.front();

  /* Do some checking on height and width, before rendering */
  uint16_t frame_width = pickup.thing_image->getWidth();
  // if(frame_width < Helpers::getTileSize())
  //  frame_width = Helpers::getTileSize();
  uint16_t frame_height = pickup.thing_image->getHeight();
  // if(frame_height < Helpers::getTileSize())
  //  frame_height = Helpers::getTileSize();
  if(frame_width >= frame_height)
  {
    if(frame_width != kPICKUP_IMG_SIZE)
    {
      frame_height *= ((kPICKUP_IMG_SIZE * 1.0) / frame_width);
      frame_width = kPICKUP_IMG_SIZE;
    }
  }
  else
  {
    if(frame_height != kPICKUP_IMG_SIZE)
    {
      frame_width *= ((kPICKUP_IMG_SIZE * 1.0) / frame_height);
      frame_height = kPICKUP_IMG_SIZE;
    }
  }

  /* Determine render height and width */
  int render_height =
      img_pick_t.getHeight() + frame_height + img_pick_b.getHeight();
  int render_width =
      img_pick_t.getWidth() * 2 + frame_width + kPICKUP_TEXT_MARGIN;

  /* Add or remove determination */
  string chg_sep = "+";
  if(pickup.thing_count < 0)
    chg_sep = "";

  /* Set up the text information */
  Text pickup_txt(font_normal);
  pickup_txt.setText(renderer, chg_sep + Text::formatNum(pickup.thing_count),
                     pickup.text_color);
  render_width += pickup_txt.getWidth();

  /* Create rendering texture */
  SDL_Texture* texture =
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                        SDL_TEXTUREACCESS_TARGET, render_width, render_height);
  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
  SDL_SetRenderTarget(renderer, texture);
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);

  /* Render the top left hand detail */
  int x_index = 0;
  int y_index = 0;
  img_pick_t.render(renderer, x_index, y_index);
  y_index += img_pick_t.getHeight();

  /* Render black square under detail */
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, kOPACITY_MAX * kOPACITY_BACKEND);
  SDL_Rect src_rect;
  src_rect.x = x_index;
  src_rect.y = y_index;
  src_rect.w = img_pick_t.getWidth();
  src_rect.h = render_height - img_pick_t.getHeight() - img_pick_b.getHeight();
  SDL_RenderFillRect(renderer, &src_rect);

  /* Render the bottom left hand detail */
  y_index += src_rect.h;
  img_pick_b.render(renderer, x_index, y_index);

  /* Render the overall rectangle */
  src_rect.x = img_pick_t.getWidth();
  src_rect.y = 0;
  src_rect.w = render_width - img_pick_t.getWidth();
  src_rect.h = render_height;
  SDL_RenderFillRect(renderer, &src_rect);

  /* Render the white lines */
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, kOPACITY_MAX);
  src_rect.h = kBORDER_WIDTH;
  SDL_RenderFillRect(renderer, &src_rect);
  src_rect.y = render_height - src_rect.h;
  SDL_RenderFillRect(renderer, &src_rect);
  src_rect.x = 0;
  src_rect.y = img_pick_t.getHeight();
  src_rect.w = kBORDER_WIDTH;
  src_rect.h = render_height - img_pick_t.getHeight() - img_pick_b.getHeight();
  SDL_RenderFillRect(renderer, &src_rect);

  /* Render dialog image */
  x_index = img_pick_t.getWidth();
  y_index = img_pick_t.getHeight();
  pickup.thing_image->render(renderer, x_index, y_index, frame_width,
                             frame_height);

  /* Render text to indicate amount of pickup */
  x_index += frame_width + kPICKUP_TEXT_MARGIN;
  y_index = (render_height - pickup_txt.getHeight()) / 2;
  pickup_txt.render(renderer, x_index, y_index);

  /* Create the base frame display texture for the right hand notification */
  SDL_SetRenderTarget(renderer, NULL);
  frame_right.setTexture(texture);

  if(!update)
  {
    pickup_status = WindowStatus::SHOWING;
    pickup_offset = 0.0;
    pickup_time = pickup.time_visible;
  }
  else
  {
    if(pickup_status == WindowStatus::ON)
      pickup_offset = frame_right.getWidth();
  }
}

/*
 * Description: Sets up the text to be rendered, given a sequence of lines.
 *              This calculates the number of lines for text containers to
 *              create as well as sets up the index max for stepping the
 *              lettering. Can also be used to just update the text max if text
 *              top is changed by setting delete old to false.
 *
 * Inputs: vector<vector<vector<pair<string, TextProperty>>>> lines - the
 *                           string lines to be rendered
 *         bool delete_old - if true, delete previous lines and setup new ones
 * Output: none
 */
void MapDialog::setupRenderText(
    vector<vector<vector<pair<string, TextProperty>>>> lines, bool delete_old)
{
  /* Delete if line count of 0 is passed in */
  if(delete_old)
  {
    for(auto i = text_lines.begin(); i != text_lines.end(); i++)
      delete *i;
    text_lines.clear();

    /* Create a new stack of the applicable text handlers */
    for(uint16_t i = 0; i < lines.size(); i++)
    {
      Text* line_text = new Text(font_normal);
      line_text->setAlpha(dialog_alpha);
      text_lines.push_back(line_text);
    }
    text_index = 0.0;
    text_index_max = 0;
    text_top = 0;
  }

  /* Set the initial max character length */
  for(uint16_t i = text_top; i < (text_top + kTEXT_LINES); i++)
  {
    if(i < lines.size())
      text_index_max += Text::countLength(lines[i]);
  }
}

/*=============================================================================
 * PUBLIC FUNCTIONS
 *============================================================================*/

/*
 * Description: Clears all queues of conversation items, notifications, and
 *              pickups from the dialog stack. Called on map changes or any
 *              clears.
 *
 * Inputs: bool include_convo - true to clear convo as well. default false
 * Output: none
 */
void MapDialog::clearAll(bool include_convo)
{
  /* If in conversation */
  if(dialog_mode == DialogMode::CONVERSATION)
  {
    if(include_convo)
    {
      renderOptions(nullptr);
      setupRenderText();
      conversation_info = nullptr;
      conversation_inst = nullptr;
      // setConversation();

      dialog_mode = DialogMode::DISABLED;
      dialog_status = WindowStatus::OFF;
    }
  }
  /* Otherwise, not conversation */
  else
  {
    dialog_mode = DialogMode::DISABLED;
    dialog_status = WindowStatus::OFF;
  }
  notification_queue.clear();
  notification_waiting.clear();

  /* Pick up */
  pickup_queue.clear();
  pickup_status = WindowStatus::OFF;

  /* Clear frame data */
  frame_bottom.unsetTexture();
  frame_right.unsetTexture();
}

/*
 * Description: Returns a unique list of conversation map thing IDs from the
 *              current conversation that has been set but is waiting for
 *              the thing IDs to be set. This is used in conjunction with
 *              setConversationThings() corresponding to these IDs.
 *
 * Inputs: none
 * Output: vector<int> - vector array of unique thing IDs from convo.
 */
vector<int> MapDialog::getConversationIDs()
{
  vector<int> thing_ids;

  if(isConversationWaiting())
  {
    thing_ids = calculateThingList(conversation_info);
    thing_ids.push_back(target->getID());
    thing_ids.erase(Helpers::uniqueSplit(thing_ids.begin(), thing_ids.end()),
                    thing_ids.end());
  }

  return thing_ids;
}

/*
 * Description: Return the DialogMode enumerated value
 *
 * Inputs: none
 * Output: DialogMode - enumerated DialogMode value
 */
DialogMode MapDialog::getDialogMode()
{
  return dialog_mode;
}

/*
 * Description: Returns a unique list of notification map thing IDs from the
 *              stack of waiting notifications that has been set but are
 *              waiting to be processed first. This is used in conjunction with
 *              setNotificationThings() corresponding to these IDs.
 *
 * Inputs: none
 * Output: vector<int> - vector array of unique thing IDs from notification(s).
 */

vector<int> MapDialog::getNotificationIDs()
{
  vector<int> notification_ids;

  if(isNotificationWaiting())
  {
    /* Find the IDs from all notifications */
    for(uint32_t i = 0; i < notification_waiting.size(); i++)
    {
      vector<int> id_set = calculateThingList(notification_waiting[i].text);
      if(id_set.size() > 0)
        notification_ids.insert(notification_ids.end(), id_set.begin(),
                                id_set.end());
    }

    /* Remove duplicates */
    notification_ids.erase(
        Helpers::uniqueSplit(notification_ids.begin(), notification_ids.end()),
        notification_ids.end());
  }

  return notification_ids;
}

/*
 * Description: Return the enumerated state of this dialogs WindowStatus.
 *
 * Inputs: none
 * Output: WindowStatus -- enumerated state of the WindowStatus
 */
WindowStatus MapDialog::getWindowStatus()
{
  return dialog_status;
}

/*
 * Description: Initializes a conversation with the current dialog. This is just
 *              a call that dereferences the pointer and passes it to the other
 *              initConversation(). Returns false if conversation was unable to
 *              be instantiated.
 *
 * Inputs: ConvoPair convoPair - a conversation pointer pair set
 *         MapPerson* target - the person being targetted for the conversation
 *         MapThing* source - the thing that initiated the event
 * Output: bool - status if conversation could be created.
 */
bool MapDialog::initConversation(ConvoPair convo_pair, MapPerson* target,
                                 MapThing* source)
{
  if(convo_pair.base != nullptr && convo_pair.inst != nullptr &&
     dialog_mode != DialogMode::CONVERSATION && target != nullptr &&
     isImagesSet())
  {
    conversation_delay = 0;
    conversation_info = convo_pair.base;
    conversation_inst = convo_pair.inst;
    this->source = source;
    this->target = target;
    thing_active = nullptr;
    thing_data.clear();
    conversation_ready = false;
    conversation_waiting = true;

    return true;
  }
  return false;
}

/*
 * Description: Initializes a notification to be added to the queue and
 *              displayed in a round robin manner in order of when received and
 *              when the class isn't displaying a conversation. All it needs is
 *              a string, and the time to make it display for. If single line,
 *              it will right elide if longer. If time visible is <= 0, it
 *              computes the time to display from the number of words in the
 *              notification.
 *
 * Inputs: string notification - string data to display
 *         bool single_line - if it should be at max one line
 *         int time_visible - length of time the notification is visible.
 * Output: bool - status if notification could be pushed onto queue.
 */
bool MapDialog::initNotification(string notification, bool single_line,
                                 int time_visible)
{
  if(img_convo.isTextureSet() && !notification.empty())
  {
    /* Calculate the available space */
    int width = img_convo.getWidth() - (kMARGIN_SIDES << 1);
    if(width < 0)
      width = 0;

    /* Calculate the display time if invalid */
    if(time_visible <= 0)
    {
      vector<string> words = Helpers::split(notification, ' ');
      time_visible = (words.size() * kMSEC_PER_WORD) + kMSEC_PER_WORD;
    }

    /* Set up the queue entry */
    Notification queue_entry;
    queue_entry.text = notification;
    if(single_line)
      queue_entry.text_lines = 1;
    else
      queue_entry.text_lines = kNOTIFY_MAX_LINES;
    queue_entry.thing_image = nullptr;
    queue_entry.thing_count = 0;
    queue_entry.time_visible = time_visible;

    /* Append to running queue */
    notification_waiting.push_back(queue_entry);

    return true;
  }

  return false;
}

/*
 * Description: Initializes a pickup notification that can be pushed onto the
 *              queue. This takes a thing image, the number to notify about and
 *              the length of time visible. Fails if there is no image or count
 *              is invalid. If time visible is <= 0 or default, it uses the
 *              default 5s to display. This will be displayed when the right
 *              notification is available.
 *
 * Inputs: Frame* thing_image - the thing reference image
 *         int thing_count - the number of things to notify
 *         int time_visible - the ms of time to show pickup
 * Output: bool - status if the pickup could be queued.
 */
bool MapDialog::initPickup(Frame* thing_image, int thing_count,
                           int time_visible)
{
  if(thing_image != nullptr && thing_image->isTextureSet() &&
     thing_count != 0 && isImagesSet(false, true))
  {
    uint16_t notification_index = 0;
    bool notification_found = false;

    /* First check to see if a pickup already exists */
    for(uint16_t i = 0; i < pickup_queue.size(); i++)
    {
      if(!notification_found && pickup_queue[i].thing_image == thing_image &&
         ((pickup_queue[i].thing_count < 0 && thing_count < 0) ||
          (pickup_queue[i].thing_count > 0 && thing_count > 0)))
      {
        notification_index = i;
        notification_found = true;
      }
    }

    /* Check if it's the head of the list and it's hiding. If so, don't add to
     * existing */
    if(notification_index == 0 && pickup_status == WindowStatus::HIDING)
      notification_found = false;

    /* If the notification is found, update the current one */
    if(notification_found)
    {
      pickup_queue[notification_index].thing_count += thing_count;

      /* If it is the head of the list and it's currently being shown, reset
       * time */
      if(notification_index == 0 && pickup_status != WindowStatus::OFF)
        pickup_time = pickup_queue[notification_index].time_visible;

      pickup_update = true;
    }
    /* Otherwise, create new pickup event */
    else
    {
      Notification pickup;
      pickup.text = "";
      pickup.thing_image = thing_image;
      pickup.thing_count = thing_count;
      if(thing_count > 0)
        pickup.text_color = kCOLOR_ADD;
      else
        pickup.text_color = kCOLOR_REM;

      /* Determine time visible, if given time is invalid */
      if(time_visible <= 0)
        pickup.time_visible = kPICKUP_DISPLAY_TIME;
      else
        pickup.time_visible = time_visible;

      /* Push onto stack */
      pickup_queue.push_back(pickup);
    }

    return true;
  }

  return false;
}

/*
 * Description: Checks if a conversation is active in the bottom dialog mode.
 *
 * Inputs: none
 * Output: bool - true if there is an active conversation
 */
bool MapDialog::isConversationActive()
{
  return (conversation_info != nullptr &&
          dialog_mode == DialogMode::CONVERSATION);
}

/*
 * Description: Checks if a conversation is ready to be displayed but not active
 *
 * Inputs: none
 * Output: bool - true if there is an ready conversation
 */
bool MapDialog::isConversationReady()
{
  return conversation_ready;
}

/*
 * Description: Returns if there is a conversation waiting for final
 *              configuration. This is finished with getConversationIDs() and
 *              then getting the appropriate thing pointers and calling
 *              setConversationThings().
 *
 * Inputs: none
 * Output: bool - true if a conversation is waiting
 */
bool MapDialog::isConversationWaiting()
{
  return conversation_waiting;
}

/*
 * Description: Checks if the applicable images are set for rendering. It has
 *              inputs to switch between conversation and pickup options. These
 *              correspond to loadImage*() functions.
 *
 * Inputs: bool conversation - check conversation images (default true)
 *         bool pickup - check pickup images (default false)
 * Output: bool - true if the selected images are set
 */
bool MapDialog::isImagesSet(bool conversation, bool pickup)
{
  bool loaded = true;

  /* Check conversation image data */
  if(conversation)
  {
    loaded &= img_convo.isTextureSet();
    loaded &= img_name_l.isTextureSet();
    loaded &= img_name_r.isTextureSet();
  }

  /* Check pickup image data */
  if(pickup)
  {
    loaded &= img_pick_b.isTextureSet();
    loaded &= img_pick_t.isTextureSet();
  }

  return loaded;
}

/*
 * Description: Returns if there is a notificaiton waiting for final processing.
 *              This is finished with getNotificationIDs() and then getting the
 *              appropriate thing pointers and calling setNotificationThings().
 *
 * Inputs: none
 * Output: bool - true if a notification is waiting
 */
bool MapDialog::isNotificationWaiting()
{
  return (notification_waiting.size() > 0);
}

/*
 * Description: Checks if the dialog has been paused.
 *
 * Inputs: none
 * Output: bool - true if paused
 */
bool MapDialog::isPaused()
{
  return paused;
}

/*
 * Description: Pass key down events into here when a conversation is active.
 *              This is important for going between conversation sections and
 *              selecting options or entering data.
 *
 * Inputs: SDL_KeyboardEvent event - the keyboard event that fired
 * Output: none
 */
void MapDialog::keyDownEvent(KeyHandler& key_handler)
{
  if(!paused && dialog_alpha == kOPACITY_MAX)
  {
    if(key_handler.isDepressed(GameKey::ACTION))
    {
      key_handler.setHeld(GameKey::ACTION);
      if(isConversationActive() && dialog_status == WindowStatus::ON)
      {
        uint16_t text_bottom = text_top + kTEXT_LINES;
        bool multiple = (conversation_info->next.size() > 1);

        /* If the dialog letters are being shifted, finish the shift */
        if(text_offset_max != 0)
        {
          text_offset = text_offset_max;
        }
        /* If the letters still need to be displayed, finish */
        else if(text_index < text_index_max)
        {
          text_index = text_index_max;
          text_update = true;
        }
        /* Otherwise, check if the dialog needs to be shifted */
        else if(text_bottom < text_strings.size())
        {
          text_offset = 0.0;
          text_offset_max = (TTF_FontHeight(font_normal) + (kLINE_SPACING)) *
                            (kTEXT_LINES - 1);
        }
        else if(multiple || (conversation_delay >= conversation_info->delay))
        {
          /* If end of conversation is reached, start hiding it */
          if(conversation_info->next.size() == 0 ||
             conversation_inst->next.size() == 0)
          {
            dialog_status = WindowStatus::HIDING;
            executeEvent();
          }
          /* Otherwise, there is a next conversation. Proceed */
          else
          {
            /* Do the initial conversation shift */
            executeEvent();
            conversation_info = &(conversation_info->next[dialog_option]);
            conversation_inst = &(conversation_inst->next[dialog_option]);

            /* If multiple options, shift to the next one */
            if(multiple)
            {
              executeEvent();
              if(conversation_info->next.size() == 0 ||
                 conversation_inst->next.size() == 0)
              {
                dialog_status = WindowStatus::HIDING;
              }
              else
              {
                conversation_info = &(conversation_info->next[0]);
                conversation_inst = &(conversation_inst->next[0]);
              }
            }

            conversation_update = true;
          }
        }

        /* Play key sound */
        if(event_handler != nullptr)
          event_handler->triggerSound(Sound::kID_SOUND_MENU_NEXT,
                                      SoundChannels::MENUS);
      }
    }
    if(key_handler.isDepressed(GameKey::MOVE_UP) &&
       text_index >= text_index_max)
    {
      key_handler.setHeld(GameKey::MOVE_UP);
      if(dialog_option > 0)
      {
        dialog_option--;
        if(event_handler != nullptr)
          event_handler->triggerSound(Sound::kID_SOUND_MENU_CHG,
                                      SoundChannels::MENUS);
      }
      if(dialog_option < dialog_option_top)
        dialog_option_top = dialog_option;
    }
    if(key_handler.isDepressed(GameKey::MOVE_DOWN) &&
       text_index >= text_index_max)
    {
      key_handler.setHeld(GameKey::MOVE_DOWN);
      if(dialog_option < (conversation_info->next.size() - 1))
      {
        dialog_option++;
        if(event_handler != nullptr)
          event_handler->triggerSound(Sound::kID_SOUND_MENU_CHG,
                                      SoundChannels::MENUS);
      }
      if(dialog_option >= (dialog_option_top + kTEXT_OPTIONS))
        dialog_option_top = dialog_option - kTEXT_OPTIONS + 1;
    }
  }
}

/*
 * Description: Flushes any latched key press events. Not currently being used.
 *
 * Inputs: none
 * Output: none
 */
void MapDialog::keyFlush()
{
}
 
/*
 * Description: Loads the pie clock images for when one snapshot of the
 *              conversation is displayed but delayed. 
 *
 * Inputs: std::string path_start - the start of the path sequence
 *         int frames - the number of frames. First is empty. Last is full. 
 *                      Rest is sequencing
 *         std::string path_finish - the end of the path sequence
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if set was successful (false if either fails)
 */
bool MapDialog::loadImageClock(std::string path_start, int frames,
                               std::string path_finish, SDL_Renderer* renderer)
{
  if(renderer != nullptr && frames > 0)
  {
    /* Remove all existing and set animation time */
    img_convo_c.removeAll();
    img_convo_c.setAnimationTime(kHOLD_DELAY);

    /* Base path */
    std::string base_path = "";
    if(system_options != nullptr)
      base_path = system_options->getBasePath();

    /* Add normal sequence */
    img_convo_c.insertSequence(base_path + path_start, frames,
                               path_finish, renderer);

    /* Add in reversed sequence */
    for(int i = (frames - 2); i >= 1; i--)
    {
      /* Generate path */
      std::string path = (base_path + path_start);
      if(i < 10)
        path += "0";
      path += std::to_string(i) + path_finish;

      /* Add frame and horizontally flip */
      Frame* edit_frame = img_convo_c.insertTail(path, renderer);
      if(edit_frame != nullptr)
        edit_frame->flipHorizontal();
    }

    return (img_convo_c.getSize() > 0);
  }
  return false;
}

/*
 * Description: Loads the conversation image. This is the main bottom part that
 *              holds the text information. Needs to be set for conversations
 *              or notifications to work.
 *
 * Inputs: string path - path to the image for the conversation
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if set was successful
 */
bool MapDialog::loadImageConversation(string path, SDL_Renderer* renderer)
{
  string base_path = "";
  if(system_options != nullptr)
    base_path = system_options->getBasePath();

  return img_convo.setTexture(base_path + path, renderer);
}

/*
 * Description: Loads the dialog shift images for when one snapshot of the
 *              conversation is displayed. The next is to go to the next person
 *              for talking. The more is the indicator for that more text needs
 *              to be displayed.
 *
 * Inputs: std::string path_next - the path to the next indicator image
 *         std::string path_more - the path to the more text indicator image
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if set was successful (false if either fails)
 */
bool MapDialog::loadImageDialogShifts(std::string path_next,
                                      std::string path_more,
                                      SDL_Renderer* renderer)
{
  bool success = true;

  string base_path = "";
  if(system_options != nullptr)
    base_path = system_options->getBasePath();

  /* Set all the frame information */
  success &= img_convo_m.setTexture(base_path + path_more, renderer);
  success &= img_convo_n.setTexture(base_path + path_next, renderer);

  return success;
}

/*
 * Description: Loads the triangles that encapsulate either side of the name
 *              that is shown on top of the conversation (the rest is drawn with
 *              code). The right one is conjured by flipping the image from the
 *              path given while the left one just uses the path directly.
 *
 * Inputs: string path - image path for the left (and right) delimiter
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if the set was successful on both
 */
bool MapDialog::loadImageNameLeftRight(string path, SDL_Renderer* renderer)
{
  bool success = true;

  string base_path = "";
  if(system_options != nullptr)
    base_path = system_options->getBasePath();

  img_name_r.flipHorizontal();
  success &= img_name_r.setTexture(base_path + path, renderer);
  return (success & img_name_l.setTexture(base_path + path, renderer));
}

/*
 * Description: Loads the images for the options indicator based on positioning
 *              The circle is the end indicator that no more options are
 *              available. The triangle is to indicate more options are
 *              available. This triangle needs to be point up, since the down
 *              one is conjured by vertically flipping the path.
 *
 * Inputs: string path_circle - path to the circle image
 *         string path_triangle - path to the triangle image (point up)
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if the set was successful on all 3
 */
bool MapDialog::loadImageOptions(string path_circle, string path_triangle,
                                 SDL_Renderer* renderer)
{
  bool success = true;

  string base_path = "";
  if(system_options != nullptr)
    base_path = system_options->getBasePath();

  /* Set all the frame information */
  success &= img_opt_c.setTexture(base_path + path_circle, renderer);
  success &= img_opt_u.setTexture(base_path + path_triangle, renderer);
  img_opt_d.flipVertical();
  success &= img_opt_d.setTexture(base_path + path_triangle, renderer);

  return success;
}

/*
 * Description: Loads the triangles used on the pickup for closing off the
 *              rectangle cleanly. Ensure that the path is the top right hand
 *              triangle as the other one is conjured by flipping it.
 *
 * Inputs: string path - path to the triangle image
 *         SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if the images were both set successfully
 */
bool MapDialog::loadImagePickupTopBottom(string path, SDL_Renderer* renderer)
{
  bool success = true;

  string base_path = "";
  if(system_options != nullptr)
    base_path = system_options->getBasePath();

  img_pick_b.flipVertical();
  success &= img_pick_b.setTexture(base_path + path, renderer);
  return (success & img_pick_t.setTexture(base_path + path, renderer));
}

/*
 * Description: Render both the bottom notification and the right hand
 *              notification. This is all based on running queues and what is
 *              available. This only deals with creating textures and rendering
 *              them to the screen.
 *
 * Inputs: SDL_Renderer* renderer - the graphical rendering engine reference
 * Output: bool - status if the render was successful
 */
bool MapDialog::render(SDL_Renderer* renderer)
{
  uint16_t x_index = 0;
  uint16_t y_index = 0;

  /* If the conversation has changed, update the rendering sprites/data */
  if((dialog_mode == DialogMode::DISABLED && conversation_ready) ||
     conversation_update)
  {
    setupConversation(renderer);

    if(conversation_ready)
    {
      dialog_status = WindowStatus::SHOWING;
      dialog_offset = 0.0;
    }
    conversation_ready = false;
    conversation_update = false;
  }
  /* Otherwise, try to initiate a notification */
  else if(dialog_mode == DialogMode::DISABLED && !notification_queue.empty())
  {
    setupNotification(renderer);
  }

  /* Render the main frame (same for both notificaiton and conversation */
  x_index = (system_options->getScreenWidth() - img_convo.getWidth()) / 2;
  if(dialog_mode == DialogMode::CONVERSATION || dialog_mode == DialogMode::NOTIFICATION)
  {
    frame_bottom.render(renderer, x_index,
                        system_options->getScreenHeight() - dialog_offset);
  }

  if(dialog_mode == DialogMode::CONVERSATION)
  {
    /* Compute the new parts of the rendering text */
    if(text_update)
    {
      uint16_t index = text_top;
      uint16_t length = text_index;
      while(index < (text_top + kTEXT_LINES) && index < text_lines.size() &&
            length > 0)
      {
        /* If length is >= string size, render whole string. Otherwise,
         * only render the portion visible */
        int line_length = Text::countLength(text_strings[index]);
        if(length >= line_length)
        {
          text_lines[index]->setText(renderer, text_strings[index]);
          text_lines[index]->setAlpha(dialog_alpha);
          length -= line_length;
        }
        else
        {
          text_lines[index]->setText(renderer, text_strings[index], length);
          text_lines[index]->setAlpha(dialog_alpha);
          length = 0;
        }
        index++;
      }
      text_update = false;
    }

    /* Compute y base index and proceed to render text */
    x_index += kMARGIN_SIDES;
    y_index =
        system_options->getScreenHeight() - img_convo.getHeight() + kMARGIN_TOP;
    if(dialog_status == WindowStatus::ON)
    {
      /* Get the line offset, for alpha rendering */
      float line_offset = 0.0;
      if(text_offset_max > 0)
        line_offset = text_offset * (kTEXT_LINES - 1) / text_offset_max;

      /* Render the applicable text lines */
      for(uint16_t i = text_top; i < (text_top + kTEXT_LINES); i++)
      {
        /* Check in on the opacity for text offset shift */
        uint16_t index_rating = i - text_top;
        if(text_offset_max > 0)
        {
          if(line_offset >= (index_rating + 1))
            text_lines[i]->setAlpha(0);
          else if(static_cast<int>(line_offset) == index_rating)
            text_lines[i]->setAlpha(dialog_alpha *
                                    (1 - (line_offset - index_rating)));
        }

        /* Proceed to render the text, if it's in the valid range */
        if(i < text_lines.size())
        {
          text_lines[i]->render(renderer, x_index, y_index - text_offset);
          y_index += text_lines[i]->getHeight() + kLINE_SPACING;
        }
      }

      /* Render the options, if applicable */
      if(conversation_info->next.size() > 1 && text_index >= text_index_max)
      {
        uint8_t index = dialog_option_top;
        y_index += kLINE_SPACING;

        /* Loop through and render them all, properly offset */
        while(index < text_options.size() &&
              index < (dialog_option_top + kTEXT_OPTIONS))
        {
          /* If this option is the active one, render a highlight box under */
          if(index == dialog_option)
          {
            uint8_t m = kHIGHLIGHT_MARGIN;
            SDL_Rect highlight_rect = {0, 0, 0, 0};
            highlight_rect.x = x_index + kOPTION_OFFSET - m;
            highlight_rect.y = y_index - m;
            highlight_rect.w = text_options[index]->getWidth() + (m << 1);
            highlight_rect.h = text_options[index]->getHeight() + (m << 1);

            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 45);
            SDL_RenderFillRect(renderer, &highlight_rect);
          }

          /* Determine if there is more than the display options */
          if(conversation_info->next.size() > kTEXT_OPTIONS)
          {
            Frame* frame = nullptr;

            /* If it's the top option, render beside */
            if(index == dialog_option_top)
            {
              if(dialog_option_top == 0)
                frame = &img_opt_c;
              else
                frame = &img_opt_u;
            }

            /* If it's the bottom option, render beside */
            if(index == (dialog_option_top + kTEXT_OPTIONS - 1))
            {
              if(index == (conversation_info->next.size() - 1))
                frame = &img_opt_c;
              else
                frame = &img_opt_d;
            }

            /* Render the frame if non-null and set */
            if(frame != nullptr)
              frame->render(
                  renderer,
                  x_index + (kOPTION_OFFSET - frame->getWidth() - 1) / 2,
                  y_index +
                      (text_options[index]->getHeight() - frame->getHeight()) /
                          2);
          }

          /* Finally, render the option text and increment index */
          text_options[index]->render(renderer, x_index + kOPTION_OFFSET,
                                      y_index);
          y_index += text_options[index]->getHeight() + (kLINE_SPACING << 1);
          index++;
        }
      }
      /* Otherwise, check the parameters for displaying the dialog shifters */
      else if(text_index >= text_index_max)
      {
        x_index += (img_convo.getWidth() / 2 - kMARGIN_SIDES);

        /* If the display is not done, show the shifting more pointers */
        uint16_t top_index = text_top + kTEXT_LINES;
        if(top_index < text_strings.size() && text_offset_max == 0)
        {
          uint16_t offset = kBUBBLES_OFFSET;

          for(uint8_t i = kBUBBLES_COUNT; i > 0; i--)
          {
            if(animation_shifter >= i)
              img_convo_m.render(renderer, x_index - img_convo_m.getWidth() / 2,
                                 system_options->getScreenHeight() - offset);
            offset += kBUBBLES_SPACING;
          }
        }
        /* Else, the words have ended. Show the next shifter arrow */
        else if(text_offset_max == 0)
        {
          if(conversation_delay >= conversation_info->delay)
          {
            img_convo_n.render(renderer, x_index - img_convo_n.getWidth() / 2,
                               system_options->getScreenHeight() -
                                   img_convo_n.getHeight() -
                                   static_cast<int>(animation_cursor));
          }
          else
          {
            if(img_convo_c.isFramesSet())
            {
              img_convo_c.render(renderer, 
                  x_index - img_convo_c.getFirstFrame()->getWidth() / 2,
                  system_options->getScreenHeight() -
                      img_convo_c.getFirstFrame()->getHeight() - kHOLD_OFFSET);
            }
          }
        }
      }
    }
  }

  /* Setup (if applicable) and render the pickup portion */
  if(pickup_status == WindowStatus::OFF && !pickup_queue.empty())
  {
    setupPickup(renderer);
  }
  if(pickup_status != WindowStatus::OFF)
  {
    /* If the pickup needs an update, update it first */
    if(pickup_update)
    {
      setupPickup(renderer, pickup_update);
      pickup_update = false;
    }

    frame_right.render(
        renderer, system_options->getScreenWidth() - pickup_offset, kPICKUP_Y);
  }

  return true;
}

/*
 * Description: Sets the configuration options from the top level of the
 *              application. This is critical for operation or this class will
 *              crash immediately when the map is made visible.
 *
 * Inputs: Options* running_config - the new config to set in the class
 * Output: bool - status if the set was successful
 */
bool MapDialog::setConfiguration(Options* running_config)
{
  if(running_config != NULL)
  {
    system_options = running_config;
    createFonts();

    return true;
  }

  return false;
}

/*
 * Description: Sets the conversation map things that are needed for rendering
 *              and referencing. These correspond to the IDs from
 *              getConversationIDs(). This call will only work if a conversation
 *              is waiting.
 *
 * Inputs: vector<MapThing*> things - vector array of things
 * Output: bool - status if the set was successful
 */
bool MapDialog::setConversationThings(vector<MapThing*> things)
{
  if(conversation_waiting)
  {
    thing_data = things;
    conversation_waiting = false;
    conversation_ready = true;
    return true;
  }

  return false;
}

/*
 * Description: Sets the event handler to create and manage all existing events
 *              that get fired throughout interaction with the class. This is
 *              necessary to ensure that any events work.
 *
 * Inputs: EventHandler* event_handler - the new handler pointer (NULL to unset)
 * Output: none
 */
void MapDialog::setEventHandler(EventHandler* event_handler)
{
  this->event_handler = event_handler;
}

/*
 * Description: Sets the item data to the list of all core items for use within
 *              the dialog reference.
 *
 * Inputs: std::vector<ItemData> data - the set of item core data structs
 * Output: none
 */
void MapDialog::setItemData(std::vector<ItemData> data)
{
  item_data = data;
}

/*
 * Description: Sets the notification map things that are needed for rendering
 *              and referencing. These correspond to the IDs from
 *              getNotificationIDs(). This call will only work if one or more
 *              notifications are waiting.
 *
 * Inputs: vector<MapThing*> things - vector array of things
 * Output: bool - status if the set was successful
 */
bool MapDialog::setNotificationThings(vector<MapThing*> things)
{
  /* Loop through all waiting notifications and clean-up thing references */
  for(uint32_t i = 0; i < notification_waiting.size(); i++)
  {
    notification_waiting[i].text =
        replaceIDReferences(notification_waiting[i].text, &things);
  }

  /* Load from waiting into main queue */
  if(notification_waiting.size() > 0)
  {
    notification_queue.insert(notification_queue.end(),
                              notification_waiting.begin(),
                              notification_waiting.end());
    notification_waiting.clear();
    return true;
  }
  return false;
}

/*
 * Description: Sets if the class should be paused. This causes all the displays
 *              to hide and be invisible from sight. All update calls are held
 *              also, until pause is deactivated.
 *
 * Inputs: bool paused - true if the class should be paused
 * Output: none
 */
void MapDialog::setPaused(bool paused)
{
  this->paused = paused;
}

/*
 * Description: Updates all the logic between the bottom notification and the
 *              top right hand notification. This handles timing for showing/
 *              hiding as well as updating animations and opacity (for pause).
 *
 * Inputs: int cycle_time - the update time for the last scan (ms)
 * Output: none
 */
void MapDialog::update(int cycle_time)
{
  /* Modify the opacity of the dialog information based on the paused status */
  if(paused && dialog_alpha > 0)
  {
    float alpha_diff = cycle_time * 1.0 * kOPACITY_MAX / kPAUSE_TIME;
    if(alpha_diff < 1)
      alpha_diff = 1.0;

    /* Compute the change in alpha */
    if(alpha_diff >= dialog_alpha)
      setAlpha(0);
    else
      setAlpha(dialog_alpha - alpha_diff);

    /* If the dialog alpha reaches 0, do the cleanup */
    if(dialog_alpha == 0)
    {
      /* Complete the animation sequence if it's occurring during a pause */
      if(dialog_status == WindowStatus::HIDING)
        dialog_offset = 0.0;
      else if(dialog_status == WindowStatus::SHOWING)
        dialog_offset = frame_bottom.getHeight();

      /* Complete the pickup animation sequence */
      if(pickup_status == WindowStatus::HIDING)
        pickup_offset = 0.0;
      else if(pickup_status == WindowStatus::SHOWING)
        pickup_offset = frame_right.getWidth();
    }
  }
  else if(!paused && dialog_alpha < kOPACITY_MAX)
  {
    float alpha_diff = cycle_time * 1.0 * kOPACITY_MAX / kPAUSE_TIME;
    if(alpha_diff < 1)
      alpha_diff = 1.0;

    /* Compute the change in alpha */
    if(dialog_alpha + alpha_diff > kOPACITY_MAX)
      setAlpha(kOPACITY_MAX);
    else
      setAlpha(dialog_alpha + alpha_diff);
  }

  /* Ignore all updating if paused */
  if(!paused && dialog_alpha == kOPACITY_MAX)
  {
    /* If hiding, shift the display onto the screen */
    if(dialog_status == WindowStatus::HIDING)
    {
      dialog_offset -= cycle_time / kSHIFT_TIME;
      if(dialog_offset <= 0.0)
      {
        dialog_status = WindowStatus::OFF;
        dialog_offset = 0.0;

        /* Clean up conversation */
        if(dialog_mode == DialogMode::CONVERSATION)
        {
          renderOptions(NULL);
          setupRenderText();
          conversation_info = nullptr;
          conversation_inst = nullptr;
          // setConversation();
        }
        /* Or clean up the notification */
        else if(dialog_mode == DialogMode::NOTIFICATION)
        {
          /* If a conversation caused this pause, minimize the time visible */
          if(conversation_ready)
          {
            if((notification_queue.front().time_visible / 2) >
               notification_time)
            {
              notification_queue.front().time_visible /= 2;
            }
          }
          else
          {
            notification_queue.erase(notification_queue.begin());
          }
        }

        dialog_mode = DialogMode::DISABLED;
      }
    }
    /* If showing, shift the display onto the screen */
    else if(dialog_status == WindowStatus::SHOWING)
    {
      dialog_offset += cycle_time / kSHIFT_TIME; // ~4.5 @ 16.666 ms
      if(dialog_offset >= frame_bottom.getHeight())
      {
        dialog_status = WindowStatus::ON;
        dialog_offset = frame_bottom.getHeight();
      }
    }
    /* If ON, do action appropriate to the dialog mode */
    else if(dialog_status == WindowStatus::ON)
    {
      /* This controls how long the notification box is displayed */
      if(dialog_mode == DialogMode::NOTIFICATION)
      {
        if(conversation_ready)
        {
          dialog_status = WindowStatus::HIDING;
        }
        else if(cycle_time >= notification_time)
        {
          notification_time = 0;
          dialog_status = WindowStatus::HIDING;
        }
        else
        {
          notification_time -= cycle_time;
        }
      }
      /* This controls the bouncing cursor at the bottom of each conversational
       * box to signal going to the next set */
      else if(isConversationActive())
      {
        /* Shift the text up, if scrolling to more information */
        if(text_offset_max > 0)
        {
          text_offset += cycle_time / kTEXT_SHIFT;
          if(text_offset >= text_offset_max)
          {
            text_offset = 0.0;
            text_offset_max = 0;

            /* Finally, shift the text so new fonts are rendered */
            animation_shifter = 0.0;
            text_top += (kTEXT_LINES - 1);
            text_index = Text::countLength(text_strings[text_top]);
            text_index_max = 0;

            for(uint16_t i = text_top; i < (text_top + kTEXT_LINES); i++)
            {
              if(i < text_strings.size())
                text_index_max += Text::countLength(text_strings[i]);
            }
          }
        }
        /* Shift the letters displayed, if not entirely displayed */
        else if(text_index < text_index_max)
        {
          text_index += cycle_time / kTEXT_DISPLAY_SPEED;
          if(text_index > text_index_max)
            text_index = text_index_max;
          text_update = true;
        }

        /* If there is a conversation delay, add it up during operation */
        if(conversation_delay < conversation_info->delay)
        {
          conversation_delay += cycle_time;
        }

        /* Up motion of next cursor */
        if(animation_cursor_up)
        {
          animation_cursor +=
              1.0 * cycle_time * kCURSOR_HEIGHT / kCURSOR_ANIMATE;
          if(animation_cursor >= kCURSOR_HEIGHT)
          {
            animation_cursor = kCURSOR_HEIGHT;
            animation_cursor_up = false;
          }
        }
        /* Down motion of next cursor */
        else
        {
          animation_cursor -=
              1.0 * cycle_time * kCURSOR_HEIGHT / kCURSOR_ANIMATE;
          if(animation_cursor <= 0)
          {
            animation_cursor = 0;
            animation_cursor_up = true;
          }
        }

        /* Update the hold clock cursor */
        img_convo_c.update(cycle_time);

        /* The animating shifter, for text that is too long to fit in the
         * viewing window */
        animation_shifter += cycle_time / kBUBBLES_ANIMATE;
        if(animation_shifter >= (kBUBBLES_COUNT + 1))
          animation_shifter = 0;
      }
    }

    /* The portion that updates the pickup notification - if hiding */
    if(pickup_status == WindowStatus::HIDING)
    {
      pickup_offset -= cycle_time * 1.0 / kSHIFT_TIME;
      if(pickup_offset <= 0.0)
      {
        pickup_status = WindowStatus::OFF;
        pickup_offset = 0.0;

        /* Remove from queue since completed */
        pickup_queue.erase(pickup_queue.begin());
      }
    }
    /* If showing, shift it out until fully visible */
    else if(pickup_status == WindowStatus::SHOWING)
    {
      pickup_offset += cycle_time / kSHIFT_TIME;
      if(pickup_offset >= frame_right.getWidth())
      {
        pickup_status = WindowStatus::ON;
        pickup_offset = frame_right.getWidth();
      }
    }
    /* If visible, display for the duration as set of the pickup time */
    else if(pickup_status == WindowStatus::ON)
    {
      if(cycle_time > pickup_time)
      {
        pickup_time = 0;
        pickup_status = WindowStatus::HIDING;
      }
      else
      {
        pickup_time -= cycle_time;
      }
    }
  }
}
