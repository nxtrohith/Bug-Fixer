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
    char* type_end = NULL;
    char* name_end = NULL;
    
    // Skip type (find first space after type)
    type_end = strchr(line, ' ');
    if (type_end == NULL) return NULL;
    
    // Skip any additional spaces
    while (*type_end == ' ') type_end++;
    
    // Find end of variable name (semicolon, equals sign, comma, bracket, etc.)
    name_end = strpbrk(type_end, ";=,[(\n");
    if (name_end == NULL) return NULL;
    
    // Extract the variable name
    int name_len = name_end - type_end;
    if (name_len >= 50 || name_len <= 0) return NULL;
    
    strncpy(var_name, type_end, name_len);
    var_name[name_len] = '\0';
    
    // Trim trailing spaces
    int i = name_len - 1;
    while (i >= 0 && var_name[i] == ' ') {
        var_name[i] = '\0';
        i--;
    }
    
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
    char* type_end = NULL;
    
    // Find end of type (space after type name)
    type_end = strchr(line, ' ');
    if (type_end == NULL) return NULL;
    
    // Extract the type
    int type_len = type_end - line;
    if (type_len >= 20 || type_len <= 0) return NULL;
    
    strncpy(var_type, line, type_len);
    var_type[type_len] = '\0';
    
    return var_type;
}