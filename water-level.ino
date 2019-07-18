#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---- To be set by Users ----

const int TriggerPin = 12;      //Trig pin
const int EchoPin = 11;         //Echo pin
float threeshold = 30;          //Distance between sensor when full
long update = 10;               //Update time in seconds
float depth = 250;              //In cm at full capacity
float full_capacity = 1000;     //In liters
uint8_t i2caddr = 0x3C;         //Display Address, get with i2c_scanner

#define SCREEN_WIDTH 128        // OLED display width, in pixels
#define SCREEN_HEIGHT 64        // OLED display height, in pixels

// ---- End User set ---

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int percentages[9];
float Duration = 0;
float d_percentage = depth / 100;
float per_cm_capacity = full_capacity / depth;
long update_time = update * 1000;

void setup() {
    pinMode(TriggerPin,OUTPUT);  // Trigger is an output pin
    pinMode(EchoPin,INPUT);      // Echo is an input pin
    Serial.begin(9600);          // Serial Output

    if(!display.begin(SSD1306_SWITCHCAPVCC, i2caddr)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    delay(100);
}

void loop() {
    display_level(get_level());
    delay(update_time);
}

void display_level(int percent){
    display.clearDisplay();
    display.fillRect(0, 0, display.width(), 15, INVERSE);
    // Setup Header
    display.setTextSize(1);
    display.setTextColor(INVERSE);
    display.setCursor(5,5);
    display.println("Nivel del Agua");
    display.setCursor(0,20);
    // Create Graphs
    draw_bat();
    draw_percent(percent);
    draw_graph(percent);
    // Display current capacity
    show_current_capacity();
    display.display();
}

void draw_bat() {
    // Draw the Rectangles
    int sizes[6][4] = {
        {5, 18, 2, 42},
        {7, 18, 6, 2},
        {22, 18, 6, 2},
        {28, 18, 2, 42},
        {7, 58, 21, 2},
        {11, 16, 13, 2}
    };
    for (int i=0; i<6; i++) {
        display.fillRect(sizes[i][0], sizes[i][1], sizes[i][2], sizes[i][3], INVERSE);
    }
}

void draw_percent(int percent) {
    display.fillRect(8, 21, 19, 38, INVERSE);
    float a = 0.38;
    float b = 100 - percent;
    float empty = a * b;
    display.fillRect(8, 21, 19, empty, INVERSE);
    display.setTextSize(1);
    display.setTextColor(INVERSE);
    display.setCursor(10,35);
    display.print(percent);
    display.print("%");
}

void draw_graph(int percent) {
    // Register percentages
    // If any is 0
    bool any_zero = false;
    for (int i = 0; i<9; i++) {
        if (percentages[i] == 0) {
            percentages[i] = percent;
            any_zero = true;
            break;
        }
    }
    // Else
    if (!any_zero){
        for (int i = 0; i<8; i++) {
            percentages[i] = percentages[i+1];
        }
        percentages[8] = percent;
    }
    for (int i = 0; i<9; i++) {
        int x = 40 + (5 * i * 2);
        if (percentages[i] != 0) {
            float a = 0.4;
            float b = 100 - percentages[i];
            float empty = a * b;
            float h = 40 - empty;
            float y = empty + 20;
            display.fillRect(x, y, 5, h, INVERSE);
        }
    }
}

float calc_distance(long time) {
  return ((time * 0.034) / 2);     // Actual calculation in cm
}

float get_distance() {
    digitalWrite(TriggerPin, LOW);                   
    delayMicroseconds(2);
    // Trigger pin to HIGH
    digitalWrite(TriggerPin, HIGH);
    // 10us high
    delayMicroseconds(10);
    // Trigger pin to HIGH
    digitalWrite(TriggerPin, LOW);
 
    // Waits for the echo pin to get high and returns the Duration in microseconds
    Duration = pulseIn(EchoPin,HIGH);
    // Use function to calculate the distance
    float Distance_cm = (depth - calc_distance(Duration)) + threeshold;
 
    // Output to serial
    Serial.print("Distance = ");
    Serial.print(Distance_cm);
    Serial.println(" cm");
    if (Distance_cm < 0) return 0;
    else return Distance_cm;
}

int get_level() {
    float distance = get_distance();
    float return_val;
    if (distance < depth) { 
        Serial.print(distance);
        Serial.print(" / (");
        Serial.print(depth);
        Serial.print(" / 100)");
        Serial.print(" = ");
        return_val = distance / d_percentage;
    } else {
        return_val = 0;
    }
    Serial.println(return_val);
    if (return_val < 0) return 0;
    else return return_val;
}

void show_current_capacity() {
    display.setTextSize(1);
    display.setTextColor(INVERSE);
    display.setCursor(93,5);
    display.print(get_current_capacity());  
    display.print("L");
}

int get_current_capacity() {
    float current_distance = get_distance();
    float return_val;
    if (current_distance < depth) {
        return_val = current_distance * per_cm_capacity;
    } else {
        return_val = full_capacity;
    }
    Serial.print("L ");
    Serial.print(return_val);
    Serial.print(" ");
    Serial.print(current_distance);
    Serial.println("");
    return return_val;
}