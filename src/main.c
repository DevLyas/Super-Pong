#include "../include/pong.h"

int main() {
  SetRandomSeed(time(NULL));
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Super Pong");
  InitAudioDevice();

  Font bree_font_h1  = LoadFontEx("../assets/font/BreeSerif-Regular.ttf", 96, 0, 0);
  Font bree_font_h2  = LoadFontEx("../assets/font/BreeSerif-Regular.ttf", 48, 0, 0);
  Font cardo_font_h1 = LoadFontEx("../assets/font/Cardo-Bold.ttf", 96, 0, 0);
  Font cardo_font_h2 = LoadFontEx("../assets/font/Cardo-Bold.ttf", 48, 0, 0);

  Texture2D heart = LoadTexture("../assets/img/heart.png");
  Texture2D arrow = LoadTexture("../assets/img/arrow.png");

  Sound title_sfx    = LoadSound("../assets/sfx/title.wav");
  Sound button_sfx   = LoadSound("../assets/sfx/button.wav");
  Sound navigate_sfx = LoadSound("../assets/sfx/navigate.wav");
  Sound gameover_sfx = LoadSound("../assets/sfx/gameover.wav");
  Sound win_sfx      = LoadSound("../assets/sfx/win.wav");

  Game game;

  game.paused = false;
  game.finished = false;
  game.winner = NONE;

  game.screen = LOGO;

  game.pause_menu   = PAUSE_RESUME_BUTTON;
  game.title_menu   = TITLE_PLAY_BUTTON;
  game.endgame_menu = ENDGAME_RETRY_BUTTON;
  game.button_delay = 0;

  game.min_timer = 0;
  game.sec_timer = 0;

  Player player = {
    .paddle = (Rectangle){
      .x = SCREEN_WIDTH/2.0f - PADDLE_WIDTH/2.0f,
      .y = SCREEN_HEIGHT - PADDLE_MARGIN,
      .width = PADDLE_WIDTH,
      .height = PADDLE_HEIGHT
    },
    .speed = PADDLE_SPEED,
    .score = 0
  };

  Player ai = {
    .paddle = (Rectangle){
      .x = SCREEN_WIDTH/2.0f - PADDLE_WIDTH/2.0f,
      .y = PADDLE_MARGIN,
      .width = PADDLE_WIDTH,
      .height = PADDLE_HEIGHT
    },
    .speed = PADDLE_SPEED * AI_SPEED_FACTOR,
    .score = 0
  };

  Ball ball = {
    .c = (Vector2) { SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f },
    .r = BALL_RADIUS,
    .speed = BALL_SPEED,
    .dir = rand_dir(),
    .is_out = false,
    .is_reseted = false,
    .is_visible = true,
    .reset_timer = 0,
    .visibility_timer = 0,
    .opacity = 255.0f,
    .hit_sfx = LoadSound("../assets/sfx/hit.wav")
  };

  float logo_opacity = 0;
  int   logo_state   = 0;
  float text_opacity = 0;
 
  bool sfx_played = false;

  SetTargetFPS(60);
  // Game loop
  while (!WindowShouldClose() && !game.finished) {
    // Update section
 
    game.dynamic_width  = GetScreenWidth();
    game.dynamic_height = GetScreenHeight();

    switch (game.screen) {
      case LOGO:
        run_logo_screen(&game, &logo_state, &logo_opacity, &text_opacity, &title_sfx, &sfx_played);
      break;

      case TITLE:
        run_title_screen(&game, &navigate_sfx, &button_sfx);
      break;

      case GAMEPLAY:
        run_gameplay_screen(&game, &navigate_sfx, &button_sfx, &sfx_played, &player, &ai, &ball);
      break;

      case ENDGAME:
        run_endgame_screen(&game, &navigate_sfx, &button_sfx, &win_sfx, &gameover_sfx, &sfx_played, &player, &ai, &ball);
      break;
      }

    // ------------------------------------------------------------------------------------------------------------------
    // draw section
    // ------------------------------------------------------------------------------------------------------------------

    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (game.screen) {
      case LOGO:
        draw_logo(&game, &bree_font_h1, &bree_font_h2, &heart, logo_opacity, text_opacity);
      break;

      case TITLE:
        draw_title(&game, &cardo_font_h2, &arrow);
      break;

      case GAMEPLAY:
        draw_gameplay(&game, &cardo_font_h2, &arrow, &player, &ai, &ball);
      break;

      case ENDGAME:
        draw_endgame(&game, &cardo_font_h1, &cardo_font_h2, &arrow);
      break;
    }
    EndDrawing();
  }

  UnloadFont(bree_font_h1);
  UnloadFont(bree_font_h2);
  UnloadFont(cardo_font_h1);
  UnloadFont(cardo_font_h2);

  UnloadTexture(heart);
  UnloadTexture(arrow);

  UnloadSound(title_sfx);
  UnloadSound(button_sfx);
  UnloadSound(ball.hit_sfx);
  UnloadSound(navigate_sfx);
  UnloadSound(win_sfx);
  UnloadSound(gameover_sfx);

  CloseWindow();
}
