#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Structure to store variable information
typedef struct VariableInfo {
    char name[50];           // Variable name
    char type[20];           // Variable type (int, char, etc.)
    int declaration_line;    // Line where variable was declared
    int is_initialized;      // Whether variable was initialized at declaration
    int is_freed;            // Whether variable has been freed (for pointers)
    struct VariableInfo* next;
} VariableInfo;

// Structure to store function information
typedef struct FunctionInfo {
    char name[50];           // Function name
    int start_line;          // Line where function starts
    int end_line;            // Line where function ends
    struct FunctionInfo* next;
} FunctionInfo;

// Function to create a new variable info node
VariableInfo* createVariableInfo(char* name, char* type, int line, int initialized) {
    VariableInfo* newVar = (VariableInfo*)malloc(sizeof(VariableInfo));
    if (newVar == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    
    strcpy(newVar->name, name);
    strcpy(newVar->type, type);
    newVar->declaration_line = line;
    newVar->is_initialized = initialized;
    newVar->is_freed = 0;
    newVar->next = NULL;
    
    return newVar;
}

// Function to add a variable to the tracking list
void addVariable(VariableInfo** head, char* name, char* type, int line, int initialized) {
    VariableInfo* newVar = createVariableInfo(name, type, line, initialized);
    
    if (*head == NULL) {
        *head = newVar;
        return;
    }
    
    VariableInfo* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    
    current->next = newVar;
}

// Function to find a variable in the tracking list
VariableInfo* findVariable(VariableInfo* head, char* name) {
    VariableInfo* current = head;
    
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Function to mark a variable as freed
void markVariableAsFreed(VariableInfo* head, char* name) {
    VariableInfo* var = findVariable(head, name);
    if (var != NULL) {
        var->is_freed = 1;
    }
}
// Function to extract variable name from a declaration
char* extractVariableFromDeclaration(char* line) {
    static char var_name[50];
    char* current_pos = line;

    // 1. Skip leading whitespace before the type
    while (*current_pos && isspace((unsigned char)*current_pos)) {
        current_pos++;
    }
    if (*current_pos == '\0') return NULL; // Line is empty or all whitespace

    // 2. Skip the type keyword
    char* type_keyword_end = current_pos;
    while (*type_keyword_end && !isspace((unsigned char)*type_keyword_end)) {
        type_keyword_end++;
    }
    // 3. Skip spaces between the type and the variable name
    char* name_start_ptr = type_keyword_end;
    while (*name_start_ptr && isspace((unsigned char)*name_start_ptr)) {
        name_start_ptr++;
    }
    if (*name_start_ptr == '\0' || *name_start_ptr == ';') return NULL; // No variable name found (e.g., "int ;")

    // 4. Find the end of the variable name
    char* name_end_ptr = name_start_ptr;
    // Variable name ends at ';', '=', ',', '[', '(', or newline
    name_end_ptr = strpbrk(name_start_ptr, ";=,[(\n");
    if (name_end_ptr == NULL) {
        return NULL;
    }

    int name_len = name_end_ptr - name_start_ptr;
    if (name_len >= 50 || name_len <= 0) return NULL;

    strncpy(var_name, name_start_ptr, name_len);
    var_name[name_len] = '\0';

    // Trim trailing spaces from the extracted variable name
    int i = name_len - 1;
    while (i >= 0 && isspace((unsigned char)var_name[i])) {
        var_name[i] = '\0';
        i--;
    }
    if (strlen(var_name) == 0) return NULL;

    return var_name;
}

// Function to extract variable name from malloc/calloc
char* extractVariableFromAllocation(char* line) {
    static char var_name[50];
    char* equals_pos = NULL;
    char* name_end = NULL;
    
    // Find assignment operator
    equals_pos = strchr(line, '=');
    if (equals_pos == NULL) return NULL;
    
    // Go backwards from equals to find variable name
    char* name_start = equals_pos - 1;
    
    // Skip spaces before equals
    while (name_start > line && *name_start == ' ') name_start--;
    
    // Find start of variable name (after type or space)
    char* name_end_rev = name_start;
    while (name_end_rev > line && 
           (isalnum(*name_end_rev) || *name_end_rev == '_')) name_end_rev--;
    
    // Extract the variable name
    int name_len = name_start - name_end_rev;
    if (name_len >= 50 || name_len <= 0) return NULL;
    
    strncpy(var_name, name_end_rev + 1, name_len);
    var_name[name_len] = '\0';
    
    return var_name;
}

// Function to extract variable name from free() call
char* extractVariableFromFree(char* line) {
    static char var_name[50];
    char* open_paren = NULL;
    char* close_paren = NULL;
    
    // Find opening parenthesis after "free"
    open_paren = strstr(line, "free(");
    if (open_paren == NULL) return NULL;
    open_paren += 5; // Move past "free("
    
    // Find closing parenthesis
    close_paren = strchr(open_paren, ')');
    if (close_paren == NULL) return NULL;
    
    // Extract the variable name
    int name_len = close_paren - open_paren;
    if (name_len >= 50 || name_len <= 0) return NULL;
    
    strncpy(var_name, open_paren, name_len);
    var_name[name_len] = '\0';
    
    // Trim spaces
    int i = 0;
    while (var_name[i] == ' ') i++;
    
    if (i > 0) {
        memmove(var_name, var_name + i, name_len - i + 1);
    }
    
    i = strlen(var_name) - 1;
    while (i >= 0 && var_name[i] == ' ') {
        var_name[i] = '\0';
        i--;
    }
    
    return var_name;
}

// Function to determine if a line contains a variable declaration
int isVariableDeclaration(char* line) {
    // Check for common C types
    return (strstr(line, "int ") != NULL || 
            strstr(line, "char ") != NULL || 
            strstr(line, "float ") != NULL || 
            strstr(line, "double ") != NULL || 
            strstr(line, "long ") != NULL || 
            strstr(line, "short ") != NULL || 
            strstr(line, "struct ") != NULL);
}

// Function to determine if a variable is initialized in its declaration
int isInitialized(char* line) {
    return (strchr(line, '=') != NULL);
}

// Function to extract variable type from declaration
char* extractVariableType(char* line) {
    static char var_type[20];
    char* type_start = line;

    // Skip leading whitespace
    while (*type_start && isspace((unsigned char)*type_start)) {
        type_start++;
    }
    if (*type_start == '\0') { // Empty line or only whitespace
        return NULL;
    }

    char* type_end_ptr = type_start;
    // Find end of the type keyword (the next space or end of string)
    while (*type_end_ptr && !isspace((unsigned char)*type_end_ptr) && *type_end_ptr != '*') { // also stop if '*' is encountered for pointers like char*
        type_end_ptr++;
    }

    int type_len = type_end_ptr - type_start;
    if (type_len <= 0 || type_len >= 20) {
        return NULL;
    }

    strncpy(var_type, type_start, type_len);
    var_type[type_len] = '\0';
    return var_type;
}

char* extractFunction(char* line) {
    static char function_name[50];
    char* open_paren = strchr(line, '(');
    if (!open_paren) return NULL;
    char* name_start = open_paren - 1;
    while (name_start > line && *name_start == ' ') name_start--;
    char* name_end = name_start;
    while (name_end > line && (isalnum(*name_end) || *name_end == '_')) name_end--;
    if (!isalnum(*name_end) && *name_end != '_') name_end++;
    int name_len = name_start - name_end + 1;
    if (name_len <= 0 || name_len >= 50) return NULL;
    strncpy(function_name, name_end, name_len);
    function_name[name_len] = '\0';
    return function_name;
}

FunctionInfo* createFunctionInfo(char* name, int start_line) {
    FunctionInfo* newFunc = malloc(sizeof(FunctionInfo));
    if (!newFunc) { printf("Memory allocation failed!\n"); exit(1); }
    strcpy(newFunc->name, name);
    newFunc->start_line = start_line;
    newFunc->end_line = -1;
    newFunc->next = NULL;
    return newFunc;
}

void addFunction(FunctionInfo** head, char* name, int start_line) {
    FunctionInfo* newFunc = createFunctionInfo(name, start_line);
    if (!*head) { *head = newFunc; return; }
    FunctionInfo* current = *head;
    while (current->next) current = current->next;
    current->next = newFunc;
}

// Function to add error description to a function
// REMOVED: void addFunctionError(FunctionInfo* func, const char* error) {
//     func->has_errors = 1;
//     
//     // If there's already an error, append to it
//     if (func->error_desc[0] != '\0') {
//         strcat(func->error_desc, "; ");
//         strcat(func->error_desc, error);
//     } else {
//         strcpy(func->error_desc, error);
//     }
// }

// Function to extract all functions from a file
FunctionInfo* extractAllFunctions(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    
    char line[256];
    int line_number = 1;
    FunctionInfo* functions = NULL;
    FunctionInfo* current_function = NULL;
    int brace_count = 0;
    int in_function = 0;
    
    // Debug output
    printf("Scanning file for functions...\n");
    
    while (fgets(line, sizeof(line), file)) {
        // More lenient function detection
        // Look for patterns that might indicate a function declaration
        if (!in_function && 
            strchr(line, '(') != NULL && 
            !strstr(line, "=") && 
            !strstr(line, "if") && 
            !strstr(line, "for") && 
            !strstr(line, "while") && 
            !strstr(line, "#include") && 
            !strstr(line, "#define")) {
            
            char* func_name = extractFunction(line);
            if (func_name != NULL && strlen(func_name) > 0) {
                // printf("Found function: %s at line %d\n", func_name, line_number);
                addFunction(&functions, func_name, line_number);
                current_function = functions;
                while (current_function->next != NULL) {
                    current_function = current_function->next;
                }
                in_function = 1;
                brace_count = 0; // Reset brace count for new function
                
                // Check for opening brace on same line
                if (strchr(line, '{') != NULL) {
                    brace_count++;
                }
            }
        }
        
        // Count braces to track function body
        if (in_function) {
            // Count opening braces
            char* ptr = line;
            while ((ptr = strchr(ptr, '{')) != NULL) {
                brace_count++;
                ptr++;
            }
            
            // Count closing braces
            ptr = line;
            while ((ptr = strchr(ptr, '}')) != NULL) {
                brace_count--;
                ptr++;
                
                // If brace count reaches 0, we've found the end of the function
                if (brace_count == 0) {
                    current_function->end_line = line_number;
                    in_function = 0;
                }
            }
            
            // Safety check: if we reach a new function signature while still in a function,
            // force end the current function and start a new one
            if (strchr(line, '(') != NULL && 
                !strstr(line, "=") && 
                !strstr(line, "if") && 
                !strstr(line, "for") && 
                !strstr(line, "while") && 
                (strstr(line, "int ") || strstr(line, "void ") || 
                 strstr(line, "char ") || strstr(line, "float ") || 
                 strstr(line, "double ") || strstr(line, "long ") || 
                 strstr(line, "struct "))) {
                
                // End current function
                if (current_function != NULL) {
                    current_function->end_line = line_number - 1;
                    // addFunctionError(current_function, "Possible missing closing brace");
                }
                
                // Start new function
                char* func_name = extractFunction(line);
                if (func_name != NULL && strlen(func_name) > 0) {
                    // printf("Found function: %s at line %d\n", func_name, line_number);
                    // addFunction(&functions, func_name, line_number);
                    // current_function = functions;
                    // while (current_function->next != NULL) {
                    //     current_function = current_function->next;
                    // }
                    brace_count = 0;
                    
                    // Check for opening brace on same line
                    if (strchr(line, '{') != NULL) {
                        brace_count++;
                    }
                }
            }
            
            // Check for common syntax errors
            if (current_function != NULL) {
                // Missing semicolons
                if (strstr(line, "for") == NULL && // Ignore for loop conditions
                    strstr(line, "if") == NULL && // Ignore if conditions
                    strstr(line, "while") == NULL && // Ignore while conditions
                    strstr(line, "{") == NULL && // Ignore opening braces
                    strstr(line, "}") == NULL && // Ignore closing braces
                    strstr(line, "//") == NULL && // Ignore comments
                    strchr(line, ';') == NULL && // No semicolon
                    strlen(line) > 2) { // Not an empty line
                    
                    // Check if line should have a semicolon
                    if (isalnum(line[0]) || strchr(line, '=') != NULL || 
                        strstr(line, "printf") != NULL || strstr(line, "return") != NULL) {
                        // addFunctionError(current_function, "Missing semicolon");
                    }
                }
                
                // Extra semicolons
                if (strstr(line, ");") != NULL && strstr(line, ");;") != NULL) {
                    // addFunctionError(current_function, "Extra semicolon");
                }
                
                // Mismatched if-else brackets
                if (strstr(line, "if") != NULL && strchr(line, '(') != NULL && 
                    strchr(line, ')') != NULL && strchr(line, ';') != NULL && 
                    strchr(line, '{') == NULL) {
                    // addFunctionError(current_function, "Semicolon after if condition");
                }
            }
        }
        
        line_number++;
    }
    
    // Handle case where the last function doesn't have a closing brace
    if (in_function && current_function != NULL) {
        current_function->end_line = line_number - 1;
        // addFunctionError(current_function, "Missing closing brace");
    }
    
    fclose(file);
    return functions;
}

// Function to display all extracted functions and their errors
void displayFunctions(FunctionInfo* head) {
    if (head == NULL) {
        printf("No functions found!\n");
        return;
    }
    FunctionInfo* current = head;
    int count = 1;
    printf("\n===== FUNCTIONS DETECTED =====\n\n");
    while (current != NULL) {
        printf("Function #%d: %s", count, current->name);
        printf("\tFrom line: %d\n\n", current->start_line);
        current = current->next;
        count++;
    }
}

// Function to free the memory allocated for the function list
void freeFunctionList(FunctionInfo* head) {
    FunctionInfo* current = head;
    FunctionInfo* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}

void displayVariables(VariableInfo* head) {
    printf("\nCollected Variables:\n");
    if (head == NULL) {
        printf("No variables found.\n");
        return;
    }
    VariableInfo* current = head;
    while (current != NULL) {
        printf("Name: %s, Type: %s, Declared at line: %d, Initialized: %s, Freed: %s\n",
            current->name,
            current->type,
            current->declaration_line,
            current->is_initialized ? "Yes" : "No",
            current->is_freed ? "Yes" : "No");
            printf("Variables Detected:\n\n");
            printf("Name: %s\nType: %s\nLine: %d\tInitialised: %s\tfreed: ", current->name);
        current = current->next;
    }
}

// Function to extract all variables from a file
VariableInfo* extractAllVariables(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }
    
    char line[256];
    int line_number = 1;
    VariableInfo* variables = NULL;
    
    printf("Scanning file for variables...\n");
    
    while (fgets(line, sizeof(line), file)) {
        // Check for variable declarations
        if (isVariableDeclaration(line)) {
            char* var_name = extractVariableFromDeclaration(line);
            char* var_type = extractVariableType(line);
            int initialized = isInitialized(line);
            
            if (var_name != NULL && var_type != NULL) {
                addVariable(&variables, var_name, var_type, line_number, initialized);
            }
        }

        // Check for memory allocations
        if (strstr(line, "malloc") || strstr(line, "calloc")) {
            char* var_name = extractVariableFromAllocation(line);
            if (var_name != NULL) {
                // Find if variable already exists
                VariableInfo* var = findVariable(variables, var_name);
                if (var == NULL) {
                    // If not found, add as a pointer type
                    addVariable(&variables, var_name, "pointer", line_number, 1);
                }
            }
        }
        
        // Check for memory deallocations
        if (strstr(line, "free")) {
            char* var_name = extractVariableFromFree(line);
            if (var_name != NULL) {
                markVariableAsFreed(variables, var_name);
            }else{
                printf("Error extracting variable from free() call at line %d\n", line_number);
            }
        }

        
        line_number++;
    }
    
    fclose(file);
    return variables;
}