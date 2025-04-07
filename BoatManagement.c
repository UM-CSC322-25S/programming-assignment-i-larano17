#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOATS 120
#define MAX_NAME 128
#define MAX_LINE 256

// Enum for boat location
typedef enum {
    SLIP,
    LAND,
    TRAILOR,
    STORAGE
} PlaceType;

// Union to store extra info.
typedef union {
    int slipNumber;           
    char bayLetter;           
    char trailorTag[20];     
    int storageNumber;        
} ExtraInfo;


typedef struct {
    char name[MAX_NAME];      
    int length;              
    PlaceType type;           
    ExtraInfo extra;          
    double amountOwed;        
} Boat;

// Array of pointers
Boat *boats[MAX_BOATS];
int boatCount = 0;

// Remove newline from a string
void trimNewline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len-1] == '\n') {
        str[len-1] = '\0';
    }
}

// Convert string to PlaceType
PlaceType getPlaceType(const char *typeStr) {
    if (strcasecmp(typeStr, "slip") == 0) {
        return SLIP;
    } else if (strcasecmp(typeStr, "land") == 0) {
        return LAND;
    } else if (strcasecmp(typeStr, "trailor") == 0) {
        return TRAILOR;
    } else if (strcasecmp(typeStr, "storage") == 0) {
        return STORAGE;
    }
    return SLIP;
}

// Return the monthly rate
double getMonthlyRate(PlaceType type) {
    switch (type) {
        case SLIP: return 12.50;
        case LAND: return 14.00;
        case TRAILOR: return 25.00;
        case STORAGE: return 11.20;
        default: return 0.0;
    }
}

// Create a Boat from a CSV line 
Boat *createBoatFromCSV(const char *csvLine) {
    Boat *newBoat = malloc(sizeof(Boat));
    if (!newBoat) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }
    char lineCopy[MAX_LINE];
    strncpy(lineCopy, csvLine, MAX_LINE);
    lineCopy[MAX_LINE - 1] = '\0';
    
    char *token = strtok(lineCopy, ",");
    if (!token) return NULL;
    strncpy(newBoat->name, token, MAX_NAME);
    newBoat->name[MAX_NAME - 1] = '\0';

    // Parse length
    token = strtok(NULL, ",");
    if (!token) { free(newBoat); return NULL; }
    newBoat->length = atoi(token);

    // Parse boat type
    token = strtok(NULL, ",");
    if (!token) { free(newBoat); return NULL; }
    newBoat->type = getPlaceType(token);

    // Parse extra info.
    token = strtok(NULL, ",");
    if (!token) { free(newBoat); return NULL; }
    switch (newBoat->type) {
        case SLIP:
            newBoat->extra.slipNumber = atoi(token);
            break;
        case LAND:
            newBoat->extra.bayLetter = token[0];  // first character of the bay letter
            break;
        case TRAILOR:
            strncpy(newBoat->extra.trailorTag, token, sizeof(newBoat->extra.trailorTag));
            newBoat->extra.trailorTag[sizeof(newBoat->extra.trailorTag) - 1] = '\0';
            break;
        case STORAGE:
            newBoat->extra.storageNumber = atoi(token);
            break;
    }

    // Parse
    token = strtok(NULL, ",");
    if (!token) { free(newBoat); return NULL; }
    newBoat->amountOwed = atof(token);

    return newBoat;
}

// Load boat data from CSV
void loadBoatData(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf(stderr, "Could not open file %s for reading.\n", filename);
        exit(1);
    }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        trimNewline(line);
        if (strlen(line) == 0) continue;
        Boat *boat = createBoatFromCSV(line);
        if (boat && boatCount < MAX_BOATS) {
            boats[boatCount++] = boat;
        }
    }
    fclose(fp);
}


void saveBoatData(const char *filename) {
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Could not open file %s for writing.\n", filename);
        exit(1);
    }
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        // Write in CSV format
        switch (b->type) {
            case SLIP:
                fprintf(fp, "%s,%d,slip,%d,%.2f\n", b->name, b->length, b->extra.slipNumber, b->amountOwed);
                break;
            case LAND:
                fprintf(fp, "%s,%d,land,%c,%.2f\n", b->name, b->length, b->extra.bayLetter, b->amountOwed);
                break;
            case TRAILOR:
                fprintf(fp, "%s,%d,trailor,%s,%.2f\n", b->name, b->length, b->extra.trailorTag, b->amountOwed);
                break;
            case STORAGE:
                fprintf(fp, "%s,%d,storage,%d,%.2f\n", b->name, b->length, b->extra.storageNumber, b->amountOwed);
                break;
        }
    }
    fclose(fp);
}

// Comparator function for sorting boats alphabetically
int compareBoats(const void *a, const void *b) {
    Boat * const *boatA = a;
    Boat * const *boatB = b;
    return strcasecmp((*boatA)->name, (*boatB)->name);
}

void printInventory() {
    // Sort the array
    qsort(boats, boatCount, sizeof(Boat *), compareBoats);
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        // Print according to type
        switch (b->type) {
            case SLIP:
                printf("%-20s %2d'    slip   # %d   Owes $%7.2f\n", 
                       b->name, b->length, b->extra.slipNumber, b->amountOwed);
                break;
            case LAND:
                printf("%-20s %2d'    land      %c   Owes $%7.2f\n", 
                       b->name, b->length, b->extra.bayLetter, b->amountOwed);
                break;
            case TRAILOR:
                printf("%-20s %2d' trailor %s   Owes $%7.2f\n", 
                       b->name, b->length, b->extra.trailorTag, b->amountOwed);
                break;
            case STORAGE:
                printf("%-20s %2d' storage   # %d   Owes $%7.2f\n", 
                       b->name, b->length, b->extra.storageNumber, b->amountOwed);
                break;
        }
    }
}

// Find a boat index by name
int findBoatIndexByName(const char *name) {
    for (int i = 0; i < boatCount; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Add a new boat by reading CSV
void addBoat() {
    char input[MAX_LINE];
    printf("Please enter the boat data in CSV format                 : ");
    if (!fgets(input, sizeof(input), stdin))
        return;
    trimNewline(input);
    Boat *boat = createBoatFromCSV(input);
    if (boat) {
        if (boatCount < MAX_BOATS) {
            boats[boatCount++] = boat;
        } else {
            printf("Boat inventory full. Cannot add more boats.\n");
            free(boat);
        }
    }
}

// Remove name
void removeBoat() {
    char name[MAX_NAME];
    printf("Please enter the boat name                               : ");
    if (!fgets(name, sizeof(name), stdin))
        return;
    trimNewline(name);
    int index = findBoatIndexByName(name);
    if (index == -1) {
        printf("No boat with that name\n");
        return;
    }
    free(boats[index]);
    // Pack the array by shifting remaining boats
    for (int i = index; i < boatCount - 1; i++) {
        boats[i] = boats[i+1];
    }
    boatCount--;
}

// Process a payment for a boat
void processPayment() {
    char name[MAX_NAME];
    char input[MAX_LINE];
    printf("Please enter the boat name                               : ");
    if (!fgets(name, sizeof(name), stdin))
        return;
    trimNewline(name);
    int index = findBoatIndexByName(name);
    if (index == -1) {
        printf("No boat with that name\n");
        return;
    }
    Boat *b = boats[index];
    printf("Please enter the amount to be paid                       : ");
    if (!fgets(input, sizeof(input), stdin))
        return;
    double payment = atof(input);
    if (payment > b->amountOwed) {
        printf("That is more than the amount owed, $%.2f\n", b->amountOwed);
    } else {
        b->amountOwed -= payment;
    }
}

// Change the monthly charges
void updateMonthlyCharges() {
    for (int i = 0; i < boatCount; i++) {
        Boat *b = boats[i];
        double rate = getMonthlyRate(b->type);
        b->amountOwed += rate * b->length;
    }
}

// Show the menu options
void displayMenu() {
    printf("\n(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s BoatData.csv\n", argv[0]);
        return 1;
    }
    loadBoatData(argv[1]);
    printf("Welcome to the Boat Management System\n");
    printf("-------------------------------------\n");

    char choice[MAX_LINE];
    int exitFlag = 0;
    while (!exitFlag) {
        displayMenu();
        if (!fgets(choice, sizeof(choice), stdin))
            break;
        char option = tolower(choice[0]);
        switch (option) {
            case 'i':
                printInventory();
                break;
            case 'a':
                addBoat();
                break;
            case 'r':
                removeBoat();
                break;
            case 'p':
                processPayment();
                break;
            case 'm':
                updateMonthlyCharges();
                break;
            case 'x':
                exitFlag = 1;
                break;
            default:
                printf("Invalid option %c\n", choice[0]);
                break;
        }
    }
    saveBoatData(argv[1]);
    printf("\nExiting the Boat Management System\n");
    return 0;
}

