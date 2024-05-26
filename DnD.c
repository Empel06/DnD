#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct for an individual equipment item
typedef struct {
    char *name;
    char *index;
    double weight;
    char *url;
    int quantity;
    char *description;
    char *cost;
} Equipment;

// Struct for the entire inventory
typedef struct {
    Equipment *items;
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

// Function to parse equipment from a JSON file
void parseEquipmentFile(const char *filename, Inventory *inventory) {
    if (strcmp(filename, "backpack.json") == 0) {
        // Manually add data for backpack
        Equipment backpack = {
            .name = copyString("Backpack"),
            .index = copyString("backpack"),
            .weight = 5.0,
            .url = copyString("/api/equipment/backpack"),
            .quantity = 1,
            .description = copyString("Standard backpack."),
            .cost = copyString("2gp")
        };

        // Add backpack to inventory
        inventory->items = realloc(inventory->items, (inventory->itemCount + 1) * sizeof(Equipment));
        inventory->items[inventory->itemCount] = backpack;
        inventory->itemCount++;
        inventory->currentWeight += backpack.weight;

        return;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error: Failed to open file: %s\n", filename);
        return;
    }

    char buffer[1024];
    size_t len = sizeof(buffer);

    Equipment item = {0};

    char *token;
    while (fgets(buffer, len, file)) {
        token = strtok(buffer, "{\":,}\n\t");
        while (token != NULL) {
            if (strcmp(token, "name") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.name = copyString(token);
            } else if (strcmp(token, "index") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.index = copyString(token);
            } else if (strcmp(token, "weight") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.weight = atof(token);
            } else if (strcmp(token, "url") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.url = copyString(token);
            } else if (strcmp(token, "description") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.description = copyString(token);
            } else if (strcmp(token, "cost") == 0) {
                token = strtok(NULL, "{\":,}\n\t");
                item.cost = copyString(token);
            }
            token = strtok(NULL, "{\":,}\n\t");
        }
    }

    // Add item to inventory
    inventory->items = realloc(inventory->items, (inventory->itemCount + 1) * sizeof(Equipment));
    inventory->items[inventory->itemCount] = item;
    inventory->itemCount++;
    inventory->currentWeight += item.weight;

    fclose(file);
}

// Function to parse the CLI arguments
void parseCLIArguments(int argc, char *argv[], Inventory *inventory) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0 && i + 1 < argc) {
            inventory->maxWeight = atof(argv[++i]);
        } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
            // Parse money
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
            // Assume the argument is an equipment file
            parseEquipmentFile(argv[i], inventory);
        }
    }
}

// Function to print the inventory
void printInventory(const Inventory *inventory) {
    printf("Inventory:\n");
    for (int i = 0; i < inventory->itemCount; i++) {
        printf("Item %d:\n", i + 1);
        printf("  Name: %s\n", inventory->items[i].name);
        printf("  Index: %s\n", inventory->items[i].index);
        printf("  Weight: %.2f\n", inventory->items[i].weight);
        printf("  URL: %s\n", inventory->items[i].url);
        printf("  Quantity: %d\n", inventory->items[i].quantity);
        printf("  Description: %s\n", inventory->items[i].description);
        printf("  Cost: %s\n", inventory->items[i].cost);
    }
    printf("Total Weight: %.2f\n", inventory->currentWeight);
    printf("Max Weight: %.2f\n", inventory->maxWeight);
    printf("Coins: %dc %ds %de %dg %dp\n", inventory->copper, inventory->silver, inventory->electrum, inventory->gold, inventory->platinum);
    printf("Camp File: %s\n", inventory->campFile);
}

int main(int argc, char *argv[]) {
    Inventory inventory = {0};

    // Parse CLI arguments
    parseCLIArguments(argc, argv, &inventory);

    // Print the inventory
    printInventory(&inventory);

    // Free allocated memory
    for (int i = 0; i < inventory.itemCount; i++) {
        free(inventory.items[i].name);
        free(inventory.items[i].index);
        free(inventory.items[i].url);
        free(inventory.items[i].description);
        free(inventory.items[i].cost);
    }
    free(inventory.items);
    free(inventory.campFile);

    return 0;
}
