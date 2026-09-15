#include "raylib.h"
#include <random>
#include <vector>
#include <optional>

// Global variables
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

Color BlockColors[7] =
    {
        {0, 240, 240, 255}, // I,
        {0, 0, 240, 255},   // J,
        {240, 160, 0, 255}, // L,
        {240, 240, 0, 255}, // O,
        {0, 240, 0, 255},   // S,
        {160, 0, 240, 255}, // T,
        {240, 0, 0, 255}    // Z
};

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

    // Default constructor: creates random blocks
    Block(Texture2D *texture = nullptr, Position position = {0, 0})
    {
        // initialize random number generator
        std::random_device rd;
        std::mt19937 gen(rd()); // Mersenne Twister engine for random number generation
        std::uniform_int_distribution<> dis(0, 6);
        // Sample shape and color pair
        this->type = static_cast<BlockType>(dis(gen));
        this->color = BlockColors[this->type];
        this->size = {20, 20};
        initBlockMatrix();
        this->position = {position.x, position.y - static_cast<float>(this->getBlockHeight())};
        this->texture = texture;
    }

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

    Block rotate()
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
        Block rotatedBlock = Block(this->type, this->color, this->position, this->size, this->texture); // Create a new block with the rotated matrix
        rotatedBlock.matrix = newMatrix;
        rotatedBlock.matrixShape = newMatrixShape;
        return rotatedBlock;
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
                        DrawTexture(*this->texture, this->position.x + (col * this->size.width), this->position.y + (row * this->size.height), this->color);
                    }
                    else
                    {
                        printf("Warning: Block at row %d, col %d has no texture assigned.\n", row, col);
                        exit(1);
                    }
                }
            }
        }
    }

    int getBlockHeight()
    {
        return this->matrixShape.rows * this->size.height;
    }

    int getBlockWidth()
    {
        return this->matrixShape.cols * this->size.width;
    }
};

class Brick
{
public:
    Texture2D *texture;
    Position position;
    Color color;
};

class TetrisBoard
{
public:
    Shape boardShape = {20, 10};
    Size blockSize = {20, 20};
    std::optional<Brick> bricks[20][10]{std::nullopt}; // 2D array to hold the bricks on the board
    bool pieceLanded = false;
    bool gameOver = false;
    Position position = {
        static_cast<float>(screenWidth - this->getBoardWidth()) / 2,
        20,
    };

    void draw()
    {
        DrawRectangle(this->position.x, this->position.y, this->boardShape.cols * this->blockSize.width, this->boardShape.rows * this->blockSize.height, GRAY);
        this->drawBricks();
    }

    void drawBricks()
    {
        for (int row = 0; row < this->boardShape.rows; row++)
        {
            for (int col = 0; col < this->boardShape.cols; col++)
            {
                std::optional<Brick> brick = this->bricks[row][col];
                if (brick.has_value())
                {
                    if (brick->texture == nullptr)
                    {
                        printf("Warning: Brick at row %d, col %d has no texture assigned.\n", row, col);
                        exit(1);
                    }
                    DrawTexture(*brick->texture, col * this->blockSize.width + this->position.x, row * this->blockSize.height + this->position.y, brick->color);
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

                    int boardRow = static_cast<int>((brick.position.y - this->position.y) / this->blockSize.height);
                    int boardCol = static_cast<int>((brick.position.x - this->position.x) / this->blockSize.width);

                    this->addBrick(brick, boardRow, boardCol);
                }
            }
        }
    }

    int getBoardHeight()
    {
        return this->boardShape.rows * this->blockSize.height;
    }

    int getBoardWidth()
    {
        return this->boardShape.cols * this->blockSize.width;
    }

    void manageCollisions(Block &block, int verticalStep)
    {
        for (int row = 0; row < block.matrixShape.rows; row++)
        {
            for (int col = 0; col < block.matrixShape.cols; col++)
            {
                if (block.matrix[row][col] && block.position.y >= this->position.y)
                {

                    if (this->collidesWithBorder(block) || this->collidesWithExistingBricks(block))
                    {

                        block.position.y -= verticalStep; // Move the block one slot up when collision is detected

                        if (block.position.y < this->position.y)
                        {
                            this->gameOver = true;
                            return;
                        }

                        this->addBlock(block);
                        Block newBlock = Block(block.texture, this->position);
                        block = newBlock;
                        this->pieceLanded = true;
                        return;
                    }
                }
            }
        }
    }

    bool collidesWithBorder(Block &block)
    {
        for (int row = 0; row < block.matrixShape.rows; row++)
        {
            for (int col = 0; col < block.matrixShape.cols; col++)
            {
                if (block.matrix[row][col])
                {
                    // Collision with left border
                    if (block.position.x < this->position.x)
                    {
                        printf("Collision with left border\n");
                        return true;
                    }

                    // Collision with right border
                    if (block.position.x + (col * block.size.width) >= this->position.x + this->getBoardWidth())
                    {
                        printf("Collision with right border\n");
                        return true;
                    }

                    // Collision with bottom border
                    if (block.position.y + (row * block.size.height) >= this->position.y + this->getBoardHeight())
                    {
                        printf("Collision with bottom border\n");
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool collidesWithExistingBricks(Block &block)
    {
        for (int row = 0; row < block.matrixShape.rows; row++)
        {
            for (int col = 0; col < block.matrixShape.cols; col++)
            {
                int boardRow = static_cast<int>((block.position.y + row * block.size.height - this->position.y) / this->blockSize.height);
                int boardCol = static_cast<int>((block.position.x + (col * block.size.width) - this->position.x) / this->blockSize.width);

                if (block.matrix[row][col] &&
                    boardRow >= 0 && boardRow < this->boardShape.rows &&
                    boardCol >= 0 && boardCol < this->boardShape.cols &&
                    this->bricks[boardRow][boardCol].has_value())
                {
                    return true;
                }
            }
        }
        return false;
    }

    void manageCompleteLines()
    {
        for (int row = 0; row < this->boardShape.rows; row++)
        {
            bool lineComplete = true;
            for (int col = 0; col < this->boardShape.cols; col++)
            {
                if (!this->bricks[row][col].has_value())
                {
                    lineComplete = false;
                    break;
                }
            }
            if (lineComplete)
            {
                for (int col = 0; col < this->boardShape.cols; col++)
                {
                    this->bricks[row][col] = std::nullopt;

                    // Move all rows above down by one
                    for (int r = row; r > 0; r--)
                    {
                        this->bricks[r][col] = this->bricks[r - 1][col];
                        this->bricks[r - 1][col] = std::nullopt;
                    }
                }
            }
        }
    }
};

typedef enum GameScreen
{
    GAME_MENU,
    GAME_PLAY,
    GAME_OVER
} GameScreen;

int main()
{
    InitWindow(screenWidth, screenHeight, windowTitle);
    SetTargetFPS(60);
    int frameCount = 0;

    // LOAD TEXTURES
    Texture2D brickTexture = LoadTexture("assets/sprites/brick-var-4.png");

    // CREATE OBJECTS
    TetrisBoard board;
    Block block = Block(&brickTexture, board.position);
    GameScreen gameScreen = GAME_PLAY;

    // GAME LOOP
    while (!WindowShouldClose())
    {
        // GAME LOGIC
        frameCount++;
        int blockVerticalStep = 0;

        switch (gameScreen)
        {
        case GAME_MENU:
            break;

        case GAME_PLAY:

            if (IsKeyPressed(KEY_ENTER))
            {
                Block rotatedBlock = block.rotate();
                if (!board.collidesWithBorder(rotatedBlock) && !board.collidesWithExistingBricks(rotatedBlock))
                {
                    block = rotatedBlock;
                }
            }

            if (frameCount % 60 == 0)
            {
                block.position.y += 20;
                blockVerticalStep += 20;
            }

            if (IsKeyPressed(KEY_LEFT))
            {
                block.position.x -= block.size.width;

                if (board.collidesWithExistingBricks(block) || board.collidesWithBorder(block))
                {
                    block.position.x += block.size.width;
                }
            }

            if (IsKeyPressed(KEY_RIGHT))
            {
                block.position.x += block.size.width;
                if (board.collidesWithExistingBricks(block) || board.collidesWithBorder(block))
                {
                    block.position.x -= block.size.width;
                }
            }

            if (IsKeyDown(KEY_DOWN))
            {
                block.position.y += 20;
                blockVerticalStep += 20;
            }

            board.manageCollisions(block, blockVerticalStep);
            if (board.gameOver)
            {
                gameScreen = GAME_OVER;
            }

            // Do line clearing only when a piece has landed
            if (board.pieceLanded)
            {
                board.manageCompleteLines();
                board.pieceLanded = false;
            }
            break;

        case GAME_OVER:
            break;
        }

        // DRAWING
        BeginDrawing();
        ClearBackground(BLACK);

        switch (gameScreen)
        {
        case GAME_MENU:
            break;
        case GAME_PLAY:
            board.draw();
            block.draw();
            break;
        case GAME_OVER:
            ClearBackground(BLUE);
            board.draw();
            block.draw();
            break;
        }

        EndDrawing();
    }

    // Unload textures
    UnloadTexture(brickTexture);

    CloseWindow();
    return 0;
}