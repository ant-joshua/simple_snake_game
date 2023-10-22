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
    Texture2D headTexture = {};
    float headAngle = -90;
    bool addSegment = false;

    Snake() {
        Image image = LoadImage("../Graphics/snake_small.png");
        headTexture = LoadTextureFromImage(image);
        UnloadImage(image);

    }

    ~Snake() {
        UnloadTexture(headTexture);
    }

    void Draw() {
        for (auto &i: body) {
            float x = i.x;
            float y = i.y;

            auto segment = Rectangle{(float) x * cellSize, (float) y * cellSize, cellSize, cellSize};


            headAngle = atan2(direction.y, direction.x) * RAD2DEG + 270;
            headAngle = fmod(headAngle, 360.0f);

            auto headPosition = Vector2{x * cellSize, y * cellSize};

            if (Vector2Equals(i, body[0])) {
                DrawTextureEx(headTexture, headPosition, headAngle,1, darkGreen);
            } else {
                DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            }

        }


    }

    void Update() {
        body.push_front(Vector2Add(body[0], direction));
        if(addSegment){

            addSegment = false;
        }else{
            body.pop_back();
        }



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
        DrawTexture(texture, position.x * cellSize, position.y * cellSize, WHITE);
    }
};

class Game {
public:
    Snake snake = Snake();
    Food food = Food(snake.body);
    int score = 0;
    char buffer[100];

    void Draw() {
        food.Draw();
        snake.Draw();
    }

    void Update() {
        snake.Update();
        CheckCollisionWithFood();
    }

    void CheckCollisionWithFood() {
        if (Vector2Equals(snake.body[0], food.position)) {

            food.position = food.GenerateRandomPos(snake.body);
            snake.addSegment = true;


            score++;

            cout << "Score: " << score << "\n";
        }
    }


};

int main() {
    cout << "Hello World!" << endl;

    InitWindow(cellSize * cellCount, cellSize * cellCount, "Hello World");
    SetTargetFPS(60);


    Game game = Game();

    cout << GetWorkingDirectory();

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (eventTriggered(0.2)) {
            game.Update();
        }

        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = {0, -1};
        }
        if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = {0, 1};
        }
        if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = {-1, 0};
        }
        if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = {1, 0};
        }

        // Drawing
        ClearBackground(green);
        game.Draw();


        EndDrawing();
    }

    CloseWindow();
    return 0;
}

