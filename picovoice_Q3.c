/******************************************************************************
 * @file picovoice_screenningQ3.c
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
 *
 * Requirements:
 * 1. Design and implement a stack class that accomodates any data type.
 *      Approach: void pointer with type casting.
 *
 * 2. Discuss pros & cons of different stack implementations.
 *      Two of the most common data types are array and linked list, and both can be
 *      used to implement a stack with unique characteristics (pros/cons).
 *
 *      FIRST OPTION: array
 *      Implementing a stack with an array is simple and straigh-forward. An index counter/
 *      runner can simualte the top of the stack wrapped around the data structure. Although
 *      its size is not flexible and difficult to increase or decrease, this approach
 *      can provide us with a better performance, compared to the linked list implementation
 *      (despite their simliar time complexity O(1)).
 *      This is because there are no node pointers involved, eliminating constant memory 
 *      management process of handling the nodes. For the same reason, this approach offers better 
 *      space complexity O(1) for most of its operations since there is no temporary space or nodes
 *      are required. One exception may be when its stack size must be modified, then the space and
 *      time complexity may equate to O(n) or more.
 *
 *      SECOND OPTION: linked list.
 *      Implementing a stack with a linked list is more of a text book approach. It requires node
 *      operations, which are additional overhead costs compared to the array implementation.
 *      However, this approach provides the most flexibility in terms of the stack size modifications,
 *      and require no extra efforts once implemented. Another downside may be in its constant memory
 *      allocations which can result in memory fragmentations. In terms of space complexity, this approach
 *      requires extra memory allocation for the node handles when n reaches certain point.
 * 
 *      Another design consideration involves the node data types. If the stack only deals with primitive
 *      data structure, push() and pop() operations can be rather straight forward since there is no need 
 *      for extra memory handling. These data values can be safely stored in the environment stack until
 *      discarded. However, if stack accomodates complex data structure via reference types, more
 *      careful stack design is required to avoid potential memory leak problems as well as user
 *      experiences. Using reference data structure also complicates when you only want to push and pop 
 *      primitive data structures since all data must be converted to reference types.
 *      Three different approach can be considered regarding this issue:
 *      
 *      FIRST OPTION: Allocate new memory block and copy the data, or simply return the handle and let
 *                  the end user handle the memeory freeing.
 *      SECOND OPTION: Create wrapper class that resembles Java or Python Object class.
 *      THIRD OPTION: Return the reference value for the data, and store the node on another stack, until
 *                  clear or kill requests are made for the complex data structure. Also, automatically
 *                  handle all allocated resources if forgotten by the users when the stack is destroyed.
 * 
 *      Third option may provide the best balace between the three, so the following exercise will implement
 *      stack using linked list and reference handling for the data structure with some automation for
 *      maintainabilit and ease of use.
 *
 * 3. What is the best approach in an embedded real-time system?
 *      In an embedded real-time system, agents and environments are monitored in real time by the system.
 *      This requires to us to choose TIME over SPACE, meaning stack implementation with array may be preferable
 *
 * 4. What is the best approach with very limited memory environment?
 *      In a very limited resource system environment, stack design should choose SPACE over TIME, or rather
 *      bounded by its memory resources. A hybrid of both may be preferable in this case. When dealing with small
 *      number of stacking, linked list implementation provides more flexible SPACE management over the cost of TIME.
 *      However, when the system deals with large amount of push() operations, converting linked-list to array 
 *      implementation can be considered. 
 */

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define bool int

/******************************************************************************
 * @brief DataStructure TStack implementation.
 * 
 */
struct TNode {
    void* data;
    struct TNode* next;
};
typedef struct TNode TNode;
typedef TNode* PTNode;

/**
 * @brief Double stack implemntations.
 * top represents the actual top of the stack
 * orphan represents the history of popped nodes
 * 
 *  
 */
typedef struct TStack {
    TNode* top; // actual stack.
    TNode* orphan; // history stack. Also holds the actual complex data structure that has been popped.
} TStack;
typedef TStack* PTStack;

PTStack create_stack(); // initialize stack.
void kill_stack(PTStack stack); // auto handling of gc

bool is_stack_empty(PTStack stack);
void clear_orphans(PTStack stack); // frees allocated memory resources of popped nodes.

void stack_push(PTStack stack, void* data, size_t data_size);
void* stack_pop(PTStack stack);

/******************************************************************************
 * Testing implemented stack class (struct with functions) 
 * with custom complex data structure.
 *
 * Each stages of stack operations are labelled through 1-4 for clarity.
 */
typedef struct TypeMyComplexDataStructure {
    int a;
    float b;
    char c;
} TMyCustom;

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

    // Create Stack (step 1)
    PTStack my_stack = create_stack();

    // Push operations (step 2)
    for (int i = 0; i < 5; i++) {
        stack_push(my_stack, &data[i], sizeof(TMyCustom)); 

        printf("Pushed item #%d: integer(%d) real_number(%.2f) letter(%c)\n", 
            i+1, data[i].a, data[i].b, data[i].c);

    }

    printf("\n");

    // Pop operations (step 3)
    int count = 5;
    while (!is_stack_empty(my_stack)) {
        TMyCustom* item = (TMyCustom*)stack_pop(my_stack);

        printf("Popped item #%d: integer(%d) real_number(%.2f) letter(%c)\n", 
            count--, item->a, item->b, item->c);
    }

    // destorying the stack (step 4)
    kill_stack(my_stack);
    return 0;
}

/******************************************************************************
 * @brief TStack function implemenations.
 */

PTStack create_stack() {
    PTStack node = (PTStack)malloc(sizeof(TStack));
    node->top = NULL;
    node->orphan = NULL;
    return node;
}

void kill_stack(PTStack stack) {
    // pop all nodes.
    while (!is_stack_empty(stack)) {
        stack_pop(stack);
    }
    clear_orphans(stack); // clear memory resources.
    free(stack); // kill the stack instance.
}

bool is_stack_empty(PTStack stack) {
    return (stack->top == NULL);
}

/**
 * @brief The most unique design point of my stack implementation. 
 *      Previously popped nodes are temporarily held which can provide
 *      two benefits:
 *          1. Even when the data pushed into this stack is complex and dynamic,
 *          there's no need to allocate extra memory block to return it to the user.
 *          2. History feature can be easily implemented, such as undo feature. 
 * @param stack 
 */
void clear_orphans(PTStack stack) {
    PTNode orphan = stack->orphan;
    PTNode temp;
    while (orphan) {
        if (orphan->data) {
            free(orphan->data);
        }
        temp = orphan;
        orphan = orphan->next;
        free(temp);
    }
}

void stack_push(PTStack stack, void* data, size_t data_size) {
    PTNode new_top = (PTNode)malloc(sizeof(TNode));
    new_top->data = malloc(data_size);
    memcpy(new_top->data, data, data_size);
    new_top->next = stack->top;
    stack->top = new_top;
}

void* stack_pop(PTStack stack) {
    TNode* temp = stack->top;
    void* item = temp->data;
    stack->top = temp->next;
    
    temp->next = stack->orphan;
    stack->orphan = temp;
    return item;
}
