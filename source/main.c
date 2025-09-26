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
#define DEFAULT_SERVER_IP "192.168.1.1" // please change in case i didn't do config.ini correctly!
#define CONFIG_FILE "config.ini"
#define MAX_LINE 256
#define LCD_TOGGLE_KEY KEY_L
#define CONNECTION_TIMEOUT_MS 5000

typedef struct {
    u32 buttons;            // 4 bytes
    circlePosition circlepad; // 4 bytes (x, y as s16)
    circlePosition cstick;  // 4 bytes (x, y as s16)
} controllerstate;

typedef struct {
    char serverip[64];
    int port;
    u32 lcdtogglekey;
} config;

void readconfigfile(config *cfg) {
    strcpy(cfg->serverip, DEFAULT_SERVER_IP);
    cfg->port = DEFAULT_PORT;
    cfg->lcdtogglekey = LCD_TOGGLE_KEY;
    
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        printf("config.ini not found at %s, using defaults\n", CONFIG_FILE);
        printf("creating default config file...\n");
        
        file = fopen(CONFIG_FILE, "w");
        if (file) {
            fprintf(file, "serverip=%s\n", DEFAULT_SERVER_IP);
            fprintf(file, "port=%d\n", DEFAULT_PORT);
            fprintf(file, "lcdtogglekey=KEY_L\n");
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
            } else if (strcmp(key, "lcdtogglekey") == 0) {
                if (strcmp(value, "KEY_L") == 0) cfg->lcdtogglekey = KEY_L;
                else if (strcmp(value, "KEY_R") == 0) cfg->lcdtogglekey = KEY_R;
                else if (strcmp(value, "KEY_ZL") == 0) cfg->lcdtogglekey = KEY_ZL;
                else if (strcmp(value, "KEY_ZR") == 0) cfg->lcdtogglekey = KEY_ZR;
                else if (strcmp(value, "KEY_SELECT") == 0) cfg->lcdtogglekey = KEY_SELECT;
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
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("connection failed\n");
        return -1;
    }
    
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    
    return sockfd;
}

int checkconnection(int sockfd) {
    if (sockfd < 0) return 0;
    
    char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, "ping");
    
    ssize_t sent = send(sockfd, buffer, 5, 0);
    if (sent <= 0) return 0;
    
    fd_set readfds;
    struct timeval timeout;
    
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    memset(buffer, 0, sizeof(buffer));
    
    if (select(sockfd + 1, &readfds, NULL, NULL, &timeout) > 0) {
        ssize_t received = recv(sockfd, buffer, sizeof(buffer), 0);
        if (received > 0 && strcmp(buffer, "pong") == 0) {
            return 1;
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
    
    const char *icons[] = {"[----]", "[#---]", "[##--]", "[###-]", "[####]"};
    int level = percentage;
    
    if (level < 0) level = 0;
    if (level > 4) level = 4;
    
    if (charging) {
        snprintf(buffer, size, "%s CHRG", icons[level]);
    } else {
        snprintf(buffer, size, "%s %d%%", icons[level], (level + 1) * 20);
    }
}

void printstatusmessage(int sockfd, const config *cfg, int isconnected, int lcdstate) {
    consoleClear();
    
    char batterystatus[32];
    getbatterystatus(batterystatus, sizeof(batterystatus));
    
    printf("\x1b[1;1H╔════════════════════════════╗");
    printf("\x1b[2;1H║  3DS CONTROLLER            ║");
    printf("\x1b[3;1H╠════════════════════════════╣");
    
    if (isconnected) {
        printf("\x1b[4;1H║ STATUS: \x1b[32mCONNECTED\x1b[0m         ║");
    } else {
        printf("\x1b[4;1H║ STATUS: \x1b[31mDISCONNECTED\x1b[0m      ║");
    }
    
    printf("\x1b[5;1H║ SERVER: %-18s ║", cfg->serverip);
    printf("\x1b[6;1H║ PORT:   %-18d ║", cfg->port);
    printf("\x1b[7;1H║ BATT:   %-18s ║", batterystatus);
    printf("\x1b[8;1H║ LCD:    %-18s ║", lcdstate ? "ON" : "OFF");
    printf("\x1b[9;1H╠════════════════════════════╣");
    printf("\x1b[10;1H║ CONTROLS:                  ║");
    printf("\x1b[11;1H║ • Circle Pad: Movement     ║");
    printf("\x1b[12;1H║ • C-Stick: Right Analog    ║");
    printf("\x1b[13;1H║ • A/B/X/Y: Action Buttons  ║");
    printf("\x1b[14;1H║ • L/R/ZL/ZR: Triggers      ║");
    printf("\x1b[15;1H║ • D-Pad: Directional       ║");
    printf("\x1b[16;1H║ • START+SELECT: Exit       ║");
    
    char keylabel[10] = "L";
    if (cfg->lcdtogglekey == KEY_R) strcpy(keylabel, "R");
    else if (cfg->lcdtogglekey == KEY_ZL) strcpy(keylabel, "ZL");
    else if (cfg->lcdtogglekey == KEY_ZR) strcpy(keylabel, "ZR");
    else if (cfg->lcdtogglekey == KEY_SELECT) strcpy(keylabel, "SELECT");
    
    printf("\x1b[17;1H║ • HOLD %s: Toggle LCD      ║", keylabel);
    printf("\x1b[18;1H╚════════════════════════════╝");
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
    u32 keyheldtime = 0;
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 keysdown = hidKeysHeld();
        
        if ((keysdown & KEY_START) && (keysdown & KEY_SELECT)) 
            break;
        
        u32 currenttime = osGetTime();
        
        if (currenttime - connectionchecktime > 2000) {
            isconnected = checkconnection(sockfd);
            connectionchecktime = currenttime;
            
            if (!isconnected && sockfd >= 0) {
                close(sockfd);
                sockfd = -1;
            } else if (!isconnected) {
                sockfd = initsocket(&cfg);
            }
        }
        
        if (keysdown & cfg.lcdtogglekey) {
            if (lcdtoggleheldtime == 0) {
                lcdtoggleheldtime = currenttime;
            } else if (currenttime - lcdtoggleheldtime > 1000) {
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
            sendcontrollerstate(sockfd, &state);
        }
        
        if (currenttime - laststatusupdate > 3000) {
            printstatusmessage(sockfd, &cfg, isconnected, lcdstate);
            laststatusupdate = currenttime;
        }
        
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        gspWaitForVBlank();
        
        // i accidently flooded my network so this was added lol
        svcSleepThread(33333333);
    }
    
    if (sockfd >= 0) {
        close(sockfd);
    }
    
    ptmuExit();
    socExit();
    gfxExit();
    return 0;
}