Readme file for stock_management.c

Method for compiling if necessary:

	gcc stock_management.c -o stock_management -lsqlite3

Database entity relationship diagram:

	--------------------------       -------------------------      -----------------------
	| PRODUCT                 |     | PRODUCT_CAT	          |     | CATEGORY             |
	|                         |     |			  |     |                      |
	| PK FK productID INTEGER |     | PK productID  INTEGER   |\    | PK categoryID INTEGER|
	|       name	  TEXT    |-----| FK categoryID INTEGER   |-----|    name       TEXT   |
	|       price     REAL    |     |			  |/    |                      |
        |       quantity  REAL    |     |			  |     |                      | 
        --------------------------       -------------------------       -----------------------

	PK - Primary Key
	FK - Foreign Key

sqlite3 library reference:
	
	https://www.sqlite.org/cintro.html


