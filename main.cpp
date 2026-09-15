#include "raylib.h"
#include <random>
#include <vector>
#include <optional>
#include <string>
#include <map>

// Global variables
const int screenWidth = 800;
const int screenHeight = 450;
const int targetFPS = 60;
const char *windowTitle = "Tetris";

class DifficultyLevel
{
public:
    int levelChangeNumberOfLines;
    int blockVerticalSpeed;        // Frames per step. Lower is faster.
    int blockHorizontalSpeed = 20; // Frames per step. Lower is faster.
};

DifficultyLevel difficultyLevels[10] =
    {
        {10, 60},
        {20, 50},
        {30, 40},
        {40, 30},
        {50, 20},
        {60, 10},
        {70, 5, 5},
        {80, 3, 3},
        {90, 2, 2},
        {100, 1, 1}};

class SoundEffect
{
public:
    std::string audioFilePath;
    Sound sound;

    SoundEffect() {}

    SoundEffect(std::string audioFilePath)
    {
        this->audioFilePath = audioFilePath;
        this->sound = LoadSound(audioFilePath.c_str());
    }

    void play()
    {
        PlaySound(this->sound);
    }

    void unload()
    {
        UnloadSound(this->sound);
    }
};

typedef enum SoundEffectTrigger
{
    BLOCK_LANDING,
    BLOCK_COLLISION,
    GAME_OVER_SFX,
    SINGLE_LINE_CLEAR,
    DOUBLE_LINE_CLEAR,
    TRIPLE_LINE_CLEAR,
    TETRIS_LINE_CLEAR,
    GAME_START,
    LEVEL_UP
} SoundEffectTrigger;

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
    int linesCleared = 0;
    int linesClearedLastCombo = 0;
    int score = 0;
    int difficultyLevelIndex = 0;
    DifficultyLevel difficultyLevel = difficultyLevels[this->difficultyLevelIndex];

    TetrisBoard() {}

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
        int linesCleared = 0;
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
                linesCleared++;
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
        this->updateScore(linesCleared);
    }

    void updateScore(int linesCleared)
    {
        int numLinesPoints;
        switch (linesCleared)
        {
        case 1:
            numLinesPoints = 100;
            break;
        case 2:
            numLinesPoints = 300;
            break;
        case 3:
            numLinesPoints = 500;
            break;
        case 4:
            numLinesPoints = 800;
            break;
        default:
            numLinesPoints = 0;
            break;
        }
        this->score += numLinesPoints * (this->difficultyLevelIndex + 1);
        this->linesCleared += linesCleared;
        this->linesClearedLastCombo = linesCleared;
    }

    bool hasLeveledUp()
    {
        int maxDifficultyLevelIndex = sizeof(difficultyLevels) / sizeof(DifficultyLevel) - 1;
        if (this->linesCleared >= this->difficultyLevel.levelChangeNumberOfLines)
        {
            if (this->difficultyLevelIndex < maxDifficultyLevelIndex)
            {
                this->difficultyLevelIndex++;
            }
            this->difficultyLevel = difficultyLevels[this->difficultyLevelIndex];
            return true;
        }
        else
        {
            return false;
        }
    }

    void reset()
    {
        this->score = 0;
        this->linesCleared = 0;
        this->linesClearedLastCombo = 0;
        this->difficultyLevelIndex = 0;
        this->difficultyLevel = difficultyLevels[this->difficultyLevelIndex];
        this->gameOver = false;
        this->pieceLanded = false;

        // clear the board
        for (int row = 0; row < this->boardShape.rows; row++)
        {
            for (int col = 0; col < this->boardShape.cols; col++)
            {
                this->bricks[row][col] = std::nullopt;
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

    // Load sound effects
    InitAudioDevice();
    std::map<SoundEffectTrigger, SoundEffect> soundEffects;
    soundEffects[BLOCK_LANDING] = SoundEffect("assets/audio/block-landing.wav");
    // soundEffects[BLOCK_COLLISION] = SoundEffect("assets/audio/block-collision.wav");
    soundEffects[GAME_OVER_SFX] = SoundEffect("assets/audio/game-over.mp3");
    soundEffects[SINGLE_LINE_CLEAR] = SoundEffect("assets/audio/single-line-clear.wav");
    soundEffects[DOUBLE_LINE_CLEAR] = SoundEffect("assets/audio/double-line-clear.wav");
    soundEffects[TRIPLE_LINE_CLEAR] = SoundEffect("assets/audio/triple-line-clear.wav");
    soundEffects[TETRIS_LINE_CLEAR] = SoundEffect("assets/audio/tetris-line-clear.wav");
    soundEffects[GAME_START] = SoundEffect("assets/audio/game-start.wav");
    soundEffects[LEVEL_UP] = SoundEffect("assets/audio/level-up.wav");

    // CREATE OBJECTS
    TetrisBoard board;
    Block block = Block(&brickTexture, board.position);
    GameScreen gameScreen = GAME_PLAY;

    // GAME START
    soundEffects[GAME_START].play();

    // GAME LOOP
    while (!WindowShouldClose())
    {
        // GAME LOGIC
        frameCount++;
        int blockVerticalStep = 0;
        int lastHorizontalMovementFrame = 0;

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

            if (frameCount % board.difficultyLevel.blockVerticalSpeed == 0 || IsKeyDown(KEY_DOWN))
            {
                block.position.y += 20;
                blockVerticalStep += 20;
            }

            if (IsKeyPressed(KEY_LEFT) || (IsKeyDown(KEY_LEFT) && (frameCount - lastHorizontalMovementFrame) % board.difficultyLevel.blockHorizontalSpeed == 0))
            {
                block.position.x -= block.size.width;
                lastHorizontalMovementFrame = frameCount;

                if (board.collidesWithExistingBricks(block) || board.collidesWithBorder(block))
                {
                    block.position.x += block.size.width;
                }
            }

            if (IsKeyPressed(KEY_RIGHT) || (IsKeyDown(KEY_RIGHT) && (frameCount - lastHorizontalMovementFrame) % board.difficultyLevel.blockHorizontalSpeed == 0))
            {
                lastHorizontalMovementFrame = frameCount;
                block.position.x += block.size.width;
                if (board.collidesWithExistingBricks(block) || board.collidesWithBorder(block))
                {
                    block.position.x -= block.size.width;
                }
            }

            board.manageCollisions(block, blockVerticalStep);
            if (board.gameOver)
            {
                gameScreen = GAME_OVER;
                soundEffects.at(GAME_OVER_SFX).play();
            }

            // Do line clearing only when a piece has landed
            if (board.pieceLanded)
            {
                board.manageCompleteLines();
                soundEffects.at(BLOCK_LANDING).play();
                board.pieceLanded = false;
            }

            // Sound effects for clearing lines
            switch (board.linesClearedLastCombo)
            {
            case 1:
                soundEffects.at(SINGLE_LINE_CLEAR).play();
                board.linesClearedLastCombo = 0;
                break;
            case 2:
                soundEffects.at(DOUBLE_LINE_CLEAR).play();
                board.linesClearedLastCombo = 0;
                break;
            case 3:
                soundEffects.at(TRIPLE_LINE_CLEAR).play();
                board.linesClearedLastCombo = 0;
                break;
            case 4:
                soundEffects.at(TETRIS_LINE_CLEAR).play();
                board.linesClearedLastCombo = 0;
                break;
            }

            // Evaluate difficulty level
            if (board.hasLeveledUp())
                soundEffects.at(LEVEL_UP).play();
            break;

        case GAME_OVER:

            if (IsKeyPressed(KEY_ENTER))
            {
                board.reset();
                gameScreen = GAME_PLAY;
                soundEffects[GAME_START].play();
            }

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
            DrawText(TextFormat("SCORE: %05d", board.score), 20, 20, 20, GRAY);
            DrawText(TextFormat("Lines cleared: %01d", board.linesCleared), 20, 60, 20, GRAY);
            DrawText(TextFormat("Level: %01d", board.difficultyLevelIndex + 1), 20, 40, 20, GRAY);

            break;
        case GAME_OVER:
            ClearBackground(BLUE);
            board.draw();
            block.draw();
            DrawText("PRESS ENTER", 20, 20, 20, BLACK);
            DrawText("TO RESTART", 20, 40, 20, BLACK);
            break;
        }

        EndDrawing();
    }

    // Unload textures
    UnloadTexture(brickTexture);

    // Unload sound effects
    for (auto &soundEffect : soundEffects)
    {
        soundEffect.second.unload();
    }

    CloseWindow();
    return 0;
}
