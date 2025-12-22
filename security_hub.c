#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct {
    int id;       
    char name[20];  
    int is_armed; 
    int alert_level; 
} Zone;

Zone house_zones[4] = {
    {0, "FRONT", 0, 0},
    {1, "GARAGE", 0, 0},
    {2, "WHOUSE", 0, 0},
    {3, "VAULT", 1, 0}
};

char history[3][10]; 
int history_idx = 0; 

// ===============================
// LOCKDOWN LOGIC
// ===============================

void trigger_lockdown(char *reason) {
    printf("\n [!!!] CRITICAL: LOCKDOWN INITIATED [!!!]\n");
    printf(" >> Reason: %s\n", reason);
    for(int i = 0; i < 4; i++) {
        house_zones[i].is_armed = 1;
        house_zones[i].alert_level = 100; 
    }
    printf(" >> All zones ARMED and SEALED.\n\n");
}

//vhecks if any zone hit 100% AFTER commands finish
void check_security_integrity() {
    for(int i = 0; i < 4; i++) {
        if(house_zones[i].alert_level >= 100) {
            char msg[50];
            sprintf(msg, "Zone %s reached 100%% Alert!", house_zones[i].name);
            trigger_lockdown(msg);
            return; // Only trigger once
        }
    }
}

// ===============================
// HELPER FUNCTIONS
// ===============================

void record_history(char *cmd) {
    strncpy(history[history_idx], cmd, 9);
    history[history_idx][9] = '\0'; 
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
// COMMAND HANDLERS
// ==============================
void handle_status() {
    printf("\n=== SECURITY HUB DASHBOARD ===\n");
    printf("ID | NAME     | STATUS  | ALERT\n");
    printf("-------------------------------\n");
    for(int i = 0; i < 4; i++) {
        printf("%d  | %-8s | %-6s | %d%%\n", 
               house_zones[i].id, house_zones[i].name, 
               house_zones[i].is_armed ? "ARMED" : "OFF", 
               house_zones[i].alert_level);
    }
    printf("-------------------------------\n");
}

void dispatch_zone(int id_in, char *cmd, int val) {
    if(id_in < 0 || id_in > 3) return;
    
    if(strcmp(cmd, "ARM") == 0) {
        house_zones[id_in].is_armed = (val > 0) ? 1 : 0;
        printf(" >> $[200]:(Zone %s is now %s)#\n", house_zones[id_in].name, val ? "ARMED" : "OFF");
        record_history("ARM");
    }
    else if(strcmp(cmd, "ALERT") == 0) {
        if(val >= 0 && val <= 100) {
            house_zones[id_in].alert_level = val;
            printf(" >> $[200]:(Zone %s Alert Level set to %d%%)#\n", house_zones[id_in].name, val);
            record_history("ALERT");
        }
    }
}

// ===============================
// MAIN LOOP
// ===============================
int main() {
    char rx_buffer[50];
    int index = 0;

    printf("=== SERIAL COMMAND SYSTEM ===\n");
    printf("Format: STATUS, LOCKDOWN, RESET, MASTER:ACT:VAL, ZONE:ID/NAME:ACT:VAL\n\n");
    
    while(1) {
        int c = getchar();
        if(c == EOF) break;

        if(c == '\n') {
            rx_buffer[index] = '\0';
            to_upper(rx_buffer);

            char *cmd = strtok(rx_buffer, ":"); 
            if (cmd != NULL) {
                if (strcmp(cmd, "STATUS") == 0) {
                    handle_status();
                }
                else if (strcmp(cmd, "LOCKDOWN") == 0) {
                    trigger_lockdown("Manual override initiated by operator.");
                }
                else if(strcmp(cmd, "MASTER") == 0) {
                    char *actstr = strtok(NULL, ":");
                    char *valstr = strtok(NULL, ":");
                    if(actstr && valstr) {
                        int val_st = atoi(valstr);
                        for(int i = 0; i < 4; i++) dispatch_zone(i, actstr, val_st);
                        
                        // Check integrity AFTER the master loop finishes
                        check_security_integrity();
                    }
                }
                else if(strcmp(cmd, "RESET") == 0) {
                	// 1. Loop through all zones
                	for(int i = 0; i < 4; i++) {
                		house_zones[i].alert_level = 0; // Clear alerts
                		house_zones[i].is_armed = 0;
                		// Disarm system
                		}
                		// 2. Announce the "All Clear"
                		printf(" >> $[200]:(SYSTEM NORMALIZED - ALL ZONES RESET)#\n");
                		}

                else if (strcmp(cmd, "ZONE") == 0) {
                    char *id_str  = strtok(NULL, ":"); 
                    char *act_str = strtok(NULL, ":"); 
                    char *val_str = strtok(NULL, ":"); 
                    
                    if (id_str && act_str && val_str) {
                        int target_id = -1;
                        if(is_numeric(id_str)) target_id = atoi(id_str);
                        else {
                            for (int k = 0; k < 4; k++) {
                                if(strcmp(house_zones[k].name, id_str) == 0) {
                                    target_id = k;
                                    break;
                                }
                            }
                        }

                        if(target_id != -1) {
                            dispatch_zone(target_id, act_str, atoi(val_str));
                            // Check integrity after the specific zone is updated
                            check_security_integrity();
                        }
                        else printf(" >> [ERROR] Unknown Zone: %s\n", id_str);
                    }
                }
            }
            index = 0; 
        } else {
            if(index < 49) rx_buffer[index++] = (char)c;
        }
    }
    return 0;
}