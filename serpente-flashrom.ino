#include <Arduino.h>
#include <SPI.h>

/* According to Serial Flasher Protocol Specification - version 1 */
#define S_ACK 0x06
#define S_NAK 0x15
#define S_CMD_NOP    0x00  /* No operation         */
#define S_CMD_Q_IFACE   0x01  /* Query interface version      */
#define S_CMD_Q_CMDMAP    0x02  /* Query supported commands bitmap    */
#define S_CMD_Q_PGMNAME   0x03  /* Query programmer name      */
#define S_CMD_Q_SERBUF    0x04  /* Query Serial Buffer Size     */
#define S_CMD_Q_BUSTYPE   0x05  /* Query supported bustypes     */
#define S_CMD_Q_CHIPSIZE  0x06  /* Query supported chipsize (2^n format)  */
#define S_CMD_Q_OPBUF   0x07  /* Query operation buffer size      */
#define S_CMD_Q_WRNMAXLEN 0x08  /* Query Write to opbuf: Write-N maximum length */
#define S_CMD_R_BYTE    0x09  /* Read a single byte       */
#define S_CMD_R_NBYTES    0x0A  /* Read n bytes         */
#define S_CMD_O_INIT    0x0B  /* Initialize operation buffer      */
#define S_CMD_O_WRITEB    0x0C  /* Write opbuf: Write byte with address   */
#define S_CMD_O_WRITEN    0x0D  /* Write to opbuf: Write-N      */
#define S_CMD_O_DELAY   0x0E  /* Write opbuf: udelay        */
#define S_CMD_O_EXEC    0x0F  /* Execute operation buffer     */
#define S_CMD_SYNCNOP   0x10  /* Special no-operation that returns NAK+ACK  */
#define S_CMD_Q_RDNMAXLEN 0x11  /* Query read-n maximum length      */
#define S_CMD_S_BUSTYPE   0x12  /* Set used bustype(s).       */
#define S_CMD_O_SPIOP   0x13  /* Perform SPI operation.     */
#define S_CMD_S_SPI_FREQ  0x14  /* Set SPI clock frequency      */
#define S_CMD_S_PIN_STATE 0x15  /* Enable/disable output drivers    */

#define BIT( n ) ( 1UL << n )

uint16_t InterfaceVersion = 0x0001;
uint32_t SupportedCommands = BIT( 0x0 ) | BIT( 0x1 ) | BIT( 0x2 ) | BIT( 0x3 ) | BIT( 0x4 ) | BIT( 0x5 ) | BIT( 0x11 ) | BIT( 0x10 ) | BIT( 0x12 ) | BIT( 0x13 );
const uint16_t SerialBufferSize = 16384;
uint8_t SupportedBusTypes = BIT( 3 ); // SPI
uint16_t OperationBufSize = 16384;
uint32_t MaxReadLength = 16384;

SPISettings TransferSettings = SPISettings( F_CPU / 2, MSBFIRST, SPI_MODE0 );

char ProgrammerName[ 16 ] = {
  'S', 'E', 'R', 'P', 'E', 'N', 'T', 'E', 'F', 'L', 'A', 'S', 'H', 0, 0, 0
};

void SerprogAction( char c ) {
  static uint8_t Buffer[ SerialBufferSize + 1 ];
  uint32_t DelayUS = 0;
  int SLength = 0;
  int RLength = 0;
  int Length = 0;
  int i = 0;

  switch ( c ) {
    case S_CMD_NOP: {
      Serial.write( S_ACK );
      break;
    }
    case S_CMD_Q_IFACE: {
      Serial.write( S_ACK );
      Serial.write( ( uint8_t* ) &InterfaceVersion, 2 );
      break;
    }
    case S_CMD_Q_CMDMAP: {
      Serial.write( S_ACK );
      Serial.write( ( uint8_t* ) &SupportedCommands, 3 );

      for ( i = 0; i < 32 - 3; i++ ) {
        Serial.write( ( uint8_t ) 0 );
      }

      break;
    }
    case S_CMD_Q_PGMNAME: {
      Serial.write( S_ACK );
      Serial.write( ( uint8_t* ) ProgrammerName, 16 );
      break;
    }
    case S_CMD_Q_SERBUF: {
      Serial.write( S_ACK );
      Serial.write( ( uint8_t* ) &SerialBufferSize, 2 );

      break;
    }
    case S_CMD_Q_BUSTYPE: {
      Serial.write( S_ACK );
      Serial.write( &SupportedBusTypes, 1 );
      break;
    }
    case S_CMD_SYNCNOP: {
      Serial.write( S_NAK );
      Serial.write( S_ACK );
      break;
    }
    case S_CMD_S_BUSTYPE: {
      i = Serial.read( );

      Serial.write( i & SupportedBusTypes ? S_ACK : S_NAK );
      break;
    }
    case S_CMD_O_SPIOP: {
      // Red LED: Activity
      digitalWrite( PIN_LED, LOW );

      Serial.readBytes( ( char* ) &SLength, 3 );
      Serial.readBytes( ( char* ) &RLength, 3 );

      SPI1.beginTransaction( TransferSettings );
      digitalWrite( SS1, LOW );
        Serial.readBytes( ( char* ) Buffer, SLength );
        SPI1.transfer( Buffer, SLength );
        SPI1.transfer( Buffer, RLength );

        Serial.write( S_ACK );
        Serial.write( ( uint8_t* ) Buffer, RLength );
      digitalWrite( SS1, HIGH );
      SPI1.endTransaction( );

      digitalWrite( PIN_LED, HIGH );
      break;
    }
    case S_CMD_Q_RDNMAXLEN: {
      Serial.write( S_ACK );
      Serial.write( ( const char* ) &MaxReadLength, 3 );

      break;
    }
    default: {
      break;
    }
  };
}

void setup( void ) {
    Serial.begin( 115200 );

  pinMode( PIN_LED, OUTPUT );
  pinMode( PIN_LED2, OUTPUT );
  pinMode( PIN_LED3, OUTPUT );

  // Keep the other LEDs off
  digitalWrite( PIN_LED, HIGH );
  digitalWrite( PIN_LED3, HIGH );

  // Green light means ready
  digitalWrite( PIN_LED2, LOW );

    // This suspends execution until the serial port is open
  while ( ! Serial )
  ;

  SPI1.begin( );

  // Make sure we have control over the flash SS pin
  pinMode( SS1, OUTPUT );
  digitalWrite( SS1, HIGH );
}

void loop( void ) {
  while ( true ) {
    while ( Serial.available( ) ) {
      SerprogAction( Serial.read( ) );
    }
  }
}
