#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FUNCS 100
#define MAX_NAME_LEN 100

// Structure for adjacency list node
typedef struct Node {
    int index;
    struct Node* next;
} Node;

// Graph structure
Node* adjList[MAX_FUNCS];
int visited[MAX_FUNCS];
int recStack[MAX_FUNCS];
char functionNames[MAX_FUNCS][MAX_NAME_LEN];
int funcCount = 0;

// Create a new node
Node* createNode(int index) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->index = index;
    newNode->next = NULL;
    return newNode;
}

// Add edge from function u to function v
void addEdge(int u, int v) {
    Node* newNode = createNode(v);
    newNode->next = adjList[u];
    adjList[u] = newNode;
}

// Get or assign function index
int getFunctionIndex(char* name) {
    for (int i = 0; i < funcCount; i++) {
        if (strcmp(functionNames[i], name) == 0)
            return i;
    }
    strcpy(functionNames[funcCount], name);
    funcCount++;
    return funcCount - 1;
}

// DFS to detect cycles
int isCyclic(int v) {
    visited[v] = 1;
    recStack[v] = 1;

    Node* temp = adjList[v];
    while (temp != NULL) {
        int neighbor = temp->index;
        if (!visited[neighbor] && isCyclic(neighbor))
            return 1;
        else if (recStack[neighbor])
            return 1;
        temp = temp->next;
    }

    recStack[v] = 0;
    return 0;
}

// Driver function to detect infinite recursion
void detectInfiniteRecursion(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }

    // Initialize structures
    for (int i = 0; i < MAX_FUNCS; i++) {
        adjList[i] = NULL;
        visited[i] = 0;
        recStack[i] = 0;
    }

    char line[256];
    char currentFunction[MAX_NAME_LEN] = "";

    while (fgets(line, sizeof(line), file)) {
        // Skip comment lines
        if (strstr(line, "//") == line) continue;

        // Detect function definitions
        char name[MAX_NAME_LEN];
        if (sscanf(line, "void %[^()](", name) == 1 ||
            sscanf(line, "int %[^()](", name) == 1 ||
            sscanf(line, "float %[^()](", name) == 1 ||
            sscanf(line, "double %[^()](", name) == 1 ||
            sscanf(line, "char %[^()](", name) == 1) {
            strcpy(currentFunction, name);
            getFunctionIndex(name);  // Register function
        }

        // Detect function calls (stricter check)
        for (int i = 0; i < funcCount; i++) {
            char pattern[MAX_NAME_LEN + 3];
            sprintf(pattern, "%s(", functionNames[i]);

            if (strstr(line, pattern)) {
                if (strlen(currentFunction) > 0) { // Ensure we are inside a function context
                    int u = getFunctionIndex(currentFunction);
                    int v = getFunctionIndex(functionNames[i]);
                    addEdge(u, v);
                }
            }
        }
    }
    fclose(file);

    // Check for cycles
    for (int i = 0; i < funcCount; i++) {
        // Reset visited and recStack for each component of the graph
        for (int j = 0; j < funcCount; j++) {
            visited[j] = 0;
            recStack[j] = 0;
        }
        if (isCyclic(i)) {
            printf("⚠️ Infinite recursion detected involving function: %s\n", functionNames[i]);
            // It's good practice to clean up allocated memory, though in this case, the program exits.
            // Freeing adjacency lists:
            for(int k=0; k < funcCount; ++k) {
                Node* current = adjList[k];
                while(current != NULL) {
                    Node* next = current->next;
                    free(current);
                    current = next;
                }
                adjList[k] = NULL; // Avoid dangling pointers
            }
            return; // Exit after first detection to avoid redundant messages or deeper issues.
        }
    }

    printf("✅ No infinite recursion detected.\n");
    // Clean up allocated memory
     for(int i=0; i < funcCount; ++i) {
        Node* current = adjList[i];
        while(current != NULL) {
            Node* next = current->next;
            free(current);
            current = next;
        }
        adjList[i] = NULL;
    }
}

// int main() {
//     // Example: You'll want to change "test.c" to the file you want to analyze.
//     // Or, better, pass the filename as a command-line argument.
    
//     return 0;
// }