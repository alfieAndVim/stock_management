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

int getLastID(sqlite3 *db){

    char *errMsg = 0;
    sqlite3_stmt *res;
    char *query = "SELECT productID FROM PRODUCT";

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
        
        return -1;

    } else {
        int done = 0;
        int lastID = -1;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){
                lastID = sqlite3_column_int(res, 0);
                printf("last id %d\n", lastID);
            } else {
                done = 1;
            }
        }

        return lastID;
    }

}

/*Prints all categories out to the user from the showCategories query function*/
int categoryCallback(void *unused, int numOfCols, char **fields, char **colNames){

    int i;

    for(i=0; i<numOfCols; i++){

       if(fields[i] != NULL){
           printf("%s   ", fields[i]);
       }
    }
    printf("\n");

    return 0;
}

/*Performs a query for all categories on the category table*/
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

/*Gets the category id associated with a specific category name*/
int getCategoryID(sqlite3 *db, char *categoryName){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char *query = "SELECT * FROM CATEGORY";

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 99;

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

struct product{

    char name[20];
    int productID;
    int categoryID;
    double price;
    double quantity;
};

char * getCategory(sqlite3 *db, int productID){

    char *category = malloc(sizeof(char) * 20);

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[130];

    sprintf(query, "SELECT CATEGORY.name FROM CATEGORY, PRODUCT_CAT WHERE CATEGORY.categoryID = PRODUCT_CAT.categoryID AND PRODUCT_CAT.productID = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    
    } else {

        sqlite3_bind_int(res, 1, productID);

        int done = 0;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){
                strcpy(category, sqlite3_column_text(res, 0));
            
            } else {
                done = 1;
            }
        }
    }

    return category;
}

void readStockByName(sqlite3 *db, char *name){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    sprintf(query, "SELECT DISTINCT PRODUCT.productID, PRODUCT.name, PRODUCT.quantity, PRODUCT.price, PRODUCT_CAT.categoryID FROM PRODUCT, PRODUCT_CAT WHERE PRODUCT.productID = PRODUCT_CAT.productID AND PRODUCT.name = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

    } else {

        sqlite3_bind_text(res, 1, name, -1, NULL);

        int done = 0;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){

                printf("%s  ", sqlite3_column_text(res, 0));
                printf("Name:   %s  ", sqlite3_column_text(res, 1));
                printf("Quantity:   %s  ", sqlite3_column_text(res, 2));
                printf("Price:  %s  ", sqlite3_column_text(res, 3));
                printf("Category:   %s  ", getCategory(db, sqlite3_column_int(res, 4)));
                printf("\n");
            
            } else {
                
                done = 1;
            }
        }
    }


}

void readStockByCategory(sqlite3 *db, char *category){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    int categoryID = getCategoryID(db, category);

    sprintf(query, "SELECT DISTINCT PRODUCT.productID, PRODUCT.name, PRODUCT.quantity, PRODUCT.price FROM PRODUCT, PRODUCT_CAT WHERE PRODUCT.productID = PRODUCT_CAT.productID AND PRODUCT_CAT.categoryID = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error:  %s\n", errMsg);
        sqlite3_free(errMsg);
    
    } else {

        sqlite3_bind_int(res, 1, categoryID);

        int done = 0;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){

                printf("%s  ", sqlite3_column_text(res, 0));
                printf("Name:   %s  ", sqlite3_column_text(res, 1));
                printf("Quantity:   %s  ", sqlite3_column_text(res, 2));
                printf("Price:  %s  ", sqlite3_column_text(res, 3));
                printf("Category:   %s  ", getCategory(db, sqlite3_column_int(res, 0)));
                printf("\n");
            
            } else {

                done = 1;
            }
        }
    } 

}

int readStockByID(sqlite3 *db, int id){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    sprintf(query, "SELECT DISTINCT PRODUCT.productID, PRODUCT.name, PRODUCT.quantity, PRODUCT.price, PRODUCT_CAT.categoryID FROM PRODUCT, PRODUCT_CAT WHERE PRODUCT.productID = PRODUCT_CAT.productID AND PRODUCT.productID = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

    } else {

        sqlite3_bind_int(res, 1, id);

        int done = 0;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){

                printf("%s  ", sqlite3_column_text(res, 0));
                printf("Name:   %s  ", sqlite3_column_text(res, 1));
                printf("Quantity:   %s  ", sqlite3_column_text(res, 2));
                printf("Price:  %s  ", sqlite3_column_text(res, 3));
                printf("Category:   %s  ", getCategory(db, sqlite3_column_int(res, 4)));
                printf("\n");
            
            } else {
                
                done = 1;
            }
        }
    }

    

}


int readAllStock(sqlite3 *db){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char *query = "SELECT * FROM PRODUCT";

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    
    } else {

        int done = 0;

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){
                printf("%d   ", sqlite3_column_int(res, 0));
                printf("Name %s   ", sqlite3_column_text(res, 1));
                printf("Price %s    ", sqlite3_column_text(res, 2));
                printf("Quantity %s    ", sqlite3_column_text(res, 3));
                printf("Category %s    ", getCategory(db, sqlite3_column_int(res, 0)));
                printf("\n");
            } else {
                done = 1;
            }
        }
    }



    return 0;
}

int changeProductName(sqlite3 *db, int id, char *name){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    sprintf(query, "UPDATE PRODUCT SET name = '%s' WHERE productID = %d", name, id);

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        
        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 1;
    
    } else {

        printf("Name has been changed successfully\n");
    }

    
}

int modifyStock(sqlite3 *db){

    printf("All stock items\n\n");
    readAllStock(db);

    int userChoiceID;

    int input = 0;
    do{
        printf("Please enter the number for the stock item you wish to modify\n");
        input = scanf("%d", &userChoiceID);
        while(getchar() != '\n'){
            getchar();
        }
        int lastId = getLastID(db);
        if((userChoiceID > lastId) || (userChoiceID < 0)){
            input = 0;
        }
    } while(input != 1);

    printf("You have chosen to modify:\n");

    readStockByID(db, userChoiceID);

    printf("1. Change the name\n");
    printf("2. Change the price\n");
    printf("3. Change the quantity\n");
    printf("4. Change the category\n");
    
    int userChoice;

    do{
        printf("Please select an option to proceed:     ");
        input = scanf("%d", &userChoice);
        while(getchar() != '\n'){
            getchar();
        }
        if((userChoice > 4) || (userChoice < 1)){
            input = 0;
        }
    } while(input != 1);

    char name[50];

    switch(userChoice){

        case 1:

            printf("You have selected to change the name\n");
            printf("Please give the name that you would like to change the product to:  ");
            fgets(name, 50, stdin);
            name[strcspn(name, "\n")] = 0;

            changeProductName(db, userChoiceID, name);

    } 
}

int insertData(sqlite3 *db, struct product tempProduct){

    printf("Name: %s, CategoryID: %d, Price: %g, Quantity: %g\n", tempProduct.name, tempProduct.categoryID, tempProduct.price, tempProduct.quantity);

    char name[20];
    int productID;
    int categoryID;
    double price;
    double quantity;

    strcpy(name, tempProduct.name);
    productID = tempProduct.productID;
    categoryID = tempProduct.categoryID;
    price = tempProduct.price;
    quantity = tempProduct.quantity;

    char *errMsg = 0;
    char data[128];

    /*Adding data to the product table*/
    sprintf(data, "INSERT INTO PRODUCT VALUES('%d', '%s', '%lf', '%lf')", productID, name, price, quantity);

    int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    } else {
        printf("Data added successfully\n");
    }

    /*Adding data to the productCat table*/
    char productCatData[128];

    sprintf(productCatData, "INSERT INTO PRODUCT_CAT VALUES('%d', '%d')", productID, categoryID);

    rc = sqlite3_exec(db, productCatData, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    } else {
        printf("Data added successfully\n");
    }

    return 0;
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
    } while(categoryID == 99);
    

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

    printf("The next id is %d\n", getLastID(db) + 1);

    struct product tempProduct;

    name[strcspn(name, "\n")] = 0;

    strcpy(tempProduct.name, name);
    tempProduct.productID = getLastID(db) + 1;
    tempProduct.categoryID = categoryID;
    tempProduct.price = price;
    tempProduct.quantity = quantity;

    insertData(db, tempProduct);

    return 0;


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
                readStockByName(initialisation, "eggs");
                break;
            case 3:
                printf("Track Stock by Category\n");
                printf("----------------------------------\n");
                readStockByCategory(initialisation, "Food");
                break;
            case 4:
                printf("Modify Stock\n");
                printf("----------------------------------\n");
                modifyStock(initialisation);
                break;
            case 5:
                printf("View Entire Stock\n");
                printf("----------------------------------\n");
                readAllStock(initialisation);
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


