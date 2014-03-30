/*

This is a the classic "copy the sequence of coloured lights".
Originally called Simon.

There are coloured lights, each with its own button.
The computer picks a random colour, which the player then has to copy (by pressing that colour's button).
The computer then picks another colour, and adds it to the original. Now the player has to copy both colours.
This continues, with the sequence getting longer and longer until the player gets it wrong, at which point
the game is over.

*/

int COLOURS = 4;

int ledPins[ 4 ]= {6,7,8,9};
int buttonPins[ 4 ] = {2,3,4,5};
int speakerPin = 10;
int speedPin = 0;

int NONE = -1;
int colour = NONE; // The currently lit LED. 0 to 3, or NONE when no colour is lit.


int tones[4] = {700,800,900,1000}; // Each colour has a different tone

/*
  "state" determines what is to happen next, and the following are the vaues that
  "state" can have. The numbers don't mean anything (as long as they are all different).
*/
int STATE_NONE = -1;
int STATE_RESTART = 1; // Press any key to start a new game.
int STATE_LISTEN = 2;  // The program plays the sequence that the player will soon repeat.
int STATE_REPEAT = 3;  // The player is trying to repeat the sequence.

int state = STATE_RESTART;

const int MAX_SEQUENCE = 100;
int sequence[ MAX_SEQUENCE ]; // The list of random colours (0..3) in the sequence so far.
int sequenceSize = 0; // The number of items in the sequence so far
int repeatIndex = 0; // The number of colours the player has repeated.

int delayDuration = 500;

int awaitState = HIGH;

void setup()
{
  Serial.begin( 9600 );
  
  for ( int i = 0; i < COLOURS; i ++ ) {
    pinMode( ledPins[ i ], OUTPUT );
    pinMode( buttonPins[ i ], INPUT );
    digitalWrite( ledPins[ i ], LOW ); // All lights off at the beginning
  }
  pinMode( speakerPin, OUTPUT );
}

void loop()
{

  if ( state == STATE_RESTART ) {
    
    awaitRestart();
    
  } else if ( state == STATE_LISTEN ) {
    computersTurn();
    
  } else if ( state == STATE_REPEAT ) {
    playersTurn();
  }

}

void awaitRestart()
{
 
  colour ++;
  if ( (colour >= COLOURS) || (colour < 0) ) {
    colour = 0;
    awaitState = (awaitState == HIGH) ? LOW : HIGH;
  }
  
  Serial.print( awaitState ); Serial.println( colour );
  digitalWrite( ledPins[ colour ], awaitState );
  
  delayDuration = analogRead( speedPin );
  
  delay( delayDuration );
  
  for ( int i = 0; i < COLOURS; i ++ ) {
    if ( ! digitalRead( buttonPins[ i ] ) ) {

      randomSeed( millis() );
      changeState( STATE_LISTEN );
      break;
    }
  }
}

void changeState( int newState )
{
  colour = NONE;
  repeatIndex = 0;
  state = newState;

  for ( int i = 0; i < COLOURS; i ++ ) {
    digitalWrite( ledPins[ i ], LOW );
  }

}


void computersTurn()
{

  if ( sequenceSize >= MAX_SEQUENCE ) {
    // The player has remembered the maximum size sequence - they win!
  } else {
    // Add a ramdon colour to the sequence, and then play it back.
    
    int newColour = random( 4 );
    sequence[ sequenceSize ] = newColour;
    sequenceSize ++;
    playSequence( delayDuration );
    
    changeState( STATE_REPEAT );
  }
}

void playSequence( int duration )
{
  delay( duration * 2 );
  
  for ( int i = 0; i < sequenceSize; i ++ ) {
    int colour = sequence[ i ];
    digitalWrite( ledPins[ colour ], HIGH ); // Turn the LED ON
    playTone( tones[ colour ], duration );   // Play the note
    digitalWrite( ledPins[ colour ], LOW );  // Turn the LED OFF
    delay( duration / 4 );
  }
  
}

void playersTurn()
{

  if ( colour == NONE ) {
    
    for ( int i = 0; i < COLOURS; i ++ ) {
      boolean value = digitalRead( buttonPins[ i ] );
      // Serial.print( "button down? : " ); Serial.print( i ); Serial.print( " = " ); Serial.println( value );
      if ( ! value ) {
        colour = i;
        digitalWrite( ledPins[ colour ], HIGH ); // Turn the LED ON
        break;
      }
    }

    
  } else {

    boolean value = digitalRead( buttonPins[ colour ] );
    if ( ! value ) {
      // Button is still pressed
      playTone( tones[ colour ], 10 );
      
    } else {
      // Button has been released
      digitalWrite( ledPins[ colour ], LOW ); // Turn the LED OFF
      
      checkColour();
      
    }
  }
  
}


/*
  Checks that the correct colour has been chosen.
  If it isn't then the game is over, and the buzzer will sound.
  Press any key to start a new game.
*/
boolean checkColour()
{ 
  Serial.print( repeatIndex ); Serial.print( " = " ); Serial.print( sequence[ repeatIndex ] ); Serial.print( " vs " );Serial.println( colour );
  int requiredColour = sequence[ repeatIndex ];
  
  if ( requiredColour == colour ) {
    // Yes they have chosen the correct color
    repeatIndex ++;
    colour = NONE;

    if ( repeatIndex == sequenceSize ) {
      // They have repeated the whole index, so now its the computers turn
      delayDuration -= 10; // Zé, é este o comando que eu incluí para aumentar a velocidade (diminuir o delay em 10 unidades).
      changeState( STATE_LISTEN );
    }
    
  } else {
    // Errou!
    tone (speakerPin, 131, 550); // Música do Pitfall
    delay (600);
    tone (speakerPin, 147, 200);
    delay (200);
    tone (speakerPin, 156, 400);
    delay (400);
    tone (speakerPin, 131, 400);
    delay (400);
    tone (speakerPin, 185, 1000);
    delay (1000);
    sequenceSize = 0;
    changeState( STATE_RESTART );
  }
}

/*
  Plays a note.
  The pitch is different for each colour. See the array "tones".z
*/
void playTone( int tone, int duration )
{
  for ( long i = 0; i < duration * 1000L; i += tone * 2 ) {
    digitalWrite( speakerPin, HIGH );
    delayMicroseconds( tone );
    digitalWrite( speakerPin, LOW );
    delayMicroseconds( tone );
  }
}
