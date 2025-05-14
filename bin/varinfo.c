#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct var_info{
    char name[50];
    char type[50];
    int declaration_line;
    int is_initialized;
    int is_array;
    int array_size;
    int is_freed;
    struct var_info *next;
} var_info;

var_info* CreateVarInfo(char* name, char* type, int line, int initialised){
    var_info* newvar = (var_info*)malloc(sizeof(var_info));
    if(newvar == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
    
    strcpy(newvar->name, name);
    strcpy(newvar->type, type);
    newvar->declaration_line = line;
    newvar->is_initialized = initialised;
    newvar->is_freed = 0;

    return newvar;
}

void AddVar(var_info** head, char* name, char* type, int line, int initialised){
    var_info* newvar = CreateVarInfo(name, type, line, initialised);

    if(*head == NULL){
        *head = newvar;
        return;
    }

    var_info* current = *head;
    while(current->next != NULL){
        current = current->next;
    }

    current->next = newvar;
}

var_info* FindVar(var_info* head, char* name){
    var_info* current = head;

    while(current != NULL){
        if (!strcmp(current->name, name)){
            return current;
        }
        current = current->next;
    }

    return NULL;
}

void mark_var_freed(var_info* head, char* name){
    var_info* var = FindVar(head, name);
    if(var != NULL){
        var->is_freed = 1;
    }
}

char* ExtractVarFromDeclaration(char* line){
    static char var_name[50];
    char* type_end = NULL;
    char* name_end = NULL;

    //Finds the type and places pointer at the end of the type
    type_end = strchr(line, ' ');
    if(type_end == NULL)    return NULL;

    //Skip any whitespaces after the type
    while(*type_end == ' ' || *type_end == '\t')    type_end++;

    //Finds the end of the variable name and places pointer at the end of the name
    name_end = strpbrk(type_end, ";=,[(\n");
    if(name_end == NULL)    return NULL;

    //copy the var name
    int name_length = name_end - type_end;
    if(name_length >= 50 || name_length <= 0) return NULL;

    strncpy(var_name, type_end, name_length);
    var_name[name_length] = '\0';

    //Trimming trailing whitespaces
    int i = name_length - 1;
    while(i >= 0 && var_name[i] == ' ' || var_name[i] == '\t'){
        var_name[i] = '\0';
        i--;
    }

    return var_name;
}