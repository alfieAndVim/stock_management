#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

sqlite3* initialiseDatabase(){

    sqlite3* db;
    char* errMsg = 0;

    int rc = sqlite3_open("stock_data.db", &db);
    if (rc != SQLITE_OK) {
        printf("\n\nThe database has not been initialised successfully\n\n");
        printf("\n%s\n", sqlite3_errmsg(db));
    } else {
        printf("\n\nThe database has been initialised successfully\n\n");
        return db;
    }
}

void closeDB(sqlite3* db){

    sqlite3_close(db);
}

void createTable(sqlite3* db){

    char* errMsg = 0;
    char* data = "CREATE TABLE IF NOT EXISTS PRODUCT(productID INTEGER PRIMARY KEY, name TEXT, price REAL, quantity REAL);";

    int rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {

        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    }
    
    errMsg = 0;
    char productCatData[100] = "CREATE TABLE IF NOT EXISTS PRODUCT_CAT(productID INTEGER, categoryID INTEGER);";
    data = productCatData;
    
    rc = sqlite3_exec(db, data, 0, 0, &errMsg);

    if(rc != SQLITE_OK) {
        printf("\n%s\n", sqlite3_errmsg(db));
        sqlite3_free(errMsg);
    }

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

void main(){

    sqlite3* initialisation = initialiseDatabase();
    createTable(initialisation);
    closeDB(initialisation);

    printf("Onto the next stage\n\n");
}


