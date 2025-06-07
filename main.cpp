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
#include <AsyncDelay.h>
#include <string.h>

#define DICE_COLOR          0xEA6292    // Dice digits color

AsyncDelay doubleTapCheck;

const byte leftButton = 4;
const byte rightButton = 5;
const byte switchPin = 7;
volatile bool leftFlag = false;
volatile bool rightFlag = false;
volatile bool switchFlag = false;
bool switchState = false;
bool gameState = false;
bool fightState = false;
bool switchCallout = true;
bool potState = false;
bool attackState = false;
bool blockState = false;
bool normBlock = false;
bool diceBlock = false;
bool normAttack = false;
bool diceAttack = false;
bool playerStunned = false;
bool enemyStunned = false;
bool shakeDetected = false;
bool enemyAttack = false;
bool enemyAlive = false;
bool rollState = false;

class enemies {
  public:
    int eDmg;
    int eHp;
    int blockChance;
    String eName;
};

int block = random(1, 101);

enemies enemyOrder[5];
enemies currentEnemy;

class player{
  public:
    int pDmg = 4;
    int pHp = 10;
    int hPots = 0;
    int sPots = 0;
    int score = 0;
    String pName;
};

player p1;


unsigned long rollStartTime;
bool rolling;
bool newRoll;
bool tapDetected;
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

  pinMode(leftButton, INPUT_PULLDOWN);
  pinMode(rightButton, INPUT_PULLDOWN);
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(leftButton), ileft, FALLING);
  attachInterrupt(digitalPinToInterrupt(rightButton), iright, FALLING);
  attachInterrupt(digitalPinToInterrupt(switchPin), iswitch, CHANGE);

  Serial.begin(9600);
  while(!Serial);

  enemyOrder[0] = {2, 6, 50, "Gremlin"};
  enemyOrder[1] = {6, 2, 10, "Goblin"};
  enemyOrder[2] = {1, 17, 80, "Dwarf"};
  enemyOrder[3] = {4, 12, 40, "Treens"};
  enemyOrder[4] = {6, 10, 0, "Drunk"};

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
  Serial.println("This is a roguelite game where randomized enemies will appear based on your current score. Your hp, dmg, score, and potions will be presented to you.");
  Serial.println("");
  delay(3000);
  Serial.println("You will have the first turn, and can either choose to block or attack. Tap the left button to choose to block and the right button to attack.");
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
  
  /*
  CircuitPlayground.setBrightness(100);
  CircuitPlayground.setAccelRange(LIS3DH_RANGE_8_G);
  CircuitPlayground.setAccelTap(2, TAP_THRESHOLD);
  
  // Setup tap detection and callback function
  attachInterrupt(digitalPinToInterrupt(4), tapCallback, RISING);
  
  // Seed the random function with light sensor value
  randomSeed(CircuitPlayground.lightSensor());

  // Initialize the global states
  newRoll = false;
  rolling = false;
  tapDetected = false;
  */
}

///////////////////////////////////////////////////////////////////////////////
void loop() {

  if (leftFlag && rightFlag && !gameState) {
    delay(5);
    Game();
    gameState = true;
    switchState = digitalRead(switchPin);
    leftFlag = false;
    rightFlag = false;
  } else if ((leftFlag || rightFlag) && !gameState) {
    delay(5);
    leftFlag = false;
    rightFlag = false;
  }

  if (switchFlag) {
    delay(5);
    switchState = digitalRead(switchPin);
    switchFlag = false;
  }

  if (switchState && gameState && !fightState && switchCallout && enemyAlive && !playerStunned && !enemyStunned) {
    SwitchCallout();
    switchCallout = false;
  } else if (!switchState && gameState && !fightState && enemyAlive) {
    Fight();
    fightState = true;
    switchCallout = true;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && !potState && enemyAlive && !playerStunned && !enemyStunned) {
    Pots();
    potState = true;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && potState && enemyAlive && !playerStunned && !enemyStunned) {
    Fight();
    potState = false;
  }

  if ((switchState || !switchState) && gameState && fightState && !attackState && !blockState && (potState || !potState) && shakeDetected && enemyAlive && !playerStunned && !enemyStunned) {
    SkipTurn();
    shakeDetected = false;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && !potState && leftFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    Block();
    blockState = true;
    leftFlag = false;
  }

  if (!switchState && gameState && fightState && !attackState && !blockState && !potState && rightFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    Attack();
    attackState = true;
    rightFlag = false;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && potState && leftFlag && (p1.hPots > 0) && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    HealthPot();
    leftFlag = false;
  } else if (switchState && gameState && fightState && !attackState && !blockState && potState && leftFlag && (p1.sPots == 0) && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    NoPots();
    Pots();
    leftFlag = false;
  }

  if (switchState && gameState && fightState && !attackState && !blockState && potState && rightFlag && (p1.sPots > 0) && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    StrengthPot();
    rightFlag = false;
  } else if (switchState && gameState && fightState && !attackState && !blockState && potState && rightFlag && (p1.sPots == 0) && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    NoPots();
    Pots();
    rightFlag = false;
  }

  if (!switchState && gameState && fightState && !attackState && blockState && !potState && !normBlock && !diceBlock && leftFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    normBlock = true;
    NormBlock();
    // Serial.print(normBlock);
    leftFlag = false;
    // delay(5000);
  }

  if (!switchState && gameState && fightState && !attackState && blockState && !potState && !normBlock && !diceBlock && rightFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    diceBlock = true;
    leftFlag = false;
  }

  if (switchState && gameState && fightState && attackState && !blockState && potState && !normAttack && !diceAttack && leftFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    NormAttack();
    normAttack = true;
    rightFlag = false;
  }

  if (switchState && gameState && fightState && attackState && !blockState && potState && !normAttack && !diceAttack && rightFlag && enemyAlive && !playerStunned && !enemyStunned) {
    delay(5);
    diceAttack = true;
    rightFlag = false;
  }

  if (!switchState && gameState && fightState && enemyAttack && enemyAlive && !playerStunned && !enemyStunned) {
    Fight();
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
  } else if (switchState && gameState && fightState && enemyAttack && enemyAlive && !playerStunned && !enemyStunned) {
    SwitchCallout();
    switchCallout = false;
    attackState = false;
    blockState = false;
    normAttack = false;
    diceAttack = false;
    normBlock = false;
    diceBlock = false;
    enemyAttack = false;
  }

  if ((switchState || !switchState) && gameState && fightState && !enemyAttack && enemyAlive && playerStunned && !enemyStunned) {
    LoseTurn();
    playerStunned = false;
  } else if ((switchState || !switchState) && gameState && fightState && !enemyAttack && enemyAlive && !playerStunned && enemyStunned) {
    GainTurn();
    enemyStunned = false;
  }


/*
  for (int i = 0; i < 5; i++) {
    Serial.print(enemyOrder[i].eHp);
    Serial.print(", ");
    Serial.print(enemyOrder[i].eDmg);
    Serial.print(", ");
    Serial.print(enemyOrder[i].blockChance);
    Serial.print(", ");
    Serial.println(enemyOrder[i].eName);
    delay(1000);
  }

  // Compute total acceleration
  // Serial.print(p1.pName);
  delay(5000);
*/
  
  
  X = 0;
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

  totalAccel = sqrt(X*X + Y*Y + Z*Z);
  
  // Check for rolling
  if ((totalAccel > 20) && (switchState || !switchState) && gameState && fightState && !blockState && !attackState && !normAttack && !diceAttack && !normBlock && !diceBlock && (potState || !potState) && !shakeDetected && !rollState) {
    shakeDetected = true;
  }

  if (!switchState && gameState && fightState && (blockState || attackState) && !normAttack && !normBlock && (diceAttack || diceBlock) && !potState && !shakeDetected && !rollState && !enemyAttack) {
    DiceRoll();
    rollState = true;
  }

  if ((totalAccel > 20) && !switchState && gameState && fightState && (blockState || attackState) && !normAttack && !normBlock && (diceAttack || diceBlock) && !potState && !shakeDetected && rollState && !enemyAttack) {
    shakeDetected = true;
    // rollState = false;
  }

  if (!switchState && gameState && fightState && blockState && !attackState && !normAttack && !diceAttack && !normBlock && diceBlock && !potState && shakeDetected && rollState && !enemyAttack) {
    rollStartTime = millis();
    newRoll = true;
    rolling = true;
    DiceBlock();
  }

  if (!switchState && gameState && fightState && !blockState && attackState && !normAttack && diceAttack && !normBlock && !diceBlock && !potState && shakeDetected && rollState && !enemyAttack) {
    rollStartTime = millis();
    newRoll = true;
    rolling = true;
    DiceAttack();
  }
  
  // Rolling momentum
  // Keep rolling for a period of time even after shaking has stopped.
  if (newRoll) {
    if (millis() - rollStartTime > 1000) rolling = false;
  }

  // Compute a random number from 1 to 6
  rollNumber = random(1,7);

  // Display status on NeoPixels
  if (rolling) {
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
    for (int p=0; p<rollNumber; p++) {
      CircuitPlayground.setPixelColor(dicePixels[rollNumber-1][p], DICE_COLOR);
    }
    delay(3000);
  }
  
}

void Game() {
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

  int scoreToRange = map(p1.score, 0, 60, 0, 20);

  switch (scoreToRange) {
    case 0:
      currentEnemy = enemyOrder[random(0, 2)];
      break;
    case 1:
      currentEnemy = enemyOrder[random(1,3)];
      break;
    case 2:
      currentEnemy = enemyOrder[random(2,4)];
      break;
    case 3:
      currentEnemy = enemyOrder[random(3,5)];
      break;
    case 4:
      currentEnemy = enemyOrder[random(4,6)];
      break;
    case 5:
      currentEnemy = enemyOrder[random(5,7)];
      break;
    case 6:
      currentEnemy = enemyOrder[random(6,8)];
      break;
    case 7:
      currentEnemy = enemyOrder[random(7,9)];
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
      currentEnemy = enemyOrder[random(19, 20)];
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

  if (currentEnemy.eHp > 0) {
    enemyAlive = true;
  }
  
}

void Fight() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Your HP: ");
  Serial.print(p1.pHp);
  Serial.print("    Your DMG: ");
  Serial.print(p1.pDmg);
  Serial.print("    Your Health Potions: ");
  Serial.print(p1.hPots);
  Serial.print("    Your Strength Potions: ");
  Serial.print(p1.sPots);
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
  Serial.print("Will you block or will you attack? Right button is block, left button is attack.");
  Serial.println("");
  Serial.print("If you need to heal, you may flip the switch to access your potions. Left button for your health potions, and right button for strength potions.");
  
}

void Pots() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("You currently have ");
  Serial.print(p1.hPots);
  Serial.print(" health potions and ");
  Serial.print(p1.sPots);
  Serial.print(" strength potions.");
  Serial.println("");
  Serial.println("Press the left button to use a health potion for +10 HP (does not exceed cap), and press the right button to deal +4 DMG to your next attack!");
  Serial.println("");
  Serial.print("Hit the switch if you don't have any potions, or if you don't want to use any to go back to fighting!");

}

void DiceRoll() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Shake the Arduino to roll the dice!");
}

void Block() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to block! If the enemy attacks they become stunned, and if they block, you become stunned!");
  Serial.println("");
  Serial.print("Press the left button again to do a normal block, or press the right button to roll for damage dealt to the target or to you!");
}

void NormBlock() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to block normally!");
  Serial.println("");

  EnemyAttack();
  enemyAttack = true;
}

void DiceBlock() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Rolling...");
  if (newRoll) {
    if (millis() - rollStartTime > 1000) rolling = false;
  }

  if (rolling) {
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
    for (int p=0; p<rollNumber; p++) {
      CircuitPlayground.setPixelColor(dicePixels[rollNumber-1][p], DICE_COLOR);
    }
    delay(3000);
  }

  Serial.println("");
  Serial.println("");
  Serial.print("You rolled a ");
  Serial.print(rollNumber);
  Serial.print("!");
  delay(3000);

  EnemyAttack();
  enemyAttack = true;
  shakeDetected = false;
}

void Attack() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to attack! If the enemy attacks you both fight, and if they block, you become stunned!");
  Serial.println("");
  Serial.print("Press the left button again to do a normal attack, or press the right button to roll for damage dealt!");
}

void NormAttack() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("You chose to attack normally!");
  Serial.println("");

  EnemyAttack();
  enemyAttack = true;
}

void DiceAttack() {
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Rolling...");
  if (newRoll) {
    if (millis() - rollStartTime > 1000) rolling = false;
  }

  if (rolling) {
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
    for (int p=0; p<rollNumber; p++) {
      CircuitPlayground.setPixelColor(dicePixels[rollNumber-1][p], DICE_COLOR);
    }
    delay(3000);
  }

  Serial.println("");
  Serial.println("");
  Serial.print("You rolled a ");
  Serial.print(rollNumber);
  Serial.print("!");
  delay(3000);

  EnemyAttack();
  enemyAttack = true;
  shakeDetected = false;
}

void HealthPot() {

}

void StrengthPot() {

}

void NoPots() {

}

void EnemyAttack() {
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

  if (block <= currentEnemy.blockChance) {
    if (normBlock || diceBlock) {
      Serial.print(currentEnemy.eName);
      Serial.println(" has decided to block and has rammed into you with their own block, stunning you!");
      playerStunned = true;

      if (diceBlock) {
        if (rollNumber < 3) {
          lowRoll = 0;

          for (int chance = 3; chance = rollNumber; chance--) {
            lowRoll++;
          }

          p1.pHp = p1.pHp - lowRoll;
          Serial.print("Because you rolled a ");
          Serial.print(rollNumber);
          Serial.print(", you took ");
          Serial.print(lowRoll);
          Serial.print(" extra damage!");
        } else if (rollNumber >= 3) {
          highRoll = 0;

          for (int chance = 3; chance = rollNumber; chance++) {
            highRoll++;
          }

          currentEnemy.eHp = currentEnemy.eHp - highRoll;
          Serial.print("Because you rolled a ");
          Serial.print(rollNumber);
          Serial.print(", you dealt ");
          Serial.print(highRoll);
          Serial.print(" extra damage!");
        }
      }
      delay(5000);
    } else if (normAttack || diceAttack) {
      Serial.print(currentEnemy.eName);
      Serial.print(" has decided to block and has blocked your attack, stunning you!");
      playerStunned = true;
      delay(5000);
    }
  } else if (block > currentEnemy.blockChance) {
    if (normBlock || diceBlock) {
      Serial.print(currentEnemy.eName);
      Serial.print(" decided to attack and you blocked it's attack, stunning it!");
      enemyStunned = true;
    } else if (normAttack || diceAttack) {
      if (diceAttack) {
        if (rollNumber < 3) {
          lowRoll = 0;

          for (int chance = 3; chance = rollNumber; chance--) {
            lowRoll++;
          }

          currentEnemy.eHp = currentEnemy.eHp - p1.pDmg + lowRoll;
          Serial.print("Because you rolled a ");
          Serial.print(rollNumber);
          Serial.print(", you dealt ");
          Serial.print(lowRoll);
          Serial.print(" less damage!");
        } else if (rollNumber >= 3) {
          highRoll = 0;

          for (int chance = 3; chance = rollNumber; chance++) {
            highRoll++;
          }

          currentEnemy.eHp = currentEnemy.eHp - p1.pDmg - highRoll;
          Serial.print("Because you rolled a ");
          Serial.print(rollNumber);
          Serial.print(", you dealt ");
          Serial.print(highRoll);
          Serial.print(" extra damage!");
        }
      }

      Serial.print("The ");
      Serial.print(currentEnemy.eName);
      Serial.println(" fights back and deals damage back to you!");
      delay(1000);

      currentEnemy.eHp = currentEnemy.eHp - p1.pDmg;
      Serial.println("");
      Serial.print("You have dealt ");
      if (rollNumber = 0) {
        Serial.print(p1.pDmg);
      } else if (rollNumber > 0 && rollNumber < 3) {
        Serial.print(p1.pDmg - lowRoll);
      } else if (rollNumber >=3 && rollNumber < 7) {
        Serial.print(p1.pDmg + highRoll);
      }
      Serial.print(" DMG to the ");
      Serial.print(currentEnemy.eName);
      delay(1500);
      Serial.print(", and they dealt ");
      Serial.print(currentEnemy.eDmg);
      Serial.print(" DMG baack to you!");
      delay(5000);
    }
  }
  EnemyCheck();
}

void SkipTurn() {

}

void LoseTurn() {
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

  if (block <= currentEnemy.blockChance) {
    Serial.print(currentEnemy.eName);
    Serial.print("Has decided to waste their turn by");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(" blocking?");
  }
}

void GainTurn() {

}

void EnemyCheck() {

}

void ileft() {
  leftFlag = true;
}

void iright() {
  rightFlag = true;
}

void iswitch() {
  switchFlag = true;
}

void SwitchCallout() {
  Serial.println("");
  Serial.print("Flip the switch to play the game. It can't run when the switch is on this side. The switch can be activated once you begin the fight.");
}