#include "p4.hpp"

void gameInit(Puissance4_t *game) {
    for (int lin = LIN_MAX - 1; lin != -1; lin--) {
        for (int col = 0; col != COL_MAX; col++) {
            game->grid[col * 6 + lin] = FREE;
        }
    }

    game->turn = 0;
    game->player = ROUGE;
}

void gameShow(Puissance4_t *game) {
    for (int lin = LIN_MAX - 1; lin != -1; lin--) {
        printf("+---+---+---+---+---+---+---+\n");
        for (int col = 0; col != COL_MAX; col++) {
            switch (game->grid[col * 6 + lin]) {
            case ROUGE:
                printf("| X ");
                break;

            case JAUNE:
                printf("| O ");
                break;

            case FREE:
                printf("|   ");
                break;

            default:
                printf("| E ");
                break;
            }
        }
        printf("|\n");
    }
    printf("+---+---+---+---+---+---+---+\n");
}

string gameShowToString(array<uint8_t, (COL_MAX * LIN_MAX)> grid) {
    string output = string();

    for (int lin = LIN_MAX - 1; lin != -1; lin--) {
        output.append("+---+---+---+---+---+---+---+\n");
        for (int col = 0; col != COL_MAX; col++) {
            switch (grid[col * 6 + lin]) {
            case ROUGE:
                output.append("| X ");
                break;

            case JAUNE:
                output.append("| O ");
                break;

            case FREE:
                output.append("|   ");
                break;

            default:
                output.append("| E ");
                break;
            }
        }
        output.append("|\n");
    }
    output.append("+---+---+---+---+---+---+---+\n");

    return output;
}


int gameTurn(Puissance4_t *game, int col) {
    int move;
    int state;

    if ((col >= 0 && col < COL_MAX) && (move = testValidity(col, game)) > 0)
        game->grid[move] = game->player;
    else {
        return -1;
    }

    state = testWin(move, game);

    if (state == RUNNING) {
        game->player = (game->player == ROUGE) ? JAUNE : ROUGE;
        game->turn++;
    }

    return state;
}

void gameEngine(Puissance4_t *game) {
    gameInit(game);

    gameShow(game);

    if (game->player == ROUGE)
        printf("Player Cross won!\n");
    else
        printf("Player Circle won!\n");
}

int testValidity(int col, Puissance4_t *game) {
    for (int lin = 0; lin < LIN_MAX; lin++) {
        if (game->grid[col * 6 + lin] == FREE) {
            return col * 6 + lin;
        }
    }

    return -1;
}

int testWin(int move, Puissance4_t *game) {
    int polarity[2] = {1, -1};
    int direction[NB_DIR] = {1, 6, 5, 7};

    int successives;
    int test_move;
    int dir;

    for (size_t i = 0; i < NB_DIR; i++) {
        for (size_t j = 0; j < 2; j++) {
            successives = 0;
            dir = direction[i] * polarity[j];
            test_move = move + dir;

            if (game->grid[test_move] == game->player)
                successives += testDir(test_move, dir, game) + 1;
            if (successives >= 4) {
                return WIN;
            }
        }
    }

    // a verifier
    if (game->turn == 42) {
        return DRAW;
    }

    return RUNNING;
}

int testDir(int move, int direction, Puissance4_t *game) {
    if (game->grid[move + direction] == game->player)
        return testDir(move + direction, direction, game) + 1;
    return false;
}
