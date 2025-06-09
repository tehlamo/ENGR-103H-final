///////////////////////////////////////////////////////////////////////////////
// Circuit Playground D6 Dice
// 
// Roll them bones.
//
// Author: Carter Nelson
// MIT License (https://opensource.org/licenses/MIT)
///////////////////////////////////////////////////////////////////////////////
// Circuit Playground Total Acceleration
//
// Author: Carter Nelson
// MIT License (https://opensource.org/licenses/MIT)

#include <Adafruit_CircuitPlayground.h>
#include <string.h>

#define DICE_COLOR          0xEA6292    // Dice digits color

const byte leftButton = 4; // Left button
const byte rightButton = 5; // Right button
const byte switchPin = 7; // Switch
volatile bool leftFlag = false; // Interrupt flags
volatile bool rightFlag = false;
volatile bool switchFlag = false;
bool switchState = false; // Switch state
bool gameState = false; // Game state
bool fightState = false; // Check if in fight
bool switchCallout = true; // Check if switch is in the right position to start fight
bool potState = false; // Check if in potion state
bool attackState = false; // Check if attacking
bool blockState = false; // Check if blocking
bool normBlock = false; // Blocked normally
bool diceBlock = false; // Blocked with dice
bool normAttack = false; // Attacked normally
bool diceAttack = false; // Attacked with dice
bool playerStunned = false; // Check if stunned
bool enemyStunned = false; // Check if enemy stunned
bool shakeDetected = false; // Check acclerometer
bool enemyAttack = false; // Check if enemy attacked
bool enemyAlive = false; // Check if enemy alive
bool waitingRoll = false; // Dice roll wait
bool rolled = false; // Dice rolled
bool numberGiven = false; // Number was given
bool items = false; // Make sure items are on
bool inCardSelection = false; // Selection area
bool nextTurn = false; // Next turn/round
bool gameOver = false; // Game over

int cardRandomizer1; // Random card gens
int cardRandomizer2;
int cardRandom1;
int cardRandom2;

int block; // Dictates block chance
int savedNumber; // Saves a dice roll
int sPotsTaken = 0; // Strength potion counter

class enemies { // Enenmy class
  public:
    int eDmg;
    int eHp;
    int blockChance;
    String eName;
};

enemies enemyOrder[22]; // Array of enemies
enemies currentEnemy; // Shows the current enemy

class player{ // Player stats
  public:
    int pDmg = 4;
    int pHp = 10;
    int pMaxHp = 10;
    int hPots = 0;
    int sPots = 0;
    int tempDmg;
    int score = 0;
    String pName;
};

player p1; // Template player
player currentPlayer; // The player of the game

bool hPotCard = false; // All different generated cards
bool sPotCard = false;
bool hCard = false;
bool sCard = false;

unsigned long rollStartTime; // Used for dice roll
bool rolling;
bool newRoll;
uint8_t rollNumber;
float X, Y, Z, totalAccel;

int lowRoll = 0;
int highRoll = 0;

uint8_t dicePixels[6][6] = {  // Pixel pattern for dice roll
  { 2, 0, 0, 0, 0, 0 } ,      // Roll = 1
  { 4, 9, 0, 0, 0, 0 } ,      //        2
  { 0, 4, 7, 0, 0, 0 } ,      //        3
  { 1, 3, 6, 8, 0, 0 } ,      //        4
  { 0, 2, 4, 5, 9, 0 } ,      //        5
  { 0, 2, 4, 5, 7, 9 } ,      //        6
};

///////////////////////////////////////////////////////////////////////////////
/*
void tapCallback() {
  tapDetected = true;
}
*/

///////////////////////////////////////////////////////////////////////////////
void setup() {
  CircuitPlayground.begin();

  pinMode(leftButton, INPUT_PULLDOWN); // Buttons and switch
  pinMode(rightButton, INPUT_PULLDOWN);
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(leftButton), ileft, FALLING); // Interrupts
  attachInterrupt(digitalPinToInterrupt(rightButton), iright, FALLING);
  attachInterrupt(digitalPinToInterrupt(switchPin), iswitch, CHANGE);

  Serial.begin(9600);
  while(!Serial);

  enemyOrder[0] = {2, 6, 50, "Gremlin"}; // List of all the enemies within the game
  enemyOrder[1] = {6, 2, 20, "Goblin"};
  enemyOrder[2] = {1, 17, 80, "Dwarf"};
  enemyOrder[3] = {4, 12, 40, "Treens"};
  enemyOrder[4] = {6, 10, 0, "Drunk"};
  enemyOrder[5] = {2, 15, 10, "Crapper"};
  enemyOrder[6] = {5, 13, 50, "Ogre"};
  enemyOrder[7] = {8, 5, 0, "Hitman"};
  enemyOrder[8] = {1, 4, 95, "Ninja"};
  enemyOrder[9] = {6, 14, 60, "Paladin"};
  enemyOrder[10] = {10, 2, 10, "Sparrow"};
  enemyOrder[11] = {15, 1, 0, "Glass Cannon"};
  enemyOrder[12] = {8, 16, 50, "Dark Knight"};
  enemyOrder[13] = {12, 12, 0, "Giant"};
  enemyOrder[14] = {2, 50, 0, "Blubber"};
  enemyOrder[15] = {10, 20, 50, "Doubly"};
  enemyOrder[16] = {1, 1, 99, "Blocky"};
  enemyOrder[17] = {17, 21, 20, "Monarch"};
  enemyOrder[18] = {21, 17, 80, "Spawn"};
  enemyOrder[19] = {30, 15, 10, "Lavagoulge"};
  enemyOrder[20] = {20, 30, 50, "Gaurdian of Tulk"};
  enemyOrder[21] = {43, 68, 66, "Tulk, Leader of Worldenders"};

  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("What is your name?");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");

  while (Serial.available() == 0) {
    //Wait in the empty loop until input is made and entered
  }

  p1.pName = Serial.readStringUntil('\n');

  
  Serial.print("Ready to play, ");
  Serial.print(p1.pName);
  Serial.println("? Here are the basic rules of the game!");
  Serial.println("");
  delay(3000);
  Serial.println("This is a rogue-like game where randomized enemies will appear based on your current score. Your hp, dmg, score, and potions will be presented to you.");
  Serial.println("");
  delay(3000);
  Serial.println("You will have the first turn, and can either choose to block, attack, or to skip your turn. Tap the left button to choose to block, the right to attack, or shake to skip.");
  Serial.println("");
  delay(3000);
  Serial.println("If you chose attack, you can either roll a dice to add or subtract dmg through RNG by double tapping the right button, or attack normally by tapping the right button once.");
  Serial.println("");
  delay(3000);
  Serial.println("If the enemy blocks your attack, you will be stunned for a turn. If you block an enemies attack, they become stunned for a turn. If you both block, you become stunned for a turn.");
  Serial.println("");
  delay(3000);
  Serial.println("If you both attack, then you will attack first, then the enemy, and even if the enemy's hp reaches 0 or below, you will still be hit, similar to martyrdom.");
  Serial.println("");
  delay(3000);
  Serial.println("After each enemy is defeated, you can choose between two options randomly chosen for you to add to your own stats/items to collect.");
  Serial.println("");
  delay(3000);
  Serial.print("Once you reach a score of 60, you will complete the game and win, where the game will be restarted. Note that no high score will be saved.");
  Serial.println("");
  delay(3000);
  Serial.println("Now you can hit both buttons to start the game!");
  delay(3000);
}

///////////////////////////////////////////////////////////////////////////////
void loop() {
  block = random(1, 101); // Randomize block pattern

  if (leftFlag && rightFlag && !gameState && !gameOver) { // Starting the game
    delay(5);
    Game();
    gameState = true;
    switchState = digitalRead(switchPin);
    leftFlag = false;
    rightFlag = false;
  } else if ((leftFlag || rightFlag) && !gameState) { // Prevent pressing one then the other at different intervals
    delay(5);
    leftFlag = false;
    rightFlag = false;
  }

  if (switchFlag) { // Reads the switch position
    delay(5);
    switchState = digitalRead(switchPin);
    switchFlag = false;
  }

  if (switchState && gameState && !fightState && switchCallout && enemyAlive && !playerStunned) { // Ensure the switch is on the right side
    SwitchCallout();
    switchCallout = false;
  } else if (!switchState && gameState && !fightState && enemyAlive) { // If on right side start the fight
    Fight();
    fightState = true;
    switchCallout = true;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && !potState && enemyAlive && !playerStunned) { // If switch flipped during fight, toggle potion menu
    Pots();
    // potState = true;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && potState && enemyAlive && !playerStunned) { // If switch flipped during potion menu, back to fight
    Fight();
    // potState = false;
  }

  if ((switchState || !switchState) && gameState && fightState && !attackState && !blockState && (potState || !potState) && shakeDetected && enemyAlive && !playerStunned) { // Shake to skip your turn
    SkipTurn();
    shakeDetected = false;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && !potState && leftFlag && enemyAlive && !playerStunned) { // Blocking with left button
    delay(5);
    Block();
    // blockState = true;
    leftFlag = false;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && !potState && rightFlag && enemyAlive && !playerStunned) { // Attacking with right button
    delay(5);
    Attack();
    // attackState = true;
    rightFlag = false;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && potState && leftFlag && (currentPlayer.hPots > 0) && enemyAlive && !playerStunned) { // Check for pots
    delay(5);
    HealthPot();
    leftFlag = false;
  } else if (switchState && gameState && fightState && !attackState && !blockState && potState && leftFlag && (currentPlayer.sPots == 0) && enemyAlive && !playerStunned) { // No pots and get sent back to menu
    delay(5);
    NoPots();
    Pots();
    leftFlag = false;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && potState && rightFlag && (currentPlayer.sPots > 0) && enemyAlive && !playerStunned) { // Check for pots
    delay(5);
    StrengthPot();
    rightFlag = false;
  } else if (switchState && gameState && fightState && !attackState && !blockState && potState && rightFlag && (currentPlayer.sPots == 0) && enemyAlive && !playerStunned) { // No pots and get sen back to menu
    delay(5);
    NoPots();
    Pots();
    rightFlag = false;
  }

  if (!switchState && gameState && fightState && !attackState && blockState && !potState && !normBlock && !diceBlock && leftFlag && enemyAlive && !playerStunned) { // Normal block chosen with left
    delay(5);
    // normBlock = true;
    NormBlock();
    // Serial.print(normBlock);
    leftFlag = false;
    // delay(5000);
  }

  if (!switchState && gameState && fightState && !attackState && blockState && !potState && !normBlock && !diceBlock && rightFlag && enemyAlive && !playerStunned) { // Dice block chosen with right
    delay(5);
    // diceBlock = true;
    DiceRoll();
    rightFlag = false;
  }

  if (!switchState && gameState && fightState && attackState && !blockState && !potState && !normAttack && !diceAttack && leftFlag && enemyAlive && !playerStunned) { // Normal attack chosen with left
    delay(5);
    // normAttack = true;
    NormAttack();
    leftFlag = false;
  }

  if (!switchState && gameState && fightState && attackState && !blockState && !potState && !normAttack && !diceAttack && rightFlag && enemyAlive && !playerStunned) { // Dice attack chosen with right
    delay(5);
    // diceAttack = true;
    DiceRoll();
    rightFlag = false;
  }
  
  X = 0; // Accelerometer calculations
  Y = 0;
  Z = 0;
  for (int i=0; i<10; i++) {
    X += CircuitPlayground.motionX();
    Y += CircuitPlayground.motionY();
    Z += CircuitPlayground.motionZ();
    delay(1);
  }
  X /= 10;
  Y /= 10;
  Z /= 10;

  totalAccel = sqrt(X*X + Y*Y + Z*Z); // Actual calculation
  
  // Check for rolling
  if ((totalAccel > 20) && (switchState || !switchState) && gameState && fightState && !blockState && !attackState && !normAttack && !diceAttack && !normBlock && !diceBlock && (potState || !potState) && !shakeDetected && !waitingRoll && !rolled && !numberGiven) { // Shake and call set shake to true
    // shakeDetected = true;
    DetectShake();
  }

  if (!switchState && gameState && fightState && (blockState || attackState) && !normAttack && !normBlock && (diceAttack || diceBlock) && !potState && !shakeDetected && !waitingRoll && !enemyAttack && !rolled && !numberGiven) { // Wait for player to roll dice
    // DiceRoll();
    // waitingRoll = true;
    // shakeDetected = false;
    WaitingRoll();
  }

  if ((totalAccel > 20) && !switchState && gameState && fightState && (blockState || attackState) && !normAttack && !normBlock && (diceAttack || diceBlock) && !potState && !shakeDetected && waitingRoll && !enemyAttack && !rolled && !numberGiven) { // Roll the dice action
    // shakeDetected = true;
    DetectShake();
    rollStartTime = millis();
    rolling = true;
    newRoll = true;
    // waitingRoll = false;
    AfterRoll();
  }

  if (!switchState && gameState && fightState && blockState && !attackState && !normAttack && !diceAttack && !normBlock && diceBlock && !potState && shakeDetected && !waitingRoll && !enemyAttack && rolled && !numberGiven) { // Use dice in dice block choice
    DiceBlock();
    EnemyAttack();
    shakeDetected = false;
    // numberGiven = true;
  }

  if (!switchState && gameState && fightState && !blockState && attackState && !normAttack && diceAttack && !normBlock && !diceBlock && !potState && shakeDetected && !waitingRoll && !enemyAttack && rolled && !numberGiven) { // Use dice in dice attack choice
    DiceAttack();
    EnemyAttack();
    shakeDetected = false;
    // numberGiven = true;
  }

  if (!switchState && gameState && fightState && (blockState || attackState) && (normAttack || diceAttack || normBlock || diceBlock) && !potState && !shakeDetected && !waitingRoll && !enemyAttack && !rolled && numberGiven) { // Check if enemy is alive or if you have 60 points
    EnemyCheck();
  }

  if (!switchState && gameState && fightState && (blockState || attackState) && !normAttack && !normBlock && (diceAttack || diceBlock) && !potState && shakeDetected && !waitingRoll && !enemyAttack && !rolled && !numberGiven) { // Start roll
    if (newRoll) {
      if (millis() - rollStartTime > 3000) rolling = false; // erial.print("  Something");
    }

    if (rolling) { // Continue rolling for a while
      // rollNumber = random(1, 7);
      // Make some noise and show the dice roll number
      CircuitPlayground.playTone(random(400,2000), 20, false);        
      CircuitPlayground.clearPixels();
      for (int p=0; p<rollNumber; p++) {
        CircuitPlayground.setPixelColor(dicePixels[rollNumber-1][p], DICE_COLOR);
      }    
      delay(20);    
    } else if (newRoll) {
      // Show the dice roll number
      newRoll = false;
      CircuitPlayground.clearPixels();
      for (int p=0; p<rollNumber; p++) { // Choose a number to land on
        CircuitPlayground.setPixelColor(dicePixels[rollNumber-1][p], DICE_COLOR);
      }

      savedNumber = rollNumber; // Save the number
      // Serial.print(savedNumber);
      // shakeDetected = false;
      rolled = true; // Dice is rolled
    }
  }

  if (!shakeDetected) { // LEDs for HP
    const int hpLEDs[5] = {0, 1, 2, 3, 4};

    int HP = map(currentPlayer.pHp, 0, currentPlayer.pMaxHp, 0, 5);

    for (int i = 0; i < 5; i++) {
      if (i < HP) {
        // This "block" is still present (full)
        CircuitPlayground.setPixelColor(hpLEDs[i], 0x00FF00);  // Green
      } else {
        // This "block" is missing (damaged)
        CircuitPlayground.setPixelColor(hpLEDs[i], 0);          // Off
      }
    }
  }

  if (playerStunned && !enemyStunned) { // Player is stunned LEDs
    PlayerStunLEDs();
  }

  if (!playerStunned && enemyStunned) { // Enemy is stunned LEDs
    EnemyStunLEDs();
  }

  if (!playerStunned && !enemyStunned && !shakeDetected) { // Idle LEDs
    Idle();
  }

  if (hPotCard || sPotCard || hCard || sCard) { // Check if cards have been chosen to allow player to select them
    if (leftFlag) {
      delay(5);
      CardSelect1();
      leftFlag = false;
    } else if (rightFlag) {
      delay(5);
      CardSelect2();
      rightFlag = false;
    }
  }

  if (!inCardSelection && !gameState && !enemyAlive && !fightState && nextTurn && !gameOver) { // ALlows player to return to the game after selecting a card
    Game();
    gameState = true;
    nextTurn = false;
  }
  // Rolling momentum
  // Keep rolling for a period of time even after shaking has stopped.

  rollNumber = random(1, 7); // Randomize rolling
  
}

void Game() { // Game function that chooses an enemy
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Choosing an enemy");
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.println(".");
  delay(1000);

  if (currentPlayer.score > 0) {
    currentPlayer = currentPlayer;
  } else {
    currentPlayer = p1;
  }


  int scoreToRange = map(currentPlayer.score, 0, 60, 0, 20); // Enemies picked based on score

  switch (scoreToRange) { // Random enemy picker
    case 0:
      currentEnemy = enemyOrder[random(0, 2)];
      break;
    case 1:
      currentEnemy = enemyOrder[random(1, 3)];
      break;
    case 2:
      currentEnemy = enemyOrder[random(2, 4)];
      break;
    case 3:
      currentEnemy = enemyOrder[random(3, 5)];
      break;
    case 4:
      currentEnemy = enemyOrder[random(4, 6)];
      break;
    case 5:
      currentEnemy = enemyOrder[random(5, 7)];
      break;
    case 6:
      currentEnemy = enemyOrder[random(6, 8)];
      break;
    case 7:
      currentEnemy = enemyOrder[random(7, 9)];
      break;
    case 8:
      currentEnemy = enemyOrder[random(8, 10)];
      break;
    case 9:
      currentEnemy = enemyOrder[random(9, 11)];
      break;
    case 10:
      currentEnemy = enemyOrder[random(10, 12)];
      break;
    case 11:
      currentEnemy = enemyOrder[random(11, 13)];
      break;
    case 12:
      currentEnemy = enemyOrder[random(12, 14)];
      break;
    case 13:
      currentEnemy = enemyOrder[random(13, 15)];
      break;
    case 14:
      currentEnemy = enemyOrder[random(14, 16)];
      break;
    case 15:
      currentEnemy = enemyOrder[random(15, 17)];
      break;
    case 16:
      currentEnemy = enemyOrder[random(16, 18)];
      break;
    case 17:
      currentEnemy = enemyOrder[random(17, 19)];
      break;
    case 18:
      currentEnemy = enemyOrder[random(18, 20)];
      break;
    case 19:
      currentEnemy = enemyOrder[19, 21];
      break;
    case 20:
      currentEnemy = enemyOrder[21];
  }

  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("You've encountered a ");
  Serial.print(currentEnemy.eName);
  Serial.println("!");
  Serial.println("");
  delay(2000);

  if (currentEnemy.eHp > 0) {
    enemyAlive = true;
  }
  
}

void Fight() { // Actual fighting
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Your HP: ");
  Serial.print(currentPlayer.pHp);
  if (sPotsTaken > 0) { // Check for how many pots used to boost DMG
    Serial.print("    Your Temporary DMG: ");
    Serial.print(currentPlayer.tempDmg);
  } else if (sPotsTaken == 0) {
    Serial.print("    Your DMG: ");
    Serial.print(currentPlayer.pDmg);
  }
  Serial.print("    Your Health Potions: ");
  Serial.print(currentPlayer.hPots);
  Serial.print("    Your Strength Potions: ");
  Serial.print(currentPlayer.sPots);
  Serial.print("    Your Score: ");
  Serial.print(currentPlayer.score);
  Serial.println("");
  Serial.println("");
  Serial.print(currentEnemy.eName);
  Serial.print("'s HP: ");
  Serial.print(currentEnemy.eHp);
  Serial.print("    ");
  Serial.print(currentEnemy.eName);
  Serial.print("'s DMG: ");
  Serial.print(currentEnemy.eDmg);
  Serial.println("");
  Serial.println("");
  Serial.print("Will you block or will you attack? Right button is block, left button is attack. If you want to skip, shake the Arduino!");
  Serial.println("");
  Serial.print("If you need to heal, you may flip the switch to access your potions. Left button for your health potions, and right button for strength potions.");

  fightState = true;
  potState = false; // Won't open the potion menu
}

void Pots() { // Potion menu
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("You currently have ");
  Serial.print(currentPlayer.hPots);
  Serial.print(" health potions and ");
  Serial.print(currentPlayer.sPots);
  Serial.print(" strength potions.");
  Serial.println("");
  Serial.println("Press the left button to use a health potion for +10 HP (does not exceed your max HP), and press the right button to deal +4 DMG to your next attack!");
  Serial.println("");
  Serial.print("Hit the switch if you don't have any potions, or if you don't want to use any to go back to fighting!");

  potState = true; // Potion menu open
}

void DiceRoll() { // Wait for player to roll the dice
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Shake the Arduino to roll the dice!");

  if (attackState) {
    diceAttack = true; // Check off which you're rolling for
  } else if (blockState) {
    diceBlock = true;
  }
}

void DetectShake() { // State of shake
  shakeDetected = true;
}

void WaitingRoll() { // After the shake detected, let the dice roll
  waitingRoll = true;
  shakeDetected = false;
}

void AfterRoll() { // Roll is finished
  waitingRoll = false;
}

void Block() { // Choosing block
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to block! If the enemy attacks they become stunned, and if they block, you become stunned!");
  Serial.println("");
  Serial.print("Press the left button again to do a normal block, or press the right button to roll for damage dealt to the target or to you!");

  blockState = true; // Set blcoking to be true
}

void NormBlock() { // Chose to normal block
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to block normally!");
  Serial.println("");

  normBlock = true; // Differentiate the two
  diceBlock = false;

  EnemyAttack(); // Enemy's turn
  enemyAttack = true;
}

void DiceBlock() {
  Serial.println("");
  Serial.println("");
  Serial.print("You rolled a ");
  Serial.print(savedNumber);
  Serial.print("!");
  delay(3000);

  rolled = false; // Roll is done
}

void Attack() { // Check for attacking
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to attack! If the enemy attacks you both fight, and if they block, you become stunned!");
  Serial.println("");
  Serial.print("Press the left button again to do a normal attack, or press the right button to roll for damage dealt!");

  attackState = true; // Attacking
}

void NormAttack() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to attack normally!");
  Serial.println("");

  normAttack = true; // Differentiate with dice or no dice
  diceAttack = false;

  EnemyAttack(); // ENemy's turn
  enemyAttack = true;
}

void DiceAttack() { // Activate dice within your attack
  Serial.println("");
  Serial.println("");
  Serial.print("You rolled a ");
  Serial.print(savedNumber);
  Serial.print("!");
  delay(3000);

  // shakeDetected = false;
  rolled = false; // Roll finished
}

void HealthPot() { // Check for use of health pot
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");

  if ((currentPlayer.pHp + 10) < (currentPlayer.pMaxHp + 10)) { // Ensures HP doesn't exceed max
    if ((currentPlayer.pHp + 10) > currentPlayer.pMaxHp) {
      currentPlayer.pHp = currentPlayer.pMaxHp;
      Serial.print("You are now at max HP!");
      currentPlayer.hPots = currentPlayer.sPots - 1;
    } else if ((currentPlayer.pHp + 10) < currentPlayer.pMaxHp) {
      currentPlayer.pHp = currentPlayer.pHp + 10;
      Serial.print("You have healed by 10 HP!");
      currentPlayer.hPots = currentPlayer.hPots - 1;
    }
  } else if ((currentPlayer.pHp + 10) >= (currentPlayer.pMaxHp + 10)) { // Ensure health potions aren't wasted
    Serial.print("You are unable to use any Health potions as you are already at max HP."); 
  }
  delay(3000);

  Pots(); // Bring back to potion menu
}

void StrengthPot() { // Strength pot
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  sPotsTaken++; // Adds to the counter
  
  currentPlayer.tempDmg = currentPlayer.pDmg + (4 * sPotsTaken); // Continually makes the DMG stonger for one round
}

void NoPots() { // If no pots, don't do anything
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Currently, you have 0");
  if (leftFlag) {
    delay(5);
    Serial.print(" Health potions, ");
    leftFlag = false;
  } else if (rightFlag) {
    delay(5);
    Serial.print(" Strength potions, ");
    rightFlag = false;
  }
  Serial.print(" and are unable to use any.");
  delay(2000);
}

void EnemyAttack() { // Enemy's turn
  if (!enemyStunned) { // Check to see if enemy is stunned
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print(currentEnemy.eName);
    Serial.print(" is choosing what to do");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.println(".");
    Serial.println("");
    delay(1000);
    // Serial.print(block);
    // Serial.print(normBlock);

    if (block <= currentEnemy.blockChance) { // Enemy is blocking
      if (normBlock || diceBlock) {
        Serial.print(currentEnemy.eName);
        Serial.println(" has decided to block and has rammed into you with their own block, stunning you!");

        if (diceBlock) { // Dice block rolls for extra damage dealt to you or them during the block
          if (savedNumber < 3) {
            lowRoll = 0;

            for (int chance = 3; chance > savedNumber; chance--) {
              lowRoll++;
            }

            currentPlayer.pHp = currentPlayer.pHp - lowRoll;
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you took ");
            Serial.print(lowRoll);
            Serial.print(" extra damage!");
          } else if (savedNumber >= 3) {
            highRoll = 0;

            for (int chance = 3; chance < savedNumber; chance++) {
              highRoll++;
            }

            currentEnemy.eHp = currentEnemy.eHp - highRoll;
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you dealt ");
            Serial.print(highRoll);
            Serial.print(" extra damage!");
          }
        }
        
        playerStunned = true; // Player will always be stunned when block on block
        delay(3000);
      } else if (normAttack || diceAttack) { // Stunned if attacking
        Serial.print(currentEnemy.eName);
        Serial.print(" has decided to block and has blocked your attack, stunning you!");
        playerStunned = true;
        delay(3000);
      }
    } else if (block > currentEnemy.blockChance) { // Check if enemy is attacking
      if (normBlock || diceBlock) {
        Serial.print(currentEnemy.eName);
        Serial.print(" decided to attack and you blocked it's attack, stunning it!");
        enemyStunned = true;

        if (diceBlock) { // Deal or take extra damage while blocking
          if (savedNumber < 3) {
            lowRoll = 0;

            for (int chance = 3; chance > savedNumber; chance--) {
              lowRoll++;
            }

            currentPlayer.pHp = currentPlayer.pHp - lowRoll;

            Serial.println("");
            Serial.println("");
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you took ");
            Serial.print(lowRoll);
            Serial.print(" extra damage!");
          } else if (savedNumber >= 3) {
            highRoll = 0;

            for (int chance = 3; chance < savedNumber; chance++) {
              highRoll++;
            }

            currentEnemy.eHp = currentEnemy.eHp - highRoll;

            Serial.println("");
            Serial.println("");
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you dealt ");
            Serial.print(highRoll);
            Serial.print(" extra damage!");
          }
        }
      } else if (normAttack || diceAttack) { // Attacking while attacking
        if (diceAttack) { // Roll for more or less damage dealt on attack
          if (sPotsTaken > 0) {
            if (savedNumber < 3) {
              // Serial.print("low check");
              lowRoll = 0;

              for (int chance = 3; chance > savedNumber; chance--) {
                lowRoll++;
              }

              currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg + lowRoll;

              Serial.println("");
              Serial.println("");
              Serial.print("Because you rolled a ");
              Serial.print(savedNumber);
              Serial.print(", you dealt ");
              Serial.print(lowRoll);
              Serial.print(" less damage! ");
            } else if (savedNumber >= 3) {
              // Serial.print("high check");
              highRoll = 0;

              for (int chance = 3; chance < savedNumber; chance++) {
                highRoll++;
                // Serial.print("high for");
              }

              currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg - highRoll;

              Serial.println("");
              Serial.println("");
              Serial.print("Because you rolled a ");
              Serial.print(savedNumber);
              Serial.print(", you deal ");
              Serial.print(highRoll);
              Serial.print(" extra damage!");
            }
          } else if (sPotsTaken == 0) { // If none strength potions are taken at all
            if (savedNumber < 3) {
              // Serial.print("low check");
              lowRoll = 0;

              for (int chance = 3; chance > savedNumber; chance--) {
                lowRoll++;
              }

              currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg + lowRoll;

              Serial.println("");
              Serial.println("");
              Serial.print("Because you rolled a ");
              Serial.print(savedNumber);
              Serial.print(", you dealt ");
              Serial.print(lowRoll);
              Serial.print(" less damage! ");
            } else if (savedNumber >= 3) {
              // Serial.print("high check");
              highRoll = 0;

              for (int chance = 3; chance < savedNumber; chance++) {
                highRoll++;
                // Serial.print("high for");
              }

              currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg - highRoll;

              Serial.println("");
              Serial.println("");
              Serial.print("Because you rolled a ");
              Serial.print(savedNumber);
              Serial.print(", you deal ");
              Serial.print(highRoll);
              Serial.print(" extra damage!");
            }
          }
        } else if (normAttack) { // Normal attack damage with and without potion
          if (sPotsTaken > 0) {
            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg;
          } else if (sPotsTaken == 0) {
            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg;
          }
        }

        Serial.println("");
        Serial.println("");
        Serial.print("The ");
        Serial.print(currentEnemy.eName);
        Serial.println(" fights back and deals damage back to you!");
        delay(1000);

        // currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg;
        Serial.println("");
        Serial.print("You have dealt ");
        // Serial.print(savedNumber);
        if ((savedNumber > 0) && (savedNumber < 3) && (sPotsTaken == 0)) {
          Serial.print(currentPlayer.pDmg - lowRoll);
        } else if ((savedNumber > 3) && (savedNumber < 7) && (sPotsTaken == 0)) {
          Serial.print(currentPlayer.pDmg + highRoll);
        } else if (sPotsTaken == 0) {
          Serial.print(currentPlayer.pDmg);
        } else if ((savedNumber > 0) && (savedNumber < 3) && (sPotsTaken > 0)) {
          Serial.print(currentPlayer.tempDmg - lowRoll);
        } else if ((savedNumber > 3) && (savedNumber < 7) && (sPotsTaken > 0)) {
          Serial.print(currentPlayer.tempDmg + highRoll);
        } else if (sPotsTaken > 0) {
          Serial.print(currentPlayer.tempDmg);
        }
        Serial.print(" DMG to the ");
        Serial.print(currentEnemy.eName);
        delay(1500);
        Serial.print(", and they dealt ");
        Serial.print(currentEnemy.eDmg);
        Serial.print(" DMG baack to you!");

        currentPlayer.pHp = currentPlayer.pHp - currentEnemy.eDmg; // Player takes damage
        delay(3000);
      }
    }

    Serial.println("");
    Serial.println(""); // Stats shown here
    Serial.print("You now have ");
    Serial.print(currentPlayer.pHp);
    Serial.print(" HP left, and the ");
    Serial.print(currentEnemy.eName);
    Serial.print(" has ");
    if (currentEnemy.eHp > 0) {
      Serial.print(currentEnemy.eHp);
      Serial.print(" HP left!");
    } else if (currentEnemy.eHp <= 0) {
      Serial.print(currentEnemy.eName);
      Serial.print(" 0 HP left!");
    }
    delay(2000);
    
    if (playerStunned) { // Check if anyone is stunned
      enemyAttack = false;
      LoseTurn();
    } else if (enemyStunned) {
      enemyAttack = false;
      GainTurn();
    } else if (!playerStunned && !enemyStunned) {
      numberGiven = true;
      enemyAttack = false;
      // EnemyCheck();
    }
    //numberGiven = true;
    //enemyAttack = false;
    EnemyCheck(); // Check if enemy is alive
  } else if (enemyStunned) { // Enemy stunned = free turn
    if (normBlock || diceBlock) {
      Serial.println("");
      Serial.println("");
      Serial.println("");
      Serial.print("Why did you waste your free turn on a block?");
      delay(3000);
    } else if (normAttack || diceAttack) {
      if (diceAttack) {
        if (sPotsTaken > 0) {
          if (savedNumber < 3) {
            // Serial.print("low check");
            lowRoll = 0;

            for (int chance = 3; chance > savedNumber; chance--) {
              lowRoll++;
            }

            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg + lowRoll;
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you dealt ");
            Serial.print(lowRoll);
            Serial.print(" less damage! ");
          } else if (savedNumber >= 3) {
            // Serial.print("high check");
            highRoll = 0;

            for (int chance = 3; chance < savedNumber; chance++) {
              highRoll++;
              // Serial.print("high for");
            }

            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg - highRoll;

            Serial.println("");
            Serial.println("");
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you deal ");
            Serial.print(highRoll);
            Serial.print(" extra damage!");
          }
        } else if (sPotsTaken == 0) {
          if (savedNumber < 3) {
            // Serial.print("low check");
            lowRoll = 0;

            for (int chance = 3; chance > savedNumber; chance--) {
              lowRoll++;
            }

            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg + lowRoll;
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you dealt ");
            Serial.print(lowRoll);
            Serial.print(" less damage! ");
          } else if (savedNumber >= 3) {
            // Serial.print("high check");
            highRoll = 0;

            for (int chance = 3; chance < savedNumber; chance++) {
              highRoll++;
              // Serial.print("high for");
            }

            currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg - highRoll;

            Serial.println("");
            Serial.println("");
            Serial.print("Because you rolled a ");
            Serial.print(savedNumber);
            Serial.print(", you deal ");
            Serial.print(highRoll);
            Serial.print(" extra damage!");
          }
        }
      } else if (normAttack) {
        if (sPotsTaken > 0) {
          currentEnemy.eHp = currentEnemy.eHp - currentPlayer.tempDmg;
        } else if (sPotsTaken == 0) {
          currentEnemy.eHp = currentEnemy.eHp - currentPlayer.pDmg;
        }
      }
    }
    Serial.println("");
    Serial.println("");
    Serial.print("You have dealt ");
    // Serial.print(savedNumber);
    if ((savedNumber > 0) && (savedNumber < 3) && (sPotsTaken == 0)) {
      Serial.print(currentPlayer.pDmg - lowRoll);
    } else if ((savedNumber > 3) && (savedNumber < 7) && (sPotsTaken == 0)) {
      Serial.print(currentPlayer.pDmg + highRoll);
    } else if (sPotsTaken == 0) {
      Serial.print(currentPlayer.pDmg);
    } else if ((savedNumber > 0) && (savedNumber < 3) && (sPotsTaken > 0)) {
      Serial.print(currentPlayer.tempDmg - lowRoll);
    } else if ((savedNumber > 3) && (savedNumber < 7) && (sPotsTaken > 0)) {
      Serial.print(currentPlayer.tempDmg + highRoll);
    } else if (sPotsTaken > 0) {
      Serial.print(currentPlayer.tempDmg);
    }
    Serial.print(" DMG to the ");
    Serial.print(currentEnemy.eName);
    Serial.print("!");
    delay(3000);

    enemyStunned = false;
    EnemyCheck();
  }
}

void SkipTurn() { // Shaking skips turn and stuns any enemy that blocks itself
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Because you decided to skip your turn, ");
  Serial.print(currentEnemy.eName);
  Serial.print(" is choosing what to do");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.println(".");
  Serial.println("");
  delay(1000);

  if (block <= currentEnemy.blockChance) {
    Serial.print(currentEnemy.eName);
    Serial.print(" has decided to waste their turn by");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(" blocking?");
    delay(3000);
    Serial.println("");
    Serial.println("");
    Serial.print("Even worse, they managed to stun themselves.");
    delay(3000);

    enemyStunned = true;
    enemyStunned2 = true;
  } else if (block > currentEnemy.blockChance) {
    Serial.print(currentEnemy.eName);
    Serial.print(" has decided to attack you, dealing ");
    Serial.print(currentEnemy.eDmg);
    Serial.print (" DMG!");
    delay(3000);

    currentPlayer.pHp = currentPlayer.pHp - currentEnemy.eDmg;
  }

  fightState = false; // Set everything else to false to ensure that game runs smoothly again in the fight
  attackState = false;
  blockState = false;
  normAttack = false;
  diceAttack = false;
  normBlock = false;
  diceBlock = false;
  enemyAttack = false;
  numberGiven = false;
  rolled = false;
  shakeDetected = false;
  waitingRoll = false;


  Fight();
}


void LoseTurn() { // You got stunned and lost a turn
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Because you are stunned, ");
  Serial.print(currentEnemy.eName);
  Serial.print(" is choosing what to do");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.print(".");
  delay(1000);
  Serial.println(".");
  Serial.println("");
  delay(1000);

  if (block <= currentEnemy.blockChance) { // Enemy gets free turn to do whatever they please
    Serial.print(currentEnemy.eName);
    Serial.print(" has decided to waste their turn by");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(" blocking?");
    delay(3000);
  } else if (block > currentEnemy.blockChance) {
    Serial.print(currentEnemy.eName);
    Serial.print(" has decided to attack you, dealing ");
    Serial.print(currentEnemy.eDmg);
    Serial.print (" DMG!");
    delay(3000);

    currentPlayer.pHp = currentPlayer.pHp - currentEnemy.eDmg;
  }

  playerStunned = false; // After, player is no longer stunned

  fightState = false;
  attackState = false;
  blockState = false;
  normAttack = false;
  diceAttack = false;
  normBlock = false;
  diceBlock = false;
  enemyAttack = false;
  numberGiven = false;
  rolled = false;
  shakeDetected = false;
  waitingRoll = false;


  Fight();
}

void GainTurn() { // You gain one extra turn (enemy stunned)
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print(currentEnemy.eName);
  Serial.print(" has been stunned!");
  Serial.println("");
  Serial.println("");
  delay(3000);

  fightState = false;
  attackState = false;
  blockState = false;
  normAttack = false;
  diceAttack = false;
  normBlock = false;
  diceBlock = false;
  enemyAttack = false;
  numberGiven = false;
  rolled = false;
  shakeDetected = false;
  waitingRoll = false;

  
  Fight();
}

void EnemyCheck() { // Checks if enemy died
  playerStunned2 = false;
  enemyStunned2 = false;

  if (currentEnemy.eHp <= 0 && !items) { // Get ready to randomize cards
    items = true;

    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have sucessfully killed the ");
    Serial.print(currentEnemy.eName);
    Serial.print("!");

    if (currentPlayer.score > 60) { // Game over: win
      Serial.println("");
      Serial.println("");
      Serial.println("");
      Serial.println("");
      Serial.println("");
      Serial.print("Game is over! You win! Unplug the Arduion to play again!");

      gameState = false;
      fightState = false;
      attackState = false;
      blockState = false;
      normAttack = false;
      diceAttack = false;
      normBlock = false;
      diceBlock = false;
      enemyAttack = false;
      numberGiven = false;
      rolled = false;
      shakeDetected = false;
      waitingRoll = false;

      gameOver = true;
    } else if (currentPlayer.score <= 60) { // Continue going if score isn't past 60 and collect items
      currentPlayer.score++;
      enemyAlive = false;
      Items();
    }

    //Game();
  } else if (currentEnemy.eHp > 0) { // If enemy still alive keep fighting
    fightState = false;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    rolled = false;
    shakeDetected = false;
    waitingRoll = false;


    Fight();
  } else if (currentPlayer.pHp <= 0) { // Player loses when HP reaches 0 or below
    gameOver = true;

    gameState = false;
    fightState = false;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    rolled = false;
    shakeDetected = false;
    waitingRoll = false;
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("Game is over! You lose! Unplug the Arduion to play again!");
  }
}

void Items() { // Card randomization
  do {
    cardRandomizer1 = random(0, 4);
    cardRandomizer2 = random(0, 4);
  } while (cardRandomizer1 == cardRandomizer2);
  
  cardRandom1 = cardRandomizer1;
  cardRandom2 = cardRandomizer2;

  Cards();
  inCardSelection = true; // Ready to select a card
}

void Cards() { // Cards are chosen and set to true if chosen
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("There are two randomized cards, one on the right and one on the left.");
  delay(3000);
  Serial.println("");
  Serial.println("");
  Serial.print("After you choose, a random item or stat will be added to your inventory or player!");
  delay(3000);

  if (cardRandom1 == 0) {
    hPotCard = true;
  } else if (cardRandom1 == 1) {
    sPotCard = true;
  } else if (cardRandom1 == 2) {
    hCard = true;
  } else if (cardRandom1 == 3) {
    sCard = true;
  }

  if (cardRandom2 == 0) {
    hPotCard = true;
  } else if (cardRandom2 == 1) {
    sPotCard = true;
  } else if (cardRandom2 == 2) {
    hCard = true;
  } else if (cardRandom2 == 3) {
    sCard = true;
  }
}

void CardSelect1() { // Player can select a card based on the right and left buttons and each is randomized but never matching with the other one
  if (cardRandom1 == 0) {
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your left, and it adds 1 Health Potion to your inventory!");
    delay(3000);

    currentPlayer.hPots++;

    hPotCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom1 == 1) { // Check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your left, and it adds 1 Strength Potion to your inventory!");
    delay(3000);

    currentPlayer.sPots++;

    sPotCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom1 == 2) { // check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your left, and it adds 4 HP to your total health!");
    delay(3000);

    currentPlayer.pMaxHp = currentPlayer.pMaxHp + 4;

    hCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom1 == 3) { // Check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your left, and it adds 1 DMG to your player DMG!");
    delay(3000);

    currentPlayer.pDmg++;

    sCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  }
}

void CardSelect2() {
  if (cardRandom2 == 0) { // Check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your right, and it adds 1 Health Potion to your inventory!");
    delay(3000);

    currentPlayer.hPots++;

    hPotCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom2 == 1) { // check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your right, and it adds 1 Strength Potion to your inventory!");
    delay(3000);

    currentPlayer.sPots++;

    sPotCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom2 == 2) { // check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your right, and it adds 4 HP to your total health!");
    delay(3000);

    currentPlayer.pMaxHp = currentPlayer.pMaxHp + 4;

    hCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  } else if (cardRandom2 == 3) { // Check what card
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("You have chosen the card on your right, and it adds 1 DMG to your player DMG!");
    delay(3000);

    currentPlayer.pDmg++;

    sCard = false;

    fightState = false;
    enemyAlive = true;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
    numberGiven = false;
    items = false;

    inCardSelection = false;
    gameState = false;
    nextTurn = true;
  }
}

void ileft() { // Left interrupt flag
  leftFlag = true;
}

void iright() { // RIght interrupt flag
  rightFlag = true;
}

void iswitch() { // Switch interrupt flag
  switchFlag = true;
}

void SwitchCallout() { // Function that calls out player for having the switch on the wrong side
  Serial.println("");
  Serial.print("Flip the switch to play the game. It can't run when the switch is on this side. The switch can be activated once you begin the fight.");
}

void PlayerStunLEDs() { // LEDs display for when the player is stunned, not working
  static unsigned long lastUpdate = 0;
  static bool toggle = false;

  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    toggle = !toggle;

    for (int red = 5; red < 10; red + 2) {
      if (toggle) {
        CircuitPlayground.setPixelColor(red, 0xFF0000);
        CircuitPlayground.setPixelColor(red + 1, 0x000000);
      } else {
        CircuitPlayground.setPixelColor(red + 1, 0xFF0000);
        CircuitPlayground.setPixelColor(red, 0x000000);
      }
    }
  }
}

void EnemyStunLEDs() { // LEDs for when the enemy is stunned
  static unsigned long lastUpdate = 0;
  static bool toggle = false;

  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    toggle = !toggle;

    for (int red = 5; red < 10; red += 2) {
      if (toggle) {
        CircuitPlayground.setPixelColor(red, 0xFFFF00);
        CircuitPlayground.setPixelColor(red + 1, 0x000000);
      } else {
        CircuitPlayground.setPixelColor(red + 1, 0xFFFF00);
        CircuitPlayground.setPixelColor(red, 0x000000);
      }
    }
  }
}

void Idle() { // Idle color for ambience
  static unsigned long lastUpdate = 0;
  static bool toggle = false;

  if (millis() - lastUpdate > 500) {
    lastUpdate = millis();
    toggle = !toggle;
  }
  static uint8_t brightness = 0;
  static bool increasing = true;
        
  for (int blue = 5; blue < 10; blue++) {
    if (toggle) {
      CircuitPlayground.setPixelColor(blue, 0, 0, brightness);

      if (increasing) {
        brightness += 5;
        if (brightness >= 255) increasing = false;
      } else {
        brightness -= 5;
        if (brightness <= 0) increasing = true;
      }
    } else {
      CircuitPlayground.setPixelColor(blue, 0, 0, brightness);

      if (increasing) {
        brightness += 5;
        if (brightness >= 255) increasing = false;
      } else {
        brightness -= 5;
        if (brightness <= 0) increasing = true;
      }
    }
  }
}