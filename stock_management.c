#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

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
int createTable(sqlite3 *db){

    /*Product table to hold the product productID, name, price and quantity*/
    char *errMsg = 0;
    char *data = "CREATE TABLE IF NOT EXISTS PRODUCT(productID INTEGER PRIMARY KEY, name TEXT, price REAL, quantity REAL);";

    int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {

        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);

        return 1;
    }
    
    /*Product Category table to link the productID to the categoryID*/
    errMsg = 0;
    char productCatData[100] = "CREATE TABLE IF NOT EXISTS PRODUCT_CAT(productID INTEGER, categoryID INTEGER);";
    data = productCatData;
    
    rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);

        return 1;
    }

    /*Category table to link the categoryID to the category name*/
    errMsg = 0;
    char catData[100] = "CREATE TABLE IF NOT EXISTS CATEGORY(categoryID INTEGER PRIMARY KEY, name TEXT);";
    data = catData;

    rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);

        return 1;
    }
    

    printf("\nTables configured successfully\n");

    return 0;
}


/*Fetches the last primary key for the Product table so only unique productID's will be added to the database*/
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
            } else {
                done = 1;
            }
        }

        return lastID;
    }

}

long int strToInt(char *string){

    long int temp;

    string[strcspn(string, "\n")] = 0;
    temp = strtol(string, NULL, 10);

    return temp;
}

int intCheck(char *string){

    int i;
    int success = 0;
    int length = strlen(string);

    for(i=0; i<length; i++){

        if(isdigit(string[i]) > 0){

            success = 1;
        } else {
            return 0;
        }
    }

    return success;
}

long double strToDbl(char *string){

    long double temp;

    string[strcspn(string, "\n")] = 0;
    temp = strtod(string, NULL);

    return temp;
}

int doubleCheck(char *string){

    int i;
    int success = 0;
    int length = strlen(string);

    int stopCount = 0;

    for(i=0; i<length; i++){

        if((isdigit(string[i]) > 0) || (string[i] == '.')){

            success = 1;
            if(string[i] == '.'){
                stopCount += 1;
            }

            if(stopCount > 1){
                return 0;
            }
        } else {
            return 0;
        }
    }

    return success;
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

/*Use of structs, used when adding data to the database so only a single data structure needs to be passed through as a parameter*/
struct product{

    char name[20];
    int productID;
    int categoryID;
    double price;
    double quantity;
};

/*Returns the category name of a product given its productID (primary key of product table)*/
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

/*Gives a list of all of the stock that match a specific name inputted by the user*/
int readStockByName(sqlite3 *db){


    char name[100];

    printf("Please enter the name you wish to search for:   ");
    fgets(name, 100, stdin);
    name[strcspn(name, "\n")] = 0;

    if(((strcmp(name, "q")) == 0) || ((strcmp(name, "Q")) == 0)){
            return 1;
        }

    printf("You have chosen to search for %s\n\n", name);

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

    return 0;

}

/*Gives a list of all of the stock which match a specific category inputted by the user*/
int readStockByCategory(sqlite3 *db){

    showCategories(db);

    char category[200];

    do{
        printf("Please enter the category you wish to search for:   ");
        fgets(category, 200, stdin);
        category[strcspn(category, "\n")] = 0;

        if(((strcmp(category, "q")) == 0) || ((strcmp(category, "Q")) == 0)){
            return 1;
        }

        if(getCategoryID(db, category) == 99){
            printf("Please make sure that you have chosen a listed category\n");
        } else {
            printf("You have chosen to search for %s\n\n", category);
        }
        
    } while(getCategoryID(db, category) == 99);
    
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

    return 0;

}

/*Gives a list of the individual stock item which matches an identifier*/
int readStockByID(sqlite3 *db, int id){

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    sprintf(query, "SELECT DISTINCT PRODUCT.productID, PRODUCT.name, PRODUCT.quantity, PRODUCT.price, PRODUCT_CAT.categoryID FROM PRODUCT, PRODUCT_CAT WHERE PRODUCT.productID = PRODUCT_CAT.productID AND PRODUCT.productID = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 1;

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
                printf("Category:   %s  ", getCategory(db, sqlite3_column_int(res, 0)));
                printf("\n");
            
            } else {
                
                done = 1;
            }
        }
    }

    return 0;

}

int checkStockByID(sqlite3 *db, int id){

    int count = 0;

    char *errMsg = 0;
    sqlite3_stmt *res;

    char query[300];

    sprintf(query, "SELECT DISTINCT PRODUCT.productID, PRODUCT.name, PRODUCT.quantity, PRODUCT.price, PRODUCT_CAT.categoryID FROM PRODUCT, PRODUCT_CAT WHERE PRODUCT.productID = PRODUCT_CAT.productID AND PRODUCT.productID = ?");

    int rc = sqlite3_prepare_v2(db, query, -1, &res, 0);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 99;

    } else {

        sqlite3_bind_int(res, 1, id);

        int done = 0;
        

        while(!done){

            int step = sqlite3_step(res);

            if(step == SQLITE_ROW){

                count += 1;
            
            } else {
                
                done = 1;
            }
        }
    }

    return count;
}

/*Gives a list of all of the stock which resides in the database*/
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

/*Function used to change the product name given the productID of the product*/
int changeProductName(sqlite3 *db, int id, char *name){

    char *errMsg = 0;

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

    return 0;
}

/*Function used to change the product price give the productID of the product*/
int changeProductPrice(sqlite3 *db, int id, double price){

    char *errMsg = 0;
    
    char query[300];

    sprintf(query, "UPDATE PRODUCT SET price = '%lf' WHERE productID = %d", price, id);

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 1;
    
    } else {

        printf("Price has been changed successfully\n");
    }

    return 0;
}

/*Function used to change the quantity of the stock item given the productID of the product*/
int changeProductQuantity(sqlite3 *db, int id, double quantity){

    char *errMsg = 0;

    char query[300];

    sprintf(query, "UPDATE PRODUCT SET quantity = '%lf' WHERE productId = %d", quantity, id);

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 1;
    
    } else {

        printf("Quantity has been changed successfully\n");

    }

    return 0;
}

/*Function used to change the category of the product given the productID of the product*/
int changeProductCategory(sqlite3 *db, int id, int categoryID){

    char *errMsg = 0;

    char query[300];

    sprintf(query, "UPDATE PRODUCT_CAT SET categoryID = %d WHERE productID = %d", categoryID, id);

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){

        printf("SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);

        return 1;
    
    } else {

        printf("Category has been changed sucessfully\n");
    }

    return 0;
}

int deleteStock(sqlite3 *db, int id){


    /*deleting from product table*/
    char *errMsg = 0;

    char query[300];

    int success = 0;

    sprintf(query, "DELETE FROM PRODUCT WHERE productID = %d", id);

    int rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){

        printf("SQL error:  %s\n", errMsg);
        sqlite3_free(errMsg);

        success = 0;

        return 1;
    
    } else {

        success = 1;
    }

    sprintf(query, "DELETE FROM PRODUCT_CAT WHERE productID = %d", id);

    rc = sqlite3_exec(db, query, 0, 0, &errMsg);

    if(rc != SQLITE_OK){

        printf("SQL error:  %s\n", errMsg);
        sqlite3_free(errMsg);

        success = 0;

        return 1;
    
    } else {

        success = 1;
    }

    if(success == 1){

        printf("Stock has been successfully deleted\n");
    }

    return 0;
}



/*Function that handles the user interaction when modifying an individual stock item*/
int modifyStock(sqlite3 *db){

    printf("All stock items\n\n");
    readAllStock(db);
    printf("\n");

    long int userChoiceID;
    char tempUserChoiceID[10];

    int input = 0;
    do{
        
        printf("Please enter the number for the stock item you wish to modify:  ");
        
        fgets(tempUserChoiceID, 10, stdin);
        tempUserChoiceID[strcspn(tempUserChoiceID, "\n")] = 0;
        if(!(intCheck(tempUserChoiceID))){
            input = 0;
        } else {
            input = 1;
            userChoiceID = strToInt(tempUserChoiceID);
            if((checkStockByID(db, userChoiceID) > 0) && (checkStockByID(db, userChoiceID)) != 99){
                input = 1;
            } else {
                input = 0;
            }
        }
        
        
    } while(input != 1);

    printf("You have chosen to modify:\n\n");

    readStockByID(db, userChoiceID);

    printf("\n1. Change the name\n");
    printf("2. Change the price\n");
    printf("3. Change the quantity\n");
    printf("4. Change the category\n");
    printf("5. Delete stock\n");
    
    long int userChoice;
    char tempUserChoice[10];

    input = 0;

    do{
        printf("\nPlease select an option to proceed:     ");
        
        fgets(tempUserChoice, 10, stdin);
        tempUserChoice[strcspn(tempUserChoice, "\n")] = 0;
        if(!(intCheck(tempUserChoice))){
            input = 0;
        } else {
            input = 1;
            userChoice = strToInt(tempUserChoice);
            if((userChoice > 5) || (userChoice < 1)){
                input = 0;
            }
        }
        
    } while(input != 1);

    char name[50];
    long double price;
    char tempPrice[10];
    long double quantity;
    char tempQuantity[10];
    char category[50];
    char delete[3];

    switch(userChoice){

        case 1:

            printf("You have selected to change the name\n\n");
            printf("Please give the name that you would like to change the product to:  ");
            fgets(name, 50, stdin);
            name[strcspn(name, "\n")] = 0;

            if(((strcmp(name, "q")) == 0) || ((strcmp(name, "Q")) == 0)){
                return 1;
            }

            changeProductName(db, userChoiceID, name);

            return 0;

        case 2:

            input = 0;

            printf("You have selected to change the price\n\n");

            do{
                printf("Please give the new total price of the stock product:   ");
                fgets(tempPrice, 10, stdin);
                tempPrice[strcspn(tempPrice, "\n")] = 0;
                if(!(doubleCheck(tempPrice))){
                    input = 0;
                
                } else {
                    price = strToDbl(tempPrice);
                    input = 1;
                }
            } while(input != 1);
            

            changeProductPrice(db, userChoiceID, price);

            return 0;

        case 3:

            input = 0;

            printf("You have selected to change the quantity\n\n");
            do{
                printf("Please give the new quantity of the stock product:  ");
                fgets(tempQuantity, 10, stdin);
                tempQuantity[strcspn(tempQuantity, "\n")] = 0;
                if(!(doubleCheck(tempQuantity))){
                    input = 0;
                } else {
                    quantity = strToDbl(tempQuantity);
                    input = 1;
                }
            } while(input != 1);
            

            changeProductQuantity(db, userChoiceID, quantity);

            return 0;

        case 4:

            printf("You have selected to change the category\n\n");

            do{
                printf("Please enter the new category of the stock product:   ");
                printf("Category options\n\n");
                showCategories(db);
                fgets(category, 50, stdin);
                category[strcspn(category, "\n")] = 0;

                if(getCategoryID(db, category) == 99){
                    printf("Please make sure to choose an available category\n");
                }

            } while(getCategoryID(db, category) == 99);

            changeProductCategory(db, userChoiceID, getCategoryID(db, category));
            
            return 0;

        case 5:

            input = 0;
            printf("You have selected to remove the stock\n\n");

            do{
                printf("Warning, this modification is irreversible, are you sure you want to proceed [Y/n] ");
                fgets(delete, 2, stdin);
                getchar();
                delete[strcspn(delete, "\n")] = 0;

                if((strcmp(delete, "Y") == 0) || (strcmp(delete, "y") == 0)){

                    /*Delete function*/
                    deleteStock(db, userChoiceID);
                    input = 1;

                    return 0;
                
                }
                else if((strcmp(delete, "N") == 0) || (strcmp(delete, "n") == 0)){

                    printf("Deletion was aborted\n");
                    input = 1;

                    return 0;
                }

                else {
                    input = 0;
                }
                
            } while(input == 0);
            

    }

    return 0; 
}

/*Takes the stock data structure passed from the addStock function and adds the data to the database*/
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

    int success = 0;

    /*Adding data to the product table*/
    sprintf(data, "INSERT INTO PRODUCT VALUES('%d', '%s', '%lf', '%lf')", productID, name, price, quantity);

    int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
        success = 0;

    } else {
        success = 1;
    }

    /*Adding data to the productCat table*/
    char productCatData[128];

    sprintf(productCatData, "INSERT INTO PRODUCT_CAT VALUES('%d', '%d')", productID, categoryID);

    rc = sqlite3_exec(db, productCatData, 0, 0, &errMsg);

    if(rc != SQLITE_OK){
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
        success = 0;

    } else {
        success = 1;
    }

    if(success = 1){

        printf("Data has been added successfully\n");
    }

    return 0;
}


/*Handles the user interaction for adding stock to the database*/
int addStock(sqlite3 *db){

    char name[20];
    char category[20];
    int categoryID;
    long double price;
    char tempPrice[10];
    long double quantity;
    char tempQuantity[10];

    int input = 0;
    
    printf("Please enter the name of the product  ");
    fgets(name, 20, stdin);
    name[strcspn(name, "\n")] = 0;

    if(((strcmp(name, "q")) == 0) || ((strcmp(name, "Q")) == 0)){
            return 1;
        }

    showCategories(db);

    do{
        printf("Please enter the category of the product  ");
        fgets(category, 20, stdin);
        category[strcspn(category, "\n")] = 0;
        categoryID = getCategoryID(db, category);
    } while(categoryID == 99);
    

    input = 0;

    do{
        printf("Please enter the price for the %s ", name);
        fgets(tempPrice, 10, stdin);
        tempPrice[strcspn(tempPrice, "\n")] = 0;
        if(!(doubleCheck(tempPrice))){
            input = 0;
        
        } else {
            price = strToDbl(tempPrice);
            input = 1;
        }
    } while (input != 1);

    do{
        printf("Please enter the quantity of %s ", name);
        fgets(tempQuantity, 10, stdin);
        tempQuantity[strcspn(tempQuantity, "\n")] = 0;
        if(!(doubleCheck(tempQuantity))){
            input = 0;
        
        } else {
            quantity = strToDbl(tempQuantity);
            input = 1;
        }
    } while (input != 1);

    printf("\nYou have chosen to add %Lf %s at a price of %Lf under the category %s\n", quantity, name, price, category);

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

/*A function to clear the Category table when the program starts up*/
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
                readStockByName(initialisation);
                break;
            case 3:
                printf("Track Stock by Category\n");
                printf("----------------------------------\n");
                readStockByCategory(initialisation);
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
            default:
                printf("Please make sure to choose one of the displayed options\n");
        }
    }
}


