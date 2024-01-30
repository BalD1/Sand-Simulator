#include "SFML/Graphics.hpp"
#include <iostream>
#include <cstdlib>
#include <random>
#include <map>

const int winSizeX = 800;
const int winSizeY = 800;

const int cellSizeX = 5;
const int cellSizeY = 5;

const int gridSizeX = winSizeX / cellSizeX;
const int gridSizeY = winSizeY / cellSizeY;

const float cellBaseFallTime = .15f;

const float gravity = 9.81f;

const sf::Color fullCellColor_0 = sf::Color(215, 198, 143);
const sf::Color fullCellColor_1 = sf::Color(228,214,172);

static sf::RectangleShape emptyCell;
static sf::RectangleShape fullCell;
static sf::RectangleShape wallCell;
static sf::RectangleShape holeCell;

const int emptyCellID = 0;
const int fullCellID = 1;
const int wallCellID = 2;
const int holeCellID = 3;

const std::map<int, sf::Color> cellsColor
{
    {0, sf::Color::Transparent},
    {2, sf::Color::Red},
    {3, sf::Color::Transparent},
};

struct CellData
{
    int ID;
    sf::Color color;

    float fallTimer;
    int fallMultiplier;
};

void DrawGrid(CellData grid[gridSizeX][gridSizeY], sf::RenderWindow& window)
{
    window.clear(sf::Color(18, 33, 43));

    for (size_t x = 0; x < gridSizeX; x++)
    {
        for (size_t y = 0; y < gridSizeY; y++)
        {
            switch (grid[x][y].ID)
            {
                case emptyCellID:
                    emptyCell.setPosition(cellSizeX * x, cellSizeY * y);
                    emptyCell.setFillColor(grid[x][y].color);
                    window.draw(emptyCell);
                    break;
                case fullCellID:
                    fullCell.setFillColor(grid[x][y].color);
                    fullCell.setPosition(cellSizeX * x, cellSizeY * y);
                    window.draw(fullCell);
                    break;
                case wallCellID:
                    wallCell.setPosition(cellSizeX * x, cellSizeY * y);
                    wallCell.setFillColor(grid[x][y].color);
                    window.draw(wallCell);
                    break;
                case holeCellID:
                    holeCell.setPosition(cellSizeX * x, cellSizeY * y);
                    holeCell.setFillColor(grid[x][y].color);
                    window.draw(holeCell);
                    break;
            }
        }
    }
    window.display();
}

void ProcessGrid(CellData(&grid)[gridSizeX][gridSizeY], float dt)
{
    bool leftToRight = rand() % 100 < 50;
    for (int x = leftToRight ? 0 : gridSizeX - 1; leftToRight ? x < gridSizeX : x >= 0; leftToRight ? x++ : x--)
    {
        for (int y = gridSizeY - 2; y >= 0; y--)
        {
            if (grid[x][y].ID == fullCellID)
            {
                if (grid[x][y + 1].ID == wallCellID) continue;
                bool isBEmpty = (grid[x][y + 1].ID == emptyCellID) || (grid[x][y + 1].ID == holeCellID);
                bool isBRightEmpty = x < gridSizeX && 
                                    ((grid[x + 1][y + 1].ID == emptyCellID) || (grid[x + 1][y + 1].ID == holeCellID)) &&
                                    ((grid[x + 1][y].ID == emptyCellID) || (grid[x + 1][y].ID == holeCellID));
                bool isBLeftEmpty = x > 0 && 
                                    ((grid[x - 1][y + 1].ID == emptyCellID) || (grid[x - 1][y + 1].ID == holeCellID)) &&
                                    ((grid[x - 1][y].ID == emptyCellID) || (grid[x - 1][y].ID == holeCellID));

                float& timer = grid[x][y].fallTimer;
                int& incrModifier = grid[x][y].fallMultiplier;

                // to refactor
                // if none of below are empty
                if (!isBEmpty && !isBRightEmpty && !isBLeftEmpty)
                {
                    timer = cellBaseFallTime;
                    incrModifier = 1;
                    continue;
                }

                if (timer > 0)
                {
                    timer -= dt * incrModifier;
                    continue;
                }
                int cellsFallIdx = 1;

                while (timer + cellBaseFallTime < 0 && y + cellsFallIdx < (gridSizeY - 1))
                {
                    cellsFallIdx++;
                    timer += cellBaseFallTime;
                }
                if (y + cellsFallIdx >= gridSizeY)
                    std::cout << y + cellsFallIdx << std::endl;

                sf::Vector2i nextCell = sf::Vector2i(x,y);
                if (isBEmpty)
                {
                    nextCell = sf::Vector2i(x, y + cellsFallIdx);
                }
                else
                {
                    if (isBRightEmpty && isBLeftEmpty)
                    {
                        if (rand() % 100 < 50)
                            nextCell = sf::Vector2i(x - 1, y + cellsFallIdx);
                        else
                            nextCell = sf::Vector2i(x + 1, y + cellsFallIdx);
                    }
                    else if (isBRightEmpty)
                        nextCell = sf::Vector2i(x + 1, y + cellsFallIdx);
                    else if (isBLeftEmpty)
                        nextCell = sf::Vector2i(x - 1, y + cellsFallIdx);
                }

                CellData lastCellData = grid[x][y];
                grid[x][y].ID = emptyCellID;
                grid[x][y].color = cellsColor.at(emptyCellID);

                for (size_t i = 1; i <= cellsFallIdx; i++)
                {
                    if (grid[nextCell.x][y + i].ID == fullCellID)
                    {
                        nextCell.y -= (cellsFallIdx - (i -1));
                        break;
                    }
                }
                if (grid[nextCell.x][nextCell.y].ID == holeCellID) continue;

                grid[nextCell.x][nextCell.y].ID = fullCellID;
                grid[nextCell.x][nextCell.y].color = lastCellData.color;

                int& nextIncrModifier = grid[nextCell.x][nextCell.y].fallMultiplier;
                nextIncrModifier = incrModifier + 1;
                float& nextTimer = grid[nextCell.x][nextCell.y].fallTimer;
                nextTimer = cellBaseFallTime + timer;
            }
        }
    }
}

void SpawnCell(CellData(&grid)[gridSizeX][gridSizeY], sf::Vector2i pos, int cellType)
{
    if (pos.x < 0 || pos.x >= gridSizeX) return;
    if (pos.y < 0 || pos.y >= gridSizeY) return;
    if (grid[pos.x][pos.y].ID == cellType) return;
    grid[pos.x][pos.y].ID = cellType;

    if (cellType == fullCellID)
    {
        grid[pos.x][pos.y].fallTimer = cellBaseFallTime;
        grid[pos.x][pos.y].fallMultiplier = 1;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr_r(fullCellColor_0.r, fullCellColor_1.r);
        std::uniform_int_distribution<> distr_g(fullCellColor_0.g, fullCellColor_1.g);
        std::uniform_int_distribution<> distr_b(fullCellColor_0.b, fullCellColor_1.b);

        grid[pos.x][pos.y].color = sf::Color(distr_r(gen),
            distr_g(gen),
            distr_b(gen));
    }
    else if (cellsColor.count(cellType))
        grid[pos.x][pos.y].color = cellsColor.at(cellType);
}

sf::Vector2i GetGridMousePos(sf::RenderWindow& window)
{
    return sf::Vector2i(sf::Mouse::getPosition(window).x / cellSizeX, sf::Mouse::getPosition(window).y / cellSizeY);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(winSizeX, winSizeY), "Sand Simulator");
    window.setFramerateLimit(60);

    emptyCell.setSize(sf::Vector2f(cellSizeX, cellSizeY));
    emptyCell.setOutlineColor(sf::Color::Black);
    emptyCell.setOutlineThickness(1);

    fullCell.setSize(sf::Vector2f(cellSizeX, cellSizeY));
    fullCell.setOutlineColor(sf::Color::Black);
    fullCell.setOutlineThickness(1);

    wallCell.setSize(sf::Vector2f(cellSizeX, cellSizeY));
    wallCell.setOutlineColor(sf::Color::Black);
    wallCell.setOutlineThickness(1);

    holeCell.setSize(sf::Vector2f(cellSizeX, cellSizeY));
    holeCell.setOutlineColor(sf::Color::Red);
    holeCell.setOutlineThickness(2);

    CellData grid[gridSizeX][gridSizeY];

    for (size_t x = 0; x < gridSizeX; x++)
    {
        for (size_t y = 0; y < gridSizeY; y++)
        {
            grid[x][y] = {0, sf::Color::Transparent, 0, 0};
        }
    }

    bool isLeftMousePressed = false;

    int cellType = 0;

    const float cellSpawnCD = .025f;
    float cellSpawnTimer = 0;
    sf::Time dt;
    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf::Event::Closed:
                    window.close();
                    break;
                case sf::Event::KeyPressed:
                    switch (event.key.code)
                    {
                        case sf::Keyboard::Num1:
                            cellType = emptyCellID;
                            break;
                        case sf::Keyboard::Num2:
                            cellType = fullCellID;
                            break;
                        case sf::Keyboard::Num3:
                            cellType = wallCellID;
                            break;
                        case sf::Keyboard::Num4:
                            cellType = holeCellID;
                            break;
                    }
                    break;
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left)
                        isLeftMousePressed = true;
                    if (event.mouseButton.button == sf::Mouse::Right)
                    {
                        sf::Vector2i mousePos = GetGridMousePos(window);
                        std::cout << "x:" + std::to_string(mousePos.x) + " y:" + std::to_string(mousePos.y) + " : " + std::to_string(grid[mousePos.x][mousePos.y].ID) << std::endl;
                    }
                    break;
                case sf::Event::MouseButtonReleased:
                    if (event.mouseButton.button == sf::Mouse::Left)
                        isLeftMousePressed = false;
                    break;
            }
        }

        cellSpawnTimer -= dt.asSeconds();
        if (isLeftMousePressed && cellSpawnTimer <= 0)
        {
            cellSpawnTimer = cellSpawnCD;
            sf::Vector2i mouseGridPos = GetGridMousePos(window);
            SpawnCell(grid, mouseGridPos, cellType);
        }
        ProcessGrid(grid, dt.asSeconds());
        DrawGrid(grid, window);
        dt = clock.restart();
    }

    return 0;
}