/*
 * Investio v0.2, a game prototype by Lauren E. Scott.
 */

#include <sifteo.h>
#include "assets.gen.h"
#define		BASE		32
#define		CORN		0
#define		COFFEE		1
#define		CLOTH		2
using namespace Sifteo;

static Metadata M = Metadata()
    .title("Sensors SDK Example")
    .package("com.sifteo.sdk.investio", "0.2")
    .icon(Icon)
    .cubeRange(0, CUBE_ALLOCATION);

static VideoBuffer vid[CUBE_ALLOCATION];
static TiltShakeRecognizer motion[CUBE_ALLOCATION];


enum ScreenType { MONEY = 0, MARKET_PRICES, BSCORN, BSCOFFEE, BSCLOTH, SAVINGS, CURR_EVENTS, MARKET };
int player_money[BASE+1];
int player_savings[BASE+1];
int units_of_corn[BASE+1];
int units_of_coffee[BASE+1];
int units_of_cloth[BASE+1];
int market_prices[3];
ScreenType currentScreen = MONEY;
ScreenType screens[BASE+1];
bool neighbored = false;
bool endGame = false;
bool tiltedRight = false;
bool tiltedLeft = false;

class SensorListener {
public:
    struct Counter {
        unsigned touch;
        unsigned neighborAdd;
        unsigned neighborRemove;
    } counters[CUBE_ALLOCATION];

    void install()
    {
        Events::neighborAdd.set(&SensorListener::onNeighborAdd, this);
        Events::neighborRemove.set(&SensorListener::onNeighborRemove, this);
        Events::cubeAccelChange.set(&SensorListener::onAccelChange, this);
        Events::cubeTouch.set(&SensorListener::onTouch, this);
        Events::cubeConnect.set(&SensorListener::onConnect, this);
        
        for (int i = 0; i < BASE; i++) {
        	player_money[i] = 200;
        	player_savings[i] = 0;
        	screens[i] = MONEY;
        	units_of_corn[i] = 0;
        	units_of_coffee[i] = 0;
        	units_of_cloth[i] = 0;
        }
        
        screens[BASE] = MARKET;
        units_of_corn[BASE] = 200;
        units_of_coffee[BASE] = 200;
        units_of_cloth[BASE] = 200;
        player_money[BASE] = 100000000;
        
        market_prices[CORN] = 10;
        market_prices[COFFEE] = 15;
        market_prices[CLOTH] = 8;

        // Handle already-connected cubes
        for (CubeID cube : CubeSet::connected())
            onConnect(cube);
    }
    
    void computeMarketPrices(int elapsedSeconds)
    {
    	switch (elapsedSeconds) {
    		case 20:
    			market_prices[CORN] = market_prices[CORN] * 1.2;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.4;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.1;
    			break;
    		case 40:
    			market_prices[CORN] = market_prices[CORN] * 1.6;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.2;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1;
    			break;
    		case 60:
    			market_prices[CORN] = market_prices[CORN] * 1.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 0.3;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.2;
    			break;
    		case 80:
    			market_prices[CORN] = market_prices[CORN] * 1.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.3;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.5;
    			break;
    		case 100:
    			market_prices[CORN] = market_prices[CORN] * 1.3;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.1;
    			market_prices[CLOTH] = market_prices[CLOTH] * 0.7;
    			break;
    		case 110:
    			market_prices[CORN] = market_prices[CORN] * 0.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1;
    			market_prices[CLOTH] = market_prices[CLOTH] * 0.8;
    			break;
    		case 120:
    			market_prices[CORN] = market_prices[CORN] * 0.1;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.7;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.4;
    			break;
    		case 140:
    			endGame = true;
    			break;
    		default:
    			break;
    	}
    }
    
        void computeSavings()
    {
    	for (int i = 0; i < CUBE_ALLOCATION; i++) {
    		player_savings[i] = player_savings[i] * 1.05;
    	}
    }
    
    void writeScreen(unsigned id) {
    	String<32> str1;
    	String<32> str2;
    	String<32> str3;
    	String<32> str4;
    	String<32> str5;
    	String<64> clear;
    	
    	currentScreen = screens[id];
    	
    	switch (currentScreen) {
    		case 0:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,3), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str5.clear();
    			str1 << "Money";
    			vid[id].bg0rom.text(vec(1,3), str1);
    			str2 << "$ " << player_money[id];
    			vid[id].bg0rom.text(vec(1,10), str2);
    			str5 << "Player " << id;
    			vid[id].bg0rom.text(vec(1,1), str5);
    			break;
    		case 1:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,3), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str5.clear();
    			str1 << "Market Prices";
    			vid[id].bg0rom.text(vec(1,1), str1);
    			str2 << "Corn: $" << market_prices[CORN] << "\n";
    			vid[id].bg0rom.text(vec(1,4), str2);
    			str3 << "Coffee: $" << market_prices[COFFEE] << "\n";
    			vid[id].bg0rom.text(vec(1,6), str3);
    			str4 << "Cloth: $" << market_prices[CLOTH] << "\n";
    			vid[id].bg0rom.text(vec(1,8), str4);
    			break;
    		case 2:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,3), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str5.clear();
    			str1 << "Corn   $" << player_money[id];
    			str2 << "Buy       Sell";
    			str3 << units_of_corn[id] << " units";
    			str4 << "Market price:";
    			str5 << "$" << market_prices[CORN];
    			vid[id].bg0rom.text(vec(1,1), str1);
    			vid[id].bg0rom.text(vec(1,6), str2);
    			vid[id].bg0rom.text(vec(1,10), str3);
    			vid[id].bg0rom.text(vec(1,12), str4);
    			vid[id].bg0rom.text(vec(1,14), str5);
    			break;
    		case 3:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,3), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str1 << "Coffee   $" << player_money[id];
    			str2 << "Buy       Sell";
    			str3 << units_of_coffee[id] << " units";
    			str4 << "Market price:";
    			str5 << "$" << market_prices[COFFEE];
    			vid[id].bg0rom.text(vec(1,1), str1);
    			vid[id].bg0rom.text(vec(1,6), str2);
    			vid[id].bg0rom.text(vec(1,10), str3);
    			vid[id].bg0rom.text(vec(1,12), str4);
    			vid[id].bg0rom.text(vec(1,14), str5);
    			break;
    		case 4:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,3), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str1 << "Cloth   $" << player_money[id];
    			str2 << "Buy       Sell";
    			str3 << units_of_cloth[id] << " units";
    			str4 << "Market price:";
    			str5 << "$" << market_prices[CLOTH];
    			vid[id].bg0rom.text(vec(1,1), str1);
    			vid[id].bg0rom.text(vec(1,6), str2);
    			vid[id].bg0rom.text(vec(1,10), str3);
    			vid[id].bg0rom.text(vec(1,12), str4);
    			vid[id].bg0rom.text(vec(1,14), str5);
    			break;
    		case 5:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1.clear();
    			str2.clear();
    			str3.clear();
    			str4.clear();
    			str5.clear();
    			str1 << "Savings";
    			vid[id].bg0rom.text(vec(1,1), str1);
    			str2 << player_savings[id];
    			vid[id].bg0rom.text(vec(1,10), str2);
    			break;
    		case 6:
    			clear << "                  ";
    			vid[id].bg0rom.text(vec(1,1), clear);
    			vid[id].bg0rom.text(vec(1,10), clear);
    			vid[id].bg0rom.text(vec(1,4), clear);
    			vid[id].bg0rom.text(vec(1,6), clear);
    			vid[id].bg0rom.text(vec(1,8), clear);
    			vid[id].bg0rom.text(vec(1,12), clear);
    			vid[id].bg0rom.text(vec(1,14), clear);
    			str1 << "Current Events";
    			vid[id].bg0rom.text(vec(1,1), str1);
    			str2 << "No events!";
    			vid[id].bg0rom.text(vec(1,10), str2);
    			break;
    		case MARKET:
    			break;
    		default:
    			break;
    	}
    }

    

private:
    void onConnect(unsigned id)
    {
        CubeID cube(id);
        uint64_t hwid = cube.hwID();

        bzero(counters[id]);
        LOG("Cube %d connected\n", id);

        vid[id].initMode(BG0_ROM);
        vid[id].attach(id);
        motion[id].attach(id);
        
        String<32> str1;
        String <32> str2, str3;
        str1 << "Money";
        str2 << player_money[id];
        str3 << "Player " << id;
        vid[cube].bg0rom.text(vec(1,3), str1);
        vid[cube].bg0rom.text(vec(1,10), str2);
        vid[cube].bg0rom.text(vec(1,1), str3);
        
        // Keep track of the time that has elapsed. Use for constructing market prices.
        SystemTime start = SystemTime::now();
        TimeDelta el = SystemTime::now() - start;
/*        if ((int)el.seconds() == 5 || (int)el.seconds() == 10 || (int)el.seconds() == 15 || (int)el.seconds() == 20 || (int)el.seconds() == 25) {
        	LOG("Saved!\n");
        	computeSavings(id);
        }
*/        

/*
        // Draw the cube's identity
        String<128> str;
        str << "I am cube #" << cube << "\n";
        str << "hwid " << Hex(hwid >> 32) << "\n     " << Hex(hwid) << "\n\n";
        vid[cube].bg0rom.text(vec(1,2), str);
*/
        // Draw initial state for all sensors
        onAccelChange(cube);
        onTouch(cube);
        drawNeighbors(cube);
    }
    

    void onTouch(unsigned id)
    {
        CubeID cube(id);
        counters[id].touch++;
        LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
        
        if (tiltedRight == true && screens[id] < 9 && cube.isTouching() == 1) {
        	screens[id]++;
        }
        
        if (tiltedLeft == true && screens[id] > 0 && cube.isTouching() == 1) {
        	screens[id]--;
        }
        
        LOG("Current screen is: %d\n", screens[id]);
        writeScreen(id);
        
        if (screens[id] == SAVINGS && tiltedRight == false && tiltedLeft == false && player_money[id] > 0 && cube.isTouching() == 1) {
        	player_savings[id] = player_savings[id] + 50;
        	player_money[id] = player_money[id] - 50;
        }
    }

    void onAccelChange(unsigned id)
    {
        CubeID cube(id);

        String<64> str;

        unsigned changeFlags = motion[id].update();
        if (changeFlags) {
            // Tilt/shake changed

            LOG("Tilt/shake changed, flags=%08x\n", changeFlags);

            auto tilt = motion[id].tilt;
            
            switch (tilt.x) {
            	case -1:
            		tiltedRight = true;
            		break;
            	case 1:
            		tiltedLeft = true;
            		break;
            	case 0:
            		tiltedRight = false;
            		tiltedLeft = false;
            		LOG("Not tilted.\n");
            		break;
            }
        }
    }
            

    
    void screenManager(ScreenType currentScreen, unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide) {
    	auto neighbors = vid[firstID].physicalNeighbors();
    	String<64> str;
    	    	
    	switch (screens[firstID]) {
    		case 0:
    			LOG("We are at the Money screen.\n");
    			LOG("%d\n", firstID);
    			break;
    		case 1:
    			LOG("We are at the Market Prices screen.\n");
    			break;
    		case 2:
    			LOG("We are at the Buy/Sell Corn screen.\n");
    			if (firstSide == LEFT && (secondSide == RIGHT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCORN || screens[secondID] == MARKET) && player_money[firstID] >= 10 && units_of_corn[secondID] >= 10) {
    				LOG("We have a seller for corn!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[CORN];
    				units_of_corn[firstID] = units_of_corn[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[CORN];
    				units_of_corn[secondID] = units_of_corn[secondID] - 10;
    			}
    			else if (firstSide == RIGHT && (secondSide == LEFT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCORN || screens[secondID] == MARKET) && player_money[secondID] >= 10 && units_of_corn[firstID] >= 10) {
    				player_money[firstID] = player_money[firstID] + market_prices[CORN];
    				units_of_corn[firstID] = units_of_corn[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[CORN];
    				units_of_corn[secondID] = units_of_corn[secondID] + 10;
    			}
    			LOG("Cube1 has %d corn. Cube2 has %d corn.\n Cube1 has %d money. Cube2 has %d money.\n", units_of_corn[firstID], units_of_corn[secondID], player_money[firstID], player_money[secondID]);
    			LOG("First side: %d   Second side: %d\n", firstSide, secondSide);
    			break;
    		case 3:
    			LOG("We are at the Buy/Sell Coffee screen.\n");
    			if (firstSide == LEFT && (secondSide == RIGHT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCOFFEE || screens[secondID] == MARKET) && player_money[firstID] >= 10 && units_of_coffee[secondID] >= 10) {
    				LOG("We have a seller for corn!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[COFFEE];
    				units_of_coffee[firstID] = units_of_coffee[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[COFFEE];
    				units_of_coffee[secondID] = units_of_coffee[secondID] - 10;
    			}
    			else if (firstSide == RIGHT && (secondSide == LEFT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCOFFEE || screens[secondID] == MARKET) && player_money[secondID] >= 10 && units_of_coffee[firstID] >= 10) {
    				player_money[firstID] = player_money[firstID] + market_prices[COFFEE];
    				units_of_coffee[firstID] = units_of_coffee[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[COFFEE];
    				units_of_coffee[secondID] = units_of_coffee[secondID] + 10;
    			}
    			break;
    		case 4:
    			LOG("We are at the Buy Cloth screen.\n");
    			if (firstSide == LEFT && (secondSide == RIGHT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCLOTH || screens[secondID] == MARKET) && player_money[firstID] >= 10 && units_of_cloth[secondID] >= 10) {
    				LOG("We have a seller for corn!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[CLOTH];
    				units_of_cloth[firstID] = units_of_cloth[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[CLOTH];
    				units_of_cloth[secondID] = units_of_cloth[secondID] - 10;
    			}
    			else if (firstSide == RIGHT && (secondSide == LEFT || (secondSide == 0 && screens[secondID] == MARKET)) && (screens[secondID] == BSCLOTH || screens[secondID] == MARKET) && player_money[secondID] >= 10 && units_of_cloth[firstID] >= 10) {
    				player_money[firstID] = player_money[firstID] + market_prices[CLOTH];
    				units_of_cloth[firstID] = units_of_cloth[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[CLOTH];
    				units_of_cloth[secondID] = units_of_cloth[secondID] + 10;
    			}
    			break;
    		case 5:
    			LOG("We are at the Savings screen.\n");
    			break;
    		case 6:
    			LOG("We are at the Current Events screen.\n");
    			break;
    		default:
    			LOG("Where are we??\n");
    			break;
    	}
    }


    void onNeighborRemove(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
    {
        LOG("Neighbor Remove: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

		neighbored = false;
		
        if (firstID < arraysize(counters)) {
            counters[firstID].neighborRemove++;
            drawNeighbors(firstID);
        }
        if (secondID < arraysize(counters)) {
            counters[secondID].neighborRemove++;
            drawNeighbors(secondID);
        }
    }

    void onNeighborAdd(unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide)
    {
        LOG("Neighbor Add: %02x:%d - %02x:%d\n", firstID, firstSide, secondID, secondSide);

        if (firstID < arraysize(counters)) {
            counters[firstID].neighborAdd++;
            drawNeighbors(firstID);
        }
        if (secondID < arraysize(counters)) {
            counters[secondID].neighborAdd++;
            drawNeighbors(secondID);
        }
        
        neighbored = true;
        
        screenManager(currentScreen, firstID, firstSide, secondID, secondSide);
        
        for (int i = 0; i < CUBE_ALLOCATION; i++) {
    	writeScreen(i);
    }
        LOG("First ID: %d\n Second ID: %d\n", firstID, secondID);
        
    }

    void drawNeighbors(CubeID cube)
    {
        Neighborhood nb(cube);
        BG0ROMDrawable &draw = vid[cube].bg0rom;

        drawSideIndicator(draw, nb, vec( 1,  0), vec(14,  1), TOP);
        drawSideIndicator(draw, nb, vec( 0,  1), vec( 1, 14), LEFT);
        drawSideIndicator(draw, nb, vec( 1, 15), vec(14,  1), BOTTOM);
        drawSideIndicator(draw, nb, vec(15,  1), vec( 1, 14), RIGHT);
    }

    static void drawSideIndicator(BG0ROMDrawable &draw, Neighborhood &nb,
        Int2 topLeft, Int2 size, Side s)
    {
        unsigned nbColor = draw.ORANGE;
        draw.fill(topLeft, size,
            nbColor | (nb.hasNeighborAt(s) ? draw.SOLID_FG : draw.SOLID_BG));
    }
};


void main()
{
    static SensorListener sensors;

    sensors.install();
    
    for (int i = 0; i < CUBE_ALLOCATION; i++) {
    	sensors.writeScreen(i);
    }
    
    SystemTime startTime = SystemTime::now();
	TimeDelta elapsed = SystemTime::now() - startTime;
	int elapsedSeconds = 0;
	String<32> clear1, win, money_str1, money_str2;
	bool change1, change2, change3, change4, change5, change6, change7, change8;
	change1 = change2 = change3 = change4 = change5 = change6 = change7 = change8 = false;

    // We're entirely event-driven. Everything is
    // updated by SensorListener's event callbacks.
    while (1) {
        System::paint();
        elapsed = SystemTime::now() - startTime;
        elapsedSeconds = (int)elapsed.seconds();
        LOG("%d", elapsedSeconds);
        if (elapsedSeconds == 1) {
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 20 && change1 == false) {
        	change1 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 40 && change2 == false) {
        	change2 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
			AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 60 && change3 == false) {
        	change3 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 80 && change4 == false) {
        	change4 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 100 && change5 == false) {
        	change5 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 120 && change6 == false) {
        	change6 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 140 && change7 == false) {
        	change7 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds == 160 && change8 == false) {
        	change8 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        	sensors.computeSavings();
        	AudioChannel(1).play(Bell);
        }
        else if (elapsedSeconds >= 180) {
        	AudioChannel(1).play(Bell);
        	endGame = true;
        }
        if (endGame) {
        	String<32> str;
        	int winning_player = 0;
        	for (int j = 0; j < CUBE_ALLOCATION; j++) {
        		if ((player_money[j] + player_savings[j]) > ( player_money[winning_player] + player_savings[winning_player])) {
        			winning_player = j;
        		}
        	}
        	clear1 << "                  ";
    		
        	str << "Player " << winning_player << " wins!!";
        	for (int j = 0; j < CUBE_ALLOCATION; j++) {
        		vid[j].bg0rom.text(vec(1,1), clear1);
        		vid[j].bg0rom.text(vec(1,3), clear1);
    			vid[j].bg0rom.text(vec(1,10), clear1);
    			vid[j].bg0rom.text(vec(1,4), clear1);
    			vid[j].bg0rom.text(vec(1,6), clear1);
    			vid[j].bg0rom.text(vec(1,8), clear1);
    			vid[j].bg0rom.text(vec(1,10), clear1);
    			vid[j].bg0rom.text(vec(1,12), clear1);
    			vid[j].bg0rom.text(vec(1,14), clear1);
    			if (j != winning_player) {
        			vid[j].bg0rom.text(vec(1,6), str);
        			money_str1 << "Your money: ";
        			vid[j].bg0rom.text(vec(1,8), money_str1);
        			money_str2 << "$ " << (player_money[j] + player_savings[j]);
        			vid[j].bg0rom.text(vec(1,10), money_str2);
        		}
        		money_str1.clear();
        		money_str2.clear();
        	}
        	str.clear();
        	clear1.clear();
        	win << "You win!";
        	money_str1 << "Your money: ";
        	money_str2 << "$ " << (player_money[winning_player] + player_savings[winning_player]);
        	vid[winning_player].bg0rom.text(vec(1,6), win);
        	vid[winning_player].bg0rom.text(vec(1,8), money_str1);
        	vid[winning_player].bg0rom.text(vec(1,10), money_str2);
        	money_str1.clear();
        	money_str2.clear();
        	win.clear();
        }
    }
}
