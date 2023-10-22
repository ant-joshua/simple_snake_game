#include <iostream>
#include <utility>
#include "raylib.h"
#include "deque"
#include "raymath.h"
#include "rlgl.h"

using namespace std;

Color green = GREEN;
Color darkGreen = DARKGREEN;

float cellSize = 30;
float cellCount = 25;
int offset = 75;

double lastUpdateTime = 0;

bool ElementInDeque(deque<Vector2> deque, Vector2 element) {
    for (auto &i: deque) {
        if (Vector2Equals(i, element)) {
            return true;
        }
    }
    return false;
}

bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

class Snake {
public:
    deque<Vector2> body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
    Vector2 direction = {1, 0};
    Texture2D headTop = {};
    Texture2D headBottom = {};
    Texture2D headLeft = {};
    Texture2D headRight = {};
    Texture2D headTexture = {};
    bool addSegment = false;

    Snake() {
        Image headTopImage = LoadImage("../Graphics/snake_small.png");
        headBottom = LoadTextureFromImage(headTopImage);

        ImageRotateCW(&headTopImage);

        headLeft = LoadTextureFromImage(headTopImage);


        ImageRotateCW(&headTopImage);

        headTop = LoadTextureFromImage(headTopImage);

        ImageRotateCW(&headTopImage);

        headRight = LoadTextureFromImage(headTopImage);


        headTexture = headRight;


        UnloadImage(headTopImage);
    }

    ~Snake() {
        UnloadTexture(headTop);
        UnloadTexture(headBottom);
        UnloadTexture(headLeft);
        UnloadTexture(headRight);
    }

    void Draw() {
        for (auto &i: body) {
            float x = i.x;
            float y = i.y;

            auto segment = Rectangle{(float) offset + x * cellSize, (float) offset + y * cellSize, cellSize, cellSize};


//            headAngle = atan2(direction.y, direction.x) * RAD2DEG + 270;
//            headAngle = fmod(headAngle, 360.0f);
//
//            auto headPosition = Vector2{x * cellSize, y * cellSize};

            if (Vector2Equals(i, body[0])) {
                DrawTexture(headTexture,(float) offset +  x * cellSize, (float)offset + y * cellSize, darkGreen);
//                DrawTextureEx(headTexture, headPosition, headAngle,1, darkGreen);
            } else {
                DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            }

        }


    }

    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment) {
            addSegment = false;
        } else {
            body.pop_back();
        }
    }

    void Reset() {
        body = {Vector2{6, 9}, Vector2{5, 9}, Vector2{4, 9}};
        direction = {1, 0};
        headTexture = headRight;
    }
};

class Food {
public:
    Vector2 position = {5, 6};
    Texture2D texture{};

    explicit Food(deque<Vector2> snakeBody) {
        Image image = LoadImage("../Graphics/food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(std::move(snakeBody));
    }

    ~Food() {
        UnloadTexture(texture);
    }

    Vector2 GenerateRandomCell() {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);

        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {

        position = GenerateRandomCell();
        while (ElementInDeque(snakeBody, position)) {
            position = GenerateRandomCell();
        }

        return position;
    }

    void Draw() const {
//        DrawRectangle(position.x * cellSize, position.y * cellSize, cellSize, cellSize, darkGreen);
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }
};

class Game {
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    int score = 0;
    bool is_running = true;
    Sound eatSound{};
    Sound wallSound{};

    Game(){
        InitAudioDevice();

        eatSound = LoadSound("../Sounds/coin.wav");
        wallSound = LoadSound("../Sounds/wall.mp3");
    }

    ~Game(){
        UnloadSound(eatSound);
        UnloadSound(wallSound);
        CloseAudioDevice();
    }

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        if (!is_running) {
            return;
        }
        snake.Update();
        CheckCollisionWithFood();
        CheckCollisionWithEdge();
        CheckCollisionWithTail();
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {

            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;


            score++;

            PlaySound(eatSound);

            cout << "Score: " << score << "\n";
        }
    }

    void CheckCollisionWithEdge() {
        if ((snake.body.front().x < 0 || snake.body.front().x > cellCount - 1) ||
            (snake.body.front().y < 0 || snake.body.front().y > cellCount - 1)) {
            GameOver();
        }
    }

    void GameOver() {
        snake.Reset();
        score = 0;
        food.position = food.GenerateRandomPos(snake.body);
        is_running = false;
        PlaySound(wallSound);
    }

    void CheckCollisionWithTail() {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();

        if (ElementInDeque(headlessBody, snake.body.front())) {
            GameOver();
        }
    }


};

int main() {
    cout << "Hello World!" << endl;

    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Hello World");
    SetTargetFPS(60);

    Game game = Game();

    cout << GetWorkingDirectory();

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (eventTriggered(0.2)) {
            game.Update();
        }

        if(IsKeyPressed(KEY_SPACE)) PlaySound(game.eatSound);

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
            game.snake.headTexture = game.snake.headTop;
            game.is_running = true;
            PlaySound(game.eatSound);
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
            game.snake.headTexture = game.snake.headBottom;
            game.is_running = true;
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
            game.snake.headTexture = game.snake.headLeft;
            game.is_running = true;
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
            game.snake.headTexture = game.snake.headRight;
            game.is_running = true;
        }

        // Drawing
        ClearBackground(green);

        DrawRectangleLinesEx(Rectangle{(float) offset - 5, (float) offset - 5, (float) cellSize * cellCount + 10,
                                       (float) cellSize * cellCount + 10}, 5, darkGreen);

        DrawText("Retro Snake", offset - 5, 20, 40 , darkGreen);
        DrawText(TextFormat("%i",game.score) , offset -5, offset + cellSize * cellCount + 10, 40, darkGreen);

        game.Draw();


        EndDrawing();
    }

    CloseWindow();
    return 0;
}

