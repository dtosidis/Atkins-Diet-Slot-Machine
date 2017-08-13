#include "SlotMachine.hpp"

void SlotMachine::initialize()
{
    /*
     Setting to zero several counters
    */

	freeSpins = 0; // counting how many free games the player has currently
    totalFreeSpins = 0; //counting the total free games won
    gamesWithWin = 0; // counting games with a win
    expSpins = 0; // number of free spins propagated as a result of 1 free spin

	// initializing the array for counting how many times in a row a symbol appears
	// during one line parsing each time
	for (int8_t j=0; j<10; j++){
		Symbols[j] = 0;
	}

	// initializing the array for counting the hits of each combination except the scale
    for (int8_t i=0; i<10; i++) {
        for (int8_t j=0; j<4; j++) {
            hits[i][j] = 0;
        }
    }

	// initializing the array for counting the hits only for scale combinations
    for (int8_t i=0; i<6; i++) {
        hitsScale[i] = 0;
    }

	// initialize to zero the hit rate for current lines number being tested
	hitRate[lines_num -1] = 0;
}

int SlotMachine::readConfigFile()
{
    /*
     Opening the configuration.ini file and reading the values into the
     three arrays, pay_table, reel_strips and lines_config for initializing
     them.
    */
    try
    {
        ifstream s("configuration.ini");
        if(!s) throw runtime_error("cannot open file");
        for(int8_t r=0; r<11; ++r)
            for(int8_t c=0; c<4; ++c)
               if(!(s >>pay_table[r][c]))
                    throw runtime_error("insufficient or bad input");

        for(int8_t r=0; r<32; ++r)
            for(int8_t c=0; c<5; ++c)
               if(!(s >>reel_strips[r][c]))
                    throw runtime_error("insufficient or bad input");

        for(int8_t r=0; r<20; ++r)
            for(int8_t c=0; c<5; ++c)
               if(!(s >>lines_config[r][c]))
                    throw runtime_error("insufficient or bad input");

        s.close();
    }
    catch(const std::exception& e)
    {
        cout << "Reading error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}


void SlotMachine::randomStrips()
{
    /*
     choosing 5 random numbers between 0-31 and producing a 3x5 array,
     by using the reel_strips array for finding out in which symbol
     correspond the random values in the middle line and which symbols
     should be placed in the above and below line
    */
    
	const u_int8_t min = 0;
	const u_int8_t max = 31;
    u_int8_t rand_num;

	for(int8_t i=0; i<5; ++i){
		rand();
        rand_num = min + (rand() % (u_int8_t)(max - min + 1));
        game_strips[1][i] = reel_strips[rand_num][i];
        switch (rand_num) {
            case 0:
                game_strips[0][i] = reel_strips[31][i];
                game_strips[2][i] = reel_strips[rand_num + 1][i];
                break;
            case 31:
                game_strips[0][i] = reel_strips[rand_num - 1][i];
                game_strips[2][i] = reel_strips[0][i];
                break;
            default:
                game_strips[0][i] = reel_strips[rand_num - 1][i];
                game_strips[2][i] = reel_strips[rand_num + 1][i];
        }
    }
}


void SlotMachine::findCombinations (int line[COLS])
{
    /*
     finds how many times in a line a symbol appears and uses the Symbols array
	 for counting its symbols occurance
    */


    u_int8_t count = 0;
    // checking if the first element of the line being proceeded is the atkins symbol
    if (game_strips[line[0]][0] == 1) {
		// checking the rest symbols of the line
        for (int8_t j=1; j<COLS; j++){
            /*
             if the next one is also an atkins, it writes in the corresponding position
			 of the Symbol array how many atkins in a row appear
            */
            if (game_strips[line[j]][j] == 1) {
				if (Symbols[game_strips[line[0]][0] -1] != 0)
					Symbols[game_strips[line[0]][0] -1]++;
                else Symbols[game_strips[line[0]][0] -1] = 2;
            }
            /*
             if the next one is not an atkins and not a scale, then it assumes that all the
             atkins already appeared in a row substitute the current symbol
			 it writes in the corresponding position (given by game_strips[line[j]][j] -1)
			 of the Symbol array how many times in a row the current symbols appears
             
			 the count variable used to calculate the appearances of the current symbol
			 depending on how many atkins in a row have already appeared
			*/
            else if ((game_strips[line[j]][j] != 1) && (game_strips[line[j]][j] != 11)) {
                if (Symbols[game_strips[line[0]][0] -1] == 0)
                    count =  2;
                else count = 1 + Symbols[game_strips[line[0]][0] -1];
                Symbols[game_strips[line[j]][j] -1] = count;
                /*
                 continue here by checking the next symbol in the line if it is the same with
                 the first non atkins non scale symbol that appeared in a row
                 if the next is the same or it is an atkins, it counts one more, by increasing 
				 its corresponding value in the Symbols array
                 if it is not the same it breaks from the inner loop and then also from the outer loop
                */
                for (int8_t i=j+1; i<COLS; i++){
                    if ((game_strips[line[j]][j] == game_strips[line[i]][i]) || (game_strips[line[i]][i] == 1))
						Symbols[game_strips[line[j]][j] -1]++;
                    else break;
                }
                break;
            }
            // if the next is scale it breaks from the loop
            else if (game_strips[line[j]][j] == 11) break;
        }
    }
    // if the first symbol is not the atkins and not the scale
    else if ((game_strips[line[0]][0] != 11) && (game_strips[line[0]][0] != 1)) {
        /*
         checks if the next ones are the same with the first or they are atkins
         as long as it finds a same one it writes, in the symbol's corresponding
		 position in the Symbols array, the times in a row that it appears and as
		 soon as it finds a different one it breaks from the loop
        */
        for(int8_t l=1; l<COLS; l++) {
            if ((game_strips[line[0]][0] == game_strips[line[l]][l]) || (game_strips[line[l]][l] == 1)) {
                if (Symbols[game_strips[line[0]][0] -1] != 0)
					Symbols[game_strips[line[0]][0] -1]++;
                else Symbols[game_strips[line[0]][0] -1] = 2;
            }
            else
                break;
        }
    }

}

void SlotMachine::scaleCombinations()
{
     /*
     calculating seperately how many times the scale symbols appear on the screen as they can appear in
     any order in any row
    */
    for (int8_t i=0; i<ROWS; i++) {
        for (int8_t j=0; j<COLS; j++) {
            if (game_strips[i][j] == 11) scalesOnScreen++;
        }
    }

}

int SlotMachine::printResults()
{
	// opening the file output.txt and writing the result in it
    try
    {
        ofstream outputFile ("output.txt", ios::app);
        if(!outputFile.is_open()) throw runtime_error("cannot open file");

        outputFile.setf(ios::fixed,ios::floatfield);
        double totalHorizontal;
        double totalVertical[4];

		outputFile<<'\n';
		outputFile<<'\n';
		outputFile<<'\n';
        outputFile<<'\n';
		outputFile<<"--------------LINES: "<< (int)lines_num <<"-------------------"<<'\n';
		outputFile<<'\n';
		outputFile<<'\n';



        char* atkinsDiet[11] = {"Atkins", "Steak", "Ham", "Drum/ck", "Sausage", "Eggs", "Butter", "Cheese", "Bacon", "May/se", "Total"};

        outputFile<<"The HITS for each win are: " <<'\n';
        outputFile<<"-----------------------------------------------------------------------------------------------------"<<'\n';
        outputFile<<"Symbols"<<'\t'<<'\t'<<"5 in a Row"<<'\t'<<"4 in a Row"<<'\t'<<"3 in a Row"<<'\t'<<"2 in a Row"<<'\t'<<"Total"<<'\n';
		outputFile.precision(8);
        for (int8_t j=0; j<4; j++) {
            totalVertical[j] = 0;
        }

        for (int8_t i=0; i<10; i++) {
            totalHorizontal = 0;
            outputFile<< atkinsDiet[i]<<'\t'<<'\t';
            for (int8_t j=3; j>-1; j--){
                outputFile << hits[i][j]<<'\t'<<'\t';
                totalHorizontal = totalHorizontal + hits[i][j];
                totalVertical[j] = totalVertical[j] + hits[i][j];
            }
            outputFile << (int)totalHorizontal;
            outputFile << '\n';
        }
        outputFile<< atkinsDiet[10]<< '\t'<<'\t';
        totalHorizontal = 0;
        for (int8_t j=3; j>-1; j--) {
            outputFile<<(int)totalVertical[j]<<'\t'<<'\t';
            totalHorizontal = totalHorizontal + totalVertical[j];
        }
        outputFile << (int)totalHorizontal<<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile << '\n';


        outputFile<<"The PROBABILITIES for each win are: " <<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile<<"Symbols"<<'\t'<<'\t'<<"5 in a Row"<<'\t'<<'\t'<<"4 in a Row"<<'\t'<<'\t'<<"3 in a Row"<<'\t'<<'\t'<<"2 in a Row"<<'\t'<<'\t'<<"Total"<<'\n';
        outputFile.precision(8);
        for (int8_t j=0; j<4; j++) {
            totalVertical[j] = 0;
        }
        for (int8_t i=0; i<10; i++) {
            totalHorizontal = 0;
            outputFile<< atkinsDiet[i]<<'\t'<<'\t';
            for (int8_t j=3; j>-1; j--){
                outputFile << probabilities[i][j]<<'\t'<<'\t';
                totalHorizontal = totalHorizontal + probabilities[i][j];
                totalVertical[j] = totalVertical[j] + probabilities[i][j];
            }
            outputFile << totalHorizontal;
            outputFile << '\n';
        }
        outputFile<< atkinsDiet[10]<< '\t'<<'\t';
        totalHorizontal = 0;
        for (int8_t j=3; j>-1; j--) {
            outputFile<<totalVertical[j]<<'\t'<<'\t';
            totalHorizontal = totalHorizontal + totalVertical[j];
        }
        outputFile << totalHorizontal<<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile << '\n';

        outputFile.precision(6);

        outputFile<<"The RETURN for each win is: " <<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile<<"Symbols"<<'\t'<<'\t'<<"5 in a Row"<<'\t'<<'\t'<<"4 in a Row"<<'\t'<<'\t'<<"3 in a Row"<<'\t'<<'\t'<<"2 in a Row"<<'\t'<<'\t'<<"Total"<<'\n';

        for (int8_t j=0; j<4; j++) {
            totalVertical[j] = 0;
        }
        for (int8_t i=0; i<10; i++) {
            totalHorizontal = 0;
            outputFile<< atkinsDiet[i]<<'\t'<<'\t';
            for (int8_t j=3; j>-1; j--){
                outputFile << returnScore[i][j]<<'\t'<<'\t';
                totalHorizontal = totalHorizontal + returnScore[i][j];
                totalVertical[j] = totalVertical[j] + returnScore[i][j];
            }
            outputFile << totalHorizontal;
            outputFile << '\n';
        }
        outputFile<< atkinsDiet[10]<< '\t'<<'\t';
        totalHorizontal = 0;
        for (int8_t j=3; j>-1; j--) {
            outputFile<<totalVertical[j]<<'\t'<<'\t';
            totalHorizontal = totalHorizontal + totalVertical[j];
        }
        outputFile << totalHorizontal<<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile << '\n';

        outputFile<<"Scatter pays " <<'\n';
        outputFile<<"-------------------------------------------------------------------------------------------------------------------------------"<<'\n';
        outputFile<<"Total Scatters"<<'\t'<<"Pays"<<'\t'<<"Combinations"<<'\t'<<'\t'<<"Probability"<<'\t'<<'\t'<<"Return"<<'\n';
        for (int8_t j=0; j<4; j++) {
            totalVertical[j] = 0;
        }
        for (int j=5; j>-1; j--){
            if (j-2 >= 0)
                outputFile << j<< '\t'<<'\t'<< pay_table[10][j-2]<<'\t'<<'\t'<<hitsScale[j]<<'\t'<<'\t'<<probabilityScale[j]<<'\t'<<'\t'<<returnScale[j]<<'\n';
            else outputFile << j<< '\t'<<'\t'<< pay_table[10][0]<<'\t'<<'\t'<<hitsScale[j]<<'\t'<<probabilityScale[j]<<'\t'<<'\t'<<returnScale[j]<<'\n';
            totalVertical[1] = totalVertical[1] + hitsScale[j];
            totalVertical[2] = totalVertical[2] + probabilityScale[j];
            totalVertical[3] = totalVertical[3] + returnScale[j];
        }
        
		outputFile<<"Total"<<'\t'<<'\t'<<'\t'<<'\t'<<(int)totalVertical[1]<<'\t'<<totalVertical[2]<<'\t'<<'\t'<<totalVertical[3]<<'\n';


        outputFile<<'\n';


        double probBonus = probabilityScale[5] + probabilityScale[4] + probabilityScale[3];
        double totalExpSpins = (double)totalFreeSpins/expSpins;
        double expBaseWin = totalVertical[3] + totalHorizontal;
        double avgBonus = expBaseWin * totalExpSpins * 3;
        double returnBonus = avgBonus * probBonus;
        double x = 1 / (1 - (10 * probBonus));
        double theorExpSpins = 10 * x;
        double theorAvgBonus = expBaseWin * theorExpSpins * 3;
        double theorReturnBonus = theorAvgBonus * probBonus;
        
		if(freegames) {
			outputFile<<"Experimental Bonus Feature"<<'\n';
			outputFile<<"------------------------------------------------------"<<'\n';
			outputFile<<"Prob. Bonus"<<'\t'<< '\t'<<'\t'<<probBonus<<'\n';
			outputFile<<"Num initial free spins"<<'\t'<<'\t'<< 10<<'\n';
			outputFile<<"Multiplier"<<'\t'<< '\t'<<'\t'<< 3<<'\n';
			outputFile<<"Exp base win"<<'\t'<<'\t'<<'\t'<< expBaseWin<<'\n';
			outputFile<<"Expected spins"<<'\t'<<'\t'<<'\t'<< totalExpSpins<<'\n';
			outputFile<<"Average bonus"<<'\t'<<'\t'<<'\t'<< avgBonus<<'\n';
			outputFile<<"Bonus return"<<'\t'<<'\t'<<'\t'<< returnBonus<<'\n';
			outputFile<<"------------------------------------------------------"<<'\n';
			outputFile<<'\n';
		}
        outputFile<<"Theoretical Bonus Feature"<<'\n';
        outputFile<<"------------------------------------------------------"<<'\n';
        outputFile<<"Prob. Bonus"<<'\t'<< '\t'<<'\t'<<probBonus<<'\n';
        outputFile<<"Num initial free spins"<<'\t'<<'\t'<< 10<<'\n';
        outputFile<<"Multiplier"<<'\t'<< '\t'<<'\t'<< 3<<'\n';
        outputFile<<"Exp base win"<<'\t'<<'\t'<<'\t'<< expBaseWin<<'\n';
        outputFile<<"Expected spins"<<'\t'<<'\t'<<'\t'<< theorExpSpins<<'\n';
        outputFile<<"Average bonus"<<'\t'<<'\t'<<'\t'<< theorAvgBonus<<'\n';
        outputFile<<"Bonus return"<<'\t'<<'\t'<<'\t'<< theorReturnBonus<<'\n';

        outputFile<<"------------------------------------------------------"<<'\n';

        outputFile<<'\n';
        outputFile<<"Summary"<<'\n';
        outputFile<<"---------------------------------------------"<<'\n';
        outputFile<<"Item"<<'\t'<<'\t'<<'\t'<<"Return"<<'\n';
        //outputFile<<"Hit freq. (per line)"<< '\t'<<(double)(hitRate/lines_num) *100<<"%"<<'\n';
        outputFile<<"Line Pays"<<'\t'<<'\t'<<totalHorizontal*100<<"%"<<'\n';
        outputFile<<"Scatter (fixed wins)"<<'\t'<<totalVertical[3]*100<<"%"<<'\n';
		if (freegames) {
			outputFile<<"Scatter (free spins)"<<'\t'<<returnBonus*100<<"%"<<'\n';
			outputFile<<"Total return"<<'\t'<<'\t'<<(totalVertical[3]+totalHorizontal+returnBonus)*100<<"%"<<'\n';
		}
		else outputFile<<"Total return"<<'\t'<<'\t'<<(totalVertical[3]+totalHorizontal)*100<<"%"<<'\n';
		outputFile<<"---------------------------------------------"<<'\n';


        outputFile.close();
    }
    catch(const std::exception& e)
    {
        cout << "Reading error: " << e.what() << std::endl;
        return -1;
    }

    return 0;

}

int SlotMachine::printHitRate()
{
	/*
	 prints in a file the hit rate calculated for all the different 
	 lines number tested and also the average hit rate
	*/
	try
    {
        ofstream outputFile ("hit_rate.txt", ios::app);
        if(!outputFile.is_open()) throw runtime_error("cannot open file");
		double hitRateCount = 0;
        outputFile.setf(ios::fixed,ios::floatfield);
		for (int8_t i=0; i<20; i++){
			outputFile<<"The HitRate for "<< (int)i+1 << " lines is: " <<hitRate[i]<<'\n';
			hitRateCount = hitRateCount + hitRate[i];
		}
		outputFile<<"Average Hit Rate: "<<(double)hitRateCount/20<<'\n';
	    outputFile.close();
    }
    catch(const std::exception& e)
    {
        cout << "Reading error: " << e.what() << std::endl;
        return -1;
    }
	return 0;
}



void SlotMachine::probRetCalculations()
{
	/*
	 calculate here the probabilities and returns for printing them afterwards
	*/

	// setting to zero the 2 pair combinations that do not pay anything in the pay table
    for (int8_t i=4; i<10; i++) {
        hits[i][0] = 0;
    }

	// calculate the probabilities and the return for all the symbols except the scale
    for (int8_t i=0; i<10; i++) {
        for (int8_t j=0; j<4; j++){
            probabilities[i][j] = (double) hits[i][j] / ((NumOfSimulation + totalFreeSpins) * lines_num);
            returnScore[i][j] = (double) probabilities[i][j] * pay_table[i][j];
        }
    }

	// calculate the probabibility and return for the scale
    for (int8_t j=0; j<6; j++) {
        probabilityScale[j] = (double) hitsScale[j] / (NumOfSimulation + totalFreeSpins);
        if (j >=2)
            returnScale[j] = (double) probabilityScale[j] * pay_table[10][j-2];
        else returnScale[j] = 0;
    }

	// calculate the hit rate
    hitRate[lines_num -1] = (double) (NumOfSimulation + totalFreeSpins)/(gamesWithWin);
}


void SlotMachine::scoreHitsCalculation()
{
	int score; // used to calculate score
    score = 0;
    scalesOnScreen = 0; 

    scaleCombinations(); // sets the scalesOnScreen
    hitsScale[scalesOnScreen]++; //counts how many times the current combination of scales appears in total
	
	if (scalesOnScreen >= 3){
		score = score + pay_table[10][scalesOnScreen -2]; // calculate score from scales
		if (freegames){				// if free games are enabled 
            if (freeSpins > 0) {
                expSpins = expSpins + 10;
                freeSpins = freeSpins + 10;
            }
            else if (freeSpins == 0)
                freeSpins = 11;			// initial give 11 free games because they are reduced by 1 in the main

            totalFreeSpins = totalFreeSpins + 10; // calculate the total free spins earned
        }
    }
    
	int atkins, y_atkins;
	for (u_int8_t i=0; i<lines_num; i++) {
		
        findCombinations(lines_config[i]); // sets in the Symbols array how many combinations in a row of non scale symbols occured
        atkins = 0;
		for (int8_t j=0; j<10; j++){
			if (Symbols[j] == 0) continue; // no combination go to next one
			
			score = score + pay_table[j][Symbols[j] -2]; // calculate score of current combination
            hits[j][Symbols[j] -2]++;	//counts how many times the current combination appears in total	

			// if it was atkins store temporary its pay amount and the times it occurred
            if (j == 0) {
                atkins = pay_table[j][Symbols[j] -2];
                y_atkins = Symbols[j] - 2;
            }

			/*
			 if another symbol comes and there was before an atkins
			 check which one pays more and remove the value which pays less
			 from the score and hits calculations
			*/
            if ((j != 0) && (atkins != 0) && (atkins < pay_table[j][Symbols[j] -2])) {
                score = score - atkins;
                hits[0][y_atkins]--;
            }
            else if ((j != 0) && (atkins != 0) && (atkins >= pay_table[j][Symbols[j] -2])) {
                score = score - pay_table[j][Symbols[j] -2];
                hits[j][Symbols[j] -2]--;
            }
            Symbols[j] = 0; // setting the array entries to zero for next loop
		}
    }

    if (score > 0) gamesWithWin++; // count games with win
	
}


void SlotMachine::start()
{

    // call the randomStrips method for creating the 3x5 game_strips array with random symbols
    randomStrips();
	// evaluate the game
    scoreHitsCalculation();

}

long long milliseconds_now() 
{
	/*
	it is used for getting current time in milliseconds
	for measuring the simulation speed
	*/
	static LARGE_INTEGER s_frequency;
    static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
    if (s_use_qpc) {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return (1000LL * now.QuadPart) / s_frequency.QuadPart;
    } else {
        return GetTickCount();
    }
}


int main()
{
	long long start = milliseconds_now(); // getting the current time in milliseconds
	SlotMachine MySlotMachine;
	MySlotMachine.readConfigFile(); // calling class method for reading the config file
	char answer;
	// added asking the user if he wants to play with or without freegames
	do{
		cout<<"Do you want to play with free games? y/n"<<'\n';
		cin>> answer;
	}while((answer !='y')&&(answer !='n') );
	if (answer == 'y') freegames = true;
	
	for (lines_num = 1; lines_num < 21; lines_num++) {

		MySlotMachine.initialize(); // calling initialization class method
		for (u_int32_t run = 0; run < (NumOfSimulation + totalFreeSpins); run++) {
			srand ((unsigned) time( NULL ) * (run + 1)); // seeding the rand for producing random numbers
			MySlotMachine.start(); //calling the class method start
			/*
			 if there are free spins won reduces them by one
			 is needed for knowing when free games are won inside
			 a loop of already free games played
			*/
			if (freeSpins>0) {
				freeSpins--;
			}
		}
		MySlotMachine.probRetCalculations(); // calculate probabilities and returns
		MySlotMachine.printResults(); // disable comment for printing the hits/probabilities/returns
		
	}
	MySlotMachine.printHitRate(); // disable comment for printing the hit rates
	long long elapsed = milliseconds_now() - start; // calculate the elapced time of the application run time
	cout<<(long long)elapsed<<'\n';
	
	char c;
	cin>> c; // used for not closing the command line

    return 0;
}

