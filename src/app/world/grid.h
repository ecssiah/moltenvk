#ifndef GRID_H
#define GRID_H 1

#include <cglm/cglm.h>

#include "core/types.h"

typedef u32 SectorIndex;
typedef ivec3 SectorCoordinate;

typedef u32 CellIndex;
typedef ivec3 CellCoordinate;

typedef ivec3 GridCoordinate;

#define WORLD_RADIUS_IN_SECTORS     2
#define SECTOR_RADIUS_IN_CELLS      2
#define CELL_RADIUS                 0.5f

static u32 get_world_size_in_sectors()
{
    return 2 * WORLD_RADIUS_IN_SECTORS + 1;
}

static u32 get_world_area_in_sectors()
{
    u32 world_size_in_sectors = get_world_size_in_sectors();

    return world_size_in_sectors * world_size_in_sectors;
}

static u32 get_world_volume_in_sectors()
{
    u32 world_size_in_sectors = get_world_size_in_sectors();

    return world_size_in_sectors * world_size_in_sectors * world_size_in_sectors;
}

static u32 get_sector_size_in_cells()
{
    return 2 * SECTOR_RADIUS_IN_CELLS + 1;
}

static u32 get_sector_area_in_cells()
{
    u32 sector_size_in_cells = get_sector_size_in_cells();

    return sector_size_in_cells * sector_size_in_cells;
}

static u32 get_sector_volume_in_cells()
{
    u32 sector_size_in_cells = get_sector_area_in_cells();

    return sector_size_in_cells * sector_size_in_cells * sector_size_in_cells;
}

static u32 get_world_radius_in_cells()
{
    return WORLD_RADIUS_IN_SECTORS * get_sector_size_in_cells() + SECTOR_RADIUS_IN_CELLS;
}

static float get_cell_size()
{
    return 2.0f * CELL_RADIUS;
}                   

static float get_cell_area()
{
    float cell_size = get_cell_size();

    return cell_size * cell_size;
}

static float get_cell_volume()
{
    float cell_size = get_cell_size();

    return cell_size * cell_size * cell_size;
}

static boolean sector_index_is_valid(SectorIndex sector_index)
{
    return sector_index < get_world_volume_in_sectors();
}

static boolean cell_index_is_valid(CellIndex cell_index)
{
    return cell_index < get_sector_volume_in_cells();
}

static boolean sector_coordinate_is_valid(SectorCoordinate sector_coordinate)
{
    boolean in_x_range = sector_coordinate[0] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[0] <= WORLD_RADIUS_IN_SECTORS;
    boolean in_y_range = sector_coordinate[1] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[1] <= WORLD_RADIUS_IN_SECTORS;
    boolean in_z_range = sector_coordinate[2] >= -WORLD_RADIUS_IN_SECTORS && sector_coordinate[2] <= WORLD_RADIUS_IN_SECTORS;

    return in_x_range && in_y_range && in_z_range;
}

static boolean cell_coordinate_is_valid(CellCoordinate cell_coordinate)
{
    boolean in_x_range = cell_coordinate[0] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[0] <= SECTOR_RADIUS_IN_CELLS;
    boolean in_y_range = cell_coordinate[1] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[1] <= SECTOR_RADIUS_IN_CELLS;
    boolean in_z_range = cell_coordinate[2] >= -SECTOR_RADIUS_IN_CELLS && cell_coordinate[2] <= SECTOR_RADIUS_IN_CELLS;

    return in_x_range && in_y_range && in_z_range;
}

static boolean grid_coordinate_is_valid(GridCoordinate grid_coordinate)
{
    u32 world_radius_in_cells = get_world_radius_in_cells();

    boolean in_x_range = grid_coordinate[0] >= -world_radius_in_cells && grid_coordinate[0] <= world_radius_in_cells;
    boolean in_y_range = grid_coordinate[1] >= -world_radius_in_cells && grid_coordinate[1] <= world_radius_in_cells;
    boolean in_z_range = grid_coordinate[2] >= -world_radius_in_cells && grid_coordinate[2] <= world_radius_in_cells;

    return in_x_range && in_y_range && in_z_range;
}

static void grid_coordinate_to_world_position(GridCoordinate grid_coordinate, vec3 out_world_position)
{
    float cell_size = get_cell_size();

    out_world_position[0] = (float)grid_coordinate[0] * cell_size;
    out_world_position[1] = (float)grid_coordinate[1] * cell_size;
    out_world_position[2] = (float)grid_coordinate[2] * cell_size;
}

static void sector_index_to_sector_coordinate(SectorIndex sector_index, SectorCoordinate out_sector_coordinate)
{
    u32 world_area_in_sectors = get_world_area_in_sectors();
    u32 world_size_in_sectors = get_world_size_in_sectors();

    out_sector_coordinate[0] = sector_index % world_size_in_sectors;
    out_sector_coordinate[1] = (sector_index / world_size_in_sectors) % world_size_in_sectors;
    out_sector_coordinate[2] = sector_index / world_area_in_sectors;

    glm_ivec3_subs(out_sector_coordinate, WORLD_RADIUS_IN_SECTORS, out_sector_coordinate);
}

static SectorIndex sector_coordinate_to_sector_index(SectorCoordinate sector_coordinate)
{
    SectorCoordinate sector_coordinate_indexable;
    glm_ivec3_adds(sector_coordinate, WORLD_RADIUS_IN_SECTORS, sector_coordinate_indexable);

    SectorIndex out_sector_index = 
        sector_coordinate_indexable[0] +
        sector_coordinate_indexable[1] * get_world_size_in_sectors() +
        sector_coordinate_indexable[2] * get_world_area_in_sectors();

    return out_sector_index;
}

static void sector_coordinate_to_grid_coordinate(SectorCoordinate sector_coordinate, GridCoordinate out_grid_coordinate)
{
    glm_ivec3_scale(sector_coordinate, get_sector_size_in_cells(), out_grid_coordinate);
}

static void cell_index_to_cell_coordinate(CellIndex cell_index, CellCoordinate out_cell_coordinate)
{
    u32 sector_area_in_cells = get_sector_area_in_cells();
    u32 sector_size_in_cells = get_sector_size_in_cells();

    out_cell_coordinate[0] = cell_index % sector_size_in_cells;
    out_cell_coordinate[1] = (cell_index / sector_size_in_cells) % sector_size_in_cells;
    out_cell_coordinate[2] = cell_index / sector_area_in_cells;

    glm_ivec3_subs(out_cell_coordinate, SECTOR_RADIUS_IN_CELLS, out_cell_coordinate);
}

static CellIndex cell_coordinate_to_cell_index(CellCoordinate cell_coordinate)
{
    CellCoordinate cell_coordinate_indexable;
    glm_ivec3_adds(cell_coordinate, SECTOR_RADIUS_IN_CELLS, cell_coordinate_indexable);

    CellIndex out_cell_index = 
        cell_coordinate_indexable[0] +
        cell_coordinate_indexable[1] * get_sector_size_in_cells() +
        cell_coordinate_indexable[2] * get_sector_area_in_cells();

    return out_cell_index;
}

static void grid_coordinate_to_sector_coordinate(GridCoordinate grid_coordinate, SectorCoordinate out_sector_coordinate)
{
    GridCoordinate grid_coordinate_indexable;
    glm_ivec3_adds(grid_coordinate, get_world_radius_in_cells(), grid_coordinate_indexable);

    SectorCoordinate sector_coordinate_indexable;
    glm_ivec3_divs(grid_coordinate_indexable,get_sector_size_in_cells(), sector_coordinate_indexable);

    glm_ivec3_subs(sector_coordinate_indexable, WORLD_RADIUS_IN_SECTORS, out_sector_coordinate);
}

static void grid_coordinate_to_cell_coordinate(GridCoordinate grid_coordinate, CellCoordinate out_cell_coordinate)
{
    u32 sector_size_in_cells = get_sector_size_in_cells();

    GridCoordinate grid_coordinate_indexable;
    glm_ivec3_adds(grid_coordinate, get_world_radius_in_cells(), grid_coordinate_indexable);

    CellCoordinate cell_coordinate_indexable;
    cell_coordinate_indexable[0] = grid_coordinate_indexable[0] % sector_size_in_cells;
    cell_coordinate_indexable[1] = grid_coordinate_indexable[1] % sector_size_in_cells;
    cell_coordinate_indexable[2] = grid_coordinate_indexable[2] % sector_size_in_cells;

    glm_ivec3_subs(cell_coordinate_indexable, SECTOR_RADIUS_IN_CELLS, out_cell_coordinate);
}

static SectorIndex grid_coordinate_to_sector_index(GridCoordinate grid_coordinate)
{
    SectorCoordinate sector_coordinate;
    grid_coordinate_to_sector_coordinate(grid_coordinate, sector_coordinate);

    return sector_coordinate_to_sector_index(sector_coordinate);
}

static CellIndex grid_coordinate_to_cell_index(GridCoordinate grid_coordinate)
{
    CellCoordinate cell_coordinate;
    grid_coordinate_to_cell_coordinate(grid_coordinate, cell_coordinate);

    return cell_coordinate_to_cell_index(cell_coordinate);
}

static void indices_to_grid_coordinate(SectorIndex sector_index, CellIndex cell_index, GridCoordinate out_grid_coordinate)
{
    SectorCoordinate sector_coordinate;
    CellCoordinate cell_coordinate;

    sector_index_to_sector_coordinate(sector_index, sector_coordinate);
    cell_index_to_cell_coordinate(cell_index, cell_coordinate);

    glm_ivec3_scale(sector_coordinate, get_sector_size_in_cells(), out_grid_coordinate);
    glm_ivec3_add(out_grid_coordinate, cell_coordinate, out_grid_coordinate);
}

#endif