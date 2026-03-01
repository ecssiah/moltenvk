#pragma once

#include <cglm/cglm.h>
#include <cstdint>

typedef uint32_t SectorIndex;
typedef ivec3 SectorCoordinate;

typedef uint32_t CellIndex;
typedef ivec3 CellCoordinate;

typedef ivec3 GridCoordinate;

inline constexpr int WORLD_RADIUS_IN_SECTORS = 2;
inline constexpr int WORLD_SIZE_IN_SECTORS = 2 * WORLD_RADIUS_IN_SECTORS + 1;
inline constexpr int WORLD_AREA_IN_SECTORS = WORLD_SIZE_IN_SECTORS * WORLD_SIZE_IN_SECTORS;
inline constexpr int WORLD_VOLUME_IN_SECTORS = WORLD_SIZE_IN_SECTORS * WORLD_SIZE_IN_SECTORS * WORLD_SIZE_IN_SECTORS;

inline constexpr int SECTOR_RADIUS_IN_CELLS = 2;
inline constexpr int SECTOR_SIZE_IN_CELLS = 2 * SECTOR_RADIUS_IN_CELLS + 1;
inline constexpr int SECTOR_AREA_IN_CELLS = SECTOR_SIZE_IN_CELLS * SECTOR_SIZE_IN_CELLS;
inline constexpr int SECTOR_VOLUME_IN_CELLS = SECTOR_SIZE_IN_CELLS * SECTOR_SIZE_IN_CELLS * SECTOR_SIZE_IN_CELLS;

inline constexpr int WORLD_RADIUS_IN_CELLS = WORLD_RADIUS_IN_SECTORS * SECTOR_SIZE_IN_CELLS + SECTOR_RADIUS_IN_CELLS;

inline constexpr float CELL_RADIUS = 0.5f;
inline constexpr float CELL_SIZE = 2 * CELL_RADIUS;
inline constexpr float CELL_AREA = CELL_SIZE * CELL_SIZE;
inline constexpr float CELL_VOLUME = CELL_SIZE * CELL_SIZE * CELL_SIZE;

inline bool sector_index_is_valid(SectorIndex sector_index)
{
    return sector_index < WORLD_VOLUME_IN_SECTORS;
}

inline bool cell_index_is_valid(CellIndex cell_index)
{
    return cell_index < SECTOR_VOLUME_IN_CELLS;
}

inline bool sector_coordinate_is_valid(SectorCoordinate sector_coordinate)
{
    const bool in_x_range = sector_coordinate[0] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[0] <= WORLD_RADIUS_IN_SECTORS;
    const bool in_y_range = sector_coordinate[1] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[1] <= WORLD_RADIUS_IN_SECTORS;
    const bool in_z_range = sector_coordinate[2] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[2] <= WORLD_RADIUS_IN_SECTORS;

    return in_x_range && in_y_range && in_z_range;
}

inline bool cell_coordinate_is_valid(CellCoordinate cell_coordinate)
{
    const bool in_x_range = cell_coordinate[0] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[0] <= SECTOR_RADIUS_IN_CELLS;
    const bool in_y_range = cell_coordinate[1] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[1] <= SECTOR_RADIUS_IN_CELLS;
    const bool in_z_range = cell_coordinate[2] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[2] <= SECTOR_RADIUS_IN_CELLS;

    return in_x_range && in_y_range && in_z_range;
}

inline bool grid_coordinate_is_valid(GridCoordinate grid_coordinate)
{
    const bool in_x_range = grid_coordinate[0] >= -WORLD_RADIUS_IN_CELLS && grid_coordinate[0] <= WORLD_RADIUS_IN_CELLS;
    const bool in_y_range = grid_coordinate[1] >= -WORLD_RADIUS_IN_CELLS && grid_coordinate[1] <= WORLD_RADIUS_IN_CELLS;
    const bool in_z_range = grid_coordinate[2] >= -WORLD_RADIUS_IN_CELLS && grid_coordinate[2] <= WORLD_RADIUS_IN_CELLS;

    return in_x_range && in_y_range && in_z_range;
}

inline void grid_coordinate_to_world_position(GridCoordinate grid_coordinate, vec3 out_world_position)
{
    out_world_position[0] = (float)grid_coordinate[0] * CELL_SIZE;
    out_world_position[1] = (float)grid_coordinate[1] * CELL_SIZE;
    out_world_position[2] = (float)grid_coordinate[2] * CELL_SIZE;
}

inline void sector_index_to_sector_coordinate(SectorIndex sector_index, SectorCoordinate out_sector_coordinate)
{
    out_sector_coordinate[0] = sector_index % WORLD_SIZE_IN_SECTORS;
    out_sector_coordinate[1] = (sector_index / WORLD_SIZE_IN_SECTORS) % WORLD_SIZE_IN_SECTORS;
    out_sector_coordinate[2] = sector_index / WORLD_AREA_IN_SECTORS;

    glm_ivec3_subs(out_sector_coordinate, WORLD_RADIUS_IN_SECTORS, out_sector_coordinate);
}

inline SectorIndex sector_coordinate_to_sector_index(SectorCoordinate sector_coordinate)
{
    SectorCoordinate sector_coordinate_indexable;
    glm_ivec3_adds(sector_coordinate, WORLD_RADIUS_IN_SECTORS, sector_coordinate_indexable);

    SectorIndex out_sector_index = 
        sector_coordinate_indexable[0] +
        sector_coordinate_indexable[1] * WORLD_SIZE_IN_SECTORS +
        sector_coordinate_indexable[2] * WORLD_AREA_IN_SECTORS;

    return out_sector_index;
}

inline void sector_coordinate_to_grid_coordinate(SectorCoordinate sector_coordinate, GridCoordinate out_grid_coordinate)
{
    glm_ivec3_scale(sector_coordinate, SECTOR_SIZE_IN_CELLS, out_grid_coordinate);
}

inline void cell_index_to_cell_coordinate(CellIndex cell_index, CellCoordinate out_cell_coordinate)
{
    out_cell_coordinate[0] = cell_index % SECTOR_SIZE_IN_CELLS;
    out_cell_coordinate[1] = (cell_index / SECTOR_SIZE_IN_CELLS) % SECTOR_SIZE_IN_CELLS;
    out_cell_coordinate[2] = cell_index / SECTOR_AREA_IN_CELLS;

    glm_ivec3_subs(out_cell_coordinate, SECTOR_RADIUS_IN_CELLS, out_cell_coordinate);
}

inline CellIndex cell_coordinate_to_cell_index(CellCoordinate cell_coordinate)
{
    CellCoordinate cell_coordinate_indexable;
    glm_ivec3_adds(cell_coordinate, SECTOR_RADIUS_IN_CELLS, cell_coordinate_indexable);

    CellIndex out_cell_index = 
        cell_coordinate_indexable[0] +
        cell_coordinate_indexable[1] * SECTOR_SIZE_IN_CELLS +
        cell_coordinate_indexable[2] * SECTOR_AREA_IN_CELLS;

    return out_cell_index;
}

inline void grid_coordinate_to_sector_coordinate(GridCoordinate grid_coordinate, SectorCoordinate out_sector_coordinate)
{
    GridCoordinate grid_coordinate_indexable;
    glm_ivec3_adds(grid_coordinate, WORLD_RADIUS_IN_CELLS, grid_coordinate_indexable);

    SectorCoordinate sector_coordinate_indexable;
    glm_ivec3_divs(grid_coordinate_indexable, SECTOR_SIZE_IN_CELLS, sector_coordinate_indexable);

    glm_ivec3_subs(sector_coordinate_indexable, WORLD_RADIUS_IN_SECTORS, out_sector_coordinate);
}

inline void grid_coordinate_to_cell_coordinate(GridCoordinate grid_coordinate, CellCoordinate out_cell_coordinate)
{
    GridCoordinate grid_coordinate_indexable;
    glm_ivec3_adds(grid_coordinate, WORLD_RADIUS_IN_CELLS, grid_coordinate_indexable);

    CellCoordinate cell_coordinate_indexable;
    cell_coordinate_indexable[0] = grid_coordinate_indexable[0] % SECTOR_SIZE_IN_CELLS;
    cell_coordinate_indexable[1] = grid_coordinate_indexable[1] % SECTOR_SIZE_IN_CELLS;
    cell_coordinate_indexable[2] = grid_coordinate_indexable[2] % SECTOR_SIZE_IN_CELLS;

    glm_ivec3_subs(cell_coordinate_indexable, SECTOR_RADIUS_IN_CELLS, out_cell_coordinate);
}

inline SectorIndex grid_coordinate_to_sector_index(GridCoordinate grid_coordinate)
{
    SectorCoordinate sector_coordinate;
    grid_coordinate_to_sector_coordinate(grid_coordinate, sector_coordinate);

    return sector_coordinate_to_sector_index(sector_coordinate);
}

inline CellIndex grid_coordinate_to_cell_index(GridCoordinate grid_coordinate)
{
    CellCoordinate cell_coordinate;
    grid_coordinate_to_cell_coordinate(grid_coordinate, cell_coordinate);

    return cell_coordinate_to_cell_index(cell_coordinate);
}

inline void indices_to_grid_coordinate(SectorIndex sector_index, CellIndex cell_index, GridCoordinate out_grid_coordinate)
{
    SectorCoordinate sector_coordinate;
    CellCoordinate cell_coordinate;

    sector_index_to_sector_coordinate(sector_index, sector_coordinate);
    cell_index_to_cell_coordinate(cell_index, cell_coordinate);

    glm_ivec3_scale(sector_coordinate, SECTOR_SIZE_IN_CELLS, out_grid_coordinate);
    glm_ivec3_add(out_grid_coordinate, cell_coordinate, out_grid_coordinate);
}