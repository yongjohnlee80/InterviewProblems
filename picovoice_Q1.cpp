/******************************************************************************
 * @file picovoice_q1.cpp
 * @author Yong Sung John Lee (yongjohnlee80@outlook.com)
 * @brief Picovoice Screening Question #1
 * @version 0.1
 * @date 2022-04-07
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
 */

#ifndef __GET_BOOK_INFO_TABLE__
#define __GET_BOOK_INFO_TABLE__

#define __BOOK_INFO_TABLE_SIZE 500 // N: number of recent look ups.


#include<iostream>
#include<map>
#include<string>

using namespace std;

struct TBookInfo {
    string isbn;
    string title;
    string author;
    string language;
};

class TLookupTable {
    private:
        TBookInfo *search_queue; // queue that holds the N recent look ups.
        map<string,int> lookup_table; // hash for indexing the queue.
        int current_position = 0;
        int queue_size;
        bool dequeue_needed = false; // activates once the table is fully populated.

    public:
        TLookupTable(int max_size) {
            search_queue = new TBookInfo[max_size];
            queue_size = max_size;
        }

        ~TLookupTable() {
            delete [] search_queue;
        }

        void append(TBookInfo book_info);
        TBookInfo* search(string ISBN);
};

/******************************************************************************
 * @brief retrieves a book info based on isbn input from hypothetical database.
 * 
 * @param isbn 
 * @return TBookInfo 
 */
TBookInfo retreive_from_database(string isbn) {

    // Querying for a book info with ISBN.
    // .....
    // return retrieved record.

    // following is a mockup record populating codes for unit testing.
    TBookInfo temp;
    temp.author = "john" + isbn;
    temp.isbn = isbn;
    temp.language = "ENG" + isbn;
    temp.title = "screen question #1 solution";
    return temp;
}

/******************************************************************************
 * @brief Required wrapper function that boosts performance of 
 *          retrieve_from_database() function.
 * 
 * @param isbn 
 * @return TBookInfo 
 */
TBookInfo get_book_info(string isbn) {
    static TLookupTable _book_info_queue(__BOOK_INFO_TABLE_SIZE);
                // static local variable approach to limit access to to lookup table

    // From the lookup Table
    auto result = _book_info_queue.search(isbn);
    if(result != NULL) {
        return *result;
    }
    // From the database
    else {
        auto temp = retreive_from_database(isbn);
        _book_info_queue.append(temp); // update lookup table
        return temp;
    }
}

/******************************************************************************
 * Type LookupTable Member Function Implementations.
 */

TBookInfo* TLookupTable::search(string ISBN) {
    if(lookup_table.find(ISBN) == lookup_table.end()) {
        // not found in the look_up table.
        return NULL;
    } 
    else {
        return &search_queue[lookup_table[ISBN]];
    }
}

void TLookupTable::append(TBookInfo book_info) {
    if(dequeue_needed) {
        // if fully populated, remove the current record from the hash.
        lookup_table.erase(search_queue[current_position].isbn);
    }

    // Update the queue and hash
    search_queue[current_position] = book_info;
    lookup_table[book_info.isbn] = current_position;

    current_position++;
    if(current_position == queue_size) {
        current_position = 0;
        dequeue_needed = true;
    }
}

#endif