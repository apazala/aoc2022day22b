#include <stdio.h>
#include <stdlib.h>

#define INPUT_FILENAME "input.txt"

typedef struct tile
{
    int row;
    int col;
    int wall;
    int zone;
    struct tile *neighbors[4];
} tile;

tile *tilesMap[256][256];
int nrows;
int ncols;

tile *getTile(int row, int col)
{
    tile *thisTile = tilesMap[row][col];
    if (!thisTile)
    {
        thisTile = (tile *)calloc(1, sizeof(tile));
        thisTile->col = col;
        thisTile->row = row;
        tilesMap[row][col] = thisTile;
    }
    return thisTile;
}

typedef enum
{
    RIGHT,
    DOWN,
    LEFT,
    UP,
    NDIRECTIONS
} DIRECTION;

typedef struct player
{
    tile *position;
    DIRECTION dir;
} player;

char line[6000];

int loadInputFile(char *filename);
int nextLine(char *dst);

DIRECTION newDir[7][7];

void readBoard()
{
    for (int len = nextLine(line); len > 1; len = nextLine(line))
    {
        nrows++;
        if (len > ncols)
            ncols = len;

        int j = 0;
        while (line[j] < '#') // empty tile
            j++;
        tile *currTile = getTile(nrows, j + 1);
        currTile->wall = (line[j] == '#');
        tile *prevTile;

        for (j++; j < len && line[j] >= '#'; j++)
        {
            prevTile = currTile;
            currTile = getTile(nrows, j + 1);
            currTile->wall = (line[j] == '#');
            currTile->neighbors[LEFT] = prevTile;
            prevTile->neighbors[RIGHT] = currTile;
        }
    }

    for (int j = 1; j <= ncols; j++)
    {
        int i = 1;
        tile *currTile;
        while (!(currTile = tilesMap[i][j]) && i <= nrows)
            i++;
        if (i > nrows)
            continue; // empty column

        tile *prevTile;
        for (i++; tilesMap[i][j]; i++)
        {
            prevTile = currTile;
            currTile = tilesMap[i][j];
            currTile->neighbors[UP] = prevTile;
            prevTile->neighbors[DOWN] = currTile;
        }
    }

    // zone tag
    /*   
       [2][1]
       [3]
    [5][4]
    [6]
    */
    for (int r = 1; r <= 50; r++)
    {
        for (int c = 101; c <= 150; c++)
        {
            tilesMap[r][c]->zone = 1;
        }
    }

    for (int r = 1; r <= 50; r++)
    {
        for (int c = 51; c <= 100; c++)
        {
            tilesMap[r][c]->zone = 2;
        }
    }

    for (int r = 51; r <= 100; r++)
    {
        for (int c = 51; c <= 100; c++)
        {
            tilesMap[r][c]->zone = 3;
        }
    }

    for (int r = 101; r <= 150; r++)
    {
        for (int c = 51; c <= 100; c++)
        {
            tilesMap[r][c]->zone = 4;
        }
    }

    for (int r = 101; r <= 150; r++)
    {
        for (int c = 1; c <= 50; c++)
        {
            tilesMap[r][c]->zone = 5;
        }
    }

    for (int r = 151; r <= 200; r++)
    {
        for (int c = 1; c <= 50; c++)
        {
            tilesMap[r][c]->zone = 6;
        }
    }

    // zone wrap
    newDir[1][2] = LEFT;
    newDir[2][1] = RIGHT;
    newDir[2][3] = DOWN;
    newDir[3][2] = UP;
    newDir[3][4] = DOWN;
    newDir[4][3] = UP;
    newDir[4][5] = LEFT;
    newDir[5][4] = RIGHT;
    newDir[5][6] = DOWN;
    newDir[6][5] = UP;
    // 1->3
    for (int c = 101; c <= 150; c++)
    {
        tilesMap[50][c]->neighbors[DOWN] = tilesMap[c-50][100];
        tilesMap[c-50][100]->neighbors[RIGHT] = tilesMap[50][c];
        newDir[1][3] = LEFT;
        newDir[3][1] = UP;
    }

    //1->4
    for (int r = 1; r <= 50; r++)
    {
        tilesMap[r][150]->neighbors[RIGHT] = tilesMap[151-r][100];
        tilesMap[151-r][100]->neighbors[RIGHT] = tilesMap[r][150];
        newDir[1][4] = LEFT;
        newDir[4][1] = LEFT;
    }

    //1->6    
    for (int c = 101; c <= 150; c++)
    {
        tilesMap[1][c]->neighbors[UP] = tilesMap[200][c-100];
        tilesMap[200][c-100]->neighbors[DOWN] = tilesMap[1][c];
        newDir[1][6] = UP;
        newDir[6][1] = DOWN;
    }

    //2->6
    for(int c = 51; c <= 100; c++)
    {
        tilesMap[1][c]->neighbors[UP] = tilesMap[c+100][1];
        tilesMap[c+100][1]->neighbors[LEFT] = tilesMap[1][c];
        newDir[2][6] = RIGHT;
        newDir[6][2] = DOWN;
    }

    //2->5
    for(int r = 1; r <= 50; r++)
    {
        tilesMap[r][51]->neighbors[LEFT] = tilesMap[151-r][1];
        tilesMap[151-r][1]->neighbors[LEFT] = tilesMap[r][51];
        newDir[2][5] = RIGHT;
        newDir[5][2] = RIGHT;
    }

    //3->5
    for(int r = 51; r <= 100; r++)
    {
        tilesMap[r][51]->neighbors[LEFT] = tilesMap[101][r-50];
        tilesMap[101][r-50]->neighbors[UP] = tilesMap[r][51];
        newDir[3][5] = DOWN;
        newDir[5][3] = RIGHT;
    }

    //4->6
    for(int c = 51; c <= 100; c++)
    {
        tilesMap[150][c]->neighbors[DOWN] = tilesMap[100+c][50];
        tilesMap[100+c][50]->neighbors[RIGHT] = tilesMap[150][c];
        newDir[4][6] = LEFT;
        newDir[6][4] = UP;
    }
    
}

void forward(player *myPlayer, int n)
{
    tile *nextTile = myPlayer->position->neighbors[myPlayer->dir];
    for (int k = 0; k < n && !nextTile->wall; k++)
    {
        if(nextTile->zone!=myPlayer->position->zone){
            myPlayer->dir = newDir[myPlayer->position->zone][nextTile->zone];
        }
        myPlayer->position = nextTile;        
        nextTile = myPlayer->position->neighbors[myPlayer->dir];
    }
}

int main()
{
    player myPlayer = {NULL, RIGHT};

    if (loadInputFile(INPUT_FILENAME) < 0)
    {
        fprintf(stderr, "Can't open input file:%s\n", INPUT_FILENAME);
        return 1;
    }

    readBoard();

    for (int j = 1; myPlayer.position == NULL; j++)
    {
        if (tilesMap[1][j] && !tilesMap[1][j]->wall)
            myPlayer.position = tilesMap[1][j];
    }

    int len = nextLine(line);
    int i = 0;
    while (i < len)
    {
        switch (line[i])
        {
        case 'R':
            myPlayer.dir = (myPlayer.dir + 1) % NDIRECTIONS;
            i++;
            break;

        case 'L':
            myPlayer.dir = (myPlayer.dir - 1 + NDIRECTIONS) % NDIRECTIONS;
            i++;

        default:
            int n = 0;
            while (i < len && line[i] <= '9')
            {
                n = 10 * n + (line[i] - '0');
                i++;
            }

            forward(&myPlayer, n);
        }
    }

    int solution = 1000 * myPlayer.position->row + 4 * myPlayer.position->col + myPlayer.dir;

    printf("%d\n", solution);
}

#define BUFSIZE 26000

char fileBuf[BUFSIZE];
char *pbuf = fileBuf;
char *pbufend = fileBuf;

int loadInputFile(char *filename)
{
    FILE *pf = fopen(filename, "r");
    if (!pf)
        return -1;
    pbuf = fileBuf;
    pbufend = fileBuf + fread(fileBuf, 1, BUFSIZE, pf);
    fclose(pf);

    return 0;
}

int nextLine(char *dst)
{
    char c;
    char *orig = dst;

    // read line:
    while (pbuf < pbufend && (c = *(pbuf++)) != '\n')
    {
        *(dst++) = c;
    }
    *dst = '\0';
    // return line length
    return dst - orig;
}