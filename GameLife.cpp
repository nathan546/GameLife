//Nathan Barrett
//September 23, 2013

//g++ -o GameLife GameLife.cpp

//You can ignore any deprecated compilation warnings
//The gif library being used is C-based, so it isn't using strings

//This program outputs John Conway's Game of Life to a terminal using
//a statically allocated initial configuration.

//Another feature was added to also output the game to an animated GIF image
//Most browser's can display animated GIFs.  Give it a try.  The terminal
//will begin by asking if you want to create a GIF over a number of cycles of the game.
//The GIF will then be created in the running directory as "output.gif"

#include<iostream>
#include<vector>
#include<ctime>

//The GIF LZW compression was performed using a GIF library
//Available from http://giflib.sourceforge.net/
#include "./giflib-5.0.0/lib/gif_hash.h"
#include "./giflib-5.0.0/lib/gif_hash.c"
#include "./giflib-5.0.0/lib/gifalloc.c"
#include "./giflib-5.0.0/lib/egif_lib.c"

#include<unistd.h> // linux

#define ROWS 50
#define COLS 50

#define DEAD  ' '
#define ALIVE '*'

#define GIFMULTIPLIER 12       //Size of GIF will be ROWS*GIFMULTIPLIER+GRIDWIDTH by COLS*GIFMULTIPLIER+GRIDWIDTH
#define GRIDWIDTH     3        //The grid in the GIF will be GRIDWIDTH pixels wide   ex. 50*12+3 x 50*12+3 = 603x603

using namespace std;

class Square{
    public:
        char status;
        unsigned char color;
};


//Global variables for GIF usage
GifFileType * file;
int useGif = 0;
int gifCycles = 0;

//This functions creates the GIF output file and initializes the headers
void initGif();

//This function closes the GIF output file
void closeGif();

//This function returns the number of neighbors around a node
int how_many_monsters(int row, int col, int radius, vector< vector<Square> > &world);

//This function propagates the next move of the game's grid
//Variables: world is the game's grid
//           world_copy is a copy of the grid used for decision making.
void generation(vector< vector<Square> > &world, vector< vector<Square> > &world_copy);

//This function displays the game's grid on the terminal and to the output GIF
//Variables: world is the game's grid
void display(vector< vector<Square> > &world);

//Set up the game
int main()
{
    srand(time(0));

    vector< vector<Square> > world(ROWS, vector<Square>(COLS));
    vector< vector<Square> > copy(ROWS, vector<Square>(COLS));

    //Randomize colors/statuses
    for(int i = 0; i < ROWS; i++){
        for(int j = 0; j < COLS; j++){
            world[i][j].color = (rand()%254)+2;
            world[i][j].status = DEAD;
            //Uncomment to randomize alive/dead
            //world[i][j].status = ((rand()%2) == 0) ? ALIVE:DEAD;
        }
    }

    //Block
    world[2][2].status = world[2][3].status = world[3][3].status = world[3][2].status = ALIVE;

    //Beehive
    world[2][10].status = world[2][11].status = world[3][9].status = world[3][12].status = ALIVE;
    world[4][10].status = world[4][11].status = ALIVE;

    //Loaf
    world[2][20].status = world[2][21].status = world[3][19].status = world[3][22].status = ALIVE;
    world[4][20].status = world[4][22].status = world[5][21].status = ALIVE;

    //Boat
    world[2][30].status = world[2][31].status = world[3][30].status = world[3][32].status = ALIVE;
    world[4][31].status = ALIVE;

    //Blinker
    world[2][45].status = world[2][46].status = world[2][47].status = ALIVE;

    //Toad
    world[15][45].status = world[15][46].status = world[15][47].status = ALIVE;
    world[16][44].status = world[16][45].status = world[16][46].status = ALIVE;

    //Beacon
    world[15][30].status = world[15][31].status = world[16][30].status = world[16][31].status = ALIVE;
    world[17][32].status = world[17][33].status = world[18][32].status = world[18][33].status = ALIVE;

    //Pulsar
    world[25][15].status = world[25][16].status = world[25][17].status = world[25][21].status = ALIVE;
    world[25][22].status = world[25][23].status = world[30][22].status = world[30][23].status = ALIVE;
    world[27][13].status = world[27][18].status = world[27][20].status = world[27][25].status = ALIVE;
    world[28][13].status = world[28][18].status = world[28][20].status = world[28][25].status = ALIVE;
    world[29][13].status = world[29][18].status = world[29][20].status = world[29][25].status = ALIVE;
    world[30][15].status = world[30][16].status = world[30][17].status = world[30][21].status = ALIVE;
    world[32][15].status = world[32][16].status = world[32][17].status = world[32][21].status = ALIVE;
    world[32][22].status = world[32][23].status = world[37][22].status = world[37][23].status = ALIVE;
    world[33][13].status = world[33][18].status = world[33][20].status = world[33][25].status = ALIVE;
    world[34][13].status = world[34][18].status = world[34][20].status = world[34][25].status = ALIVE;
    world[35][13].status = world[35][18].status = world[35][20].status = world[35][25].status = ALIVE;
    world[37][15].status = world[37][16].status = world[37][17].status = world[37][21].status = ALIVE;

    //Glider
    world[15][15].status = world[16][16].status = world[17][14].status = world[17][15].status = ALIVE;
    world[17][16].status = ALIVE;

    //Blinker
    world[15][0].status = world[15][1].status = world[15][2].status = ALIVE;

    //Lightweight Spaceship
    world[25][40].status = world[25][41].status = world[25][42].status = world[25][43].status = ALIVE;
    world[26][39].status = world[26][43].status = world[27][43].status = world[28][42].status = ALIVE;
    world[28][39].status = ALIVE;

    //F-pentomino
    world[40][40].status = world[40][41].status = world[41][40].status = world[41][39].status = ALIVE;
    world[42][40].status = ALIVE;

    cout << "Would you also like to output to an image, output.gif (1 = yes)? ";
    cin >> useGif;
    if(useGif){
        cout << "How many cycles do you want to record? ";
        cin >> gifCycles;
        initGif();
    }

    while(1)
    {
        //Clear screen, display world, and wait for some time
        system("clear");
        display(world);
        usleep(800000);

        //Update the world
        generation(world, copy);
    }

    //Good practice
    world.clear();
    copy.clear();

    return 0;
}

int how_many_monsters(int row, int col, int radius, vector< vector<Square> > &world){

    //Start at top left node
    int furthestLeft = col-radius;
    int furthestTop = row-radius;

    int count = 0;

    //Don't count node itself
    if(world[row][col].status == ALIVE){
        count = -1;
    }

    //Count cols and rows
    for(int i = 0; i < radius*2+1; i++){
        row = furthestTop+i;
        if(row >= 0 && row < ROWS){
            for(int j = 0; j < radius*2+1; j++){
                    col = furthestLeft+j;
                    if(col >= 0 && col < COLS){
                        if(world[row][col].status == ALIVE){
                            count++;
                        }
                    }
            }
        }
    }

    //Return count
    return count;
}

//See prototype
void generation(vector< vector<Square> > &world,
                vector< vector<Square> > &world_copy)
{

    // copy the contents of world into world_copy
    for(int i = 0; i < ROWS; i++) {
        for(int j = 0; j < COLS; j++) {

                //Count how many neighbors a cell has
                int neighbours = how_many_monsters(i,j,1,world);

                //Dies of loneliness or overcrowding
                if(world[i][j].status == ALIVE){
                    if(neighbours <= 1 || neighbours > 3){
                        world_copy[i][j].status = DEAD;
                    }else{
                        world_copy[i][j].status = world[i][j].status;
                    }
                }else{ //Birth
                    if(neighbours == 3){
                        world_copy[i][j].status = ALIVE;
                    }else{
                        world_copy[i][j].status = world[i][j].status;
                    }
                }

                //Preserve colors
                world_copy[i][j].color = world[i][j].color;
        }
    }

    //Update world
    world = world_copy;
}

//See prototype
void display(vector< vector<Square> > &world)
{
    //Set cells alive/dead
    int i;

    //Get GIF headers ready
    if(useGif){
        gifCycles--;
        if(gifCycles == -1){
            closeGif();
            useGif = 0;
        }else{
            EGifPutExtension(file, GRAPHICS_EXT_FUNC_CODE, 4, "\x04\x64\x00\x00\x00");
            EGifPutImageDesc(file, 0, 0, COLS*GIFMULTIPLIER+GRIDWIDTH, ROWS*GIFMULTIPLIER+GRIDWIDTH, 0, NULL);
        }
    }

    //Display game of status!
    for(i = 0; i < ROWS; i++){

        //Display to terminal
        for(int j = 0; j < COLS; j++){
            cout << (char)world[i][j].status;
        }

        //Display to gif
        if(useGif){
            for(int k = 0; k < GIFMULTIPLIER; k++){                                    //Start rows
                if(k > GRIDWIDTH-1){
                    for(int j = 0; j < COLS; j++){                                     //Draw columns of each row
                        for(int z = 0; z < GRIDWIDTH; z++){
                            EGifPutPixel(file, 0);                                     //Vertical grid outline
                        }
                        if(world[i][j].status == ALIVE){
                            for(int l = 0; l < GIFMULTIPLIER-GRIDWIDTH; l++){
                              EGifPutPixel(file, world[i][j].color);                   //Draw alive pixel
                            }
                        }else{
                            for(int l = 0; l < GIFMULTIPLIER-GRIDWIDTH; l++){
                              EGifPutPixel(file, 1);                                   //Draw dead pixel
                            }
                        }
                    }
                }else{                                                                 //horizontal grid outline
                    for(int j = 0; j < COLS*GIFMULTIPLIER; j++){
                            EGifPutPixel(file, 0);                                     //Draw grid
                    }
                }
                for(int z = 0; z < GRIDWIDTH; z++){
                    EGifPutPixel(file, 0);                                             //Closing vertical grid outline
                }
            }
        }

        cout << endl;

    }

    //Finish drawing the grid if we're doing GIF output
    if(useGif){
        for(int j = 0; j < (COLS*GIFMULTIPLIER+GRIDWIDTH)*GRIDWIDTH; j++){            //Closing horizontal grid outline
            EGifPutPixel(file, 0);
        }
    }

}

//See prototype
void initGif(){

    //Make color map
    ColorMapObject *GIFcmap;
    GIFcmap = GifMakeMapObject(256, NULL);

    GIFcmap->Colors[0].Red = 0;
    GIFcmap->Colors[0].Green = 0;
    GIFcmap->Colors[0].Blue = 0;

    GIFcmap->Colors[1].Red = 255;
    GIFcmap->Colors[1].Green = 255;
    GIFcmap->Colors[1].Blue = 255;

    //Randomize color map
    for(int i = 0; i < 254; i++){
        switch(rand()%3){
            case 0:
                GIFcmap->Colors[i+2].Red = 255;
                GIFcmap->Colors[i+2].Green = rand()%128;
                GIFcmap->Colors[i+2].Blue = rand()%128;
                break;
            case 1:
                GIFcmap->Colors[i+2].Red = rand()%128;
                GIFcmap->Colors[i+2].Green = 255;
                GIFcmap->Colors[i+2].Blue = rand()%128;
                break;
            case 2:
                GIFcmap->Colors[i+2].Red = rand()%128;
                GIFcmap->Colors[i+2].Green = rand()%128;
                GIFcmap->Colors[i+2].Blue = 255;
                break;
        }
    }

    //Initialize GIF header
    char * fileName = "output.gif";
    file = EGifOpenFileName(fileName, false, NULL);
    EGifPutScreenDesc(file, COLS*GIFMULTIPLIER+GRIDWIDTH, ROWS*GIFMULTIPLIER+GRIDWIDTH, 256, 1, GIFcmap);
    EGifPutExtensionLeader(file, APPLICATION_EXT_FUNC_CODE);
    EGifPutExtensionBlock(file, 11, "NETSCAPE2.0");
    EGifPutExtensionBlock(file, 3, "\x01\x00\x00");
    EGifPutExtensionTrailer(file);

}

//See prototype
void closeGif(){
    //Close GIF file
    EGifCloseFile(file);
}
