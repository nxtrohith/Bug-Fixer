#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "VariableExtractor.c" // Assuming this is in the same directory
#include "infiniterecursion.c" // Assuming this is in the same directory

#define INITIAL_HEAP_CAPACITY 100

// --- Priority Definitions ---
#define PRIORITY_CRITICAL 1
#define PRIORITY_HIGH 2
#define PRIORITY_MEDIUM 3
#define PRIORITY_LOW 4

typedef struct token {
    char type[50];
    int line_num;
    char description[100];
    int priority;
    // No 'next' pointer needed for heap implementation
} token;

// --- Priority Queue (Min-Heap) ---
typedef struct PriorityQueue {
    token** heap;     // Array of token pointers
    int size;         // Current number of elements in heap
    int capacity;     // Max capacity of the heap
} PriorityQueue;

token* CreateToken(char* type, int line_num, char* description, int priority) {
    token* newtoken = (token*)malloc(sizeof(token));
    if (newtoken == NULL) {
        printf("Memory allocation failed for token.\n");
        exit(1);
    }
    strcpy(newtoken->type, type);
    newtoken->line_num = line_num;
    strcpy(newtoken->description, description);
    newtoken->priority = priority;
    return newtoken;
}

PriorityQueue* createPriorityQueue(int capacity) {
    PriorityQueue* pq = (PriorityQueue*)malloc(sizeof(PriorityQueue));
    if (!pq) {
        printf("Memory allocation failed for PriorityQueue struct.\n");
        exit(1);
    }
    pq->heap = (token**)malloc(sizeof(token*) * capacity);
    if (!pq->heap) {
        printf("Memory allocation failed for PriorityQueue heap.\n");
        free(pq);
        exit(1);
    }
    pq->size = 0;
    pq->capacity = capacity;
    return pq;
}

void swapTokens(token** a, token** b) {
    token* temp = *a;
    *a = *b;
    *b = temp;
}

int parent(int i) { return (i - 1) / 2; }
int leftChild(int i) { return 2 * i + 1; }
int rightChild(int i) { return 2 * i + 2; }

void heapifyUp(PriorityQueue* pq, int index) {
    while (index > 0 && pq->heap[parent(index)]->priority > pq->heap[index]->priority) {
        swapTokens(&pq->heap[parent(index)], &pq->heap[index]);
        index = parent(index);
    }
}

void heapifyDown(PriorityQueue* pq, int index) {
    int minIndex = index;
    int l = leftChild(index);
    int r = rightChild(index);

    if (l < pq->size && pq->heap[l]->priority < pq->heap[minIndex]->priority) {
        minIndex = l;
    }
    if (r < pq->size && pq->heap[r]->priority < pq->heap[minIndex]->priority) {
        minIndex = r;
    }

    if (index != minIndex) {
        swapTokens(&pq->heap[index], &pq->heap[minIndex]);
        heapifyDown(pq, minIndex);
    }
}

void enqueueToken(PriorityQueue* pq, char* type, int line_num, char* description, int priority) {
    if (pq->size == pq->capacity) {
        pq->capacity *= 2;
        pq->heap = (token**)realloc(pq->heap, sizeof(token*) * pq->capacity);
        if (!pq->heap) {
            printf("Memory reallocation failed for PriorityQueue heap.\n");
            for(int i = 0; i < pq->size; ++i) free(pq->heap[i]); // Free existing tokens
            free(pq->heap); // Free the old heap array pointer if realloc returned NULL
            free(pq);
            exit(1);
        }
    }
    token* newToken = CreateToken(type, line_num, description, priority);
    pq->heap[pq->size] = newToken;
    pq->size++;
    heapifyUp(pq, pq->size - 1);
}

token* dequeueToken(PriorityQueue* pq) {
    if (pq->size == 0) {
        return NULL;
    }
    token* root = pq->heap[0];
    pq->heap[0] = pq->heap[pq->size - 1];
    pq->size--;
    if (pq->size > 0) {
        heapifyDown(pq, 0);
    }
    return root;
}

void ShowTokensByPriority(PriorityQueue* pq) {
    if (pq == NULL || pq->size == 0) {
        printf("No bugs detected by static analyser.\n");
        return;
    }
    printf("==== Bugs Detected (sorted by priority) ====\n");
    token* t = dequeueToken(pq);
    while (t != NULL) {
        printf("Priority: %d\n", t->priority);
        printf("Type: %s\n", t->type);
        printf("Line: %d\n", t->line_num);
        printf("Description: %s\n", t->description);
        printf("\n");
        free(t); 
        t = dequeueToken(pq);
    }
     printf("===========================================\n");
}

void freePriorityQueue(PriorityQueue* pq) {
    if (pq == NULL) return;
    token* t;
    while ((t = dequeueToken(pq)) != NULL) { // Dequeue and free any remaining tokens
        free(t);
    }
    free(pq->heap);
    free(pq);
}

void analyse_code(const char* code, PriorityQueue* tokenPq) {
    FILE* file = fopen(code, "r");
    if(file == NULL){
        printf("Error opening file for analysis. Please check the file name and try again: %s\n", code);
        return;
    }

    char line[256];
    int line_num = 1;

    char stack[100];
    int top = -1;
    int bracket_positions[100];
    while(fgets(line, sizeof(line), file) != NULL){
        for(int i = 0; line[i] != '\0'; i++) {
            if(line[i] == '(' || line[i] == '{' || line[i] == '[') {
                if (top < 99) {
                    top++;
                    stack[top] = line[i];
                    bracket_positions[top] = line_num;
                }
            }
            else if(line[i] == ')' || line[i] == '}' || line[i] == ']') {
                if(top == -1) {
                    char description[100];
                    sprintf(description, "Unexpected closing bracket '%c'", line[i]);
                    enqueueToken(tokenPq, "Bracket Error", line_num, description, PRIORITY_MEDIUM);
                }
                else {
                    char expected_bracket;
                    if(line[i] == ')') expected_bracket = '(';
                    else if(line[i] == '}') expected_bracket = '{';
                    else expected_bracket = '[';

                    if(stack[top] != expected_bracket) {
                        char description[100];
                        sprintf(description, "Mismatched bracket: expected '%c' found '%c'",
                                stack[top] == '(' ? ')' : (stack[top] == '{' ? '}' : ']'), line[i]);
                        enqueueToken(tokenPq, "Bracket Error", line_num, description, PRIORITY_MEDIUM);
                    }
                    top--;
                }
            }
        }
        line_num++;
    }
    while(top >= 0) {
        char description[100];
        char expected_bracket;
        if(stack[top] == '(') expected_bracket = ')';
        else if(stack[top] == '{') expected_bracket = '}';
        else expected_bracket = ']';
        sprintf(description, "Unclosed bracket '%c' - missing '%c'", stack[top], expected_bracket);
        enqueueToken(tokenPq, "Bracket Error", bracket_positions[top], description, PRIORITY_MEDIUM);
        top--;
    }

    fseek(file, 0, SEEK_SET);
    line_num = 1;
    int in_struct_definition = 0;

    while(fgets(line, sizeof(line), file)!= NULL){
        if(line[0] == '\n' || strncmp(line, "//", 2) == 0 || strncmp(line, "/*", 2) == 0 || strncmp(line, "*/", 2) == 0 || (line[0] == '*' && line[1] == ' ' && strstr(line, "*/") == NULL)){
            line_num++;
            continue;
        }
        if(line[0] == '#'){
            line_num++;
            continue;
        }

        if(strstr(line, "struct") && strchr(line, '{')) {
            in_struct_definition = 1;
        }
        if(in_struct_definition && strchr(line, '}')) {
            char* closing_brace = strchr(line, '}');
            if (closing_brace && strchr(closing_brace, ';')) { // Typical end of struct def: };
                 in_struct_definition = 0;
            }
        }

        int len = strlen(line);
        while(len > 0 && isspace((unsigned char)line[len - 1])){
            line[len - 1] = '\0';
            len--;
        }
        if(len == 0){
            line_num++;
            continue;
        }

        int should_have_semicolon = 0;
        const char* patterns_require_semicolon[] = {
            "return", "printf", "scanf", "malloc", "free", "calloc", "realloc",
            "exit", "abort", "atexit", "strcpy", "strcat", "strlen", "strcmp",
            "strncpy", "strncat", "strncmp", "strstr", "strchr", "strrchr",
            "strspn", "strcspn", "strpbrk", "strtok", "strerror", "strtol",
            "strtoul", "strtod", "goto", "break", "continue", NULL
        };
       
        int is_assignment_or_op = (strchr(line, '=') && !strstr(line, "==") && !strstr(line, "!=") && !strstr(line, "<=") && !strstr(line, ">=") && !strchr(strchr(line, '=') + 1, '=')) ||
                                strstr(line, "++") || strstr(line, "--") ||
                                strstr(line, "+=") || strstr(line, "-=") || strstr(line, "*=") ||
                                strstr(line, "/=") || strstr(line, "%=") || strstr(line, "&=") ||
                                strstr(line, "|=") || strstr(line, "^=");

        if (is_assignment_or_op) {
            should_have_semicolon = 1;
        } else {
            for (int i = 0; patterns_require_semicolon[i] != NULL; ++i) {
                if (strstr(line, patterns_require_semicolon[i])) {
                    should_have_semicolon = 1;
                    break;
                }
            }
        }

        if (should_have_semicolon) {
            char trimmed_line_for_block_check[256];
            strcpy(trimmed_line_for_block_check, line);
            int k = strlen(trimmed_line_for_block_check) -1;
            while(k>=0 && isspace(trimmed_line_for_block_check[k])) trimmed_line_for_block_check[k--] = '\0';
            
            char last_char_in_trimmed = trimmed_line_for_block_check[strlen(trimmed_line_for_block_check)-1];

            if (last_char_in_trimmed == '{' || last_char_in_trimmed == '}' || last_char_in_trimmed == '>') { // e.g. #include <foo.h>
                 should_have_semicolon = 0;
            }
             if ((strstr(line,"if") && strchr(line,'(') ) ||
                (strstr(line,"for") && strchr(line,'(') ) ||
                (strstr(line,"while") && strchr(line,'(') ) ||
                (strstr(line,"switch") && strchr(line,'(') ) ||
                 strstr(line, "else if") ||
                (strstr(line, "else") && (last_char_in_trimmed == '{' || strlen(trimmed_line_for_block_check) == 4 /* "else" */)))
               {
                if(last_char_in_trimmed == '{') should_have_semicolon = 0;
            } else if (strstr(line, "#include") || strstr(line, "#define") || 
                       (strstr(line, "struct ") && last_char_in_trimmed == '{') ||
                       (strstr(line, "enum ") && last_char_in_trimmed == '{') ||
                       (strstr(line, "union ") && last_char_in_trimmed == '{') ) {
                should_have_semicolon = 0;
            }
        }
        
        if ((strstr(line, "int ") || strstr(line, "char ") ||
             strstr(line, "float ") || strstr(line, "double ") ||
             strstr(line, "void ") || strstr(line, "short ") || strstr(line, "long ") ||
             (strstr(line, "struct ") && !strchr(line, '{') && !in_struct_definition ) ||
             (strstr(line, "enum ") && !strchr(line, '{') && !in_struct_definition) ||
             (strstr(line, "union ") && !strchr(line, '{') && !in_struct_definition)) &&
             !strchr(line, '{') && !strstr(line, "typedef struct") && !strstr(line,"sizeof(")) {
            char *open_paren = strchr(line, '(');
            if (!open_paren || (open_paren && strchr(open_paren,')') && !strchr(open_paren,'{'))) { // Declaration or prototype
                 if (line[len-1] != ';') should_have_semicolon = 1;
            }
        }
        if (in_struct_definition && strstr(line, "}") && strchr(line, ';')) {
             should_have_semicolon = 0;
        }


        if(should_have_semicolon && !in_struct_definition && line[len - 1] != ';'){
            char description[100];
            sprintf(description, "Missing semicolon");
            enqueueToken(tokenPq, "Missing Semicolon", line_num, description, PRIORITY_LOW);
        }

        int in_string = 0;
        int consecutive_semicolons = 0;
        for(int i = 0; i < len; i++) {
            if(line[i] == '"' && (i == 0 || line[i-1] != '\\')) {
                in_string = !in_string;
            }
            if(!in_string && line[i] == ';') {
                consecutive_semicolons++;
                if(consecutive_semicolons > 1) {
                    char description[100];
                    sprintf(description, "Extra semicolon");
                    enqueueToken(tokenPq, "Extra Semicolon", line_num, description, PRIORITY_LOW);
                    break; 
                }
            } else if(!in_string && !isspace((unsigned char)line[i])){
                consecutive_semicolons = 0;
            }
        }
        line_num++;
    }

    fseek(file, 0, SEEK_SET);
    line_num = 1;
    while(fgets(line, sizeof(line), file)!= NULL){
        if( (strstr(line, "/ 0") || strstr(line, "% 0")) && !strstr(line, "/ 0.") ) {
             if (!strstr(line, "//") || (strstr(line, "//") > strstr(line, "/ 0") || strstr(line, "//") > strstr(line, "% 0"))){
                char description[100];
                sprintf(description, "Potential division by zero");
                enqueueToken(tokenPq, "Division by Zero", line_num, description, PRIORITY_CRITICAL);
             }
        }
        line_num++;
    }

    fseek(file, 0, SEEK_SET);
    line_num = 1;
    while(fgets(line, sizeof(line), file)!= NULL){
        if(strstr(line, "gets(")){
             if (!strstr(line, "//") || strstr(line, "//") > strstr(line, "gets(")){
                char description[100];
                sprintf(description, "Unsafe use of gets(). Consider fgets()");
                enqueueToken(tokenPq, "Unsafe Function", line_num, description, PRIORITY_CRITICAL);
            }
        }

        if ( (strstr(line, "strcpy(") || strstr(line, "strcat(")) &&
             !strstr(line, "strncpy(") && !strstr(line, "strncat(") ) {
             if (!strstr(line, "//") || (strstr(line, "//") > strstr(line, "strcpy(")) || (strstr(line, "//") > strstr(line, "strcat(")) ) {
                char description[100];
                sprintf(description, "Potential buffer overflow with strcpy/strcat. Use bounded versions.");
                enqueueToken(tokenPq, "Buffer Overflow Risk", line_num, description, PRIORITY_CRITICAL);
            }
        }

        const char* funcs_needing_args[] = {"free(", "malloc(", "calloc(", "realloc(", "exit(", NULL};
        for(int i=0; funcs_needing_args[i] != NULL; ++i) {
            char func_name_only[20];
            strncpy(func_name_only, funcs_needing_args[i], strlen(funcs_needing_args[i])-1);
            func_name_only[strlen(funcs_needing_args[i])-1] = '\0';

            char pattern_empty_call[30]; 
            sprintf(pattern_empty_call, "%s()", func_name_only);

            if (strstr(line, pattern_empty_call)) {
                 if (!strstr(line, "//") || strstr(line, "//") > strstr(line, pattern_empty_call)){
                    char description[100];
                    sprintf(description, "Function %s called without arguments.", func_name_only);
                    enqueueToken(tokenPq, "Missing Arguments", line_num, description, PRIORITY_MEDIUM);
                }
            }
        }
        line_num++;
    }
    
    fseek(file, 0, SEEK_SET);
    line_num = 1;
    VariableInfo* tracked_variables = NULL; 

    while(fgets(line, sizeof(line), file) != NULL){
        if(line[0] == '\n' || strncmp(line, "//", 2) == 0 || strncmp(line, "/*", 2) == 0) {
            line_num++;
            continue;
        }
        if(line[0] == '#'){
            line_num++;
            continue;
        }

        int len = strlen(line);
        while(len > 0 && isspace((unsigned char)line[len-1])) line[--len] = '\0';
        if(len == 0) { line_num++; continue; }

        char temp_line_for_var_extraction[256]; 
        strcpy(temp_line_for_var_extraction, line);

        if (isVariableDeclaration(temp_line_for_var_extraction)) {
            char* var_name_raw = extractVariableFromDeclaration(temp_line_for_var_extraction); 
            if (var_name_raw) { 
                 char var_name[50]; 
                 strncpy(var_name, var_name_raw, 49); var_name[49] = '\0';

                VariableInfo* v_exists = findVariable(tracked_variables, var_name);
                if (!v_exists) {
                    char* var_type_raw = extractVariableType(temp_line_for_var_extraction);
                    char var_type[20] = "unknown";
                    if (var_type_raw) { strncpy(var_type, var_type_raw, 19); var_type[19] = '\0';}
                    
                    addVariable(&tracked_variables, var_name, var_type, line_num, isInitialized(temp_line_for_var_extraction));
                } else {
                     v_exists->is_initialized = isInitialized(temp_line_for_var_extraction) || v_exists->is_initialized; 
                     v_exists->declaration_line = line_num; 
                }
            }
        }

        char *equals_pos = strchr(line, '=');
        if (equals_pos && equals_pos > line && *(equals_pos-1) != '=' && *(equals_pos+1) != '=') { 
            char var_name_assigned_lhs[50] = "";
            char* p_name = line;
            while(p_name < equals_pos && isspace(*p_name)) p_name++;
            char* p_name_end = p_name;
            while(p_name_end < equals_pos && (isalnum(*p_name_end) || *p_name_end == '_' || *p_name_end == '*' || *p_name_end == '[' || *p_name_end == ']')) p_name_end++;
            if (p_name_end > p_name && p_name_end - p_name < 49) {
                strncpy(var_name_assigned_lhs, p_name, p_name_end - p_name);
                var_name_assigned_lhs[p_name_end - p_name] = '\0';
                // Further trim if needed e.g. "*x", "arr[i]"
                char* final_name_token = strtok(var_name_assigned_lhs, " \t*[]");
                if(final_name_token) {
                    VariableInfo* var = findVariable(tracked_variables, final_name_token);
                    if (var) {
                        var->is_initialized = 1;
                    }
                }
            }
        }
        
        VariableInfo* current_var = tracked_variables;
        while (current_var != NULL) {
            if (current_var->declaration_line < line_num && !current_var->is_initialized) {
                char* usage = strstr(line, current_var->name);
                if (usage) {
                    int is_declaration_line = 0; // Simplified: assume not declaration if already tracked and past its line
                    int is_assignment_lhs = 0;
                    if (equals_pos && equals_pos > line) {
                         char* p = line;
                         while(p < equals_pos && isspace(*p)) p++;
                         if(strncmp(p, current_var->name, strlen(current_var->name)) == 0) {
                             char after_name = *(p + strlen(current_var->name));
                             if(isspace(after_name) || after_name == '=' || after_name == '[' || after_name == '\0')
                                is_assignment_lhs = 1;
                         }
                    }

                    int name_len = strlen(current_var->name);
                    int is_whole_word = 1;
                    if (usage > line && (isalnum((unsigned char)*(usage - 1)) || *(usage - 1) == '_')) {
                        is_whole_word = 0;
                    }
                    if (*(usage + name_len) != '\0' && (isalnum((unsigned char)*(usage + name_len)) || *(usage + name_len) == '_')) {
                        is_whole_word = 0;
                    }

                    if (is_whole_word && !is_assignment_lhs && !strstr(line, "scanf")) { 
                        char description[100];
                        sprintf(description, "Variable '%s' may be used uninitialized", current_var->name);
                        enqueueToken(tokenPq, "Uninitialized Variable", line_num, description, PRIORITY_HIGH);
                        current_var->is_initialized = 1; 
                    }
                }
            }
            current_var = current_var->next;
        }
        line_num++;
    }
    freeVariableList(tracked_variables); 
    fclose(file);
}

void report_variables(const char* filename){
    VariableInfo* Variables = NULL;
    printf("\n==== Detailed Variable Analysis (from VariableExtractor.c) ====\n");
    Variables = extractAllVariables(filename); 
    if (Variables != NULL) { // Only display if variables were found/list is not NULL
        displayVariables(Variables);
    } else {
        printf("No variables processed or found by VariableExtractor for display.\n");
    }
    freeVariableList(Variables);
    printf("=============================================================\n");
}

void report_functions(const char* filename){
    FunctionInfo* Funcs = NULL;
    printf("\n==== Function Listing (from VariableExtractor.c) ====\n");
    Funcs = extractAllFunctions(filename); 
    if (Funcs != NULL) { // Only display if functions were found
         displayFunctions(Funcs);
    } else {
        printf("No functions found by VariableExtractor.\n");
    }
    freeFunctionList(Funcs);
    printf("======================================================\n");
}

void report_infinite_recursion(const char* filename) {
    printf("\n==== Infinite Recursion Check (from infiniterecursion.c) ====\n");
    detectInfiniteRecursion(filename); 
    printf("===========================================================\n");
}

int main() {
    PriorityQueue* tokenPq = createPriorityQueue(INITIAL_HEAP_CAPACITY);
    const char* test_file_name = "testcase.txt"; 

    printf("==== Bug-Detection in C using C ====\n");
    printf("Analysing file: %s\n\n", test_file_name);

    analyse_code(test_file_name, tokenPq);
    ShowTokensByPriority(tokenPq); 
    
    freePriorityQueue(tokenPq); 

    report_variables(test_file_name);
    report_functions(test_file_name);
    report_infinite_recursion(test_file_name);

    printf("\nAnalysis complete.\n");

    return 0;
}