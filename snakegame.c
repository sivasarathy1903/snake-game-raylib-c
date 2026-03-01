#include <raylib.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 30
#define CELL_SIZE 20
#define MAX_SCORES 5


typedef enum { TITLE, START, HINT, GAMEPLAY, GAMEOVER } GameState;

int x, y, fruitX, fruitY, score, gameOver;
int tailX[100], tailY[100], tailLength;
char direction;
int isPaused = 0;
int highScores[MAX_SCORES];

int bombX[2], bombY[2];
clock_t lastBombMove;
Texture2D bgEasy, bgMedium, bgHard;

Sound eatSound, gameOverSound, clickSound;
Music bgMusic;
Music titleScreenMusic; 

Texture2D skullTexture;
Texture2D startBackgroundTexture;
Texture2D gameOverBackgroundTexture;
bool showHintScreen = false;
double hintStartTime = 0;
Texture2D hintBackgroundTexture;
const char* hints[] = {
    "1. Avoid hitting the walls!",
    "2. Don't eat the bombs!",
    "3. Eat the red fruit to grow.",
    "4. Press 'P' or click pause to take a break.",
    "5. Higher difficulty = Faster snake!"
};

typedef enum { EASY = 7, MEDIUM = 12, HARD = 17 } Difficulty;
Difficulty currentDifficulty = EASY;
bool difficultySelected = false;  

float snakeSpeedDelay = 0.15f; 
double lastSnakeMoveTime = 0;
void drawGradientText(const char* text, int posX, int posY, int fontSize);
void drawHintScreen();
void setup(void);

bool showStart = true;
Texture2D titleBackgroundTexture;
float titleAnimTime = 0;
float letterOffsets[10] = {0};
bool titleScreenDone = false;
float titleAlpha = 0;
bool showTitleScreen = true;  


float transitionAlpha = 0;
bool isTransitioning = false;
GameState nextGameState;
GameState currentState = TITLE; 


#define MAX_PARTICLES 100

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float alpha;
    Color color;
    bool active;
} Particle;

Particle particles[MAX_PARTICLES];


#define MAX_STARS 100
typedef struct {
    Vector2 position;
    float radius;
    float alpha;
    float speed;
} Star;

Star stars[MAX_STARS];


bool musicTransitioning = false;
bool AnyKeyPressed(void) {
    for (int key = 32; key < 349; key++) { 
        if (IsKeyPressed(key)) return true;
    }
    return false;
}


void initTitleScreen() {
  
    titleBackgroundTexture = LoadTexture("C:\\Users\\sudha\\Downloads\\tapas-dalai-img-20220919-004618-588.jpg");  
    
    
    titleAnimTime = 0;
    titleAlpha = 0;
    titleScreenDone = false;
    showTitleScreen = true;
    
    
    for (int i = 0; i < 10; i++) {
        letterOffsets[i] = 0;
    }
    
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
    
    
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].position.x = GetRandomValue(0, WIDTH * CELL_SIZE);
        stars[i].position.y = GetRandomValue(0, HEIGHT * CELL_SIZE);
        stars[i].radius = GetRandomValue(1, 3) / 2.0f;
        stars[i].alpha = GetRandomValue(30, 100) / 100.0f;
        stars[i].speed = GetRandomValue(1, 5) / 10.0f;
    }
    
    
    titleScreenMusic = LoadMusicStream("C:\\Users\\sudha\\Downloads\\fire-inside-you-background-dramatic-music-for-video-30-second-333568.mp3");   
    PlayMusicStream(titleScreenMusic);
    SetMusicVolume(titleScreenMusic, 0.5f);
    musicTransitioning = false;
}


void updateParticles() {
    
    if (GetRandomValue(0, 10) < 3) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            if (!particles[i].active) {
                particles[i].position = (Vector2){ 
                    GetRandomValue(0, WIDTH * CELL_SIZE),
                    HEIGHT * CELL_SIZE + 5
                };
                particles[i].velocity = (Vector2){ 
                    GetRandomValue(-100, 100) / 100.0f,
                    -GetRandomValue(100, 300) / 100.0f
                };
                particles[i].radius = GetRandomValue(2, 5);
                particles[i].alpha = 1.0f;
                particles[i].color = (Color){ 
                    50, 
                    GetRandomValue(180, 255), 
                    50, 
                    255 
                };
                particles[i].active = true;
                break;
            }
        }
    }
    
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].velocity.x;
            particles[i].position.y += particles[i].velocity.y;
            particles[i].alpha -= 0.01f;
            
            if (particles[i].alpha <= 0) {
                particles[i].active = false;
            }
        }
    }
}


void updateStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        
        stars[i].position.y -= stars[i].speed;
        
        
        if (stars[i].position.y < 0) {
            stars[i].position.y = HEIGHT * CELL_SIZE;
            stars[i].position.x = GetRandomValue(0, WIDTH * CELL_SIZE);
        }
        
        
        stars[i].alpha = GetRandomValue(30, 100) / 100.0f;
    }
}


void updateTransition() {
    if (isTransitioning) {
        transitionAlpha += 0.02f;
        
        if (transitionAlpha >= 1.0f) {
            
            currentState = nextGameState;
            transitionAlpha = 1.0f;
            
            isTransitioning = false;
        }
    } else if (transitionAlpha > 0) {
        
        transitionAlpha -= 0.02f;
        if (transitionAlpha < 0) transitionAlpha = 0;
    }
}

void startTransition(GameState newState) {
    isTransitioning = true;
    transitionAlpha = 0;
    nextGameState = newState;
}


void updateTitleScreenAudio() {
    UpdateMusicStream(titleScreenMusic);
    
  
    if (titleScreenDone && !musicTransitioning) {
        musicTransitioning = true;
    }
    
    if (musicTransitioning) {
        float currentVolume = 0.5f;
        if (currentVolume > 0.01f) {
            SetMusicVolume(titleScreenMusic, currentVolume - 0.01f);
        } else {
            StopMusicStream(titleScreenMusic);
            PlayMusicStream(bgMusic);  
            SetMusicVolume(bgMusic, 0.4f);
        }
    }
}


void updateTitleScreen() {
    titleAnimTime += GetFrameTime();
    
    
    if (titleAlpha < 1.0f) {
        titleAlpha += 0.01f;
        if (titleAlpha > 1.0f) titleAlpha = 1.0f;
    }
    
    
    for (int i = 0; i < 10; i++) {
        letterOffsets[i] = sinf(titleAnimTime * 3 + i * 0.5f) * 5.0f;
    }
    
   
    updateParticles();
    updateStars();
    
    
    updateTitleScreenAudio();
    
    
   if (titleAnimTime > 2.0f && (AnyKeyPressed() || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)))
 {
        PlaySound(clickSound);
        titleScreenDone = true;
        startTransition(START);
    }
    
    
    updateTransition();
}


void drawStars() {
    for (int i = 0; i < MAX_STARS; i++) {
        DrawCircle(
            stars[i].position.x,
            stars[i].position.y,
            stars[i].radius,
            Fade(WHITE, stars[i].alpha * titleAlpha)
        );
    }
}


void drawParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            DrawCircle(
                particles[i].position.x,
                particles[i].position.y,
                particles[i].radius,
                Fade(particles[i].color, particles[i].alpha * titleAlpha)
            );
        }
    }
}


void drawTransition() {
    if (transitionAlpha > 0) {
        DrawRectangle(0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE, 
                      Fade(BLACK, transitionAlpha));
    }
}


void drawAnimatedSnakeTitle() {
    const char* titlePart1 = "SNAKE";
    const char* titlePart2 = "GAME";
    
    int fontSize = 60;
    
    
    float letterSpacing = 10.0f; 
    float wordSpacing = 30.0f;   
    
    
    float totalWidth = 0;
    
    
    for (int i = 0; i < strlen(titlePart1); i++) {
        char letterStr[2] = {titlePart1[i], '\0'};
        totalWidth += MeasureText(letterStr, fontSize);
        if (i < strlen(titlePart1) - 1) totalWidth += letterSpacing;
    }
    
    
    totalWidth += wordSpacing;
    

    for (int i = 0; i < strlen(titlePart2); i++) {
        char letterStr[2] = {titlePart2[i], '\0'};
        totalWidth += MeasureText(letterStr, fontSize);
        if (i < strlen(titlePart2) - 1) totalWidth += letterSpacing;
    }
    
    
    int startX = (WIDTH * CELL_SIZE - totalWidth) / 2;
    int y = HEIGHT * CELL_SIZE * 0.3;
    
    
    for (int i = 0; i < strlen(titlePart1) + strlen(titlePart2); i++) {
        letterOffsets[i] = sinf(titleAnimTime * 3 + i * 0.5f) * 15.0f;
    }
    
    
    Color snakeColor = (Color){30, 144, 255, (unsigned char)(titleAlpha * 255)}; 
    
    for (int i = 0; i < strlen(titlePart1); i++) {
        char letterStr[2] = {titlePart1[i], '\0'};
        int letterWidth = MeasureText(letterStr, fontSize);
        
        
        DrawText(letterStr, 
                startX + 2, 
                y + (int)letterOffsets[i] + 2, 
                fontSize, 
                (Color){0, 0, 0, 128}); 
                

        DrawText(letterStr, 
                startX, 
                y + (int)letterOffsets[i], 
                fontSize, 
                snakeColor);
        
        startX += letterWidth + (i < strlen(titlePart1) - 1 ? letterSpacing : 0);
    }
    
    
    startX += wordSpacing;
    
    
    for (int i = 0; i < strlen(titlePart2); i++) {
        char letterStr[2] = {titlePart2[i], '\0'};
        int letterWidth = MeasureText(letterStr, fontSize);
        
        
        unsigned char r = 255;
        unsigned char g = 150 - (i * 30); 
        unsigned char b = 0;
        
        Color letterColor = (Color){r, g, b, (unsigned char)(titleAlpha * 255)};
        
        
        DrawText(letterStr, 
                startX + 2, 
                y + (int)letterOffsets[i + strlen(titlePart1)] + 2, 
                fontSize, 
                (Color){0, 0, 0, 128}); 
        
        
        DrawText(letterStr, 
                startX, 
                y + (int)letterOffsets[i + strlen(titlePart1)], 
                fontSize, 
                letterColor);
        
        startX += letterWidth + (i < strlen(titlePart2) - 1 ? letterSpacing : 0);
    }
}


void drawAnimatedSnake() {
    if (titleAnimTime > 1.0f) {
        
        float snakeX = WIDTH * CELL_SIZE / 2;
        float snakeY = HEIGHT * CELL_SIZE * 0.55f;  
        
        
        const int segmentCount = 12;
        

        float waveAmplitude = 15.0f;  
        float waveFrequency = 2.5f;   
        float waveSpeed = 4.0f;       
        

        float headSize = 24.0f;
        Color headColor = Fade(LIME, titleAlpha);
        DrawCircle(snakeX, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude, headSize, headColor);
        
        
        if (fmodf(titleAnimTime, 1.5f) < 0.3f) {
         
            DrawLineEx(
                (Vector2){ snakeX + headSize - 2, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude },
                (Vector2){ snakeX + headSize + 20, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude - 8 },
                3,
                Fade(RED, titleAlpha)
            );
            DrawLineEx(
                (Vector2){ snakeX + headSize - 2, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude },
                (Vector2){ snakeX + headSize + 20, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude + 8 },
                3,
                Fade(RED, titleAlpha)
            );
        }
        
        
        for (int i = 0; i < segmentCount; i++) {
            
            float segmentOffset = sinf(titleAnimTime * waveSpeed - i * 0.3f) * waveAmplitude;
            float segmentX = snakeX - i * 18;  
            float segmentY = snakeY + segmentOffset;
            
            
            float segmentSize = headSize * (1.0f - (float)i / (segmentCount + 2));
            
            
            Color segmentColor;
            if (i % 2 == 0) {
                segmentColor = Fade(LIME, titleAlpha);
            } else {
                segmentColor = Fade(DARKGREEN, titleAlpha);
            }
            
            
            float pulseEffect = 1.0f + sinf(titleAnimTime * 2 + i * 0.2f) * 0.1f;
            
            DrawCircle(segmentX, segmentY, segmentSize * pulseEffect, segmentColor);
            
            
            if (i % 2 == 0) {
                DrawCircle(segmentX, segmentY, segmentSize * 0.6f * pulseEffect, Fade(GREEN, titleAlpha * 0.8f));
            }
        }
        
        
        float eyeSize = headSize * 0.25f;
        DrawCircle(snakeX + headSize * 0.5f, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude - headSize * 0.3f, eyeSize, Fade(WHITE, titleAlpha));
        DrawCircle(snakeX + headSize * 0.5f, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude + headSize * 0.3f, eyeSize, Fade(WHITE, titleAlpha));
        
       
        float pupilOffset = sinf(titleAnimTime * 1.5f) * eyeSize * 0.3f;
        DrawCircle(snakeX + headSize * 0.5f + eyeSize * 0.2f, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude - headSize * 0.3f + pupilOffset, eyeSize * 0.6f, Fade(BLACK, titleAlpha));
        DrawCircle(snakeX + headSize * 0.5f + eyeSize * 0.2f, snakeY + sinf(titleAnimTime * waveSpeed) * waveAmplitude + headSize * 0.3f + pupilOffset, eyeSize * 0.6f, Fade(BLACK, titleAlpha));
    }
}


void drawPressAnyKey() {
    if (titleAnimTime > 2.0f) {
        
        float pulseScale = 1.0f + sinf(titleAnimTime * 2.0f) * 0.05f;
        
        
        float baseX = WIDTH * CELL_SIZE / 2 - 160 * pulseScale; 
        float baseY = HEIGHT * CELL_SIZE * 0.76f;  
        int fontSize = 28;
        
        
        float alpha = 0.8f + sinf(titleAnimTime * 1.5f) * 0.2f;
        Color brownColor = Fade(BROWN, alpha * titleAlpha);
        
        
        DrawTextPro(
            GetFontDefault(),
            "Press any key to continue",
            (Vector2){ baseX, baseY },
            (Vector2){ 0, 0 },  
            0.0f,  
            fontSize * pulseScale,
            1.0f,
            brownColor
        );
    }
}


void drawCreatorInfo() {
    if (titleAnimTime > 1.5f) {
        DrawText(
            "Created by: \nSivasarathy A\nSudharsan B\nSrikanth C",
            WIDTH * CELL_SIZE - 250,
            HEIGHT * CELL_SIZE - 90,
            16,
            Fade(LIGHTGRAY, titleAlpha)
        );
    }
}


void drawTitleScreen() {
   
    if (titleBackgroundTexture.id > 0) {
        DrawTexturePro(
            titleBackgroundTexture,
            (Rectangle){0, 0, titleBackgroundTexture.width, titleBackgroundTexture.height},
            (Rectangle){0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE},
            (Vector2){0, 0},
            0,
            Fade(WHITE, titleAlpha)
        );
    } else {
        
        DrawRectangleGradientV(
            0, 0, 
            WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE, 
            Fade(BLACK, titleAlpha), 
            Fade(DARKBLUE, titleAlpha)
        );
    }
    
    
    drawStars();
    
  
    drawParticles();
    
    
    drawAnimatedSnakeTitle();
    
  
    drawAnimatedSnake();
    
  
    drawPressAnyKey();
    
   
    drawCreatorInfo();
    

    drawTransition();
}

void cleanupTitleScreen() {
    if (titleBackgroundTexture.id > 0) {
        UnloadTexture(titleBackgroundTexture);
    }
    
    UnloadMusicStream(titleScreenMusic);
}

void InsertHighScore(int newScore) {
    for (int i = 0; i < MAX_SCORES; i++) {
        if (newScore > highScores[i]) {
            for (int j = MAX_SCORES - 1; j > i; j--) {
                highScores[j] = highScores[j - 1];
            }
            highScores[i] = newScore;
            break;
        }
    }
}
void drawHintScreen() {
    DrawTexturePro(hintBackgroundTexture, (Rectangle){0, 0, hintBackgroundTexture.width, hintBackgroundTexture.height},
                   (Rectangle){0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE}, (Vector2){0}, 0, WHITE);

    float time = GetTime();
    int pulse = (int)(127 + 128 * sinf(time * 2));  
    Color animatedColor = (Color){255, pulse / 2 + 70, 0, 255};  

 
    for (int i = 5; i > 0; i--) {
        DrawText("HINTS BEFORE YOU PLAY!", WIDTH * CELL_SIZE / 2 - 200 + i, 30 + i, 36, Fade(BLACK, 0.2f * i));
    }
    DrawText("HINTS BEFORE YOU PLAY!", WIDTH * CELL_SIZE / 2 - 200, 30, 36, animatedColor);

  
    for (int i = 0; i < 5; i++) {
        DrawText(hints[i], WIDTH * CELL_SIZE / 2 - 220, 100 + i * 40, 22, (Color){183, 65, 14, 255});
    }

    int remaining = 5 - (int)(GetTime() - hintStartTime);
    if (remaining < 0) remaining = 0;

    
    DrawText(TextFormat("Starting in: %d", remaining), WIDTH * CELL_SIZE / 2 - 80, 330, 30, (Color){255, 140, 0, 255});

    if (remaining <= 0) {
        showHintScreen = false;
        setup();
       
        currentState = GAMEPLAY;
    }
}

int CheckCollisionPointEllipse(Vector2 point, Vector2 center, float radiusX, float radiusY) {
    float dx = point.x - center.x;
    float dy = point.y - center.y;
    return ((dx * dx) / (radiusX * radiusX) + (dy * dy) / (radiusY * radiusY)) <= 1.0f;
}

int isOnSnake(int x_, int y_) {
    if (x_ == x && y_ == y) return 1;
    for (int i = 0; i < tailLength; i++) {
        if (tailX[i] == x_ && tailY[i] == y_) return 1;
    }
    return 0;
}

void placeFruit() {
    do {
        fruitX = rand() % (WIDTH - 2) + 1;
        fruitY = rand() % (HEIGHT - 2) + 1;
    } while (isOnSnake(fruitX, fruitY));
}

void placeBombs() {
    for (int i = 0; i < 2; i++) {
        int overlap;
        do {
            overlap = 0;
            bombX[i] = rand() % (WIDTH - 5) + 1;
            bombY[i] = rand() % (HEIGHT - 2) + 1;
            for (int j = 0; j < i; j++) {
                if (bombY[i] == bombY[j] &&
                    ((bombX[i] >= bombX[j] && bombX[i] < bombX[j] + 4) ||
                     (bombX[j] >= bombX[i] && bombX[j] < bombX[i] + 4))) {
                    overlap = 1;
                    break;
                }
            }
        } while (isOnSnake(bombX[i], bombY[i]) ||
                 (bombX[i] <= fruitX && fruitX < bombX[i] + 4 && bombY[i] == fruitY) ||
                 overlap);
    }
    lastBombMove = clock();
}

void setup() {
    gameOver = 0;
    direction = 'd';
    x = WIDTH / 2;
    y = HEIGHT / 2;
    score = 0;
    tailLength = 0;
    placeFruit();
    placeBombs();
    SetTargetFPS(60);

    switch (currentDifficulty) {
        case EASY: snakeSpeedDelay = 0.20f; break;
        case MEDIUM: snakeSpeedDelay = 0.12f; break;
        case HARD: snakeSpeedDelay = 0.07f; break;
    }

    lastSnakeMoveTime = GetTime();
}

void drawPauseButton() {
    DrawRectangle(WIDTH * CELL_SIZE - 30, 10, 20, 20, LIGHTGRAY);
    DrawRectangle(WIDTH * CELL_SIZE - 26, 14, 4, 12, GRAY);
    DrawRectangle(WIDTH * CELL_SIZE - 20, 14, 4, 12, GRAY);
}

void drawGradientText(const char* text, int posX, int posY, int fontSize) {
    
    float time = GetTime();
    
   
    for (int i = 0; i < strlen(text); i++) {
        float offsetY = sinf(time * 3 + i * 0.5f) * 3.0f;
        
        
        DrawText(TextFormat("%c", text[i]), posX + i * fontSize/1.5, posY + 3 + offsetY, fontSize, (Color){20, 20, 20, 180});
        
       
        int hue = (int)(time * 40 + i * 20) % 100;
        Color charColor = (Color){20 + hue, 200 - hue/2, 20 + hue/3, 255};
        DrawText(TextFormat("%c", text[i]), posX + i * fontSize/1.5, posY + offsetY, fontSize, charColor);
    }
    
    
    if (fmodf(time, 1.0f) > 0.5f) {
        int tongueX = posX + strlen(text) * fontSize/1.5;
        DrawText("~", tongueX, posY - 5, fontSize, RED);
    }
}

void drawGame() {
    Texture2D currentBG;

    switch (currentDifficulty) {
        case EASY: currentBG = bgEasy; break;
        case MEDIUM: currentBG = bgMedium; break;
        case HARD: currentBG = bgHard; break;
    }

    DrawTexturePro(currentBG, (Rectangle){0, 0, currentBG.width, currentBG.height},
                   (Rectangle){0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE},
                   (Vector2){0}, 0, WHITE);

    DrawRectangle(0, 0, WIDTH * CELL_SIZE, CELL_SIZE, BROWN);
    DrawRectangle(0, (HEIGHT - 1) * CELL_SIZE, WIDTH * CELL_SIZE, CELL_SIZE, BROWN);
    DrawRectangle(0, 0, CELL_SIZE, HEIGHT * CELL_SIZE, BROWN);
    DrawRectangle((WIDTH - 1) * CELL_SIZE, 0, CELL_SIZE, HEIGHT * CELL_SIZE, BROWN);

    for (int i = 0; i < tailLength; i++) {
        Color segmentColor = (i % 2 == 0) ? GREEN : DARKGREEN;
        DrawRectangleRounded((Rectangle){tailX[i] * CELL_SIZE, tailY[i] * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.5f, 8, segmentColor);
    }

    DrawRectangleRounded((Rectangle){x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE}, 0.5f, 8, YELLOW);
    DrawCircle(x * CELL_SIZE + 5, y * CELL_SIZE + 5, 2, BLACK);
    DrawCircle(x * CELL_SIZE + 15, y * CELL_SIZE + 5, 2, BLACK);

    DrawRectangle(fruitX * CELL_SIZE, fruitY * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);

    for (int b = 0; b < 2; b++) {
        float scale = 1.5f;
        float targetSize = CELL_SIZE * scale;
        float offset = (CELL_SIZE - targetSize) / 2.0f;
        DrawTexturePro(skullTexture, (Rectangle){0, 0, skullTexture.width, skullTexture.height},
            (Rectangle){bombX[b] * CELL_SIZE + offset, bombY[b] * CELL_SIZE + offset, targetSize, targetSize},
            (Vector2){0}, 0, WHITE);
    }

    DrawRectangleRounded((Rectangle){10, HEIGHT * CELL_SIZE - 40, 160, 30}, 0.3f, 10, Fade(SKYBLUE, 0.8f));
    DrawText(TextFormat("Score: %d", score), 20, HEIGHT * CELL_SIZE - 35, 20, BLACK);

    if (isPaused) DrawText("PAUSED", WIDTH * CELL_SIZE / 2 - 50, HEIGHT * CELL_SIZE / 2, 30, RED);
    drawPauseButton();
}

void handleInput() {
    Vector2 mouse = GetMousePosition();
    Rectangle pauseButton = {WIDTH * CELL_SIZE - 30, 10, 20, 20};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, pauseButton)) {
        isPaused = !isPaused;
        PlaySound(clickSound);
    } else if (!isPaused) {
        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
            PlaySound(clickSound);
        }
        if (IsKeyPressed(KEY_W) && direction != 's') direction = 'w';
        if (IsKeyPressed(KEY_S) && direction != 'w') direction = 's';
        if (IsKeyPressed(KEY_A) && direction != 'd') direction = 'a';
        if (IsKeyPressed(KEY_D) && direction != 'a') direction = 'd';
    }
}

void logic() {
    if (isPaused) return;

    if (((clock() - lastBombMove) / CLOCKS_PER_SEC) >= 5) placeBombs();

    double currentTime = GetTime();
    if ((currentTime - lastSnakeMoveTime) < snakeSpeedDelay) return;
    lastSnakeMoveTime = currentTime;

    int prevX = tailX[0], prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x; tailY[0] = y;

    for (int i = 1; i < tailLength; i++) {
        prev2X = tailX[i]; prev2Y = tailY[i];
        tailX[i] = prevX; tailY[i] = prevY;
        prevX = prev2X; prevY = prev2Y;
    }

    switch (direction) {
        case 'w': y--; break;
        case 's': y++; break;
        case 'a': x--; break;
        case 'd': x++; break;
    }

    if (x == fruitX && y == fruitY) {
        score += 10; tailLength++;
        placeFruit(); PlaySound(eatSound);
    }

    if (x <= 0 || x >= WIDTH - 1 || y <= 0 || y >= HEIGHT - 1) gameOver = 1;
    for (int i = 0; i < tailLength; i++) if (tailX[i] == x && tailY[i] == y) gameOver = 1;
    for (int b = 0; b < 2; b++) if (x == bombX[b] && y == bombY[b]) gameOver = 1;

    if (gameOver) {
        PlaySound(gameOverSound);
        InsertHighScore(score);
    }
}

void drawButton(Rectangle rect, const char* label, Color base, Color hover) {
    Vector2 mouse = GetMousePosition();
    bool hovered = CheckCollisionPointRec(mouse, rect);
    
    float shadowOffset = hovered ? 4 : 2;
    float scale = hovered ? 1.05f : 1.0f;
    Rectangle scaledRect = {
        rect.x - (rect.width * (scale - 1) / 2),
        rect.y - (rect.height * (scale - 1) / 2),
        rect.width * scale,
        rect.height * scale
    };

    DrawRectangleRounded((Rectangle){scaledRect.x + shadowOffset, scaledRect.y + shadowOffset, scaledRect.width, scaledRect.height}, 0.3f, 10, DARKGRAY);
    DrawRectangleRounded(scaledRect, 0.3f, 10, hovered ? hover : base);
    
    int textWidth = MeasureText(label, 20);
    DrawText(label, scaledRect.x + (scaledRect.width - textWidth)/2, scaledRect.y + 8, 20, WHITE);
}


void drawStartScreen() {
    DrawTexturePro(startBackgroundTexture, (Rectangle){0, 0, startBackgroundTexture.width, startBackgroundTexture.height},
                   (Rectangle){0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE}, (Vector2){0}, 0, WHITE);

   
    DrawRectangleRounded((Rectangle){WIDTH * CELL_SIZE / 2 - 180, 40, 360, 90}, 0.3f, 10, Fade(DARKGREEN, 0.7f));
    DrawRectangleRoundedLines((Rectangle){WIDTH * CELL_SIZE / 2 - 180, 40, 360, 90}, 0.3f, 10, GREEN);
    
    drawGradientText("SNAKE GAME", WIDTH * CELL_SIZE / 2 - 150, 65, 40);
    
   
    float time = GetTime();
    for (int i = 0; i < 5; i++) {
        float offset = sinf(time * 2 + i * 0.4f) * 3;
        DrawCircle(WIDTH * CELL_SIZE / 2 + 160, 85 + offset, 8 - i, GREEN);
    }
    
    if (!difficultySelected) {
       
        DrawText("Select Difficulty:", WIDTH * CELL_SIZE / 2 - 90, 150, 20, BLACK);

        const char* labels[] = {"EASY", "MEDIUM", "HARD"};
        for (int i = 0; i < 3; i++) {
            Rectangle btn = {WIDTH * CELL_SIZE / 2 - 60, 190 + i * 50, 120, 35};
            drawButton(btn, labels[i], MAROON, DARKBLUE);

            if (CheckCollisionPointRec(GetMousePosition(), btn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                currentDifficulty = (i == 0) ? EASY : (i == 1) ? MEDIUM : HARD;
                PlaySound(clickSound);
                difficultySelected = true;  
            }
        }
    } else {
        
        const char* difficultyText;
        switch (currentDifficulty) {
            case EASY: difficultyText = "EASY"; break;
            case MEDIUM: difficultyText = "MEDIUM"; break;
            case HARD: difficultyText = "HARD"; break;
            default: difficultyText = "UNKNOWN";
        }
        
        
        int boxWidth = 240;
        int boxHeight = 50;
        int boxX = WIDTH * CELL_SIZE / 2 - boxWidth/2;
        int boxY = 160;
        
        DrawRectangleRounded((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.3f, 10, Fade(SKYBLUE, 0.7f));
        DrawRectangleRoundedLines((Rectangle){boxX, boxY, boxWidth, boxHeight}, 0.3f, 10, BLUE);
        
       
        int titleWidth = MeasureText("Selected Difficulty", 20);
        DrawText("Selected Difficulty", boxX + (boxWidth - titleWidth)/2, boxY + 5, 20, BLACK);
        
        
        int diffTextWidth = MeasureText(difficultyText, 28);
        DrawText(difficultyText, boxX + (boxWidth - diffTextWidth)/2, boxY + 25, 28, DARKBLUE);
        
        Rectangle startBtn = {WIDTH * CELL_SIZE / 2 - 90, 220, 180, 50};
        drawButton(startBtn, "START GAME", DARKGREEN, GREEN);
        
       
        Rectangle backBtn = {WIDTH * CELL_SIZE / 2 - 120, 290, 240, 35};
        drawButton(backBtn, "CHANGE DIFFICULTY", MAROON, RED);
        
       
        Vector2 mouse = GetMousePosition();
       if (CheckCollisionPointRec(mouse, startBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    PlaySound(clickSound);
    showStart = false;
    showHintScreen = true;
    hintStartTime = GetTime();
    currentState = HINT; 
}

        
        if (CheckCollisionPointRec(mouse, backBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            PlaySound(clickSound);
            difficultySelected = false;  
        }
    }
}

void drawGameOverScreen() {
    DrawTexturePro(gameOverBackgroundTexture, (Rectangle){0, 0, gameOverBackgroundTexture.width, gameOverBackgroundTexture.height},
                   (Rectangle){0, 0, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE}, (Vector2){0}, 0, WHITE);

    drawGradientText("GAME OVER", WIDTH * CELL_SIZE / 2 - 120, 80, 40);

    Rectangle restartBtn = {WIDTH * CELL_SIZE / 2 - 70, 170, 140, 40};
    Rectangle quitBtn = {WIDTH * CELL_SIZE / 2 - 70, 230, 140, 40};
    drawButton(restartBtn, "RESTART", DARKPURPLE, PURPLE);
    drawButton(quitBtn, "QUIT", DARKBLUE, SKYBLUE);

    int boxX = WIDTH * CELL_SIZE / 2 - 110;
    int boxY = 290;
    DrawRectangleRounded((Rectangle){boxX, boxY, 220, 150}, 0.3f, 10, Fade(LIGHTGRAY, 0.8f));
    DrawRectangleRoundedLines((Rectangle){boxX, boxY, 220, 150}, 0.3f, 10, DARKGRAY);

    DrawText("TOP 5 SCORES:", boxX + 40, boxY + 10, 20, BLACK);
    for (int i = 0; i < MAX_SCORES; i++) {
        DrawText(TextFormat("%d. %d", i + 1, highScores[i]), boxX + 50, boxY + 40 + i * 20, 18, DARKGRAY);
    }

    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, restartBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    PlaySound(clickSound);
    showStart = true;
    difficultySelected = false;
    gameOver = 0;
    currentState = START;  
    setup();  
}

    if (CheckCollisionPointRec(mouse, quitBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        PlaySound(clickSound);
        CloseWindow();
        exit(0);
    }
}



int main(void) {
    
    InitWindow(WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE, "Snake Game");
    InitAudioDevice();

    
    initTitleScreen();

    eatSound = LoadSound("C:\\Users\\sudha\\Downloads\\food_G1U6tlb.mp3");
    gameOverSound = LoadSound("C:\\Users\\sudha\\Downloads\\negative_beeps-6008.mp3");
    clickSound = LoadSound("C:\\Users\\sudha\\Downloads\\mouse-click-290204.mp3");
    skullTexture = LoadTexture("C:\\Users\\sudha\\Downloads\\image-from-rawpixel-id-6287121-png.png");

    startBackgroundTexture = LoadTexture("C:\\Users\\sudha\\Downloads\\wp4578365.png");
    gameOverBackgroundTexture = LoadTexture("C:\\Users\\sudha\\Downloads\\pngtree-game-sunset-yellow-image_2409230.jpg");
    hintBackgroundTexture = LoadTexture("C:\\Users\\sudha\\Downloads\\wp2409705.jpg");
    bgEasy = LoadTexture("C:\\Users\\sudha\\Downloads\\background-of-green-grass-field-cartoon-drawing-free-vector.jpg");
    bgMedium = LoadTexture("C:\\Users\\sudha\\Downloads\\360_F_239608136_4qtLPCPhCCzNaSoPo2hiEfMFA1fBXa2R.jpg");
    bgHard = LoadTexture("C:\\Users\\sudha\\Downloads\\pngtree-close-up-view-of-sand-texture-in-top-down-background-image_13639125.png");
    bgMusic = LoadMusicStream("C:\\Users\\sudha\\Downloads\\Run-Amok(chosic.com).ogg");
    PlayMusicStream(bgMusic);
    SetMusicVolume(bgMusic, 0.4f);

    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_SCORES; i++) highScores[i] = 0;

    while (!WindowShouldClose()) {
    
    UpdateMusicStream(bgMusic);

    BeginDrawing();
    ClearBackground(BLACK);

    switch(currentState) {
        case TITLE:
            updateTitleScreen();
            drawTitleScreen();
            break;
        case START:
            drawStartScreen();
            break;
        case HINT:
            drawHintScreen();
            break;
        case GAMEPLAY:
            if (!gameOver) {
                handleInput();
                logic();
                drawGame();
            } else {
                currentState = GAMEOVER;
            }
            break;
        case GAMEOVER:
            drawGameOverScreen();
            break;
    }

    EndDrawing();
}


   
    UnloadSound(eatSound);
    UnloadSound(gameOverSound);
    UnloadSound(clickSound);
    UnloadMusicStream(bgMusic);
    UnloadTexture(skullTexture);
    UnloadTexture(startBackgroundTexture);
    UnloadTexture(gameOverBackgroundTexture);
    UnloadTexture(hintBackgroundTexture);
    UnloadTexture(bgEasy);
    UnloadTexture(bgMedium);
    UnloadTexture(bgHard);
    cleanupTitleScreen();

    CloseAudioDevice();
    CloseWindow();
    return 0;
}