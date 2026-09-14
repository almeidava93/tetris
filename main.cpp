#include "raylib.h"
#include <vector>

const int screenWidth = 800;
const int screenHeight = 450;
const int targetFPS = 60;
const char *windowTitle = "Tetris";

class Position
{
public:
    float x;
    float y;
};

class Size
{
public:
    float width;
    float height;
};

class Shape
{
public:
    int rows;
    int cols;
};

typedef enum BlockType
{
    I,
    J,
    L,
    O,
    S,
    T,
    Z
} BlockType;

typedef enum BlockOrientation
{
    UP,
    RIGHT,
    DOWN,
    LEFT
} BlockOrientation;

class Block
{
public:
    BlockType type;
    BlockOrientation orientation;
    Color color = GRAY;
    Position position;
    Size size;                             // width and height of each square in the block
    Shape matrixShape;                     //  dimensions of the block's matrix representation
    std::vector<std::vector<bool>> matrix; // 2D vector to represent the block's shape
    Texture2D *texture;                    // Pointer to the texture for the block

    Block(BlockType type = J, BlockOrientation orientation = UP, Color color = GRAY, Position position = {0, 0}, Size size = {20, 20})
    {
        this->type = type;
        this->orientation = orientation;
        this->color = color;
        this->position = position;
        this->size = size;
        initBlockMatrix();
    }

    void initBlockMatrix() // Initialize the block's matrix representation based on its type and the UP orientation
    {
        switch (this->type)
        {
        case I:
            this->matrixShape.cols = 4;
            this->matrixShape.rows = 1;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            break;
        case J:
            this->matrixShape.cols = 2;
            this->matrixShape.rows = 3;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            this->matrix[0][0] = false;
            this->matrix[1][0] = false;
            break;
        case L:
            this->matrixShape.cols = 2;
            this->matrixShape.rows = 3;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            this->matrix[0][1] = false;
            this->matrix[1][1] = false;
            break;
        case O:
            this->matrixShape.cols = 2;
            this->matrixShape.rows = 2;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            break;
        case Z:
            this->matrixShape.cols = 3;
            this->matrixShape.rows = 2;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            this->matrix[0][2] = false;
            this->matrix[1][0] = false;
            break;
        case S:
            this->matrixShape.cols = 3;
            this->matrixShape.rows = 2;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            this->matrix[0][0] = false;
            this->matrix[1][2] = false;
            break;
        case T:
            this->matrixShape.cols = 3;
            this->matrixShape.rows = 2;
            this->matrix = std::vector<std::vector<bool>>(this->matrixShape.rows, std::vector<bool>(this->matrixShape.cols, true));
            this->matrix[1][0] = false;
            this->matrix[1][2] = false;
            break;
        }
    }

    void rotate()
    { // Rotate the block 90 degrees clockwise
        Shape newMatrixShape = {this->matrixShape.cols, this->matrixShape.rows};
        std::vector<std::vector<bool>> newMatrix(newMatrixShape.rows, std::vector<bool>(newMatrixShape.cols, false)); // Create a new matrix with swapped dimensions

        // iterate through the original matrix and fill the new matrix with rotated values
        for (int row = 0; row < this->matrixShape.rows; row++)
        {
            for (int col = 0; col < this->matrixShape.cols; col++)
            {
                newMatrix[col][this->matrixShape.rows - 1 - row] = this->matrix[row][col];
            }
        }
        this->matrix = newMatrix;
        this->matrixShape = newMatrixShape;
    }

    void draw()
    {
        for (int row = 0; row < this->matrixShape.rows; row++)
        {
            for (int col = 0; col < this->matrixShape.cols; col++)
            {
                if (this->matrix[row][col])
                {
                    if (this->texture)
                    {
                        DrawTexture(*this->texture, this->position.x + (col * this->size.width), this->position.y + (row * this->size.height), BLUE);
                    }
                    else
                    {
                        DrawRectangle(this->position.x + (col * this->size.width), this->position.y + (row * this->size.height), this->size.width, this->size.height, this->color);
                    }
                }
            }
        }
    }

    int getBlockHeight()
    {
        return this->matrixShape.rows * this->size.height;
    }
};

int main()
{
    InitWindow(screenWidth, screenHeight, windowTitle);
    SetTargetFPS(60);
    int frameCount = 0;
    Block block(S);

    // LOAD TEXTURES
    Texture2D brickTexture = LoadTexture("assets/sprites/brick-var-1.png");
    block.texture = &brickTexture;

    // GAME LOOP
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        frameCount++;

        if (IsKeyPressed(KEY_ENTER))
        {
            block.rotate();
        }

        if (frameCount % 60 == 0)
        {
            block.position.y += 20;
        }

        if (frameCount % 10 == 0)
        {
            if (IsKeyDown(KEY_LEFT))
            {
                block.position.x -= block.size.width;
            }

            if (IsKeyDown(KEY_RIGHT))
            {
                block.position.x += block.size.width;
            }
        }

        if (IsKeyDown(KEY_DOWN))
        {
            block.position.y += 10;
        }

        if (block.position.y + block.getBlockHeight() > screenHeight)
        {
            block.position.y = screenHeight - block.getBlockHeight();
        }

        block.draw();
        EndDrawing();
    }

    // Unload textures
    UnloadTexture(brickTexture);

    CloseWindow();
    return 0;
}