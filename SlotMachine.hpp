#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <map>
#include <utility>
#include <windows.h>

using namespace std;
// using integers more effective
typedef unsigned __int8 u_int8_t;
typedef unsigned __int16 u_int16_t;
typedef unsigned __int32 u_int32_t;
typedef __int8 int8_t;

// defining constant variables
const u_int8_t ROWS=3;
const u_int8_t COLS=5;
const u_int32_t NumOfSimulation = 33554432; // change this to change the number of testing simulations
 
	
u_int8_t lines_num; // it is initialized in the main, it runs from 1 until 20 lines
u_int8_t freeSpins; // keeps how many free spins the player has currently
u_int32_t totalFreeSpins; // keeps the total number of free games won
bool freegames = false; // if false the game is without freespins, set to true for playing with free spins

// class definition
class SlotMachine
{
    public:
		// methods of the class callable by the main function
		int readConfigFile();
        void initialize();
        void start();
        int printResults();
		int printHitRate();
		void probRetCalculations();
    private:
		// methods callable only from class methods
        void randomStrips();
        void findCombinations(int line[COLS]);
        void scaleCombinations();
        void scoreHitsCalculation();
        int gamesWithWin; // counts games with a win
        int expSpins; // number of free spins propagated as a result of 1 free spin
        int Symbols[10]; // keeps how many times a symbol appears in a line
        int scalesOnScreen; // counts number of scales on the screen
        int pay_table[11][4]; // being read from the config file, contains the amount of money each combination pays
        int reel_strips[32][5]; // being read from the config file, contains the symbols distribution
        int lines_config[20][5]; // being read from the config file, contains the lines to check for a win
        int hits[10][4]; // counters of non scale combinations
        int hitsScale[6]; // counter of scale combinations
        int game_strips[3][5]; // is produced randomly from the reel_strips in each new game
        double probabilities[10][4]; // holds the probabilities of each non scale combination
        double returnScore[10][4]; // holds the return of each non scale combination
        double probabilityScale[6]; // holds the probabilities of each scale combination
        double returnScale[6]; // holds the return of each scale combination
		double hitRate[20]; // holds the hit rate per different number of lines tested
};
