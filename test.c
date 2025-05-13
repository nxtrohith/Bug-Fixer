#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "bracketcheck.c"

typedef struct token{
    char type[50];
    int line_num;
    char description[100];
    struct token *next;
} token;

//Function to create a new token into the list
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

//Function to add a new token to the list
void AddToken(token** head, char* type, int line_num, char* descripton) {
    token* newtoken = CreateToken(type, line_num, descripton);
    if(*head == NULL) {
        *head = newtoken;
        return;
    }
    else {
        token* current = *head;
        while(current->next != NULL) {
            current = current->next;
        }
        current->next = newtoken;
    }
}

//Function to print the list of tokens
void ShowTokens(token* head) {
    if(head == NULL) {
        printf("No tokens found.\n");
        return;
    }
    printf("Bug Detected:\n");
    token* current = head;
    while(current!= NULL) {
        printf("Token Type: %s\n", current->type);
        printf("Line Number: %d\n", current->line_num);
        printf("Description: %s\n", current->description);
        printf("\n");
        current = current->next;
    }
}

//Funtion to free the memory allocated for the list
void delete_tokens(token* head){
    token* current = head;
    token* next;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
}

//Function to check for bugs and create a list of tokens
void analyse_code(const char* code, token** tokenList) {
    FILE* file = fopen(code, "r");
    if(file == NULL){
        printf("Error opening file. Please check the file name and try again.\n testcase.txt\n");
        return;
    }

    char line[100];
    int line_num = 1;

    while(fgets(line, sizeof(line), file) != NULL){
        
    }
}

int main() {
    char testcase[90000];
    token* tokenList = NULL;
    printf("====Bug-Detection in C using C====\n");
    

    return 0;
}