# ENGR-103H-final
EGNR 103H Final: Rogue-like fighting game.
Instead of using GitDesktop, I used VS Code.
Also created ReadMe branch.

Game begins by pressing the left and right button. After, the game will begin and an enemy will be chosen

After an enemy is chosen, you can hit the right button to choose to attack and the left button to choose to block.
If you don't want to do anything, you can shake the Arduino hard enough to skip your turn (useful against an enemy blocking).

You could also flip the switch the other way to choose between a health potion with the left button and a strength potion with the right button.
Health heals by 10 HP up to your current max health, and strength potions add 4 damage to your next attack if it lands successfully.

If you choose to attack, you can either attack normally by pressing the left button or roll a dice to add or subtract damage to your attack.

If you choose to block, you can either block normally by pressing the left button or roll a dice to add or subtract damage dealt to you or your opponenet.

If you attack while the enemy attacks, you both will attack, even if the enemy dies you will still be hit.
If you attack while the enemy blocks, you will become stunned for one turn and the enemy will have a free turn.
If you block while the enemy blocks, you will become stunned for one turn and the enemy will have a free turn.
If you block while the enemy attacks, the enemy will become stunned for one turn and you will have a free turn.

After an enemy has been defeated, your current score will increment by 1, and enemies will progressively get harder and harder, being randomized.
You will also get to choose between 2 randomized options, being +1 health potion, +1 strength potion, +4 max hp, and +1 strength.

Once you reach 60 points, you will have to fight a boss, and if you defeat the boss, you win the game.

If you die anywhere, your highest score will be saved as long as the arduino is still plugged into the computer.

Total inputs: Accelerometer detection, right button, left button, switch ==> 4 different inputs.
Total outputs: LEDs, sound => 2 different outputs.