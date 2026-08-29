#include "raylib.h"
#include <cstdlib>
#include <ctime>

// ==================== 常量定义 ====================
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;
const int PADDLE_WIDTH = 10;
const int PADDLE_HEIGHT = 80;
const int BALL_SIZE = 8;
const int WIN_SCORE = 5;

// ==================== 游戏状态枚举 ====================
enum GameState {
    MENU,
    PLAYING,
    GAMEOVER
};

// ==================== 结构体定义 ====================
struct Player {
    float x, y;
    float speed = 5.0f;
    int score = 0;
};

struct Ball {
    float x, y;
    float speedX, speedY;
    int size;
};

// ==================== 全局变量 ====================
GameState gameState = MENU;
int difficulty = 1;  // 0=简单, 1=普通, 2=困难
Player player1, player2;
Ball ball;

// ==================== 函数声明 ====================
void ResetGame();
void ResetBall();
void DrawMenu();
void DrawGame();
void DrawGameOver();
void UpdateAI();

// ==================== 主函数 ====================
int main() {
    // 初始化窗口
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pong Game - C++ Project");
    SetTargetFPS(60);
    InitAudioDevice();  // 初始化音频设备（虽然本版本没加音效，但为以后扩展保留）

    // 初始化随机数种子
    srand((unsigned)time(NULL));

    // 初始化玩家
    ResetGame();

    while (!WindowShouldClose()) {
        // ========== 输入处理 ==========
        if (gameState == MENU) {
            // 难度选择
            if (IsKeyPressed(KEY_ONE)) { difficulty = 0; }
            if (IsKeyPressed(KEY_TWO)) { difficulty = 1; }
            if (IsKeyPressed(KEY_THREE)) { difficulty = 2; }
            if (IsKeyPressed(KEY_ENTER)) {
                gameState = PLAYING;
                ResetGame();
                // 根据难度设置球速
                float baseSpeed = 3.5f + difficulty * 1.2f;
                ball.speedX = (rand() % 2 == 0) ? baseSpeed : -baseSpeed;
                ball.speedY = (rand() % 2 == 0) ? baseSpeed : -baseSpeed;
            }
        }
        else if (gameState == PLAYING) {
            // ----- 玩家1控制（W/S） -----
            if (IsKeyDown(KEY_W) && player1.y > 0) player1.y -= player1.speed;
            if (IsKeyDown(KEY_S) && player1.y < SCREEN_HEIGHT - PADDLE_HEIGHT) player1.y += player1.speed;

            // ----- 玩家2控制（AI） -----
            UpdateAI();

            // ----- 球移动 -----
            ball.x += ball.speedX;
            ball.y += ball.speedY;

            // 上下边界反弹
            if (ball.y <= 0 || ball.y >= SCREEN_HEIGHT - ball.size) {
                ball.speedY = -ball.speedY;
            }

            // 碰撞检测：球 vs 玩家1
            if (ball.x <= player1.x + PADDLE_WIDTH &&
                ball.x >= player1.x &&
                ball.y + ball.size >= player1.y &&
                ball.y <= player1.y + PADDLE_HEIGHT) {
                ball.speedX = -ball.speedX;
                ball.x = player1.x + PADDLE_WIDTH;
                // 增加一点随机性，让反弹更有趣
                ball.speedY += (rand() % 3 - 1) * 0.5f;
            }

            // 碰撞检测：球 vs 玩家2
            if (ball.x + ball.size >= player2.x &&
                ball.x <= player2.x + PADDLE_WIDTH &&
                ball.y + ball.size >= player2.y &&
                ball.y <= player2.y + PADDLE_HEIGHT) {
                ball.speedX = -ball.speedX;
                ball.x = player2.x - ball.size;
                ball.speedY += (rand() % 3 - 1) * 0.5f;
            }

            // ----- 得分判定 -----
            if (ball.x < 0) {
                player2.score++;
                ResetBall();
                // 球向获胜方反方向发球
                ball.speedX = (rand() % 2 == 0) ? 3.5f + difficulty * 1.2f : -(3.5f + difficulty * 1.2f);
            }
            if (ball.x > SCREEN_WIDTH) {
                player1.score++;
                ResetBall();
                ball.speedX = (rand() % 2 == 0) ? 3.5f + difficulty * 1.2f : -(3.5f + difficulty * 1.2f);
            }

            // 检查是否有人获胜
            if (player1.score >= WIN_SCORE || player2.score >= WIN_SCORE) {
                gameState = GAMEOVER;
            }
        }
        else if (gameState == GAMEOVER) {
            if (IsKeyPressed(KEY_R)) {
                gameState = PLAYING;
                ResetGame();
                float baseSpeed = 3.5f + difficulty * 1.2f;
                ball.speedX = (rand() % 2 == 0) ? baseSpeed : -baseSpeed;
                ball.speedY = (rand() % 2 == 0) ? baseSpeed : -baseSpeed;
            }
            if (IsKeyPressed(KEY_M)) {
                gameState = MENU;
                ResetGame();
            }
        }

        // ========== 绘制 ==========
        BeginDrawing();
        ClearBackground(BLACK);

        switch (gameState) {
        case MENU:
            DrawMenu();
            break;
        case PLAYING:
            DrawGame();
            break;
        case GAMEOVER:
            DrawGameOver();
            break;
        }

        EndDrawing();
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// ==================== 函数实现 ====================

void ResetGame() {
    // 重置玩家位置和分数
    player1.x = 20;
    player1.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
    player1.score = 0;
    player1.speed = 5.0f;

    player2.x = SCREEN_WIDTH - 20 - PADDLE_WIDTH;
    player2.y = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
    player2.score = 0;
    player2.speed = 5.0f;

    ResetBall();
}

void ResetBall() {
    ball.x = SCREEN_WIDTH / 2;
    ball.y = SCREEN_HEIGHT / 2;
    ball.size = BALL_SIZE;
    // speed 在调用处设置，这里不重置
}

void UpdateAI() {
    // AI 只在球向自己飞来时追踪，避免"作弊"感
    float aiSpeed = player2.speed * (0.6f + difficulty * 0.15f);  // 难度越高AI越快

    if (ball.speedX > 0) {
        // 球正在向玩家2方向移动，追踪球
        if (ball.y > player2.y + PADDLE_HEIGHT / 2) {
            player2.y += aiSpeed;
        }
        else if (ball.y < player2.y + PADDLE_HEIGHT / 2) {
            player2.y -= aiSpeed;
        }
    }
    else {
        // 球向左移动时，AI 缓慢回到中央位置
        float centerY = SCREEN_HEIGHT / 2 - PADDLE_HEIGHT / 2;
        if (player2.y < centerY - 10) {
            player2.y += 0.8f;
        }
        else if (player2.y > centerY + 10) {
            player2.y -= 0.8f;
        }
    }

    // 边界限制
    if (player2.y < 0) player2.y = 0;
    if (player2.y > SCREEN_HEIGHT - PADDLE_HEIGHT) player2.y = SCREEN_HEIGHT - PADDLE_HEIGHT;
}

void DrawMenu() {
    DrawText("=== PONG GAME ===", SCREEN_WIDTH / 2 - 150, 80, 50, WHITE);

    const char* diffTexts[] = { "Easy", "Normal", "Hard" };
    Color diffColors[] = { GREEN, YELLOW, RED };

    for (int i = 0; i < 3; i++) {
        Color color = (i == difficulty) ? diffColors[i] : GRAY;
        DrawText(TextFormat("Press %d: %s", i + 1, diffTexts[i]),
            SCREEN_WIDTH / 2 - 120 + i * 180, 180, 30, color);
        if (i == difficulty) {
            DrawText("<--", SCREEN_WIDTH / 2 - 160 + i * 180, 180, 30, YELLOW);
        }
    }

    DrawText("Press ENTER to start", SCREEN_WIDTH / 2 - 140, 280, 30, LIGHTGRAY);
    DrawText("W/S: Move Left  |  AI: Right", SCREEN_WIDTH / 2 - 160, 350, 20, GRAY);
    DrawText("First to 5 wins!", SCREEN_WIDTH / 2 - 100, 400, 20, GRAY);
}

void DrawGame() {
    // 绘制两个挡板
    DrawRectangle(player1.x, player1.y, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);
    DrawRectangle(player2.x, player2.y, PADDLE_WIDTH, PADDLE_HEIGHT, WHITE);

    // 绘制球
    DrawCircle(ball.x, ball.y, ball.size, RED);

    // 绘制分数
    DrawText(TextFormat("%d", player1.score), SCREEN_WIDTH / 4 - 20, 20, 50, WHITE);
    DrawText(TextFormat("%d", player2.score), 3 * SCREEN_WIDTH / 4 - 20, 20, 50, WHITE);

    // 绘制中间虚线
    for (int i = 0; i < SCREEN_HEIGHT; i += 20) {
        DrawRectangle(SCREEN_WIDTH / 2 - 2, i, 4, 10, WHITE);
    }

    // 显示当前难度（小字）
    const char* diffNames[] = { "Easy", "Normal", "Hard" };
    DrawText(TextFormat("Difficulty: %s", diffNames[difficulty]), 10, 10, 15, GRAY);
}

void DrawGameOver() {
    const char* winner = (player1.score >= WIN_SCORE) ? "Player 1 Wins!" : "AI Wins!";
    Color winnerColor = (player1.score >= WIN_SCORE) ? GREEN : RED;

    DrawText(winner, SCREEN_WIDTH / 2 - MeasureText(winner, 60) / 2, SCREEN_HEIGHT / 2 - 80, 60, winnerColor);

    DrawText(TextFormat("Final Score: %d - %d", player1.score, player2.score),
        SCREEN_WIDTH / 2 - MeasureText(TextFormat("Final Score: %d - %d", player1.score, player2.score), 30) / 2,
        SCREEN_HEIGHT / 2 - 10, 30, WHITE);

    DrawText("Press R to rematch", SCREEN_WIDTH / 2 - MeasureText("Press R to rematch", 25) / 2,
        SCREEN_HEIGHT / 2 + 50, 25, LIGHTGRAY);

    DrawText("Press M for menu", SCREEN_WIDTH / 2 - MeasureText("Press M for menu", 20) / 2,
        SCREEN_HEIGHT / 2 + 90, 20, GRAY);
}