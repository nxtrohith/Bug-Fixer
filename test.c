#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

{    // Stack to track opening brackets
    char stack[100];
    int top = -1;
    int bracket_positions[100]; // Store line numbers of opening brackets
    //Loop only for Brakcet check.
    while(fgets(line, sizeof(line), file) != NULL){
        // Check each character in the line
        for(int i = 0; i < strlen(line); i++) {
            // Check for opening brackets
            if(line[i] == '(' || line[i] == '{' || line[i] == '[') {
                // Push to stack
                top++;
                stack[top] = line[i];
                bracket_positions[top] = line_num;
            }
            // Check for closing brackets
            else if(line[i] == ')' || line[i] == '}' || line[i] == ']') {
                // If stack is empty, we have an extra closing bracket
                if(top == -1) {
                    char description[100];
                    sprintf(description, "Unexpected closing bracket '%c' with no matching opening bracket", line[i]);
                    AddToken(tokenList, "Bracket Error", line_num, description);
                }
                // Check if brackets match
                else {
                    char expected_bracket;
                    if(line[i] == ')') expected_bracket = '(';
                    else if(line[i] == '}') expected_bracket = '{';
                    else expected_bracket = '[';
                    
                    if(stack[top] != expected_bracket) {
                        char description[100];
                        sprintf(description, "Mismatched bracket: expected '%c' but found '%c'", 
                                stack[top] == '(' ? ')' : (stack[top] == '{' ? '}' : ']'), line[i]);
                        AddToken(tokenList, "Bracket Error", line_num, description);
                    }
                    top--; // Pop from stack regardless
                }
            }
        }
        line_num++;
    }

    // Check for unclosed brackets
    while(top >= 0) {
        char description[100];
        char expected_bracket;
        if(stack[top] == '(') expected_bracket = ')';
        else if(stack[top] == '{') expected_bracket = '}';
        else expected_bracket = ']';
        sprintf(description, "Unclosed bracket '%c' - missing '%c'", stack[top], expected_bracket);
        AddToken(tokenList, "Bracket Error", bracket_positions[top], description);
        top--;
    }}

    line_num = 1;
    fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning

    // Track if we're inside a struct definition
    int in_struct_definition = 0;

    //Check for Missing semicolons(;)
    while(fgets(line, sizeof(line), file)!= NULL){
        //writing the exception cases for missing semicolons which are made to beautify the code or like whitelines and comments.
        if(line[0] == '\n' || (line[0] == '/' && line[1] == '/') || (line[0] == '/' && line[1] == '*') || (line[0] == '*' && line[1] == '/')){
            line_num++;
            continue;
        }

        //skip preprocessor directives
        if(line[0] == '#'){
            line_num++;
            continue;
        }

        // Check if entering or exiting a struct definition
        if(strstr(line, "struct") && strstr(line, "{")) {
            in_struct_definition = 1;
        }
        if(strstr(line, "}") && in_struct_definition) {
            in_struct_definition = 0;
        }

        //check for missing semicolons in statements
        {int len = strlen(line);
        int should_have_semicolon = 0;

        //remove trailing whitespace
        while(len > 0 && (isspace(line[len - 1]) || line[len - 1] == '\t')){
            line[len - 1] = '\0';
            len--;
        }

        if(len == 0){
            line_num++;
            continue;
        }

        //Check if the line should end with a semicolon
        if(strstr(line, "return") ||
           strstr(line, "printf") ||
        strstr(line, "scanf") ||
        strstr(line, "malloc") ||
        strstr(line, "free") ||
        strstr(line, "calloc") ||
        strstr(line, "realloc") ||
        strstr(line, "exit") ||
        strstr(line, "abort") ||
        strstr(line, "atexit") ||
        strstr(line, "strcpy") ||
        strstr(line, "strcat") ||
        strstr(line, "strlen") ||
        strstr(line, "strcmp") ||
        strstr(line, "strncpy") ||
        strstr(line, "strncat") ||
        strstr(line, "strncmp") ||
        strstr(line, "strstr") ||
        strstr(line, "strchr") ||
        strstr(line, "strrchr") ||
        strstr(line, "strspn") ||
        strstr(line, "strcspn") ||
        strstr(line, "strpbrk") ||
        strstr(line, "strtok") ||
        strstr(line, "strerror") ||
        strstr(line, "strtol") ||
        strstr(line, "strtoul") ||
        strstr(line, "strtod") ||
        strstr(line, "++") ||
        strstr(line, "=") ||
        strstr(line, "+=") ||
        strstr(line, "-=") ||
        strstr(line, "*=") ||
        strstr(line, "/=") ||
        strstr(line, "%=") ||
        strstr(line, "&=") ||
        strstr(line, "|=") ||
        strstr(line, "^=") ||
        strstr(line, "?")){

            //exclude lines that shouldnt end with a semicolon
            if(strstr(line, "{") || strstr(line, "}") || 
            strstr(line, "if") || strstr(line, "else") || 
            strstr(line, "for") || strstr(line, "while") || 
            strstr(line, "#include") || strstr(line, "#define")) {
             should_have_semicolon = 0; // No semicolon needed
         } else {
             should_have_semicolon = 1; // Semicolon needed
         }
        }

        //check for variable and function declaration
        if((strstr(line, "int ") || strstr(line, "char ") ||
           strstr(line, "float ") || strstr(line, "double ") ||
           strstr(line, "void ") || strstr(line, "struct ")) &&
           !strstr(line, "{")) {
            should_have_semicolon = 1;
        }

        //check if the line is ending with a semicolon
        if(should_have_semicolon && !in_struct_definition && line[len - 1] != ';'){
            char description[100];
            sprintf(description, "Missing semicolon at the end of line %d", line_num);
            AddToken(tokenList, "Missing Semicolon", line_num, description);
        }

        //check for missing semicolons in for loop
        if(strstr(line, "for") && !strstr(line, ";")){
            char description[100];
            sprintf(description, "Missing semicolon in for loop at line %d", line_num);
            AddToken(tokenList, "Missing Semicolon", line_num, description);
        }

        //check for extra semicolons
        int in_string = 0;
        int consecutive_semicolons = 0;
        for(int i = 0; i < len; i++) {
            if(line[i] == '"') {
                in_string = !in_string;
            }
            if(!in_string && line[i] == ';') {
                consecutive_semicolons++;
                if(consecutive_semicolons > 1) {
                    char description[100];
                    sprintf(description, "Extra semicolon at line %d", line_num);
                    AddToken(tokenList, "Extra Semicolon", line_num, description);
                    consecutive_semicolons = 0;
                }
            } else if(!in_string && !isspace(line[i])){
                consecutive_semicolons = 0;
            }
        }
        line_num++;
    }}

    fclose(file);
}

void sort_tokens(token** head) {
    if(*head == NULL){
        printf("No tokens found to sort.\n");
        return;
    }
    token* current = *head;
    token* index = NULL;
    int temp;
    while(current!= NULL){
        index = current -> next;
        while(index!= NULL){
            if(current->line_num > index->line_num){
                temp = current->line_num;
                current->line_num = index->line_num;
                index->line_num = temp;
            }
            index = index->next;
        }
        current = current->next;
    }
}

int main() {
    char testcase[90000];
    token* tokenList = NULL;
    printf("====Bug-Detection in C using C====\n");
    analyse_code("testcase.txt", &tokenList);
    // sort_tokens(&tokenList);
    ShowTokens(tokenList);
    delete_tokens(tokenList);

    return 0;
}