#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>

#define DEFAULT_PORT 8888
#define DEFAULT_SERVER_IP "192.168.1.1"
#define CONFIG_FILE "config.ini"
#define MAX_LINE 256
#define LCD_TOGGLE_KEY KEY_SELECT
#define CONNECTION_TIMEOUT_MS 5000
#define LCD_TOGGLE_HOLD_TIME 5000  // 5 seconds hold time for LCD toggle

typedef struct {
    u32 buttons;            
    circlePosition circlepad; 
    circlePosition cstick;  
} controllerstate;

typedef struct {
    char serverip[64];
    int port;
} config;

void readconfigfile(config *cfg) {
    strcpy(cfg->serverip, DEFAULT_SERVER_IP);
    cfg->port = DEFAULT_PORT;
    
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        printf("config.ini not found at %s, using defaults\n", CONFIG_FILE);
        printf("creating default config file...\n");
        
        file = fopen(CONFIG_FILE, "w");
        if (file) {
            fprintf(file, "serverip=%s\n", DEFAULT_SERVER_IP);
            fprintf(file, "port=%d\n", DEFAULT_PORT);
            fclose(file);
            printf("default config file created at %s\n", CONFIG_FILE);
        } else {
            printf("failed to create default config file\n");
        }
        return;
    }
    
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
        }
        
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "=");
        
        if (key && value) {
            while (*key && (*key == ' ' || *key == '\t')) key++;
            while (*value && (*value == ' ' || *value == '\t')) value++;
            
            if (strcmp(key, "serverip") == 0) {
                strcpy(cfg->serverip, value);
            } else if (strcmp(key, "port") == 0) {
                cfg->port = atoi(value);
            }
        }
    }
    
    fclose(file);
    printf("config loaded from %s\n", CONFIG_FILE);
}

int initsocket(const config *cfg) {
    int sockfd;
    struct sockaddr_in serveraddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation failed\n");
        return -1;
    }

    memset(&serveraddr, 0, sizeof(serveraddr));
    
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(cfg->port);
    
    if (inet_pton(AF_INET, cfg->serverip, &serveraddr.sin_addr) <= 0) {
        printf("invalid address\n");
        close(sockfd);
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("connection failed\n");
        close(sockfd);
        return -1;
    }
    
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    // Remove the problematic setsockopt line completely
    
    return sockfd;
}

int checkconnection(int sockfd) {
    if (sockfd < 0) return 0;
    
    char ping_buf[6] = "ping";
    char pong_buf[6] = {0};
    
    if (send(sockfd, ping_buf, 5, 0) <= 0) {
        return 0;
    }
    
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    if (select(sockfd + 1, &readfds, NULL, NULL, &timeout) > 0) {
        ssize_t received = recv(sockfd, pong_buf, sizeof(pong_buf) - 1, 0);
        if (received > 0) {
            pong_buf[received] = '\0';
            if (strcmp(pong_buf, "pong") == 0) {
                return 1;
            }
        }
    }
    
    return 0;
}

int sendcontrollerstate(int sockfd, controllerstate *state) {
    return send(sockfd, state, sizeof(controllerstate), 0);
}

void getbatterystatus(char *buffer, size_t size) {
    u8 percentage;
    PTMU_GetBatteryLevel(&percentage);
    
    u8 charging;
    PTMU_GetBatteryChargeState(&charging);
    
    char icon[6];
    
    // Convert battery level (0-4) to percentage (20-100)
    int actualpercentage = (percentage + 1) * 20;
    if (actualpercentage > 100) actualpercentage = 100;
    
    for (int i = 0; i < 5; i++) {
        icon[i] = (i <= percentage) ? '|' : ' ';
    }
    icon[5] = '\0';
    
    if (charging) {
        snprintf(buffer, size, "[%s] +", icon);
    } else {
        snprintf(buffer, size, "[%s] %d%%", icon, actualpercentage);
    }
}

void printstatusmessage(int sockfd, const config *cfg, int isconnected, int lcdstate) {
    consoleClear();
    
    char batterystatus[32];
    getbatterystatus(batterystatus, sizeof(batterystatus));
    
    printf("\x1b[0;0H+----------------------+");
    printf("\x1b[1;0H|    \x1b[1;36m3DS CONTROLLER\x1b[0m    |");
    printf("\x1b[2;0H+----------------------+");
    
    if (isconnected) {
        printf("\x1b[3;0H| Status: \x1b[32mCONNECTED\x1b[0m    |");
    } else {
        printf("\x1b[3;0H| Status: \x1b[31mNO CONNECTION\x1b[0m |");
    }
    
    printf("\x1b[4;0H| IP: %-16s |", cfg->serverip);
    printf("\x1b[5;0H| Port: %-15d |", cfg->port);
    printf("\x1b[6;0H| Battery: %-11s |", batterystatus);
    
    if (lcdstate) {
        printf("\x1b[7;0H| Display: \x1b[32mON\x1b[0m           |");
    } else {
        printf("\x1b[7;0H| Display: \x1b[31mOFF\x1b[0m          |");
    }
    
    printf("\x1b[8;0H+----------------------+");
    printf("\x1b[9;0H|      \x1b[1;33mCONTROLS\x1b[0m       |");
    printf("\x1b[10;0H| CirclePad: Left Stick |");
    printf("\x1b[11;0H| C-Stick: Right Stick  |");
    printf("\x1b[12;0H| A/B/X/Y: Face Buttons |");
    printf("\x1b[13;0H| L/R/ZL/ZR: Triggers   |");
    printf("\x1b[14;0H| D-Pad: Directional    |");
    printf("\x1b[15;0H| Hold SELECT: Toggle LCD|");
    printf("\x1b[16;0H| START+SELECT: Exit    |");
    printf("\x1b[17;0H+----------------------+");
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    
    socInit((u32*)memalign(0x1000, 0x100000), 0x100000);
    ptmuInit();
    
    config cfg;
    readconfigfile(&cfg);
    
    printf("3ds controller\n");
    printf("connecting to pc at %s:%d...\n", cfg.serverip, cfg.port);
    
    int sockfd = initsocket(&cfg);
    int isconnected = 0;
    int lcdstate = 1;
    u32 lcdtoggleheldtime = 0;
    u32 connectionchecktime = 0;
    
    printf("\ncalibrating circle pad...\n");
    printf("leave circle pad untouched\n");
    
    for (int i = 0; i < 60; i++) {
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    printf("\ncalibration complete!\n");
    
    u32 laststatusupdate = 0;
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 keysdown = hidKeysHeld();
        
        if ((keysdown & KEY_START) && (keysdown & KEY_SELECT)) 
            break;
        
        u32 currenttime = osGetTime();
        
        // Check connection more frequently
        if (currenttime - connectionchecktime > 1000) {
            isconnected = checkconnection(sockfd);
            connectionchecktime = currenttime;
            
            if (!isconnected && sockfd >= 0) {
                close(sockfd);
                sockfd = -1;
            } else if (!isconnected && sockfd < 0) {
                sockfd = initsocket(&cfg);
            }
        }
        
        // Handle LCD toggle with SELECT button only, 5 second hold time
        if (keysdown & LCD_TOGGLE_KEY) {
            if (lcdtoggleheldtime == 0) {
                lcdtoggleheldtime = currenttime;
            } else if (currenttime - lcdtoggleheldtime > LCD_TOGGLE_HOLD_TIME) {
                lcdstate = !lcdstate;
                if (lcdstate) {
                    gspLcdInit();
                    GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);
                } else {
                    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);
                }
                lcdtoggleheldtime = currenttime;
            }
        } else {
            lcdtoggleheldtime = 0;
        }
        
        controllerstate state;
        memset(&state, 0, sizeof(controllerstate));
        state.buttons = keysdown;
        
        hidCircleRead(&state.circlepad);
        hidCstickRead(&state.cstick);
        
        if (isconnected) {
            if (sendcontrollerstate(sockfd, &state) < 0) {
                // If send fails, mark as disconnected to trigger reconnect
                isconnected = 0;
            }
        }
        
        if (currenttime - laststatusupdate > 1000) {
            printstatusmessage(sockfd, &cfg, isconnected, lcdstate);
            laststatusupdate = currenttime;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        gspWaitForVBlank();

        // Reduced sleep time to improve responsiveness
        svcSleepThread(16666666);  // ~60fps
    }
    
    if (sockfd >= 0) {
        close(sockfd);
    }
    
    ptmuExit();
    socExit();
    gfxExit();
    return 0;
}
