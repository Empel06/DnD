#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structuur voor een individueel uitrustingsstuk
typedef struct {
    char *name;
    char *index;
    double weight;
    char *url;
    int quantity;
    char *description;
    char *cost;
} Equipment;

// Structuur voor de volledige inventaris
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

// Functie om geheugen toe te wijzen en een string te kopiëren
char *copyString(const char *source) {
    if (source == NULL) return NULL;

    size_t length = strlen(source);
    char *destination = malloc(length + 1);
    if (destination == NULL) {
        fprintf(stderr, "Geheugenallocatie mislukt.\n");
        exit(1);
    }
    strcpy(destination, source);
    return destination;
}

// Functie om acties naar het geschiedenisbestand te schrijven
void writeHistory(const char *action, const char *itemName) {
    FILE *file = fopen("history.txt", "a"); // Open het geschiedenisbestand om aan het einde toe te voegen
    if (file == NULL) {
        printf("Fout bij het openen van het geschiedenisbestand.\n");
        return;
    }

    // Schrijf de actie en het itemnaam naar het geschiedenisbestand
    fprintf(file, "Actie: %s\n", action);
    fprintf(file, "Item: %s\n", itemName);
    fprintf(file, "\n");

    fclose(file); // Sluit het bestand
}

// Functie om een uitrustingsstuk te gebruiken
void useItem(Equipment *item) {
    printf("Gebruik item: %s\n", item->name);
    writeHistory("Gebruik", item->name); // Voeg deze regel toe om het gebruik van een item vast te leggen
}

// Functie om uitrustingsstukken uit een JSON-bestand te parseren
void parseEquipmentFile(const char *filename, Inventory *inventory) {
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

    // Voeg item toe aan inventaris
    inventory->items = realloc(inventory->items, (inventory->itemCount + 1) * sizeof(Equipment));
    inventory->items[inventory->itemCount] = item;
    inventory->itemCount++;
    inventory->currentWeight += item.weight;

    fclose(file);
}

// Functie om de inventaris af te drukken
void printInventory(const Inventory *inventory) {
    double totalWeight = 0.0;
    double totalValue = 0.0;

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

        // Bereken totale waarde en gewicht
        totalValue += atof(inventory->items[i].cost);
        totalWeight += inventory->items[i].weight;
    }
    printf("Total Weight: %.2f\n", totalWeight);
    printf("Total Value: %.2f\n", totalValue);
    printf("Max Weight: %.2f\n", inventory->maxWeight);
    printf("Coins: %dc %ds %de %dg %dp\n", inventory->copper, inventory->silver, inventory->electrum, inventory->gold, inventory->platinum);
    printf("Camp File: %s\n", inventory->campFile);
}


// Functie om de status van de speler te tonen
void printPlayerStatus(const Inventory *inventory) {
    double weightRatio = inventory->currentWeight / inventory->maxWeight;
    if (weightRatio >= 1.0) {
        printf("Speler is encumbered.\n");
    } else {
        printf("Speler is niet encumbered.\n");
    }
}

// Functie om interactief de inventaris te beheren
void manageInventory(Inventory *inventory) {
    char choice[10];
    Equipment *current = inventory->items;

    while (1) {
        printf("next, previous, use, camp, status of exit: ");
        int success = scanf("%s", choice);
        if (success != 1) {
            printf("Fout bij het lezen van de invoer.\n");
            // Voer hier eventueel extra foutafhandeling uit
            continue; // Ga door naar de volgende iteratie van de lus
        }

        if (strcmp(choice, "next") == 0) {
            if (current < &inventory->items[inventory->itemCount - 1]) {
                current++;
            } else {
                current = inventory->items;
            }
        } else if (strcmp(choice, "previous") == 0) {
            if (current > inventory->items) {
                current--;
            } else {
                current = &inventory->items[inventory->itemCount - 1];
            }
        } else if (strcmp(choice, "use") == 0) {
            useItem(current);
        } else if (strcmp(choice, "camp") == 0) {
            printf("Verplaats item naar het kamp: %s\n", current->name);
            // Voeg hier de logica toe om het item naar het kamp te verplaatsen
            // Schrijf het verplaatste item naar de kampfile
            FILE *campFile = fopen(inventory->campFile, "a");
            if (campFile == NULL) {
                fprintf(stderr, "Error: Failed to open camp file: %s\n", inventory->campFile);
            } else {
                fprintf(campFile, "Item: %s\n", current->name);
                fprintf(campFile, "Description: %s\n", current->description);
                fprintf(campFile, "Weight: %.2f\n", current->weight);
                fclose(campFile);
                printf("Item is met succes verplaatst naar de camp file.\n");
            }
        } else if (strcmp(choice, "status") == 0) {
            printInventory(inventory); // Print de status van de speler
            printPlayerStatus(inventory); // Toon de status van de speler
        } else if (strcmp(choice, "exit") == 0) {
            printf("Beëindig inventarisbeheer.\n");
            break;
        } else {
            printf("Ongeldige keuze. Probeer opnieuw.\n");
        }
    }
}


// Functie om de CLI-argumenten te parseren
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

int main(int argc, char *argv[]) {
    Inventory inventory = {0};

    // Parse CLI arguments
    parseCLIArguments(argc, argv, &inventory);

    // Print the inventory
    printInventory(&inventory);

    // Manage the inventory interactively
     manageInventory(&inventory);

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
