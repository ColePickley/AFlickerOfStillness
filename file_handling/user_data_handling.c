#include "user_data_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void loadUserData(UserData* user_data) {
    FILE* file = fopen("user_data/settings/settings.csv", "r");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    char line[500];
    while (fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\r\n")] = '\0';
        const char *token = strtok(line, ",");
        int token_num = 0;
        while (token != NULL) {
            char* end_token;
            const int token_as_int = strtol(token, &end_token, 10);
            switch (token_num) {
                case 0:
                    user_data->frame_rate_cap = token_as_int;
                    break;
                case 1:
                    user_data->fullscreen = token_as_int;
                    break;
                case 2:
                    user_data->graphics_quality = token_as_int;
                    break;
                case 3:
                    user_data->sensitivity = token_as_int;
                    break;
                default:
                    break;
            }
            token = strtok(NULL, ",");
            token_num += 1;
        }
    }
    fclose(file);
}

void saveUserData(const UserData* user_data) {
    FILE* file = fopen("user_data/settings/settings.csv", "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    fprintf(file, "%d,%d,%d,%d\n", user_data->frame_rate_cap, user_data->fullscreen, user_data->graphics_quality,
        user_data->sensitivity);
    fclose(file);
}