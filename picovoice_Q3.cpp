/******************************************************************************
 * @file picovoice_q3.cpp
 * @author Yong Sung John Lee (yongjohnlee80@outlook.com)
 * @brief Picovoice Screening Question #3
 * @version 0.1
 * @date 2022-04-08
 *
 * @copyright Copyright (c) 2022
 *
 * Q3 [C] Design and implement a stack class (i.e. struct with accompanying functions). The interface should
 * allow storing any data type including complex structures. Describe different implementation strategies and
 * compare their pros and cons. What is the best approach in an embedded real-time system? What is the
 * best approach when memory resources are very limited? You can use “malloc” and “free” functions.
 */

#include<stdlib.h>
#include<string>
#include<exception>
#include<iostream>

using namespace std;

/**
 * @brief Custom Type Node that accomodates any data type.
 * 
 */
struct TNode {
    void* data;
    TNode* next;

    TNode(void* data, size_t data_size) {
        try {
            this->data = malloc(data_size);
            memcpy(this->data, data, data_size);
        }
        catch (exception& e) {
            cout << e.what() << std::endl;
        }
        next = NULL;
    }

    ~TNode() {
        if (data) {
            free(data);
        }
    }
};

/******************************************************************************
 * 
 * @brief TStack implements two stacks within:
 *          top represents the stacked data top, while
 *          orphan represents previously popped nodes
 *          for garbage collection purpose.
 *          When designing this class, there were two choices in dealing with
 *          popped nodes. First was to allocate new memory block and copy the popped
 *          node's data then return it, which also places the responsibility of disposing the 
 *          resource on the user. However, this approach posed a potential memory
 *          leak problem, so second approach was taken, where pointer was returned when
 *          the node is popped, and second stack was utilized for the popped nodes.
 *          clear_orphan() member function is also accessible for user to dispose the
 *          allocated memories when needed, and if missed, deconstructor will take
 *          care of freeing resources.
 *          Another benefit of this approach is having a history
 *          of popped items, in case of needing an undo feature. 
 */
class TStack {
    private:
        TNode* top = NULL;
        TNode* orphan = NULL; // gc purpose.

        void link_nodes(TNode* new_top, TNode* old_top) {
            new_top->next = old_top;
        }

    public:
        ~TStack() {
            clear_stack();
        }

        bool is_empty() { return (top == NULL); }

        void clear_stack() {
            while (!is_empty()) pop();
            clear_orphans();
        }

        void clear_orphans() {
            while (orphan) {
                auto temp = orphan;
                orphan = orphan->next;
                free(temp);
            }
        }

        void push(void* data, size_t data_size) {
            auto node = new TNode(data, data_size);

            link_nodes(node, top);
            top = node;
        }

        void* pop() {
            if (is_empty()) throw "Empty Stack";

            auto data = top->data;
            auto temp = top; // old_top
            top = top->next;

            link_nodes(temp, orphan); // old_top is added to the orphans stack.
            orphan = temp;

            return data;
        }
};

/******************************************************************************
 * Testing the TStack data strucrure with complex data type, TMyCustom.
 * 
 */
struct TMyCustom {
    int a;
    float b;
    char c;
};

int main() {

    // Populating the custom data type.
    int integers[] = { 1, 2, 3, 4, 5 };
    float real[] = { 1.1, 1.2, 1.3, 1.4, 1.5 };
    char letters[] = { 'a', 'e', 'i', 'o', 'u' };
    TMyCustom data[5];
    for (int i = 0; i < 5; i++) {
        data[i].a = integers[i];
        data[i].b = real[i];
        data[i].c = letters[i];
    }

    TStack my_stack;

    // Push
    int count = 0;
    for (auto i : data) {
        my_stack.push(&i, sizeof(TMyCustom));
        printf("Pushed item #%d: integer(%d) real_number(%.2f) letter(%c)\n", ++count, i.a, i.b, i.c);
    }
    cout << endl;

    // Pop
    while (!my_stack.is_empty()) {
        TMyCustom* item = (TMyCustom*)my_stack.pop();
        printf("Popped item #%d: integer(%d) real_number(%.2f) letter(%c)\n", count--, item->a, item->b, item->c);
    }
}