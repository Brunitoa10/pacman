#include "core/maze.h"

#include <string.h>

static const char *const CLASSIC_LAYOUT[MAZE_ROWS] = {
    "############################",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#o####.#####.##.#####.####o#",
    "#.####.#####.##.#####.####.#",
    "#..........................#",
    "#.####.##.########.##.####.#",
    "#.####.##.########.##.####.#",
    "#......##....##....##......#",
    "######.##### ## #####.######",
    "     #.##### ## #####.#     ",
    "     #.##    0     ##.#     ",
    "     #.## ###--### ##.#     ",
    "######.## #      # ##.######",
    "      .   #2 1 3 #   .      ",
    "######.## #      # ##.######",
    "     #.## ######## ##.#     ",
    "     #.##          ##.#     ",
    "     #.## ######## ##.#     ",
    "######.## ######## ##.######",
    "#............##............#",
    "#.####.#####.##.#####.####.#",
    "#.####.#####.##.#####.####.#",
    "#o..##.......P .......##..o#",
    "###.##.##.########.##.##.###",
    "###.##.##.########.##.##.###",
    "#......##....##....##......#",
    "#.##########.##.##########.#",
    "#.##########.##.##########.#",
    "#..........................#",
    "############################",
};

static int wrap_col(int col)
{
    return ((col % MAZE_COLS) + MAZE_COLS) % MAZE_COLS;
}

static bool is_ghost_marker(char ch)
{
    return ch >= '0' && ch < '0' + MAZE_MAX_GHOST_SPAWNS;
}

static Tile parse_cell(Maze *maze, char ch, Vec2 pos)
{
    if (is_ghost_marker(ch)) {
        maze->ghost_spawns[ch - '0'] = pos;
        return TILE_EMPTY;
    }
    switch (ch) {
    case '#': return TILE_WALL;
    case '-': return TILE_DOOR;
    case '.': maze->pellets_left++; return TILE_PELLET;
    case 'o': maze->pellets_left++; return TILE_POWER;
    case 'P': maze->pacman_spawn = pos; return TILE_EMPTY;
    default:  return TILE_EMPTY;
    }
}

static bool locate_house(Maze *maze)
{
    for (int r = 0; r < MAZE_ROWS; ++r) {
        for (int c = 0; c < MAZE_COLS; ++c) {
            if (maze->tiles[r][c] == TILE_DOOR) {
                maze->house_exit = vec2_make(r - 1, c);
                maze->house_center = vec2_make(r + 2, c);
                return true;
            }
        }
    }
    return false;
}

static void detect_tunnels(Maze *maze)
{
    for (int r = 0; r < MAZE_ROWS; ++r) {
        maze->tunnel_rows[r] = maze->tiles[r][0] != TILE_WALL
                            && maze->tiles[r][MAZE_COLS - 1] != TILE_WALL;
    }
}

bool maze_load(Maze *maze, const char *const layout[MAZE_ROWS])
{
    memset(maze, 0, sizeof *maze);

    for (int r = 0; r < MAZE_ROWS; ++r) {
        if (strlen(layout[r]) != MAZE_COLS) {
            return false;
        }
        for (int c = 0; c < MAZE_COLS; ++c) {
            maze->tiles[r][c] = parse_cell(maze, layout[r][c], vec2_make(r, c));
        }
    }

    detect_tunnels(maze);
    return locate_house(maze);
}

void maze_load_classic(Maze *maze)
{
    (void)maze_load(maze, CLASSIC_LAYOUT);
}

Tile maze_tile_at(const Maze *maze, Vec2 pos)
{
    if (pos.row < 0 || pos.row >= MAZE_ROWS) {
        return TILE_WALL;
    }
    return maze->tiles[pos.row][wrap_col(pos.col)];
}

bool maze_is_walkable(const Maze *maze, Vec2 pos, bool can_pass_door)
{
    const Tile tile = maze_tile_at(maze, pos);
    if (tile == TILE_WALL) {
        return false;
    }
    return tile != TILE_DOOR || can_pass_door;
}

bool maze_is_tunnel(const Maze *maze, Vec2 pos)
{
    if (pos.row < 0 || pos.row >= MAZE_ROWS || !maze->tunnel_rows[pos.row]) {
        return false;
    }
    return pos.col < TUNNEL_LENGTH || pos.col >= MAZE_COLS - TUNNEL_LENGTH;
}

Vec2 maze_neighbor(Vec2 pos, Direction dir)
{
    Vec2 next = vec2_add(pos, direction_to_vec(dir));
    next.col = wrap_col(next.col);
    return next;
}

Tile maze_consume(Maze *maze, Vec2 pos)
{
    const Tile tile = maze_tile_at(maze, pos);
    if (tile != TILE_PELLET && tile != TILE_POWER) {
        return TILE_EMPTY;
    }
    maze->tiles[pos.row][wrap_col(pos.col)] = TILE_EMPTY;
    maze->pellets_left--;
    return tile;
}

int maze_pellets_left(const Maze *maze)
{
    return maze->pellets_left;
}
