#include "SFML/Graphics.hpp"
#include <iostream>
#include <cstdlib>

const int winSizeX = 800;
const int winSizeY = 800;

const int cellSizeX = 25;
const int cellSizeY = 25;

const int gridSizeX = winSizeX / cellSizeX;
const int gridSizeY = winSizeY / cellSizeY;

const float cellBaseFallTime = .25f;

const float gravity = 9.81f;

void DrawGrid(int grid[gridSizeX][gridSizeY], sf::RenderWindow& window, sf::RectangleShape emptyCell, sf::RectangleShape fullCell)
{
    window.clear(sf::Color(18, 33, 43));
    for (size_t x = 0; x < gridSizeX; x++)
    {
        for (size_t y = 0; y < gridSizeY; y++)
        {
            if (grid[x][y] == 0)
            {
                emptyCell.setPosition(cellSizeX * x, cellSizeY * y);
                window.draw(emptyCell);
            }
            else
            {
                fullCell.setPosition(cellSizeX * x, cellSizeY * y);
                window.draw(fullCell);
            }
        }
    }
    window.display();
}

void ProcessGrid(int(&grid)[gridSizeX][gridSizeY], std::tuple<float, int>(&fallTimeGrid)[gridSizeX][gridSizeY], float dt)
{
    for (int x = gridSizeX - 1; x >= 0; x--)
    {
        for (int y = gridSizeY - 2; y >= 0; y--)
        {
            // if is full
            if (grid[x][y] == 1)
            {
                float& timer = std::get<0>(fallTimeGrid[x][y]);
                int& incrModifier = std::get<1>(fallTimeGrid[x][y]);
                if (timer > 0)
                {
                    timer -= dt * incrModifier;
                    continue;
                }

                sf::Vector2i nextCell;
                incrModifier++;
                if (grid[x][y + 1] == 0)
                {
                    grid[x][y] = 0;
                    grid[x][y + 1] = 1;
                    nextCell = sf::Vector2i(x, y + 1);
                    int& nextIncrModifier = std::get<1>(fallTimeGrid[x][y + 1]);
                    nextIncrModifier = incrModifier;
                    float& nextTimer = std::get<0>(fallTimeGrid[x][y + 1]);
                    nextTimer = cellBaseFallTime + timer;
                }
                else
                {
                    bool isBRightEmpty = x < gridSizeX && grid[x + 1][y + 1] == 0;
                    bool isBLeftEmpty = x > 0 && grid[x - 1][y + 1] == 0;

                    if (isBRightEmpty && isBLeftEmpty)
                    {
                        grid[x][y] = 0;
                        if (rand() % 100 < 50)
                        {
                            grid[x - 1][y + 1] = 1;
                            int& nextIncrModifier = std::get<1>(fallTimeGrid[x - 1][y + 1]);
                            nextIncrModifier = incrModifier;
                            float& nextTimer = std::get<0>(fallTimeGrid[x - 1][y + 1]);
                            nextTimer = cellBaseFallTime + timer;
                        }
                        else
                        {
                            grid[x + 1][y + 1] = 1;
                            int& nextIncrModifier = std::get<1>(fallTimeGrid[x + 1][y + 1]);
                            nextIncrModifier = incrModifier;
                            float& nextTimer = std::get<0>(fallTimeGrid[x + 1][y + 1]);
                            nextTimer = cellBaseFallTime + timer;
                        }
                    }
                    else if (isBRightEmpty)
                    {
                        grid[x][y] = 0;
                        grid[x + 1][y + 1] = 1;
                        int& nextIncrModifier = std::get<1>(fallTimeGrid[x + 1][y + 1]);
                        nextIncrModifier = incrModifier;
                        float& nextTimer = std::get<0>(fallTimeGrid[x + 1][y + 1]);
                        nextTimer = cellBaseFallTime + timer;
                    }
                    else if (isBLeftEmpty)
                    {
                        grid[x][y] = 0;
                        grid[x - 1][y + 1] = 1;
                        int& nextIncrModifier = std::get<1>(fallTimeGrid[x - 1][y + 1]);
                        nextIncrModifier = incrModifier;
                        float& nextTimer = std::get<0>(fallTimeGrid[x - 1][y + 1]);
                        nextTimer = cellBaseFallTime + timer;
                    }
                }

            }
        }
    }
}

void SpawnCell(int(&grid)[gridSizeX][gridSizeY], std::tuple<float, int>(&fallTimeGrid)[gridSizeX][gridSizeY], sf::Vector2i pos)
{
    if (pos.x < 0 || pos.x >= gridSizeX) return;
    if (pos.y < 0 || pos.y >= gridSizeY) return;
    if (grid[pos.x][pos.y] == 1) return;
    grid[pos.x][pos.y] = 1;
    fallTimeGrid[pos.x][pos.y] = { cellBaseFallTime, 1 };
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(winSizeX, winSizeY), "Sand Simulator");
    window.setFramerateLimit(60);

    int grid[gridSizeX][gridSizeY];
    std::tuple<float, int> fallTimeGrid[gridSizeX][gridSizeY];

    for (size_t x = 0; x < gridSizeX; x++)
    {
        for (size_t y = 0; y < gridSizeY; y++)
        {
            grid[x][y] = 0;
            fallTimeGrid[x][y] = { 0,0 };
        }
    }

    sf::RectangleShape emptyCell(sf::Vector2f(cellSizeX, cellSizeY));
    emptyCell.setFillColor(sf::Color::Transparent);
    emptyCell.setOutlineColor(sf::Color::Black);
    emptyCell.setOutlineThickness(1);

    sf::RectangleShape fullCell(sf::Vector2f(cellSizeX, cellSizeY));
    fullCell.setFillColor(sf::Color::White);
    fullCell.setOutlineColor(sf::Color::Black);
    fullCell.setOutlineThickness(1);
    //cell.setPosition((window.getSize().x / 2.f) - (cellSizeX / 2), (window.getSize().y / 2.f) - (cellSizeY / 2));

    sf::CircleShape shape(200.f, 100);
    shape.setFillColor(sf::Color(204, 77, 5)); // Color circle
    shape.setPosition(400, 400); // Center circle

    bool isMousePressed = false;

    const float cellSpawnCD = .05f;
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
                case sf::Event::MouseButtonPressed:
                    if (event.mouseButton.button == sf::Mouse::Left)
                        isMousePressed = true;
                    break;
                case sf::Event::MouseButtonReleased:
                    if (event.mouseButton.button == sf::Mouse::Left)
                        isMousePressed = false;
                    break;
            }
        }

        cellSpawnTimer -= dt.asSeconds();
        if (isMousePressed && cellSpawnTimer <= 0)
        {
            cellSpawnTimer = cellSpawnCD;
            sf::Vector2i mouseGridPos = sf::Vector2i(sf::Mouse::getPosition(window).x / cellSizeX, sf::Mouse::getPosition(window).y / cellSizeY);
            //std::cout << "mouse : {" + std::to_string(sf::Mouse::getPosition(window).x) + "," + std::to_string(sf::Mouse::getPosition(window).y) + "}" << std::endl;
            //std::cout << "{" + std::to_string(mouseGridPos.x) + "," + std::to_string(mouseGridPos.y) + "}" << std::endl;
            SpawnCell(grid, fallTimeGrid, mouseGridPos);
        }
        ProcessGrid(grid, fallTimeGrid, dt.asSeconds());
        DrawGrid(grid, window, emptyCell, fullCell);
        dt = clock.restart();
    }

    return 0;
}