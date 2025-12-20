#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// ===============================
// 1. THE SYSTEM MODEL & HISTORY
// ===============================
typedef struct {
    int id;       
    char name[20];  
    int is_armed; // 0 is off, 1 is on
    int alert_level; // 0 to 100
}Zone;

Zone house_zones[4] = {
	{0, "FRONT", 0, 0},
	{1, "GARAGE", 0, 0},
	{2, "WHOUSE", 0, 0},
	{3, "VAULT", 1, 0}
};
int telemetry_enabled = 1;

// BLACK BOX STORAGE
char history[3][10]; // 3 slots for command names
int history_idx = 0; // Current write position

// ===============================
// 2. HELPER FUNCTIONS
// ===============================

// Circular Buffer: Saves the name of successful commands
void record_history(char *cmd) {
    // Copy the name into the current slot (max 9 chars + null)
    strncpy(history[history_idx], cmd, 9);
    history[history_idx][9] = '\0'; 

    // Use Modulo (%) to wrap index: 0 -> 1 -> 2 -> 0
    history_idx = (history_idx + 1) % 3;
}

int is_numeric(char *str) {
    if (str == NULL || *str == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) return 0;
    }
    return 1;
}

void to_upper(char *letter) {
    if(letter == NULL) return;
    for(int i = 0; letter[i] != '\0'; i++) {
        letter[i] = toupper(letter[i]);
    }
}

// ==============================
// 3. COMMAND HANDLERS
// ==============================
void handle_status(int dummy_val) {
    printf("\n=== SECURITY HUB DASHBOARD ===\n");
    printf("ID | NAME     | STATUS  | ALERT\n");
    printf("-------------------------------\n");

    for(int i = 0; i < 4; i++) {
        char *arm_text;
        if (house_zones[i].is_armed == 1) {
        	arm_text = "ARMED";
}
        else {
        	arm_text = "OFF";
}

        printf("%d  | %-8s | %-6s | %d%%\n", 
               house_zones[i].id, 
               house_zones[i].name, 
               arm_text, 
               house_zones[i].alert_level);
    }
    printf("-------------------------------\n");
}

// NEW: History Playback
void handle_history(int val) {
    printf(" >> [BLACK BOX] Last 3 Successful Commands:\n");
    for(int i = 0; i < 3; i++) {
        if(history[i][0] != '\0') {
            printf("  Slot %d: %s\n", i + 1, history[i]);
        } else {
            printf("  Slot %d: [EMPTY]\n", i + 1);
        }
    }
}

// ===============================
// 5. ERROR HANDLING & DISPATCH
// ===============================

void handle_error(int err_code, char *msg) {
    // We only print if telemetry is allowed
    if (telemetry_enabled == 0) return;

    // We use our standard protocol: $[CODE]:(MESSAGE)#
    // Example: $[404]:(Unknown Action: DANCE)#
    printf(" >> $[%d]:(%s)#\n\n", err_code, msg);
}


void dispatch_zone(int id_in, char *cmd, int val) {
    // Check if ID is valid (0 to 3 for 4 zones)
    if(id_in < 0 || id_in > 3) {
        printf(" >> [ERROR] Zone %d does not exist (Use 0-3)\n", id_in);
        return;
    }
    
    // Check the ACTION string against literal words "ARM" and "ALERT"
    if(strcmp(cmd, "ARM") == 0) {
        house_zones[id_in].is_armed = (val > 0) ? 1 : 0;
        printf(" >> $[200]:(Zone %s is now %s)#\n", 
               house_zones[id_in].name, val ? "ARMED" : "OFF");
        record_history("ARM");
    }
    else if(strcmp(cmd, "ALERT") == 0) {
        if(val >= 0 && val <= 100) {
            house_zones[id_in].alert_level = val;
            printf(" >> $[200]:(Zone %s Alert Level set to %d%%)#\n", 
                   house_zones[id_in].name, val);
            record_history("ALERT");
        } else {
            printf(" >> 400: Bad Request: Alert must be 0-100\n");
        }
    }
    else {
        // If it's not ARM or ALERT, then it's a 404
        printf(" >> 404: Unknown Action: '%s'\n", cmd);
    }
}


// ===============================
// 6. MAIN LOOP
// ===============================
int main() {
    char rx_buffer[50];
    int index = 0;

    // Initialize history with empty strings
    for(int i = 0; i < 3; i++) history[i][0] = '\0';

    printf("=== SERIAL COMMAND SYSTEM ===\n");
    printf("Commands Format: ZONE:ID:ACTION:VALUE\n\n");
    
    while(1) {
    int c = getchar();
    if(c == EOF) break;

    if(c == '\n') {
        rx_buffer[index] = '\0'; // 1. Cap the string
        to_upper(rx_buffer);     // 2. Make it case-insensitive

        // --- THE NESTED STRTOK BEGINS ---
        
        // Piece 1: The Root Command (e.g., "ZONE" or "STATUS")
        char *cmd = strtok(rx_buffer, ":"); 

        if (cmd != NULL) {
            
            // Branch A: Simple Commands (No extra slicing needed)
            if (strcmp(cmd, "STATUS") == 0) {
                handle_status(0);
            }
            
            // Branch B: The Multi-Zone Command (Needs nested slicing)
            else if (strcmp(cmd, "ZONE") == 0) {
                char *id_str  = strtok(NULL, ":"); // Slices Zone ID
                char *act_str = strtok(NULL, ":"); // Slices Action (ARM/ALERT)
                char *val_str = strtok(NULL, ":"); // Slices Value
                
                if (id_str && act_str && val_str) {
                    // All 3 extra pieces found!
                    int id = atoi(id_str);
                    int val = atoi(val_str);
                    dispatch_zone(id, act_str, val);
                } else {
                    printf(" >> [ERROR] Usage: ZONE:ID:ACTION:VAL\n");
                }
            }
            
            else {
                printf(" >> 404: Unknown Command: %s\n", cmd);
            }
        }

        index = 0; // Reset buffer for next input
    } else {
        if(index < 49) rx_buffer[index++] = (char)c;
        }
    }

    return 0;
}
