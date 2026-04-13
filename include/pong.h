#include <time.h>
#include <stdlib.h>
#include <stdbool.h>

#include "raylib.h"

#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 700

#define PADDLE_WIDTH 100
#define PADDLE_HEIGHT 8
#define PADDLE_SPEED 500
#define PADDLE_MARGIN 50

#define AI_SPEED_FACTOR 0.8f

#define BALL_RADIUS 12
#define BALL_SPEED 200
#define BALL_ACCELERATION 400
#define BALL_MAX_BOUNCE_ANGLE (45 * DEG2RAD)
#define BALL_SPIN_FACTOR 0.001f
#define BALL_RESET_TIME 3.0f
#define BALL_VISIBILITY_TIME BUTTON_DELAY

#define BUTTON_DELAY 0.2f

typedef enum Screen_State {
  LOGO = 0,
  TITLE,
  GAMEPLAY,
  ENDGAME
} Screen_State;

typedef enum {
  TITLE_PLAY_BUTTON = 0,
  TITLE_QUIT_BUTTON
} Title_Menu;

typedef enum {
  PAUSE_RESUME_BUTTON = 0,
  PAUSE_RETRY_BUTTON,
  PAUSE_QUIT_BUTTON
} Pause_Menu;

typedef enum {
  ENDGAME_RETRY_BUTTON = 0,
  ENDGAME_QUIT_BUTTON
} Endgame_Menu;

typedef enum {
  LEFT = -1,
  RIGHT = 1
} Direction;

typedef enum {
  NONE = 0,
  PLAYER,
  AI
} Winner;

typedef struct Game {
  int dynamic_width;
  int dynamic_height;

  Screen_State screen;
  Title_Menu title_menu;
  Pause_Menu pause_menu;
  Endgame_Menu endgame_menu;

  float button_delay;
  int   min_timer;
  float sec_timer;


  bool paused;
  bool finished;

  Winner winner;
} Game;

typedef struct Player {
  Rectangle paddle;
  float speed;
  Vector2 vel;
  int score;
} Player;

typedef struct Ball {
  Vector2 c;
  int r;
  float speed;
  Vector2 vel;
  Vector2 dir;

  bool is_out;
  bool is_reseted;
  bool is_visible;

  float reset_timer;
  float visibility_timer;
  float opacity;

  Sound hit_sfx;
} Ball;

Vector2 rand_dir();
void player_movement(Player* player);
void player_pos_update(Player* player);
void ai_movement(Player* ai, Ball* ball);
void player_wall_coll(Player* player);
void ball_movement(Ball* ball);
void ball_pos_update(Ball* ball);
void ball_wall_coll(Ball* ball);
void player_ball_coll(Player* player, Ball* ball);
void ai_ball_coll(Player* ai, Ball* ball);
void handle_end_round(Player* player, Player* ai, Ball* ball);
void handle_endgame(Game* game, Player* player, Player* ai, Ball* ball);

void run_logo_screen(Game* game, int* logo_state, float* logo_opacity, float* text_opacity, Sound* title_sfx, bool* sfx_played);
void run_title_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx);
void run_gameplay_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx, bool* sfx_played, Player* player, Player* ai, Ball* ball);
void check_winner(Game* game, Player* player, Player* ai, bool* sfx_played);
void run_pause_menu(Game* game, Sound* navigate_sfx, Sound* button_sfx, Player* player, Player* ai, Ball* ball);
void check_if_paused(Game* game, Sound* button_sfx);
void run_endgame_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx, Sound* win_sfx, Sound* gameover_sfx, bool* sfx_played, Player* player, Player* ai, Ball* ball);
void run_endgame_menu(Game* game, Sound* navigate_sfx, Sound* button_sfx, Player* player, Player* ai, Ball* ball);

void draw_logo(Game* game, Font* bree_font_h1, Font* bree_font_h2, Texture2D* heart, float logo_opacity, float text_opacity);
void draw_title(Game* game, Font* cardo_font_h2, Texture2D* arrow);
void draw_gameplay(Game* game, Font* cardo_font_h2, Texture2D* arrow, Player* player, Player* ai, Ball* ball);
void draw_counters(Game* game, Player* player, Player* ai);
void draw_objects(Game* game, Player* player, Player* ai, Ball* ball);
void draw_frame_outline(Game* game);
void draw_pause_menu(Game* game, Font* cardo_font_h2, Texture2D* arrow);
void draw_endgame(Game* game, Font* cardo_font_h1, Font* cardo_font_h2, Texture2D* arrow);
