#include "../include/pong.h"

Vector2 rand_dir() {
  int r1 = GetRandomValue(0, 1) ? LEFT : RIGHT;
  int r2 = GetRandomValue(0, 1) ? LEFT : RIGHT;

  return (Vector2){ r1, r2 };
}

void player_movement(Player* player) {
  player->vel.x = 0;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
    player->vel.x = -player->speed;
  }
  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
    player->vel.x = player->speed;
  }
}

void player_pos_update(Player* player) {
  player->paddle.x += player->vel.x * GetFrameTime();
}

void ai_movement(Player* ai, Ball* ball) {
  float paddle_center = ai->paddle.x + ai->paddle.width/2.0f;
  int diff = paddle_center - ball->c.x;
  const int DEAD_ZONE = ai->paddle.width/8;

  if (ball->dir.y < 0 && abs(diff) > DEAD_ZONE) {
    ai->vel.x = (diff > 0 ? LEFT : RIGHT) * ai->speed;
  } else {
    ai->vel.x = 0;
  }
}

void player_wall_coll(Player* player) {
  if (player->paddle.x < 0) {
    player->paddle.x = 0;
  }
  if (player->paddle.x + player->paddle.width > SCREEN_WIDTH) {
    player->paddle.x = SCREEN_WIDTH - player->paddle.width;
  }
}

void ball_movement(Ball* ball) {
  ball->vel.x = ball->dir.x * ball->speed;
  ball->vel.y = ball->dir.y * ball->speed;
}

void ball_pos_update(Ball* ball) {
  ball->c.x += ball->vel.x * GetFrameTime();
  ball->c.y += ball->vel.y * GetFrameTime();
}

void ball_wall_coll(Ball* ball) {
  if (ball->c.x - ball->r < 0) {
    PlaySound(ball->hit_sfx);
    ball->c.x = ball->r;
    ball->dir.x = RIGHT;
  }
  if (ball->c.x + ball->r > SCREEN_WIDTH) {
    PlaySound(ball->hit_sfx);
    ball->c.x = SCREEN_WIDTH - ball->r;
    ball->dir.x = LEFT;
  }
}

void player_ball_coll(Player* player, Ball* ball) {
  if (ball->dir.y > 0 && CheckCollisionCircleRec(ball->c, ball->r, player->paddle)) {
    PlaySound(ball->hit_sfx);
    ball->c.y = player->paddle.y - ball->r;
    ball->dir.y = -1;
    ball->speed += BALL_ACCELERATION * GetFrameTime();
  }
}

void ai_ball_coll(Player* ai, Ball* ball) {
  if (ball->dir.y < 0 && CheckCollisionCircleRec(ball->c, ball->r, ai->paddle)) {
    PlaySound(ball->hit_sfx);
    ball->c.y = ai->paddle.y + ai->paddle.height + ball->r;
    ball->dir.y = 1;
    ball->speed += BALL_ACCELERATION * GetFrameTime();
  }
}

void handle_end_round(Player* player, Player* ai, Ball* ball) {
  // detect end round
  if (ball->c.y + ball->r < 0) {
    ball->is_reseted = false;
    ++player->score;
  } else if (ball->c.y - ball->r > SCREEN_HEIGHT) {
    ball->is_reseted = false;
    ++ai->score;
  }

  if (!ball->is_reseted) {
    // reset positions
    player->paddle.x = SCREEN_WIDTH/2.0f - player->paddle.width/2;
    ai->paddle.x = SCREEN_WIDTH/2.0f - ai->paddle.width/2;
    ball->c = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };

    ball->speed = 0;

    // update timers
    ball->reset_timer += GetFrameTime();
    ball->visibility_timer += GetFrameTime();
  }

  // ball reset animation
  if (!ball->is_reseted && ball->visibility_timer >= BALL_VISIBILITY_TIME) {
    if (ball->is_visible) {
      ball->opacity = 0;
      ball->is_visible = false;
    } else {
      ball->opacity = 255.0f;
      ball->is_visible = true;
    }
    ball->visibility_timer = 0;
  }

  // reset ball
  if (ball->reset_timer >= BALL_RESET_TIME) {
    ball->speed = BALL_SPEED;
    ball->dir = rand_dir();
    ball->opacity = 255.0f;
    ball->reset_timer = 0;
    ball->is_reseted = true;
  }
}
void handle_endgame(Game* game, Player* player, Player* ai, Ball* ball) {
  ball->is_reseted = false;
  handle_end_round(player, ai, ball);
  game->sec_timer = 0;
  game->min_timer = 0;
  game->winner = NONE;
  game->paused = false;
  player->score = 0;
  ai->score = 0;
}

void run_logo_screen(Game* game, int* logo_state, float* logo_opacity, float* text_opacity, Sound* title_sfx, bool* sfx_played) {
  if (!*sfx_played) {
    PlaySound(*title_sfx);
    *sfx_played = true;
  }
  if (*logo_state == 0) {
    *logo_opacity += 1/90.0f;

    if (*logo_opacity > 1) {
      *logo_opacity = 1;
      *logo_state = 1;
    }
  } else if (*logo_state == 1) {
    game->sec_timer += GetFrameTime();
    if (game->sec_timer > 3.0f) {
      game->sec_timer = 0;
      *logo_state = 2;
    }

    if (game->sec_timer > 1.0f) {
      *text_opacity += 1/30.0f;
    }

    if (*text_opacity > 1) {
      *text_opacity = 1;
    }
  } else if (*logo_state == 2) {
    *logo_opacity -= 1/90.0f;
    *text_opacity -= 1/90.0f;
    if (*logo_opacity < 0) {
      *logo_opacity = 0;
      *text_opacity = 0;
      game->screen = TITLE;
    }
  }

  if (IsKeyPressed(KEY_ENTER)) {
    game->screen = TITLE;
  }
}

void run_title_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx) {
  game->button_delay = BUTTON_DELAY;

  switch (game->title_menu) {
    case TITLE_PLAY_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB))) {
        game->title_menu = TITLE_QUIT_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }
      if (game->button_delay >= BUTTON_DELAY && IsKeyPressed(KEY_UP)) {
        game->title_menu = TITLE_QUIT_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        game->screen = GAMEPLAY;
      }

      if (game->title_menu != TITLE_PLAY_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;

    case TITLE_QUIT_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB))) {
        game->title_menu = TITLE_PLAY_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }
      if (game->button_delay >= BUTTON_DELAY && IsKeyPressed(KEY_UP)) {
        game->title_menu = TITLE_PLAY_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        game->finished = true;
      }

      if (game->title_menu != TITLE_QUIT_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;
  }
}


void run_gameplay_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx, bool* sfx_played, Player* player, Player* ai, Ball* ball) {
  if (!game->paused) {
    game->sec_timer += GetFrameTime();

    float temp = (int)game->sec_timer / 60;
    game->min_timer += temp;
    game->sec_timer -= temp * 60;

    // Player
    player_movement(player);
    player_pos_update(player);
    player_wall_coll(player);

    // AI
    ai_movement(ai, ball);
    player_pos_update(ai);
    player_wall_coll(ai);

    // Ball
    ball_movement(ball);
    ball_pos_update(ball);
    ball_wall_coll(ball);

    // Ball and player
    player_ball_coll(player, ball);
    ai_ball_coll(ai, ball);
    handle_end_round(player, ai, ball);

    check_winner(game, player, ai, sfx_played);

  } else {
    run_pause_menu(game, navigate_sfx, button_sfx, player, ai, ball);
  }

  check_if_paused(game, button_sfx);
}

void check_winner(Game* game, Player* player, Player* ai, bool* sfx_played) {
    if (player->score == 3) {
      game->winner = PLAYER;
      *sfx_played = false;
    } else if (ai->score == 3) {
      game->winner = AI;
      *sfx_played = false;
    }

    if (game->winner != NONE) {
      game->screen = ENDGAME;
    }
}

void run_pause_menu(Game* game, Sound* navigate_sfx, Sound* button_sfx, Player* player, Player* ai, Ball* ball) {
  game->button_delay = BUTTON_DELAY;

  switch (game->pause_menu) {
    case PAUSE_RESUME_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB))) {
        game->pause_menu = PAUSE_RETRY_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }
      if (game->button_delay >= BUTTON_DELAY && IsKeyPressed(KEY_UP)) {
        game->pause_menu = PAUSE_QUIT_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        game->paused = false;
      }

      if (game->pause_menu != PAUSE_RESUME_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;

    case PAUSE_RETRY_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB))) {
        game->pause_menu = PAUSE_QUIT_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }
      if (game->button_delay >= BUTTON_DELAY && IsKeyPressed(KEY_UP)) {
        game->pause_menu = PAUSE_RESUME_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        handle_endgame(game, player, ai, ball);
      }

      if (game->pause_menu != PAUSE_RETRY_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;

    case PAUSE_QUIT_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB))) {
        game->pause_menu = PAUSE_RESUME_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_UP))) {
        game->pause_menu = PAUSE_RETRY_BUTTON;
        game->button_delay = 0;
        PlaySound(*navigate_sfx);
      }

      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        handle_endgame(game, player, ai, ball);
        game->screen = TITLE;
      }
      if (game->pause_menu != PAUSE_QUIT_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;
  }
}

void check_if_paused(Game* game, Sound* button_sfx) {
  if (IsKeyPressed(KEY_P)) {
    PlaySound(*button_sfx);
    game->paused = !game->paused;
  }
}

void run_endgame_screen(Game* game, Sound* navigate_sfx, Sound* button_sfx, Sound* win_sfx, Sound* gameover_sfx, bool* sfx_played, Player* player, Player* ai, Ball* ball) {
  switch (game->winner) {
    case PLAYER:
      if (!*sfx_played) {
        PlaySound(*win_sfx);
        *sfx_played = true;
      }
    break;
    case AI:
      if (!*sfx_played) {
        PlaySound(*gameover_sfx);
        *sfx_played = true;
      }
    break;
  }
  run_endgame_menu(game, navigate_sfx, button_sfx, player, ai, ball);
}

void run_endgame_menu(Game* game, Sound* navigate_sfx, Sound* button_sfx, Player* player, Player* ai, Ball* ball) {
  game->button_delay = BUTTON_DELAY;
  switch (game->endgame_menu) {
    case ENDGAME_RETRY_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP))) {
        PlaySound(*navigate_sfx);
        game->endgame_menu = ENDGAME_QUIT_BUTTON;
        game->button_delay = 0;

      }
      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        handle_endgame(game, player, ai, ball);
        game->screen = GAMEPLAY;
      }
      if (game->endgame_menu != ENDGAME_RETRY_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;
    case ENDGAME_QUIT_BUTTON:
      if (game->button_delay >= BUTTON_DELAY && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP))) {
        PlaySound(*navigate_sfx);
        game->endgame_menu = ENDGAME_RETRY_BUTTON;
        game->button_delay = 0;
      }
      if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(*button_sfx);
        handle_endgame(game, player, ai, ball);
        game->screen = TITLE;
      }
      if (game->endgame_menu != ENDGAME_QUIT_BUTTON) {
        game->button_delay += GetFrameTime();
      }
    break;
  }
}

void draw_logo(Game* game, Font* bree_font_h1, Font* bree_font_h2, Texture2D* heart, float logo_opacity, float text_opacity) {
  DrawTextEx(
    *bree_font_h1,
    "Super",
    (Vector2){game->dynamic_width/2 - 185, game->dynamic_height/2 - 60},
    96,
    1,
    (Color){255.0f, 0, 0, 255 * logo_opacity
  });
  DrawTextEx(
    *bree_font_h1,
    "Pong",
    (Vector2){game->dynamic_width/2 + 15, game->dynamic_height/2 - 60},
    96,
    1,
    (Color){0, 0, 0, 255 * logo_opacity
  });
  DrawTextEx(
    *bree_font_h2,
    "Made With ",
    (Vector2){game->dynamic_width/2 + 45, game->dynamic_height/2 + 35},
    24,
    2,
    (Color){0, 0, 0, 255.0f * text_opacity
  });
  DrawTextureEx(
    *heart,
    (Vector2){game->dynamic_width/2 + 150, game->dynamic_height/2 + 35},
    0,
    0.05,
    (Color){255.0f, 255.0f, 255.0f, 255.0f * text_opacity
  });
}

void draw_title(Game* game, Font* cardo_font_h2, Texture2D* arrow) {
  if (game->title_menu != TITLE_PLAY_BUTTON) {
    DrawTextEx(
      *cardo_font_h2,
      "Play",
      (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 - 50},
      48,
      4,
      BLACK
    );
  }
  if (game->title_menu != TITLE_QUIT_BUTTON) {
    DrawTextEx(
      *cardo_font_h2,
      "Quit",
      (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 + 8},
      48,
      4,
      BLACK
    );
  }
  switch (game->title_menu) {
    case TITLE_PLAY_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 - 33},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Play",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 - 50},
        48,
        4,
        RED
      );
      break;

      case TITLE_QUIT_BUTTON:
        DrawTextureEx(
          *arrow,
          (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 + 24},
          0,
          0.006,
          WHITE
        );
        DrawTextEx(
          *cardo_font_h2,
          "Quit",
          (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 + 8},
          48,
          4,
          RED
        );
      break;
    }
}

void draw_gameplay(Game* game, Font* cardo_font_h2, Texture2D* arrow, Player* player, Player* ai, Ball* ball) {
  draw_counters(game, player, ai);
  draw_objects(game, player, ai, ball);
  draw_frame_outline(game);

  if (game->paused) {
    draw_pause_menu(game, cardo_font_h2, arrow);
  }
}

void draw_counters(Game* game, Player* player, Player* ai) {
  // center line
  DrawLineEx(
    (Vector2){ game->dynamic_width/2 - SCREEN_WIDTH/2, game->dynamic_height/2 },
    (Vector2){ game->dynamic_width/2 + SCREEN_WIDTH/2, game->dynamic_height/2 },
    2.0f,
    (Color){ 0, 0, 0, 255 * 0.25f }
  );
  // player score
  DrawText(
    TextFormat("%d", player->score),
    20 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 + 30 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    40,
    (Color){ 255, 0, 0, 255 * 0.5f }
  );
  // ai score
  DrawText(
    TextFormat("%d", ai->score),
    20 + game->dynamic_width/2  - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 - 65 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    40,
    (Color){ 0, 0, 255, 255 * 0.5f }
  );
  DrawRectangle(
    10 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 - 13 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    103,
    25,
    RAYWHITE
  );
  DrawCircle(
    62 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 - 5 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    4,
    (Color){ 0, 0, 0, 255 * 0.75f }
  );
  DrawCircle(
    62 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 + 5 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    4,
    (Color){ 0, 0, 0, 255 * 0.75f }
  );
  // min timer
  DrawText(
    TextFormat("%02d", (int)game->min_timer),
    20 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 - 14 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    30,
    (Color){ 0, 0, 0, 255 * 0.75f }
  );
  // sec timer
  DrawText(
    TextFormat("%02d", (int)game->sec_timer),
    71 + game->dynamic_width/2 - SCREEN_WIDTH/2,
    SCREEN_HEIGHT/2 - 14 + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    30,
    (Color){ 0, 0, 0, 255 * 0.75f }
  );
}

void draw_objects(Game* game, Player *player, Player *ai, Ball *ball) {
  // Player
  DrawRectangle(
    player->paddle.x + game->dynamic_width/2 - SCREEN_WIDTH/2,
    player->paddle.y + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    player->paddle.width,
    player->paddle.height,
    RED
  );
  // AI
  DrawRectangle(
    ai->paddle.x + game->dynamic_width/2 - SCREEN_WIDTH/2,
    ai->paddle.y + game->dynamic_height/2 - SCREEN_HEIGHT/2,
    ai->paddle.width,
    ai->paddle.height,
    BLUE
  );
  // Ball
  DrawCircle(
    ball->c.x - SCREEN_WIDTH/2 + game->dynamic_width/2,
    ball->c.y - SCREEN_HEIGHT/2 + game->dynamic_height/2,
    ball->r,
    (Color){0, 0, 0, ball->opacity}
  );
}

void draw_frame_outline(Game* game) {
  DrawRectangle(0, 0, game->dynamic_width, game->dynamic_height/2 - SCREEN_HEIGHT/2, BLACK);
  DrawRectangle(0, game->dynamic_height/2 + SCREEN_HEIGHT/2, game->dynamic_width, game->dynamic_height/2 - SCREEN_HEIGHT/2, BLACK);
  DrawRectangle(0, 0, game->dynamic_width/2 - SCREEN_WIDTH/2, game->dynamic_height, BLACK);
  DrawRectangle(game->dynamic_width/2 + SCREEN_WIDTH/2, 0, game->dynamic_width/2 - SCREEN_WIDTH/2, game->dynamic_height, BLACK);
}

void draw_pause_menu(Game* game, Font* cardo_font_h2, Texture2D* arrow) {
  // transparent background
  DrawRectangle(0, 0, game->dynamic_width, game->dynamic_height, (Color){255.0f, 255.0f, 255.0f, 255.0f * 0.5f});

  if (game->pause_menu != PAUSE_RESUME_BUTTON) {
    DrawTextEx(
      *cardo_font_h2,
      "Resume",
      (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 - 80},
      48,
      4,
      BLACK
    );
  }
  if (game->pause_menu != PAUSE_RETRY_BUTTON) {
    DrawTextEx(
      *cardo_font_h2,
      "Retry",
      (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 - 25},
      48,
      4,
      BLACK
    );
  }
  if (game->pause_menu != PAUSE_QUIT_BUTTON) {
    DrawTextEx(
      *cardo_font_h2,
      "Quit",
      (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 + 33},
      48,
      4,
      BLACK
    );
  }
  switch (game->pause_menu) {
    case PAUSE_RESUME_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 - 63},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Resume",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 - 80},
        48,
        4,
        RED
      );
    break;

    case PAUSE_RETRY_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 - 8},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Retry",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 - 25},
        48,
        4,
        RED
      );
    break;

    case PAUSE_QUIT_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 + 51},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Quit",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 + 33},
        48,
        4,
        RED
      );
    break;
  }
}

void draw_endgame(Game* game, Font* cardo_font_h1, Font* cardo_font_h2, Texture2D* arrow) {
  switch (game->winner) {
    case PLAYER:
      DrawTextEx(
        *cardo_font_h1,
        "YOU WIN",
        (Vector2){
          game->dynamic_width/2 - 200,
          game->dynamic_height/2 - 250
        },
        96,
        4,
        GREEN
      );
    break;
    case AI:
      DrawTextEx(
        *cardo_font_h1,
        "YOU LOST",
        (Vector2){
          game->dynamic_width/2 - 200,
          game->dynamic_height/2 - 250
        },
        96,
        4,
        RED
      );
    break;
  }

  switch (game->endgame_menu) {
    case ENDGAME_RETRY_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 - 33},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Retry",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 - 50},
        48,
        4,
        RED
      );
      DrawTextEx(
        *cardo_font_h2,
        "Quit",
        (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 + 8},
        48,
        4,
        BLACK
      );
    break;

    case ENDGAME_QUIT_BUTTON:
      DrawTextureEx(
        *arrow,
        (Vector2){game->dynamic_width/2 - 68, game->dynamic_height/2 + 24},
        0,
        0.006,
        WHITE
      );
      DrawTextEx(
        *cardo_font_h2,
        "Retry",
        (Vector2){game->dynamic_width/2 - 70, game->dynamic_height/2 - 50},
        48,
        4,
        BLACK
      );
      DrawTextEx(
        *cardo_font_h2,
        "Quit",
        (Vector2){game->dynamic_width/2 - 20, game->dynamic_height/2 + 8},
        48,
        4,
        RED
      );
    break;
  }
}
