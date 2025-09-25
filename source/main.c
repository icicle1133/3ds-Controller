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

#define DEFAULT_PORT 8888
#define DEFAULT_SERVER_IP "192.168.1.1" // please change in case i didn't do config.ini correctly!
#define CONFIG_FILE "config.ini"
#define MAX_LINE 256

typedef struct {
    u32 buttons;            // 4 bytes
    circlePosition circlepad; // 4 bytes (x, y as s16)
    touchPosition touch;    // 4 bytes (x, y as u16)
    circlePosition cstick;  // 4 bytes (x, y as s16)
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
        return -1;
    }
    
    if (connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        printf("connection failed\n");
        return -1;
    }
    
    return sockfd;
}

void sendcontrollerstate(int sockfd, controllerstate *state) {
    send(sockfd, state, sizeof(controllerstate), 0);
}
// please note, this will ALWAYS say connected, i am working on that.
void printstatusmessage(int sockfd, const config *cfg) {
    consoleClear();
    
    printf("3ds controller\n");
    printf("status: ");
    
    if (sockfd < 0) {
        printf("not connected\n");
        printf("check connection to %s:%d\n", cfg->serverip, cfg->port);
        printf("edit %s to configure\n", CONFIG_FILE);
    } else {
        printf("connected to %s:%d\n", cfg->serverip, cfg->port);
    }

    printf("\ncontrols:\n");
    printf("circle pad: analog movement + c-stick if you have one\n");
    printf("a, b, x, y: action buttons\n");
    printf("l, r, zl, zr: trigger buttons\n");
    printf("d-pad: directional input\n");
    printf("touch screen: mouse movement\n");
    printf("start + select: exit\n");
}

int main(int argc, char **argv) {
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    
    socInit((u32*)memalign(0x1000, 0x100000), 0x100000);
    
    config cfg;
    readconfigfile(&cfg);
    
    printf("3ds controller\n");
    printf("connecting to pc at %s:%d...\n", cfg.serverip, cfg.port);
    
    int sockfd = initsocket(&cfg);
    if (sockfd < 0) {
        printf("failed to initialize socket.\n");
    } else {
        printf("connected successfully!\n");
    }

    printstatusmessage(sockfd, &cfg);
    // now for this? i dont know if it actually works, a controller tester on linux reported about 8% avg error so..
    printf("\ncalibrating circle pad...\n");
    printf("leave circle pad untouched\n");
    
    for (int i = 0; i < 120; i++) {
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
    
    printstatusmessage(sockfd, &cfg);
    printf("\ncalibration complete!\n");
    
    u32 laststatusupdate = 0;
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 keysdown = hidKeysHeld();
        
        if ((keysdown & KEY_START) && (keysdown & KEY_SELECT)) 
            break;
        
        controllerstate state;
        memset(&state, 0, sizeof(controllerstate));
        state.buttons = keysdown;
        
        hidCircleRead(&state.circlepad);
        
        hidTouchRead(&state.touch);
        
        // n3ds only (never tested btw, may test with emulator)
        hidCstickRead(&state.cstick);
        
        if (sockfd >= 0) {
            sendcontrollerstate(sockfd, &state);
        }
        
        u32 currenttime = osGetTime();
        if (currenttime - laststatusupdate > 3000) {
            printstatusmessage(sockfd, &cfg);
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
    
    socExit();
    gfxExit();
    return 0;
}
