/******************************************************************************
 * @file picovoice_screeningQ1.c
 * @author Yong Sung John Lee (yongjohnlee80@outlook.com)
 * @brief Picovoice Screening Question #1
 * @version 0.1
 * @date 2022-04-08
 * 
 * @copyright Copyright (c) 2022
 * 
 * Q1 [C, Python] Assume we have a function get_book_info(isbn) that takes a string ISBN argument and
 * retrieves an object containing the Title, Author, and Language of a book (each represented as a string) that
 * takes a nontrivial amount of time to run (perhaps because it’s making a call to a database). Write a
 * wrapper function that increases performance by keeping results in memory for the quick lookup. To prevent
 * memory from growing unbounded, we only want to store a maximum of N book records. At any given time,
 * we should be storing the N books that we accessed most recently. Assume that N can be a large number
 * when choosing data structure(s) and algorithm(s).
 * 
 * Requiments:
 * 1. A wrapper function that increase performance by quick look up.
 *      Approach: hashing lookup
 * 
 * 2. Memory allotment limit (N). Keep the most recent N records.
 *      Approach: linked list or queue (implemented with linked list or array)
 *      There are different approches in implementing queue; first is to implement
 *      linked list with enqueue and dequeue methods, and the second is to use an array
 *      with current position counter.
 *      For this given problem, I've chosen to implment array based queue to deal with 
 *      N number of data records.
 * 
 * 3. Custom data type for book information
 *      Approach: struct
 *      There are a number of decisions to be made about the fields in this data structure.
 *      Firstly, using char pointer or std::string type will give benefits in flexibility in
 *      allocation of resources by using memory as needed, but will it unforeseably affect the
 *      lookup table size, N dynamically? 
 *      On the other hand, having the fixed sized character fields provides constant book_info
 *      record size. For this excercise, I chose the fixed number of character approach and
 *      defined a struct data type of 96 bytes to avoid memory fragmentation. This approach
 *      reduces memory handlings. I've chosen TIME over SPACE. 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __TEST_UNIT__ // for unit testing.

#ifdef __TEST_UNIT__
// caching status on simulated queries.
int num_record_found_in_table = 0;
int num_record_not_found_in_table = 0;
int num_hashing_collision = 0;
#endif


#define __BOOK_INFO_RECORD_MAX_SIZE__   1000 // N size.

/****************************************************************************************
 * @brief Custom Book Information Data Type (96 Bytes).
 * 
 */
#define __BOOK_INFO_ISBN_LENGTH__       14
#define __BOOK_INFO_TITLE_LENGTH__      57
#define __BOOK_INFO_AUTHOR_LENGTH__     21
#define __BOOK_INFO_LANGUAGE_LENGTH__   4

typedef struct MyCustomBookInfo {
    char isbn[__BOOK_INFO_TITLE_LENGTH__];
    char title[__BOOK_INFO_TITLE_LENGTH__];
    char author[__BOOK_INFO_AUTHOR_LENGTH__];
    char language[__BOOK_INFO_LANGUAGE_LENGTH__];
} TBookInfo;
typedef TBookInfo* PTBookInfo;

void set_book_info_record(PTBookInfo record, char* isbn, char* title, char* author, char* langugage);
void copy_book_info_record(PTBookInfo dest, PTBookInfo src);


/******************************************************************************
 * @brief Lookup queue and hash table and their associated functions
 *        These are declared as static to limit their scope to this file.
 * 
 */
static TBookInfo queue[__BOOK_INFO_RECORD_MAX_SIZE__];
static int isbn_hash_table[__BOOK_INFO_RECORD_MAX_SIZE__];
static int record_runner = 0;

static unsigned get_isbn_key(char* isbn); // basic hashing function.
static int search_book_info_queue(char* isbn); // searching queue.
static void append_book_info_record(PTBookInfo record); // update queue and hash.

// Mock up DB handling function. Creates mock up book info data.
TBookInfo retreive_book_info_from_db(char* isbn) {
    TBookInfo temp_record;
    set_book_info_record(&temp_record, isbn, "Picovoice Screening Questions.", "Lee, J", "ENG");
    return temp_record;
}

/******************************************************************************
 * @brief REQUIRED WRAPPER FUNCTION
 * 
 * @param isbn 
 * @return TBookInfo 
 */
TBookInfo get_book_info(char* isbn) {
    int search_result = search_book_info_queue(isbn);
    if(search_result >= 0) {

#ifdef __TEST_UNIT__
        num_record_found_in_table++;
#endif

        return queue[search_result];
    } else {
        TBookInfo record = retreive_book_info_from_db(isbn);
        append_book_info_record(&record);

#ifdef __TEST_UNIT__
        num_record_not_found_in_table++;
#endif
        return record;
    }
}

/******************************************************************************
 * @brief Unit Testing Section.
 * 
 */
#ifdef __TEST_UNIT__
void print_book_info_record(TBookInfo record) {
    printf("%s. (2022). \"%s\"(ISBN:%s) [%s]\n", record.author, record.title, record.isbn, record.language);
}

int main() {
    int k = 1234123;
    char buffer[__BOOK_INFO_ISBN_LENGTH__];

    // populate the cache
    for(int i = k; i < k+__BOOK_INFO_RECORD_MAX_SIZE__; i++) {
        get_book_info(itoa(i, buffer, 10));
    }
    // retrieve from the cache
    for(int i = k; i < k+__BOOK_INFO_RECORD_MAX_SIZE__; i++) {
        get_book_info(itoa(i, buffer, 10));
    }

    // display result stats.
    printf("==================================================================\n");
    printf("Simulating %d number of book info search queries:\n", __BOOK_INFO_RECORD_MAX_SIZE__*2);
    printf("Each ISBN number is requested twice.\n");
    printf("------------------------------------------------------------------\n");

    printf("%d number of records found in look up table.\n", num_record_found_in_table);
    printf("%d number of records not found the table.\n", num_record_found_in_table);
    printf("%d number of hashing collisions occurred\n\n", num_hashing_collision);
    printf("==================================================================\n");
    printf("Printing a sample book info record...\n");
    print_book_info_record(get_book_info("7678819239482"));
}
#endif

/******************************************************************************
 * @brief Function Implementations.
 * 
 */

void set_book_info_record(PTBookInfo record, char* isbn, char* title, char* author, char* langugage) {
    strcpy(record->isbn, isbn);
    strcpy(record->author, author);
    strcpy(record->title, title);
    strcpy(record->language, langugage);
}

void copy_book_info_record(PTBookInfo dest, PTBookInfo src) {
    set_book_info_record(dest, src->isbn, src->title, src->author, src->language);
}

// most basic hashing function. The integer value can be multiplied by a large prime number to 
// reduce the collision.
unsigned get_isbn_key(char* isbn) {
    char buffer[__BOOK_INFO_ISBN_LENGTH__];
    long i = _atoi64(isbn)  % __BOOK_INFO_RECORD_MAX_SIZE__;
    return (unsigned int)(i);
}

// update look up queue and hash table.
void append_book_info_record(PTBookInfo record) {
    // update hash table
    int key = get_isbn_key(record->isbn);
    isbn_hash_table[key] = record_runner;
    // update look up queue(cache)
    copy_book_info_record(&queue[record_runner], record);
    record_runner++;
    // dequeuing by overwriting on the old entries.
    if(record_runner==__BOOK_INFO_RECORD_MAX_SIZE__) {
        record_runner = 0;
    }
}

// Look up fuction.
int search_book_info_queue(char* isbn) {
    // using hashing key to find the record.
    int key = get_isbn_key(isbn);
    if(!strcmp(queue[isbn_hash_table[key]].isbn, isbn)) return key;

    // in case of collision, search through the cache for the item. linear search.
    for(int i = 0; i < __BOOK_INFO_RECORD_MAX_SIZE__; i++) {
        if(!strcmp(queue[i].isbn, isbn)) {

#ifdef __TEST_UNIT__
            num_hashing_collision++;
#endif
            return i;
        }
    }
    // not found in the cache
    return -1;
}