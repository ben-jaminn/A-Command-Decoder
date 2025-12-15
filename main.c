#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// [MODEL] Define the physical system
struct Robot{
	int speed;       // Range 0 to 100
	char state[20];  // IDLE, MOVING, ALARM
	int temperature; // Sensor value in Celsius
};

// [CONTROLLER] Parses text commands and updates the Model
void ProcessPacket(char *packet, struct Robot *bot) {
	char *ptr;
	
	// 1. Slice the Command (e.g., "SPEED" from "SPEED:50")
	ptr = strtok(packet, ":");
	char command[20];
	strcpy(command, ptr); // Copy to safe memory
	
	// 2. Route: Handle SPEED
	if(strcmp(command, "SPEED")==0) {
		ptr = strtok(NULL, ":"); // Get the value part
		int speed = atoi(ptr);   // Convert string to int
		
		// [SAFETY CRITICAL] Protection logic
		if(speed > 100) {
			printf("ALARM! Speed limit exceeded\n");
			sprintf(bot->state, "ALARM");
		}
		else{
			bot->speed = speed;
		}
	}
	
	// 3. Route: Handle STATE
	if(strcmp(command, "STATE")==0) {
		ptr = strtok(NULL, ":");
		strcpy(bot->state, ptr);
	}
	
	// 4. Route: Handle TEMP
	if(strcmp(command, "TEMP")==0) {
		ptr = strtok(NULL, ":");
		int temp = atoi(ptr);
		bot->temperature = temp;
	}
};

int main() {
	// [INIT] Hardware state
	struct Robot initial = {0, "IDLE", 25};
	
	// [BUFFER] The "Bucket" for incoming characters
	char rx_buffer[50];
	int index = 0;
	
	// [SIMULATION] Raw data stream (Hardware Input)
	char stream[ ] = {"SPEED:50\nTEMP:25\nSPEED:120\n"};
	
	// [DRIVER LOOP] Process the stream byte-by-byte
	for(int i = 0; i < 28; i++) {
		char c = stream[i];
		
		// CHECK: Did we hit the End of Line?
		if(c == '\n') {
			rx_buffer[index] = 0; // Null Terminate the string (Crucial!)
			
			// Fire the logic
			ProcessPacket(rx_buffer, &initial);
			
			// Telemetry Output
			printf("[STATUS] State: %s | Speed: %d | Temp: %d\n", initial.state, initial.speed, initial.temperature);
			
			// Reset the bucket for the next command
			index = 0;
		}
		else{
			// FILL: Add character to bucket and move cursor
			rx_buffer[index] = stream[i];
			index++;
		}
	}
	return 0;
}
