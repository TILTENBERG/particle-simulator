#pragma once
#include <vector>
#include <cmath>
#include <algorithm>
#include "Particle.hpp"

class SpatialGrid
{
private:
    float cellSize;
    int cols;
    int rows;
    std::vector<std::vector<int>> cells; // Storing indices of particles in each cell

public:
    SpatialGrid(float width, float height, float maxRadius)
    {
        cellSize = maxRadius * 2.0f;
        cols = static_cast<int>(std::ceil(width / cellSize));
        rows = static_cast<int>(std::ceil(height / cellSize));
        cells.resize(cols * rows);
    }

    void clear()
    {
        for (auto &cell : cells)
        {
            cell.clear();
        }
    }

    void insert(int particleIndex, const Vector2D &position)
    {
        int cx = static_cast<int>(position.getX() / cellSize);
        int cy = static_cast<int>(position.getY() / cellSize);

        // Clamp to avoid array out of bounds
        cx = std::clamp(cx, 0, cols - 1);
        cy = std::clamp(cy, 0, rows - 1);

        cells[cy * cols + cx].push_back(particleIndex);
    }

    void handleCollisions(std::vector<Particle> &particles)
    {
        clear();

        // 1. Populate the spatial grid
        for (int i = 0; i < static_cast<int>(particles.size()); ++i)
        {
            insert(i, particles[i].getPosition());
        }

        // 2. Perform broad-phase and narrow-phase collision resolution
        for (int i = 0; i < static_cast<int>(particles.size()); ++i)
        {
            const Vector2D &pos = particles[i].getPosition();
            int cx = static_cast<int>(pos.getX() / cellSize);
            int cy = static_cast<int>(pos.getY() / cellSize);
            cx = std::clamp(cx, 0, cols - 1);
            cy = std::clamp(cy, 0, rows - 1);

            // Check the current cell and its 8 neighboring cells
            for (int dy = -1; dy <= 1; ++dy)
            {
                int ny = cy + dy;
                if (ny < 0 || ny >= rows) continue;

                for (int dx = -1; dx <= 1; ++dx)
                {
                    int nx = cx + dx;
                    if (nx < 0 || nx >= cols) continue;

                    const auto &cellParticles = cells[ny * cols + nx];
                    for (int j : cellParticles)
                    {
                        // Ensure each pair is resolved exactly once (when i < j)
                        if (i < j)
                        {
                            particles[i].collision_solve(particles[j]);
                        }
                    }
                }
            }
        }
    }
};
