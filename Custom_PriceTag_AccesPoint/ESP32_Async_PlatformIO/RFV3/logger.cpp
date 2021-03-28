#include <Arduino.h>
#include "logger.h"

void init_log() {
  Serial.begin(500000);
  Serial.printf("\n\n");
  Serial.setDebugOutput(true);
  log_normal("Logger started");
}

void log_time() {
  Serial.print(millis());
}

void log_normal(String message) {
  log_time();
  Serial.println(" Normal: " + message);
}

void log_main(String message) {
  log_time();
  Serial.println(" Main: " + message);
}

void log_verbose(String message) {
  log_time();
  Serial.println(" Verbose: " + message);
}