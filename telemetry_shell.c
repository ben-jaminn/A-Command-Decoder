#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h> // Required for isdigit()

// ===============================
// 1. THE SYSTEM MODEL (Hardware State)
// ===============================
struct Robot {
    int speed;       // 0-100
    char state[20];  // IDLE, MOVING, ALARM
    int temperature; // Celsius
};

// Global instance (so all functions can see it)
struct Robot bot = {0, "IDLE", 25};

int telemetry_enabled = 1;

// ===============================
// 2. HELPER FUNCTIONS (The Tools)
// ===============================
// Returns 1 if string is a valid number, 0 otherwise
int is_numeric(char *str) {
    if (str == NULL || *str == '\0') return 0; // Empty? Bad.
    
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0; // Found a letter? Bad.
    }
    return 1; // Good!
}

//The ACK Function
void send_response(int code, char *msg){
	if(telemetry_enabled == 0){
		return;
	}
	char tx_buffer[120];
	snprintf(tx_buffer, sizeof(tx_buffer), "[%d]:(%s)#", code, msg);
	printf("%s\n", tx_buffer);
}

// ==============================
// 3. COMMAND HANDLERS (The Factories)
// ==============================
// All handlers must match this signature: void func(int)
void handle_speed(int val) {
    if(val > 100) {
        printf(" >> [ALARM] Speed %d too high! Limit is 100.\n", val);
        strcpy(bot.state, "ALARM");
    } else {
        bot.speed = val;
        printf(" >> [ACTUATOR] Motor Speed set to %d\n", val);
        if(val > 0) strcpy(bot.state, "MOVING");
    }
}

void handle_temp(int val) {
    bot.temperature = val;
    printf(" >> [SENSOR] Temperature calibrated to %dC\n", val);
}

void handle_status(int val) {
    // We ignore 'val' here because STATUS doesn't need input.
    printf(" >> [SYSTEM REPORT] State: %s | Speed: %d | Temp: %d\n", 
           bot.state, bot.speed, bot.temperature);
}

void handle_stop(int val){
	printf("EMERGENCY STOP\n");
	strcpy(bot.state, "HALTED");
	bot.speed = 0;
}

void handle_quiet(int val){
	if(val == 1){
		telemetry_enabled = 0;
		printf(">>Entering Silent mode\n");
	}
	else{
		telemetry_enabled = 1;
		send_response(200, "Telemetary Resumed");
	}
}

void to_upper(char *letter){
	if(letter == NULL) return;
	for(int i = 0; letter[i] != '\0'; i++){
		letter[i] = toupper(letter[i]);
	}
}

// ===============================
// 4. THE COMMAND TABLE (The Dictionary)
// ==============================
typedef void (*HandlerFunc)(int); // The Blueprint

struct Command {
    char name[10];
    HandlerFunc func;
};

struct Command cmd_table[] = {
    {"SPEED",  handle_speed},
    {"TEMP",   handle_temp},
    {"STATUS", handle_status},
    {"STOP", handle_stop},
    {"QUIET", handle_quiet}
};

// ===============================
// 5. THE DISPATCHER (The Search Engine)
// ===============================
void dispatch(char *cmd, int val) {
    // Calculate table size automatically
    int size = sizeof(cmd_table) / sizeof(struct Command);

    for(int i = 0; i < size; i++) {
        // If we find the command name in our table...
        if(strcmp(cmd, cmd_table[i].name) == 0) {
            // ...Run the corresponding function!
            cmd_table[i].func(val);
            printf("$[%d]:SPD=%d;TEMP=%d;ST=%s#\n\n", val, bot.speed, bot.temperature, bot.state );
            return; // Exit immediately
        }
    }
    // If loop finishes without returning, command was not found.
    printf(" >> [ERROR] Unknown Command: '%s'\n", cmd);
}

// ===============================
// 6. MAIN DRIVER LOOP (The Pipeline)
// ===============================
int main() {
    char rx_buffer[50];
    int index = 0;
    int c;

    printf("=== ROBUST ROBOT CONSOLE ===\n");
    printf("Commands: SPEED:N, TEMP:N, QUIET:N, STATUS, STOP\n\n");

    while(1) {
        c = getchar();
        if(c == EOF) break;

        // --- END OF LINE DETECTED ---
        if(c == '\n') {
            rx_buffer[index] = '\0'; // Cap the string
            to_upper(rx_buffer);
            
            // STEP 1: TOKENIZE
            // Try to split "SPEED:50" into "SPEED" and "50"
            char *cmd = strtok(rx_buffer, ":");
            char *val_str = strtok(NULL, ":");

            // STEP 2: VALIDATE & ROUTE
            
            // Check A: Empty Input?
            if(cmd == NULL) {
                index = 0;
                continue; // Skip empty lines
            }

            // Check B: Is it the STATUS command? (Special case: No value needed)
            if(strcmp(cmd, "STATUS") == 0) {
                dispatch(cmd, 0); // Send dummy 0
            }
            if(strcmp(cmd, "STOP") == 0) {
                dispatch(cmd, 0); // Send dummy 0
            }
            
            
            // Check C: Is it a Value Command? (Needs valid number)
            else {
                if(val_str == NULL) {
                    printf(" >> [ERROR] '%s' requires a value (Format: CMD:VAL)\n", cmd);
                } 
                else if(is_numeric(val_str) == 0) {
                    printf(" >> [ERROR] '%s' is not a valid number!\n", val_str);
                }
                else {
                    // All checks passed. Convert and Dispatch.
                    int value = atoi(val_str);
                    dispatch(cmd, value);
                }
            }

            index = 0; // Reset buffer
        } 
        
        // --- NORMAL CHARACTER ---
        else {
            if(index < 49) {
                rx_buffer[index] = (char)c;
                index++;
            }
        }
    }
    return 0;
}
