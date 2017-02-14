typedef enum {
  I2C_IDLE,     // do nothing
  I2C_START,    // init and write address
  I2C_STOP,     // stop
  I2C_WRITE,    // write byte
  I2C_ACK       // ack
} I2C_Operation;

typedef struct {
  I2C_Operation operation;
  int state;
  int data;
  int address;
  int dataPin;
  int clockPin;  
} I2C_Bus;

int debug = 0;

void I2C_high(int pin) {
  if(debug) Serial.print(pin == 2 ? "d" : "c");
  if(debug) Serial.println(" HI");
  pinMode(pin, INPUT);
}

void I2C_low(int pin) {
  if(debug) Serial.print(pin == 2 ? "d" : "c");
  if(debug) Serial.println(" LO");
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

void I2C_set(int pin, int state) {
  if(state) {
    I2C_high(pin);
  } else {
    I2C_low(pin);
  }
}

// true if busy
int I2C_start(I2C_Bus* bus) {
  switch(bus->state) {
    case 0:
      I2C_high(bus->dataPin);
      I2C_high(bus->clockPin);
      bus->state++;
      break;
    case 1:
      I2C_low(bus->dataPin);
      bus->state++;
      break;
    case 2:
      I2C_low(bus->clockPin);
      bus->data = (bus->address<<1)|0x00;
      bus->operation = I2C_WRITE;
      bus->state = 0;
      break;
    default:
      bus->operation = I2C_IDLE;
      bus->state = 0;
      return 0;
  }
  return 1;
}

// true if busy
int I2C_stop(I2C_Bus* bus) {
  switch(bus->state) {
    case 0:
      I2C_low(bus->dataPin);
      bus->state++;
      break;
    case 1:
      I2C_high(bus->clockPin);
      bus->state++;
      break;
    case 2:
      I2C_high(bus->dataPin);
      bus->operation = I2C_IDLE;
      bus->state = 0;
      return 0;
    default:
      bus->operation = I2C_IDLE;
      bus->state = 0;
      return 0;
  }
  return 1;
}

// true if busy
int I2C_write(I2C_Bus* bus) {
  if(bus->state%2==0) {
    I2C_low(bus->clockPin);
    I2C_set(bus->dataPin, ((bus->data<<(bus->state/2)) & 0x80) != 0);
  } else {
    I2C_high(bus->clockPin);
  }
  if(bus->state>=15) {
    bus->operation = I2C_ACK;
    bus->state = 0;
  } else {
    bus->state++;
  }
  return 1;
}

int I2C_ack(I2C_Bus* bus) {
  switch(bus->state) {
    case 0:
      I2C_low(bus->clockPin);
      I2C_high(bus->dataPin);
      bus->state++;
      break;
    case 1:
      I2C_high(bus->clockPin);
      bus->state++;
      break;
    case 2:
      if(HIGH==digitalRead(bus->dataPin)) { return -1; }
      I2C_low(bus->clockPin);
      bus->operation = I2C_IDLE;
      bus->state = 0;
      return 0;
      break;
    default:
      bus->operation = I2C_IDLE;
      bus->state = 0;
      return 0;
  }
  return 1;
}


// -------------------- //
// -------------------- //
// --- STUFF TO USE --- //
// -------------------- //
// -------------------- //


void I2C_Init(I2C_Bus* bus, int dataPin, int clockPin, int address) {
  bus->dataPin = dataPin;
  bus->clockPin = clockPin;
  bus->address = address;
  bus->operation = I2C_IDLE;
  bus->state = 0;
  bus->data = 0;
  I2C_high(bus->clockPin);
  I2C_high(bus->dataPin);
}

void I2C_Start(I2C_Bus* bus) {
  bus->operation = I2C_START;
}

void I2C_Stop(I2C_Bus* bus) {
  bus->operation = I2C_STOP;
}

void I2C_WriteByte(I2C_Bus* bus, char data) {
  bus->data = data;
  bus->operation = I2C_WRITE;  
}

// true if busy
int I2C_Loop(I2C_Bus* bus) {
  int busy = 0;
  if(debug) Serial.print("-----");
  if(debug) Serial.print(bus->operation);
  if(debug) Serial.print(" ");
  if(debug) Serial.println(bus->state);
  switch(bus->operation) {
    case I2C_IDLE:
      break;
    case I2C_START:
      busy = I2C_start(bus);
      break;
    case I2C_STOP:
      busy = I2C_stop(bus);
      break;
    case I2C_WRITE:
      busy = I2C_write(bus);
      break;
    case I2C_ACK:
      busy = I2C_ack(bus);
      break;
    default:
      bus->operation = I2C_IDLE;
      bus->state = 0;
      break;
  }
  if(debug) Serial.println("");
  return busy;
}
