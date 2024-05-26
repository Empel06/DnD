#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Struct for an individual equipment item
typedef struct Equipment {
    char *name;          
    char *index;         
    double weight;       
    char *url;           
    int quantity;        
    char *description;   
    char *cost;          
    struct Equipment *next;  // Pointer to the next equipment item in the linked list
} Equipment;

// Struct for the entire inventory
typedef struct {
    Equipment *head;      // Head of the linked list of equipment items
    int itemCount;        
    double maxWeight;     
    double currentWeight; 
    int copper;           
    int silver;           
    int electrum;         
    int gold;             
    int platinum;         
    char *campFile;       
} Inventory;

// Helper function to allocate memory and copy a string
char *copyString(const char *source) {
    if (source == NULL) return NULL;
    size_t length = strlen(source);
    char *destination = malloc(length + 1);
    if (destination == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }
    strcpy(destination, source);
    return destination;
}

// Function to create a new equipment item and add it to the inventory
void addEquipmentItem(Inventory *inventory, Equipment *newItem) {
    if (inventory->head == NULL) {
        inventory->head = newItem;
    } else {
        Equipment *current = inventory->head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newItem;
    }
    inventory->itemCount++;
    inventory->currentWeight += newItem->weight;
}

// Function to parse a JSON object from a string (basic implementation)
char* extractJSONString(char **json, const char *key) {
    char *start = strstr(*json, key);
    if (!start) return NULL;

    start = strchr(start, ':');
    if (!start) return NULL;
    start = strchr(start, '\"');
    if (!start) return NULL;
    start++;

    char *end = strchr(start, '\"');
    if (!end) return NULL;

    size_t length = end - start;
    char *value = malloc(length + 1);
    strncpy(value, start, length);
    value[length] = '\0';

    *json = end + 1;
    return value;
}

double extractJSONDouble(char **json, const char *key) {
    char *start = strstr(*json, key);
    if (!start) return 0;

    start = strchr(start, ':');
    if (!start) return 0;
    start++;

    double value = atof(start);
    char *end = strpbrk(start, ",}");
    *json = end ? end : start + strlen(start);
    return value;
}

int extractJSONInt(char **json, const char *key) {
    char *start = strstr(*json, key);
    if (!start) return 0;

    start = strchr(start, ':');
    if (!start) return 0;
    start++;

    int value = atoi(start);
    char *end = strpbrk(start, ",}");
    *json = end ? end : start + strlen(start);
    return value;
}

// Function to parse equipment from a JSON string
void parseEquipmentJSON(char *jsonString, Inventory *inventory) {
    char *cursor = jsonString;

    while ((cursor = strstr(cursor, "{")) != NULL) {
        Equipment *newItem = (Equipment *)malloc(sizeof(Equipment));
        newItem->name = extractJSONString(&cursor, "\"name\"");
        newItem->index = extractJSONString(&cursor, "\"index\"");
        newItem->weight = extractJSONDouble(&cursor, "\"weight\"");
        newItem->url = extractJSONString(&cursor, "\"url\"");
        newItem->quantity = extractJSONInt(&cursor, "\"quantity\"");
        newItem->description = extractJSONString(&cursor, "\"description\"");
        newItem->cost = extractJSONString(&cursor, "\"cost\"");
        newItem->next = NULL;

        addEquipmentItem(inventory, newItem);

        cursor = strstr(cursor, "}");
        if (!cursor) break;
        cursor++;
    }
}

// Function to parse equipment from a JSON file
void parseEquipmentFile(const char *filename, Inventory *inventory) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error: Failed to open file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *jsonString = (char *)malloc(fileSize + 1);
    if (fread(jsonString, 1, fileSize, file) != fileSize) {
        perror("Error: Failed to read file");
        free(jsonString);
        fclose(file);
        return;
    }
    jsonString[fileSize] = '\0';
    fclose(file);

    parseEquipmentJSON(jsonString, inventory);
    free(jsonString);
}

// Function to parse the CLI arguments
void parseCLIArguments(int argc, char *argv[], Inventory *inventory) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            inventory->maxWeight = atof(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            char *moneyStr = argv[++i];
            char *token = strtok(moneyStr, " ");
            while (token != NULL) {
                int amount = atoi(token);
                char *type = token + strcspn(token, "0123456789");
                switch (type[0]) {
                    case 'c': inventory->copper = amount; break;
                    case 's': inventory->silver = amount; break;
                    case 'e': inventory->electrum = amount; break;
                    case 'g': inventory->gold = amount; break;
                    case 'p': inventory->platinum = amount; break;
                }
                token = strtok(NULL, " ");
            }
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            inventory->campFile = copyString(argv[++i]);
        } else {
            parseEquipmentFile(argv[i], inventory);
        }
    }
}

// Function to print the inventory
void printInventory(const Inventory *inventory) {
    printf("Inventory:\n");
    Equipment *current = inventory->head;
    while (current != NULL) {
        printf("Item:\n");
        printf("  Name: %s\n", current->name);
        printf("  Index: %s\n", current->index);
        printf("  Weight: %.2f\n", current->weight);
        printf("  URL: %s\n", current->url);
        printf("  Quantity: %d\n", current->quantity);
        printf("  Description: %s\n", current->description);
        printf("  Cost: %s\n", current->cost);
        current = current->next;
    }
    printf("Total Weight: %.2f\n", inventory->currentWeight);
    printf("Max Weight: %.2f\n", inventory->maxWeight);
    printf("Coins: %dc %ds %de %dg %dp\n", inventory->copper, inventory->silver, inventory->electrum, inventory->gold, inventory->platinum);
    printf("Camp File: %s\n", inventory->campFile ? inventory->campFile : "None");
}

int main(int argc, char *argv[]) {
    Inventory inventory = {0};

    // Parse CLI arguments
    parseCLIArguments(argc, argv, &inventory);

    // Print the inventory
    printInventory(&inventory);

    // Free allocated memory
    Equipment *current = inventory.head;
    while (current != NULL) {
        Equipment *next = current->next;
        free(current->name);
        free(current->index);
        free(current->url);
        free(current->description);
        free(current->cost);
        free(current);
        current = next;
    }
    free(inventory.campFile);

    return 0;
}
