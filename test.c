#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

typedef struct token{
    char type[50];
    int line_num;
    char description[100];
    struct token *next;
} token;

token* CreateToken(char* type, int line_num, char* description) {
    token* newtoken = (token*)malloc(sizeof(token));
    if(newtoken == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    strcpy(newtoken->type, type);
    newtoken->line_num = line_num;
    strcpy(newtoken->description, description);
    newtoken->next = NULL;

    return newtoken;
}

int main() {
    printf("====Bug-Detection in C using C====\n");
    

    return 0;
}