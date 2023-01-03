#include "project.h"
#include <stdio.h>
#include "stdlib.h"
#include "string.h"
#include <ctype.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Pre-defined macros for handling errors inside functions
#define MEMORY_ERR              printf("Error. Memory allocation failed.\n")
#define LINE_ERR                printf("Error. Invalind line.\n")
#define DUPLICATE_ERR(name)     printf("Nation \"%s\" is already in the database.\n", name)
#define EMPTY_ERR               printf("Error. The given data base is empty.\n")
#define NOT_FOUND_ERR(name)     printf("Nation \"%s\" is not in the database.\n", name)

// Pre-defined macros for handling user input errors inside the main function
#define WRITE_ERR(name)         printf("Cannot open file %s for writing.\n", name)
#define READ_ERR(name)          printf("Cannot open file %s for reading.\n", name)
#define A_FORMAT_ERR            printf("A should be followed by exactly 1 argument.\n")
#define M_FORMAT_ERR            printf("M should be followed by exactly 4 arguments.\n")
#define Q_FORMAT_ERR            printf("Q should not be followed by any arguments.\n")
#define W_FORMAT_ERR            printf("W should be followed by exactly 1 argument.\n")
#define O_FORMAT_ERR            printf("O should be followed by exactly 1 argument.\n")
#define COMMAND_ERR(command)    printf("Invalid command %c\n", command)



/**
 * @brief Compares two countries according to their medals in descending order
 * 
 * @param a Country 1
 * @param b Country 2
 * @return 1 if b is before a and -1 if vice versa
 */
int compareFunc(const void *a, const void *b) {
    if (((struct country*)a)->gold     < ((struct country*)b)->gold   ) return  1;
    if (((struct country*)a)->gold     > ((struct country*)b)->gold   ) return -1;
    if (((struct country*)a)->silver   < ((struct country*)b)->silver ) return  1;
    if (((struct country*)a)->silver   > ((struct country*)b)->silver ) return -1;
    if (((struct country*)a)->bronze   < ((struct country*)b)->bronze ) return  1;
    if (((struct country*)a)->bronze   > ((struct country*)b)->bronze ) return -1;
    return -1;
}



/**
 * @brief Frees all the allocated memory of a given database
 * 
 * @param database The given database
 */
void freeDatabase(struct country *database) {
    struct country *current;
    struct country *previous;
    
    current = database;
    while (current) {
        previous = current;
        current = current->next;
        free(previous->name);
        free(previous);
    }
}



/**
 * @brief Adds a dynamically allocated nation to the given database
 * 
 * @param args Arguments of the command (name of country)
 * @param database The given database
 * @return Returns a pointer to the new country or a NULL pointer if an error is encountered
 */
struct country *addNation(char *args, struct country *database) {
    struct country *newCountry;
    char *countryName;
    int nameLen;
    
    // Allocate space
    nameLen = (int)strlen(args);
    countryName = (char*)calloc(nameLen + 1, sizeof(char));

    // If allocation fails, free and return
    if (!countryName) {
        MEMORY_ERR;
        free(countryName);
        return NULL;
    }

    // Copy country name
    strcpy(countryName, args);

    // Check duplicates if database is initialized
    if (database) {
        struct country *head = database;
        while (head) {
            // If duplicate found, free and return
            if (!strcmp(head->name, countryName)) {
                DUPLICATE_ERR(countryName);
                free(countryName);
                return 0;
            }
            head = head->next;
        }
    }

    // Create new country
    newCountry = (struct country*)malloc(sizeof(struct country));
    if (!newCountry) {
        MEMORY_ERR;
        free(countryName), free(newCountry);
        return 0;
    }
    // Initialize new country
    *newCountry = (struct country) { countryName, 0, 0, 0, NULL };

    return newCountry;
}



/**
 * @brief Modifies the medals of a given country in the database. The medals can only have positive integer values.
 * 
 * @param args Arguments of the command (name of country)
 * @param medals Medals to be added/reduced
 * @param database The given database
 * @return 1 if successful, 0 if an error is encountered
 */
int modifyMedals(char *args, int medals[3], struct country *database) {
    struct country *current;

    // Find the corresponding country
    current = database;
    while (current) {
        if (!strcmp(current->name, args)) {
            current->gold   = MAX(0, current->gold   + medals[0]);
            current->silver = MAX(0, current->silver + medals[1]);
            current->bronze = MAX(0, current->bronze + medals[2]);
            return 1;
        }
        current = current->next;
    }

    // If country name not found in database
    NOT_FOUND_ERR(args);
    return 0;
}



/**
 * @brief Writes the countries in the given database to output
 * 
 * @param database The given database
 * @param output The given output
 * @return Returns 1 if successful, 0 otherwise
 */
int writeDatabase(struct country *database, FILE *output) {
    int count;
    struct country *current;

    if (!database) {
        EMPTY_ERR;
        return 0;
    }

    // Count the countries in dataBase
    current = database;
    for (count = 1; current->next; count ++) {
        current = current->next;
    }

    // Save the dataBase countries in an array
    current = database;
    struct country arr[count];
    for (int i = 0; i < count; i ++) {
        arr[i] = *current;
        current = current->next;
    }

    // Sort the array using compareFunc
    qsort(arr, count, sizeof(struct country), compareFunc);

    // Write the countries to output in appropriate format
    for (int i = 0; i < count; i ++) {
        fprintf(output, "%s %d %d %d\n", arr[i].name, arr[i].gold, arr[i].silver, arr[i].bronze);
    }

    return 1;
}



/**
 * @brief Loads a database from a text file and returns a pointer to the beginning of the database
 * 
 * @param input The file to be read
 * @return A pointer to the beginning of the database, a NULL pointer if an error is encountered
 */
struct country *loadDatabase(FILE *input) {
    char line[1000];
    char countryName[1000];
    int medals[3];
    int count = 0;
    struct country *tail;
    struct country *newCountry;
    struct country *database = NULL;

    // Count newlines
    while (!feof(input)) {
        if (fgetc(input) == '\n') count ++;
    }

    // Rewind file ptr
    rewind(input);

    // Read lines
    for (int i = 0; i < count; i ++) {
        fgets(line, 1000, input);
        // If sscanf doesn't scan 4 values, skip line and print invalid line error
        if (sscanf(line, "%s %d %d %d\n", countryName, &medals[0], &medals[1], &medals[2]) == 4) {
            newCountry = addNation(countryName, database);

            if (database) {
                tail->next = newCountry; 
            } else {
                database = newCountry;
            }

            tail = newCountry;
            modifyMedals(countryName, medals, database);
        } else {
            LINE_ERR;
        }

    }
    return database;
}



/**
 * @brief This program assumes that the user inputs at most the exact number of arguments for the command. 
 * If the user inputs too few or faulty arguments, the program prints an appropriate error message and ignores 
 * the given command. However, if the user inputs too many arguments, the program executes the command and ignores 
 * the excess arguments without printing an error message.
 * 
 * If the program is asked to load a file with invalid lines, it prints an error message for every faulty line, but 
 * proceeds to load the valid lines.
 */

int main(void) {
    FILE *f; 
    char userInput[1000]; // Max length of a single line is 1000
    char identifier;
    char args[1000];
    int medals[3];
    int argNum; // Number of successfully scanned arguments inputted by the user

    struct country* database = NULL; // Pointer to the first member if database
    struct country* tail = NULL; // Pointer to the last member of database

    // Return values
    int retVal = 0;
    struct country *retPtr = NULL;
    
    while (1) {
        if (fgets(userInput, 1000, stdin)) {
            // Scan userInput
            argNum = sscanf(userInput, "%c %s %d %d %d\n", &identifier, args, &medals[0], &medals[1], &medals[2]);

            // Check identifier and call the corresponding function
            switch (identifier) 
            {
            case 'Q':
                // Check that there are no excess arguments
                if (argNum == 1) {
                    freeDatabase(database);
                    printf("SUCCESS\n");
                    return 0; 
                } else {
                    Q_FORMAT_ERR;
                }
                break;
            case 'A':
                // Check that all necessary arguments are read
                if (argNum == 2) {
                    retPtr = addNation(args, database);

                    // Check the returned pointer and set the tail ptr
                    if (retPtr) {
                        // Check if database is initialized
                        if (database) {
                            tail->next = retPtr; 
                        } else {
                            database = retPtr; 
                        }
                        tail = retPtr;
                    }
                } else {
                    A_FORMAT_ERR;
                }
                break;
            case 'L':
                retVal = writeDatabase(database, stdout);
                break;
            case 'W':
                if (argNum == 2) {
                    f = fopen(args, "w");

                    // Check if file can be opened
                    if (f) {
                        retVal = writeDatabase(database, f);
                        fclose(f);
                    } else {
                        WRITE_ERR(args);
                    }
                } else {
                    W_FORMAT_ERR;
                }
                break;
            case 'O':
                if (argNum == 2) {
                    f = fopen(args, "r");
                    
                    // Check if file can be opened
                    if (f) {
                        // Free previous data
                        freeDatabase(database);
                        retPtr = loadDatabase(f);
                        database = retPtr;
                        
                        // Set new tail
                        tail = database;
                        while (tail->next) {
                            tail = tail->next;
                        }

                        fclose(f);
                    } else {
                        READ_ERR(args);
                    }
                } else {
                    O_FORMAT_ERR;
                }
                break;
            case 'M':
                // Check that all necessary arguments are read
                if (argNum == 5) {
                    retVal = modifyMedals(args, medals, database);
                } else {
                    M_FORMAT_ERR;
                }
                break;
            default:
                // If incorrect identifier, print command error
                COMMAND_ERR(identifier);
                continue;
            }
            // If valid command
            if (retVal || retPtr) printf("SUCCESS\n");

            // Reset return values
            retVal = 0, retPtr = NULL;
        } 
    }
}
