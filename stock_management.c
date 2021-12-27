#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

/*Used to initially open the database for the rest of the program, will create the db file if the file does not exist*/
sqlite3 *initialiseDatabase(){

    sqlite3 *db;
    char *errMsg = 0;

    int rc = sqlite3_open("stock_data.db", &db);
    if (rc != SQLITE_OK) {
        printf("\n\nThe database has not been initialised successfully\n\n");
        printf("\n%s\n", sqlite3_errmsg(db));
    } else {
        printf("\n\nThe database has been initialised successfully\n\n");
        return db;
    }
}

/*Closes the database when the function is called*/
void closeDB(sqlite3 *db){

    sqlite3_close(db);
}

/*Creates the tables for the database*/
void createTable(sqlite3 *db){

    /*Product table to hold the product productID, name, price and quantity*/
    char *errMsg = 0;
    char *data = "CREATE TABLE IF NOT EXISTS PRODUCT(productID INTEGER PRIMARY KEY, name TEXT, price REAL, quantity REAL);";

    int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {

        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    }
    
    /*Product Category table to link the productID to the categoryID*/
    errMsg = 0;
    char productCatData[100] = "CREATE TABLE IF NOT EXISTS PRODUCT_CAT(productID INTEGER, categoryID INTEGER);";
    data = productCatData;
    
    rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    }

    /*Category table to link the categoryID to the category name*/
    errMsg = 0;
    char catData[100] = "CREATE TABLE IF NOT EXISTS CATEGORY(categoryID INTEGER PRIMARY KEY, name TEXT);";
    data = catData;

    rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    }
    

    printf("\nTables configured successfully\n");
}

int categoryCallback(void *unused, int numOfCols, char **fields, char **colNames){

    int i;

    char array[7][20];

    for(i=0; i<numOfCols; i++){

       if(fields[i] != NULL){
           printf("%s   ", fields[i]);
       }
    }
    printf("\n");

    return 0;
}

int showCategories(sqlite3 *db){

    printf("Category options\n");

    char *errMsg = 0;

    char *query = "SELECT name FROM CATEGORY";

    int rc = sqlite3_exec(db, query, categoryCallback, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    return 0;
}

int getCategoryID(sqlite3 *db, char *categoryName){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char *query = "SELECT * FROM CATEGORY";

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    } else {

        int done = 0;
        int categoryID = 99;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){
                if(strcmp(categoryName, sqlite3_column_text(res, 1)) == 0){
                    categoryID = sqlite3_column_int(res, 0);
                }
            } else {

                done = 1;
            }
        }

        sqlite3_finalize(res);

        return categoryID;


    }


}

int addStock(sqlite3 *db){

    char name[20];
    char category[20];
    int categoryID;
    double price;
    double quantity;

    int input = 0;
    
    printf("Please enter the name of the product  ");
    fgets(name, 20, stdin);

    showCategories(db);

    do{
        printf("Please enter the category of the product  ");
        fgets(category, 20, stdin);
        category[strcspn(category, "\n")] = 0;
        categoryID = getCategoryID(db, category);
    } while((categoryID > 5) || (categoryID < 0));
    

    do{
        printf("Please enter the price for the %s", name);
        input = scanf("%lf", &price);
        while(getchar() != '\n'){
            getchar();
        }
    } while (input != 1);

    do{
        printf("Please enter the quantity of %s", name);
        input = scanf("%lf", &quantity);
        while(getchar() != '\n'){
            getchar();
        }
    } while (input != 1);

    printf("\nYou have chosen to add %g %s at a price of %g under the category %s\n", quantity, name, price, category);
}

int clearCategories(sqlite3* db){

    char *errMsg = 0;
    char *query = "DELETE FROM CATEGORY";

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    return 0;


}

/*Function to add all the categories to the categories table along with their categoryID*/
int setCategories(sqlite3* db){

    clearCategories(db);

    char *filename = "categories.txt";
    FILE *file = fopen(filename, "r");

    if(file == NULL){
        printf("Category file could not be found\n");
        return 1;
    }

    char buffer[256];
    char *errMsg = 0;
    int count = 0;

    while(fgets(buffer, 256, file)){
        buffer[strcspn(buffer, "\n")] = 0;
        

        char data[512];
        sprintf(data, "INSERT INTO CATEGORY VALUES('%d', '%s')", count, buffer);

        int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

        if(rc != SQLITE_OK){
            printf("\n%s\n", sqlite3_errmsg(db));
            sqlite3_free(errMsg);
        }

        count += 1;
        
    }

    return 0;
}

/*Main function which displays the menu that the user can use the navigate through the program*/
void main(){

    sqlite3 *initialisation = initialiseDatabase();
    createTable(initialisation);
    setCategories(initialisation);
    
    /*
    char categoryName[] = "Healths";
    int categoryID;

    categoryID = getCategoryID(initialisation, categoryName);
    if((categoryID > 5) || (categoryID < 0)){

        printf("Incorrect category name ");
    }
    */
    
    bool exited = false;

    while(!exited){
        printf("\nWelcome to the stock management program\n");
        printf("---------------------------------------\n");
        printf("\n\nMain Menu\n\n");
        printf("1.  Add Stock\n");
        printf("2.  Track Stock by Name\n");
        printf("3.  Track Stock by Category\n");
        printf("4.  Modify Stock\n");
        printf("5.  View Entire Stock\n");
        printf("6.  Exit the Program\n");

        printf("\n\nPlease choose the number for your perferred action: ");
        int userInput;
        scanf("%d", &userInput);
        getchar();

        printf("You have selected %d\n", userInput);

        switch(userInput){
            case 1:
                printf("Add Stock\n");
                printf("----------------------------------\n");
                addStock(initialisation);
                break;
            case 2:
                printf("Track Stock by Name\n");
                printf("----------------------------------\n");
                break;
            case 3:
                printf("Track Stock by Category\n");
                printf("----------------------------------\n");
                break;
            case 4:
                printf("Modify Stock\n");
                printf("----------------------------------\n");
                break;
            case 5:
                printf("View Entire Stock\n");
                printf("----------------------------------\n");
                break;
            case 6:
                printf("Exit the Program\n");
                printf("----------------------------------\n");
                closeDB(initialisation);
                exit(0);
                break;
        }
    }
}


