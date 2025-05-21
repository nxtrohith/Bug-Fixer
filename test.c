#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "VariableExtractor.c"

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

void delete_tokens(token* head){
    token* current = head;
    token* next;
    while(current != NULL){
        next = current->next;
        free(current);
        current = next;
    }
}

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
    while(fgets(line, sizeof(line), file) != NULL){
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
            (strstr(line, "for") && strchr(line, '(')) || (strstr(line, "while") && strchr(line, '(')) || // More specific for loops/whiles
            strstr(line, "#include") || strstr(line, "#define")) {
             should_have_semicolon = 0;
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
    fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning
    line_num = 1;
    {while(fgets(line, sizeof(line), file)!= NULL){
        if(strstr(line, "/0") || strstr(line, "%0") || strstr(line, "/ 0") || strstr(line, "% 0") || strstr(line, "0 %") ||strstr(line, "0%")) {
            char description[100];
            sprintf(description, "Division by zero at line %d", line_num);
            AddToken(tokenList, "Division by Zero", line_num, description);
        }
        line_num++;
    }}
    fseek(file, 0, SEEK_SET); // Reset file pointer to the beginning
    line_num = 1;

    while(fgets(line, sizeof(line), file)!= NULL){
        //unsafe gets
        if(strstr(line, "gets")){
            char description[100];
            sprintf(description, "Unsafe option of using gets use fgets instead line: %d", line_num);
            AddToken(tokenList, "unsafe option", line_num, description);
        }

        if (strstr(line, "strcpy") != NULL || strstr(line, "strcat") != NULL) {
            if (strstr(line, "strncpy") == NULL && strstr(line, "strncat") == NULL) {
                char description[100];
                sprintf(description, "Buffer overflow: Unsafe String operation without bounds checking");
                AddToken(tokenList, "Buffer Overflow", line_num, description);
                if(strstr(line, "(") == NULL || strstr(line, ")") == NULL) {
                    char description[100];
                    sprintf(description, "Missing arguments for strcpy or strcat at line %d", line_num);
                    AddToken(tokenList, "Missing Arguments", line_num, description);
                }
            }else{
            if(strstr(line, "(") == NULL || strstr(line, ")") == NULL) {
                char description[100];
                sprintf(description, "Missing arguments for strcpy or strcat at line %d", line_num);
                AddToken(tokenList, "Missing Arguments", line_num, description);
            }
          }
        }

        // Check for null pointer dereference
        if (strstr(line, "->" ) != NULL || strstr(line, "*") != NULL) {
            if (strstr(line, "NULL") == NULL && strstr(line, "!= NULL") == NULL) {
                char description[100];
                sprintf(description, "Possible null pointer dereference at line %d", line_num);
                AddToken(tokenList, "Null Pointer", line_num, description);
            }
        }

        // check for free block without arguments
        if(strstr(line, "free") != NULL && (strstr(line, "(") == NULL || strstr(line, ")") == NULL)){
            char description[100];
            sprintf(description, "No reference for free at line %d", line_num);
            AddToken(tokenList, "free error", line_num, description);
        }
        if(strstr(line, "malloc")!= NULL && (strstr(line, "(") == NULL || strstr(line, ")") == NULL)){
            char description[100];
            sprintf(description, "No reference for malloc at line %d", line_num);
            AddToken(tokenList, "malloc error", line_num, description);
        }
        if(strstr(line, "calloc")!= NULL && (strstr(line, "(") == NULL || strstr(line, ")") == NULL)){
            char description[100];
            sprintf(description, "No reference for calloc at line %d", line_num);
            AddToken(tokenList, "calloc error", line_num, description);
        }
        if(strstr(line, "realloc")!= NULL && (strstr(line, "(") == NULL || strstr(line, ")") == NULL)){
            char description[100];
            sprintf(description, "No reference for realloc at line %d", line_num);
            AddToken(tokenList, "realloc error", line_num, description);
        }
        if(strstr(line, "exit")!= NULL && (strstr(line, "(") == NULL || strstr(line, ")") == NULL)){
            char description[100];
            sprintf(description, "No reference for exit at line %d", line_num);
            AddToken(tokenList, "exit error", line_num, description);
        }

        line_num++;
    }
    fseek(file, 0, SEEK_SET);
    line_num = 1;
    VariableInfo* tracked_variables = NULL; // List to track variables in scope

    while(fgets(line, sizeof(line), file) != NULL){
        // Skip comments and preprocessor directives for simpler analysis
        if(line[0] == '\n' || (line[0] == '/' && line[1] == '/') || (line[0] == '/' && line[1] == '*') || (line[0] == '*' && line[1] == '/')){
            line_num++;
            continue;
        }
        if(line[0] == '#'){
            line_num++;
            continue;
        }

        // Trim trailing whitespace from the line
        int len = strlen(line);
        while(len > 0 && (isspace(line[len - 1]) || line[len - 1] == '\t')){
            line[len - 1] = '\0';
            len--;
        }
        if(len == 0){ // Skip empty lines after trimming
            line_num++;
            continue;
        }
        // 1. Detect Variable Declarations and add to tracked_variables
        if (isVariableDeclaration(line)) {
            char* var_name = extractVariableFromDeclaration(line);
            char* var_type = extractVariableType(line);
            int initialized = isInitialized(line);

            if (var_name != NULL && var_type != NULL) {
                // If the variable is already tracked (e.g., re-declaration in a new scope), update its info.
                VariableInfo* existing_var = findVariable(tracked_variables, var_name);
                if (existing_var == NULL) {
                     addVariable(&tracked_variables, var_name, var_type, line_num, initialized);
                } else {
                    // Update the latest declaration line and initialization status
                    existing_var->declaration_line = line_num;
                    existing_var->is_initialized = initialized;
                }
            }
        }

        // 2. Detect Assignments and mark variables as initialized
        // This is a simplified check for assignments like `variable_name = value;`
        char *equals_pos = strchr(line, '=');
        if (equals_pos != NULL) {
            // Attempt to extract the variable name on the left-hand side of '='
            char *name_end_ptr = equals_pos - 1;
            while (name_end_ptr >= line && isspace(*name_end_ptr)) {
                name_end_ptr--;
            }

            char *name_start_ptr = name_end_ptr;
            while (name_start_ptr >= line && (isalnum(*name_start_ptr) || *name_start_ptr == '_')) {
                name_start_ptr--;
            }
            name_start_ptr++; // Move to the actual start of the variable name

            if (name_start_ptr < equals_pos) { // Ensure a variable name was found before '='
                char var_name_assigned[50];
                int name_len = name_end_ptr - name_start_ptr + 1;
                if (name_len > 0 && name_len < 50) {
                    strncpy(var_name_assigned, name_start_ptr, name_len);
                    var_name_assigned[name_len] = '\0';

                    VariableInfo* var = findVariable(tracked_variables, var_name_assigned);
                    if (var != NULL) {
                        var->is_initialized = 1; // Mark the variable as initialized upon assignment
                    }
                }
            }
        }
        
        // 3. Detect Variable Usage and Check for Uninitialization
        VariableInfo* current_var = tracked_variables;
        while (current_var != NULL) {
            // Check only if the variable has been declared at or before the current line
            // and if it's currently marked as uninitialized.
            if (current_var->declaration_line <= line_num && current_var->is_initialized == 0) {
                // Look for the variable's name in the current line
                char* found_usage = strstr(line, current_var->name);

                if (found_usage != NULL) {
                    // Apply a basic word boundary check to reduce false positives
                    // (e.g., "my_var" should not trigger if "another_my_variable" is found)
                    int is_whole_word = 1;
                    if (found_usage > line && (isalnum(*(found_usage - 1)) || *(found_usage - 1) == '_')) {
                        is_whole_word = 0;
                    }
                    if (*(found_usage + strlen(current_var->name)) != '\0' && (isalnum(*(found_usage + strlen(current_var->name))) || *(found_usage + strlen(current_var->name)) == '_')) {
                        is_whole_word = 0;
                    }

                    // Exclude lines that are themselves declarations or assignments to this variable
                    if (is_whole_word && !isVariableDeclaration(line) && strchr(line, '=') == NULL) {
                        char description[100];
                        sprintf(description, "Variable '%s' used before initialization at line %d", current_var->name, line_num);
                        AddToken(tokenList, "Uninitialized Variable", line_num, description);
                    }
                }
            }
            current_var = current_var->next;
        }
        line_num++;
    }
    fclose(file);
}

void report_variables(){
    VariableInfo* Variables = NULL;
    Variables = extractAllVariables("testcase.txt");
    printf("Extracting variables from testcase.txt...\n");
    if (Variables == NULL) {
        printf("Debug: extractAllVariables returned NULL\n");
    }
    displayVariables(Variables);
    freeVariableList(Variables); // Free the allocated variable list
}

void report_functions(){
    FunctionInfo* Funcs = NULL;
    Funcs = extractAllFunctions("testcase.txt");
    printf("Extracting functions from testcase.txt...\n");
    if (Funcs == NULL) {
        printf("Debug: extractAllFunctions returned NULL\n");
    }
    displayFunctions(Funcs);
    freeFunctionList(Funcs); // Free the allocated function list
}

int main() {
    token* tokenList = NULL;

    printf("====Bug-Detection in C using C====\n");
    analyse_code("testcase.txt", &tokenList);
    
    report_variables();
    report_functions();

    ShowTokens(tokenList);
    delete_tokens(tokenList);

    return 0;
}