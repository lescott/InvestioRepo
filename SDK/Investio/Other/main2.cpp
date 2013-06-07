/*
 * Sifteo SDK Example.
 */

#include <sifteo.h>
#include "assets.gen.h"
#define		BASE		32
#define		CORN		0
#define		COFFEE		1
#define		CLOTH		2
using namespace Sifteo;

static Metadata M = Metadata()
    .title("Investio")
    .package("com.sifteo.sdk.sensors", "1.1")
    .icon(Icon)
    .cubeRange(0, CUBE_ALLOCATION);
    
static AssetSlot MainSlot = AssetSlot::allocate()
	.bootstrap(GameAssets);


static VideoBuffer vid;
static TiltShakeRecognizer motion[CUBE_ALLOCATION];


enum ScreenType { MONEY = 0, MARKET_PRICES, BUY_CORN, SELL_CORN, BUY_COFFEE, SELL_COFFEE, BUY_CLOTH, SELL_CLOTH, SAVINGS, CURR_EVENTS, MARKET };
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
    		case 15:
    			market_prices[CORN] = market_prices[CORN] * 1.2;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.4;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.1;
    			break;
    		case 30:
    			market_prices[CORN] = market_prices[CORN] * 1.6;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.2;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1;
    			break;
    		case 45:
    			market_prices[CORN] = market_prices[CORN] * 1.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 0.3;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.2;
    			break;
    		case 60:
    			market_prices[CORN] = market_prices[CORN] * 1.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.3;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.5;
    			break;
    		case 75:
    			market_prices[CORN] = market_prices[CORN] * 1.3;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.1;
    			market_prices[CLOTH] = market_prices[CLOTH] * 0.7;
    			break;
    		case 90:
    			market_prices[CORN] = market_prices[CORN] * 0.5;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1;
    			market_prices[CLOTH] = market_prices[CLOTH] * 0.8;
    			break;
    		case 105:
    			market_prices[CORN] = market_prices[CORN] * 0.1;
    			market_prices[COFFEE] = market_prices[COFFEE] * 1.7;
    			market_prices[CLOTH] = market_prices[CLOTH] * 1.4;
    			break;
    		case 120:
    			endGame = true;
    			break;
    		default:
    			break;
    	}
    	market_prices[CORN] = market_prices[CORN] * 2;
    	market_prices[COFFEE] = market_prices[COFFEE] * 3;
    	market_prices[CLOTH] = market_prices[CLOTH] * 0.5;
    }
    

private:
    void onConnect(unsigned id)
    {
        CubeID cube(id);
        uint64_t hwid = cube.hwID();
        
        drawMoney(id);

        bzero(counters[id]);
        LOG("Cube %d connected\n", id);

        vid.initMode(BG0_SPR_BG1);
        vid.attach(id);
        vid.bg0.image(vec(0,0), Background);
        motion[id].attach(id);
        
        // Allocate 16x2 tiles on BG1 for text at the bottom of the screen
        vid.bg1.setMask(BG1Mask::filled(vec(0,0), vec(32, 32)));
        
        String<32> str1;
        String <32> str2;
        str1 << "Money";
        str2 << player_money[id];
        vid.bg1.text(vec(0,2), Font, str1);
        vid.bg1.text(vec(0,14), Font, str2);
        //vid[cube].bg0rom.text(vec(1,1), str1);
        //vid[cube].bg0rom.text(vec(1,10), str2);
        
        
        vid.sprites[1].setImage(Dollar);
        vid.sprites[1].move(0, 0);
        LOG("Got here.\n");
        
        
        // Keep track of the time that has elapsed. Use for constructing market prices.
        SystemTime start = SystemTime::now();
        TimeDelta el = SystemTime::now() - start;
        if ((int)el.seconds() == 5 || (int)el.seconds() == 10 || (int)el.seconds() == 15 || (int)el.seconds() == 20 || (int)el.seconds() == 25) {
        	LOG("Saved!\n");
        	computeSavings(id);
        }

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
        
        drawMoney(id);
    }
    
    void drawMoney(int id)
    {   
        vid.sprites[0].setImage(Markers);
        vid.sprites[0].move(4, 4);
        LOG("Got here.\n");
    }
    
    void computeSavings(unsigned id)
    {
    	player_savings[id] = player_savings[id] * 1.5;
    }

    void onTouch(unsigned id)
    {
        CubeID cube(id);
        counters[id].touch++;
        LOG("Touch event on cube #%d, state=%d\n", id, cube.isTouching());
        
        if (screens[id] == SAVINGS) {
        	player_savings[id] = player_savings[id] + 10;
        	player_money[id] = player_money[id] - 10;
        }
/*
        String<32> str;
        str << "touch: " << cube.isTouching() <<
            " (" << counters[cube].touch << ")\n";
        vid[cube].bg0rom.text(vec(1,9), str);
*/
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
            		LOG("Just tilted right.\n");
            		if (screens[id] < 9)
            			screens[id]++;
            		LOG("Current screen is: %d\n", screens[id]);
/*            		str.clear();
    				str << screens[id];
    				vid[id].bg0rom.text(vec(1,1), str); */
    				writeScreen(id, screens[id]);
            		break;
            	case 1:
            		LOG("Just tilted left.\n");
            		if (screens[id] > 0)
            			screens[id]--;
            		LOG("Current screen is: %d\n", screens[id]);
/*            		str.clear();
    				str << screens[id];
    				vid[id].bg0rom.text(vec(1,1), str); */
    				writeScreen(id, screens[id]);
            		break;
            	case 0:
            		LOG("Not tilted.\n");
            		break;
            }
            
/*             switch (tilt.y) {
            	case 1:
            		LOG("Just tilted up.\n");
            	case -1:
            		LOG("Just tilted down.\n");
            	case 0:
            		LOG("Not tilted.\n");
            }
*/
            
/*             switch (tilt.z) {
            	case 1:
            		LOG("Just tilted upright.\n");
            	case -1:
            		LOG("Just tilted upside down.\n");
            	case 0:
            		LOG("Not tilted.\n");
            }
*/            
/*            
            str << "tilt:"
                << Fixed(tilt.x, 3)
                << Fixed(tilt.y, 3)
                << Fixed(tilt.z, 3) << "\n";

            str << "shake: " << motion[id].shake;
*/
        }

    }
    
    void writeScreen(unsigned id, ScreenType currentScreen) {
    	String<32> str1;
    	String<32> str2;
    	String<32> str3;
    	String<32> str4;
    	String<64> clear;
    	
    	switch (currentScreen) {
    		case 0:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1.clear();
    			str2.clear();
    			str1 << "Money";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << "$ " << player_money[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 1:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1.clear();
    			str2.clear();
    			str1 << "Market Prices";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << "Corn: " << market_prices[CORN] << "\n";
    			vid.bg1.text(vec(1,10), Font, str2);
    			str3 << "Coffee: " << market_prices[COFFEE] << "\n";
    			vid.bg1.text(vec(1,6), Font, str3);
    			str4 << "Cloth: " << market_prices[CLOTH] << "\n";
    			vid.bg1.text(vec(1,8), Font, str4);
    			break;
    		case 2:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1.clear();
    			str2.clear();
    			str1 << "Buy Corn";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_corn[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 3:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Sell Corn";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_corn[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 4:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Buy Coffee";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_coffee[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 5:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Sell Coffee";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_coffee[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 6:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Buy Cloth";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_cloth[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 7:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Sell Cloth";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << units_of_cloth[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 8:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Savings";
    			vid.bg1.text(vec(1,1), Font, str1);
    			str2 << player_savings[id];
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case 9:
    			clear << "                  ";
    			vid.bg1.text(vec(1,1), Font, clear);
    			vid.bg1.text(vec(1,10), Font, clear);
    			vid.bg1.text(vec(1,4), Font, clear);
    			vid.bg1.text(vec(1,6), Font, clear);
    			vid.bg1.text(vec(1,8), Font, clear);
    			str1 << "Current Events";
    			vid.bg1.text(vec(1,10), Font, str2);
    			break;
    		case MARKET:
    			break;
    		default:
    			break;
    	}
    }
    
    void screenManager(ScreenType currentScreen, unsigned firstID, unsigned firstSide, unsigned secondID, unsigned secondSide) {
    	auto neighbors = vid.physicalNeighbors();
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
    			LOG("We are at the Buy Corn screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == SELL_CORN || screens[secondID] == MARKET) && player_money[firstID] > 10 && units_of_corn[secondID] > 10) {
    				LOG("We have a seller for corn!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[CORN];
    				units_of_corn[firstID] = units_of_corn[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[CORN];
    				units_of_corn[secondID] = units_of_corn[secondID] - 10;
    			}
    			LOG("Cube1 has %d corn. Cube2 has %d corn.\n Cube1 has %d money. Cube2 has %d money.\n", units_of_corn[firstID], units_of_corn[secondID], player_money[firstID], player_money[secondID]);
    			break;
    		case 3:
    			LOG("We are at the Sell Corn screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == BUY_CORN || screens[secondID] == MARKET) && player_money[secondID] > 10 && units_of_corn[firstID] > 10) {
    				LOG("We have a buyer for corn!!\n");
    				player_money[firstID] = player_money[firstID] + market_prices[CORN];
    				units_of_corn[firstID] = units_of_corn[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[CORN];
    				units_of_corn[secondID] = units_of_corn[secondID] + 10;
    			}
    			break;
    		case 4:
    			LOG("We are at the Buy Coffee screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == SELL_COFFEE || screens[secondID] == MARKET) && player_money[firstID] > 10 && units_of_coffee[secondID] > 10) {
    				LOG("We have a seller of coffee!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[COFFEE];
    				units_of_coffee[firstID] = units_of_coffee[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[COFFEE];
    				units_of_coffee[secondID] = units_of_coffee[secondID] - 10;
    			}
    			break;
    		case 5:
    			LOG("We are at the Sell Coffee screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == BUY_COFFEE || screens[secondID] == MARKET) && player_money[secondID] > 10 && units_of_coffee[firstID] > 10) {
    				LOG("We have a buyer for coffee!!\n");
    				player_money[firstID] = player_money[firstID] + market_prices[COFFEE];
    				units_of_coffee[firstID] = units_of_coffee[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[COFFEE];
    				units_of_coffee[secondID] = units_of_coffee[secondID] + 10;
    			}
    			break;
    		case 6:
    			LOG("We are at the Buy Cloth screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == SELL_CLOTH || screens[secondID] == MARKET) && player_money[firstID] > 10 && units_of_cloth[secondID] > 10) {
    				LOG("We have a seller for cloth!!\n");
    				player_money[firstID] = player_money[firstID] - market_prices[CLOTH];
    				units_of_cloth[firstID] = units_of_cloth[firstID] + 10;
    				player_money[secondID] = player_money[secondID] + market_prices[CLOTH];
    				units_of_cloth[secondID] = units_of_cloth[secondID] - 10;
    			}
    			break;
    		case 7:
    			LOG("We are at the Sell Cloth screen.\n");
    			if (neighbors.hasNeighborAt(TOP) && (screens[secondID] == BUY_CLOTH || screens[secondID] == MARKET) && player_money[secondID] > 10 && units_of_cloth[firstID] > 10) {
    				LOG("We have a buyer for cloth!!\n");
    				player_money[firstID] = player_money[firstID] + market_prices[CLOTH];
    				units_of_cloth[firstID] = units_of_cloth[firstID] - 10;
    				player_money[secondID] = player_money[secondID] - market_prices[CLOTH];
    				units_of_cloth[secondID] = units_of_cloth[secondID] + 10;
    			}
    			break;
    		case 8:
    			LOG("We are at the Savings screen.\n");
    			break;
    		case 9:
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
        LOG("First ID: %d\n Second ID: %d\n", firstID, secondID);
        
    }

    void drawNeighbors(CubeID cube)
    {
        Neighborhood nb(cube);
/*
        String<64> str;
        str << "nb "
            << Hex(nb.neighborAt(TOP), 2) << " "
            << Hex(nb.neighborAt(LEFT), 2) << " "
            << Hex(nb.neighborAt(BOTTOM), 2) << " "
            << Hex(nb.neighborAt(RIGHT), 2) << "\n";

        str << "   +" << counters[cube].neighborAdd
            << ", -" << counters[cube].neighborRemove
            << "\n\n";
*/
        //BG0ROMDrawable &draw = vid[cube].bg0rom;
 //       draw.text(vec(1,6), str);
/*
        drawSideIndicator(draw, nb, vec( 1,  0), vec(14,  1), TOP);
        drawSideIndicator(draw, nb, vec( 0,  1), vec( 1, 14), LEFT);
        drawSideIndicator(draw, nb, vec( 1, 15), vec(14,  1), BOTTOM);
        drawSideIndicator(draw, nb, vec(15,  1), vec( 1, 14), RIGHT);
*/
    }


/*

    static void drawSideIndicator(BG0ROMDrawable &draw, Neighborhood &nb,
        Int2 topLeft, Int2 size, Side s)
    {
        unsigned nbColor = draw.ORANGE;
        draw.fill(topLeft, size,
            nbColor | (nb.hasNeighborAt(s) ? draw.SOLID_FG : draw.SOLID_BG));
    } */
};




void main()
{
    static SensorListener sensors;

    sensors.install();
    
    SystemTime startTime = SystemTime::now();
	TimeDelta elapsed = SystemTime::now() - startTime;
	int elapsedSeconds = 0;
	bool change1, change2, change3, change4, change5, change6, change7, change8;
	change1 = change2 = change3 = change4 = change5 = change6 = change7 = change8 = false;

    // We're entirely event-driven. Everything is
    // updated by SensorListener's event callbacks.
    while (1) {
        System::paint();
        elapsed = SystemTime::now() - startTime;
        elapsedSeconds = (int)elapsed.seconds();
        LOG("%d", elapsedSeconds);
        if (elapsedSeconds == 15 && change1 == false) {
        	change1 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 30 && change2 == false) {
        	change2 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 45 && change3 == false) {
        	change3 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 60 && change4 == false) {
        	change4 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 75 && change5 == false) {
        	change5 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 90 && change6 == false) {
        	change6 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 105 && change7 == false) {
        	change7 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        else if (elapsedSeconds == 120 && change8 == false) {
        	change8 = true;
        	sensors.computeMarketPrices(elapsedSeconds);
        }
        if (endGame) {
        	
        }
    }
}
