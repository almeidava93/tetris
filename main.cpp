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

class Block
{
public:
    BlockType type;
    Color color = GRAY;
    Position position;
    Size size;                             // width and height of each square in the block
    Shape matrixShape;                     //  dimensions of the block's matrix representation
    std::vector<std::vector<bool>> matrix; // 2D vector to represent the block's shape
    Texture2D *texture;                    // Pointer to the texture for the block

    Block(BlockType type = J, Color color = GRAY, Position position = {0, 0}, Size size = {20, 20}, Texture2D *texture = nullptr)
    {
        this->type = type;
        this->color = color;
        this->position = position;
        this->size = size;
        this->texture = texture;
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

class Brick
{
public:
    Texture2D *texture;
    Position position; // Position of the brick on the board
    Color color;
};

class TetrisBoard
{
public:
    Shape boardShape = {20, 10};
    Size blockSize = {20, 20};
    Brick bricks[20][10]{};

    void draw()
    {
        DrawRectangle(0, 0, this->boardShape.cols * this->blockSize.width, this->boardShape.rows * this->blockSize.height, GRAY);
        this->drawBricks();
    }

    void drawBricks()
    {
        for (int row = 0; row < this->boardShape.rows; row++)
        {
            for (int col = 0; col < this->boardShape.cols; col++)
            {
                Brick brick = this->bricks[row][col];
                if (brick.texture)
                {
                    DrawTexture(*brick.texture, brick.position.x, brick.position.y, BLUE);
                }
                else
                {
                    DrawRectangle(brick.position.x, brick.position.y, this->blockSize.width, this->blockSize.height, brick.color);
                }
            }
        }
    }

    void addBrick(Brick brick, int row, int col)
    {
        if (row >= 0 && row < this->boardShape.rows && col >= 0 && col < this->boardShape.cols)
        {
            this->bricks[row][col] = brick;
        }
    }

    void addBlock(Block block)
    {
        for (int row = 0; row < block.matrixShape.rows; row++)
        {
            for (int col = 0; col < block.matrixShape.cols; col++)
            {
                if (block.matrix[row][col])
                {
                    Brick brick;
                    brick.position.x = block.position.x + (col * block.size.width);
                    brick.position.y = block.position.y + (row * block.size.height);
                    brick.color = block.color;
                    brick.texture = block.texture;

                    int boardRow = static_cast<int>(brick.position.y / this->blockSize.height);
                    int boardCol = static_cast<int>(brick.position.x / this->blockSize.width);

                    this->addBrick(brick, boardRow, boardCol);
                }
            }
        }
    }

    int getBoardHeight()
    {
        return this->boardShape.rows * this->blockSize.height;
    }
};

int main()
{
    InitWindow(screenWidth, screenHeight, windowTitle);
    SetTargetFPS(60);
    int frameCount = 0;
    Block block(S);
    TetrisBoard board;

    // LOAD TEXTURES
    Texture2D brickTexture = LoadTexture("assets/sprites/brick-var-1.png");
    block.texture = &brickTexture;

    // GAME LOOP
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);
        frameCount++;

        board.draw();

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

        if (block.position.y + block.getBlockHeight() > board.getBoardHeight())
        {
            block.position.y = board.getBoardHeight() - block.getBlockHeight();
            board.addBlock(block);
            block = Block(S);
            block.texture = &brickTexture;
        }

        block.draw();
        EndDrawing();
    }

    // Unload textures
    UnloadTexture(brickTexture);

    CloseWindow();
    return 0;
}