/*
Name of the program = EECS 348 Assignment 2
Description = Priority-queue email inbox simulator using a list-based MaxHeap.
Inputs= Commands via stdin: EMAIL sender,subject,date | NEXT | READ | COUNT
Outputs = Printed email details / counts to stdout
Collaborators = None
Other sources = Gemini, ChatGPT, Claude (helped me put comments and troubleshooting on file input and running)
Author = Aditya Kulkarni
Creation date = 9/17/2026
Revision date = 9/17/2026
Revision = Separated "OtherPerson" priority from unknown senders.
*/

#include <stdio.h>      // for printf, sscanf, fgets
#include <stdlib.h>     // for malloc, realloc, free, exit
#include <string.h>     // for strcmp, strchr, strncpy, strcpy, strlen, strcspn

/*
    Priority order:
    Boss            = 5
    Subordinate     = 4
    Peer             = 3
    ImportantPerson  = 2
    OtherPerson      = 1
    (unknown sender  = 0, kept separate from OtherPerson)
*/

typedef struct {            // defines the Email data type
    char sender[50];         // holds the sender's name, up to 49 characters
    char subject[256];       // holds the subject text, up to 255 characters

    int month;                // month the email was sent (1-12)
    int day;                  // day of the month the email was sent
    int year;                 // year the email was sent

    int priority;              // numeric priority based on sender category
} Email;                       // end of Email struct definition

typedef struct {          // defines the MaxHeap data type
    Email *emails;          // pointer to a dynamically allocated array of Emails
    int size;                // number of emails currently stored in the heap
    int capacity;             // total number of slots currently allocated
} MaxHeap;                    // end of MaxHeap struct definition


/* ---------------------------------------------------------
   Utility Functions
   --------------------------------------------------------- */

int getPriority(const char *sender) {          // returns a priority number for a sender name
    if (strcmp(sender, "Boss") == 0)             // check if sender is "Boss"
        return 5;                                  // Boss gets the highest priority
    else if (strcmp(sender, "Subordinate") == 0)   // check if sender is "Subordinate"
        return 4;                                    // Subordinate gets next priority
    else if (strcmp(sender, "Peer") == 0)          // check if sender is "Peer"
        return 3;                                    // Peer gets next priority
    else if (strcmp(sender, "ImportantPerson") == 0) // check if sender is "ImportantPerson"
        return 2;                                       // ImportantPerson gets next priority
    else if (strcmp(sender, "OtherPerson") == 0)   // check if sender is "OtherPerson"
        return 1;                                    // OtherPerson gets its own priority level
    else                                            // any name that didn't match above
        return 0;                                     // unknown sender gets the lowest priority
}                                                 // end of getPriority


int higherPriority(Email a, Email b) {         // returns 1 if email a outranks email b
    if (a.priority > b.priority)                  // compare sender priority first
        return 1;                                    // a has strictly higher priority

    if (a.priority < b.priority)                  // check if a has lower priority
        return 0;                                    // a does not outrank b

    /*
        Same sender category:
        newest email comes first.
    */
    if (a.year > b.year)                           // compare years if priorities are tied
        return 1;                                    // a's year is more recent

    if (a.year < b.year)                           // check if a's year is older
        return 0;                                    // a does not outrank b

    if (a.month > b.month)                         // years are equal, compare months
        return 1;                                    // a's month is more recent

    if (a.month < b.month)                         // check if a's month is earlier
        return 0;                                    // a does not outrank b

    if (a.day > b.day)                             // years and months are equal, compare days
        return 1;                                    // a's day is more recent

    return 0;                                      // otherwise b is equal or newer, a does not outrank b
}                                                 // end of higherPriority


void swapEmails(Email *a, Email *b) {          // swaps the contents of two Email structs
    Email temp = *a;                              // copy the first email into a temporary variable
    *a = *b;                                      // overwrite the first email with the second
    *b = temp;                                    // overwrite the second email with the saved first
}                                                 // end of swapEmails


/* ---------------------------------------------------------
   MaxHeap Functions
   --------------------------------------------------------- */

void initializeHeap(MaxHeap *heap) {           // sets up an empty heap with starting capacity
    heap->capacity = 10;                          // start with room for 10 emails
    heap->size = 0;                               // heap starts empty

    heap->emails = malloc(heap->capacity * sizeof(Email)); // allocate the initial array

    if (heap->emails == NULL) {                   // check whether allocation failed
        printf("Memory allocation failed.\n");       // report the failure
        exit(1);                                     // stop the program since we can't continue
    }                                              // end of allocation check
}                                                 // end of initializeHeap


void resizeHeap(MaxHeap *heap) {               // doubles the heap's storage when it's full
    heap->capacity *= 2;                          // double the tracked capacity

    Email *newArray =                             // attempt to grow the existing array
        realloc(heap->emails, heap->capacity * sizeof(Email));

    if (newArray == NULL) {                       // check whether reallocation failed
        printf("Memory allocation failed.\n");       // report the failure
        free(heap->emails);                          // free the old array to avoid a leak
        exit(1);                                     // stop the program since we can't continue
    }                                              // end of reallocation check

    heap->emails = newArray;                      // point the heap at the newly grown array
}                                                 // end of resizeHeap


void heapifyUp(MaxHeap *heap, int index) {     // moves a newly inserted email upward to its correct spot
    int parent;                                   // will hold the index of the current parent node

    while (index > 0) {                           // keep going until we reach the root
        parent = (index - 1) / 2;                    // compute the parent's index

        if (higherPriority(heap->emails[index],       // check if the child outranks its parent
                           heap->emails[parent])) {

            swapEmails(&heap->emails[index],             // if so, swap child and parent
                       &heap->emails[parent]);

            index = parent;                              // continue checking from the parent's position
        } else {                                      // if the child does not outrank the parent
            break;                                        // the heap property is restored, stop
        }                                              // end of if/else
    }                                              // end of while loop
}                                                 // end of heapifyUp


void heapifyDown(MaxHeap *heap, int index) {   // moves the root downward to restore heap order
    while (1) {                                   // loop until the correct position is found
        int left = 2 * index + 1;                    // index of the left child
        int right = 2 * index + 2;                   // index of the right child
        int largest = index;                         // assume the current node is largest for now

        if (left < heap->size &&                     // if the left child exists
            higherPriority(heap->emails[left],           // and it outranks the current largest
                           heap->emails[largest])) {
            largest = left;                              // update largest to the left child
        }                                              // end of left child check

        if (right < heap->size &&                    // if the right child exists
            higherPriority(heap->emails[right],          // and it outranks the current largest
                           heap->emails[largest])) {
            largest = right;                             // update largest to the right child
        }                                              // end of right child check

        if (largest != index) {                      // if a child outranked the current node
            swapEmails(&heap->emails[index],             // swap the current node with that child
                       &heap->emails[largest]);

            index = largest;                             // continue checking from the child's position
        } else {                                      // if neither child outranked the current node
            break;                                        // the heap property is restored, stop
        }                                              // end of if/else
    }                                              // end of while loop
}                                                 // end of heapifyDown


void insertEmail(MaxHeap *heap, Email email) { // adds a new email into the heap
    if (heap->size == heap->capacity) {           // check if the array is already full
        resizeHeap(heap);                            // grow the array if needed
    }                                              // end of capacity check

    heap->emails[heap->size] = email;             // place the new email at the next free slot

    heapifyUp(heap, heap->size);                  // move it upward to restore heap order

    heap->size++;                                 // record that one more email is stored
}                                                 // end of insertEmail


Email *peekEmail(MaxHeap *heap) {              // returns a pointer to the top-priority email
    if (heap->size == 0) {                        // check if the heap is empty
        return NULL;                                 // nothing to return if empty
    }                                              // end of empty check

    return &heap->emails[0];                      // otherwise return the address of the root email
}                                                 // end of peekEmail


void removeEmail(MaxHeap *heap) {              // removes the top-priority email from the heap
    if (heap->size == 0) {                        // check if the heap is already empty
        return;                                      // nothing to remove
    }                                              // end of empty check

    heap->emails[0] = heap->emails[heap->size - 1]; // move the last email into the root position

    heap->size--;                                 // record that one fewer email is stored

    if (heap->size > 0) {                         // if there is still at least one email left
        heapifyDown(heap, 0);                        // restore heap order from the root
    }                                              // end of size check
}                                                 // end of removeEmail


/* ---------------------------------------------------------
   Parsing
   --------------------------------------------------------- */

Email createEmail(char *line) {                // builds an Email struct from one input line
    Email email;                                  // the Email struct being constructed

    char *firstComma;                             // will point to the first comma in the line
    char *secondComma;                            // will point to the second comma in the line

    char senderPart[50];                          // temporary buffer to hold the sender text
    char datePart[50];                            // temporary buffer to hold the date text

    /*
        Find the two commas:
        EMAIL Sender,Subject,Date
    */
    firstComma = strchr(line, ',');               // locate the first comma in the line

    if (firstComma == NULL) {                     // check if no comma was found
        printf("Invalid EMAIL command.\n");          // report the formatting error
        exit(1);                                     // stop the program since input is malformed
    }                                              // end of first comma check

    secondComma = strchr(firstComma + 1, ',');    // locate the second comma, searching after the first

    if (secondComma == NULL) {                    // check if no second comma was found
        printf("Invalid EMAIL command.\n");          // report the formatting error
        exit(1);                                     // stop the program since input is malformed
    }                                              // end of second comma check

    /*
        Get sender.
        Skip "EMAIL ".
    */
    sscanf(line + 6, "%49[^,]", senderPart);      // read everything up to the first comma into senderPart

    strcpy(email.sender, senderPart);             // copy the sender text into the Email struct

    /*
        Get subject.
    */
    int subjectLength = (int)(secondComma - firstComma - 1); // compute how many characters are in the subject

    strncpy(email.subject,                        // copy that many characters into the subject field
            firstComma + 1,
            subjectLength);

    email.subject[subjectLength] = '\0';          // manually terminate the subject string

    /*
        Get date.
    */
    strcpy(datePart, secondComma + 1);            // copy everything after the second comma into datePart

    sscanf(datePart, "%d-%d-%d",                  // parse the month, day, and year out of datePart
           &email.month,
           &email.day,
           &email.year);

    email.priority = getPriority(email.sender);   // compute and store the priority for this sender

    return email;                                 // return the fully built Email struct
}                                                 // end of createEmail


/* ---------------------------------------------------------
   Main
   --------------------------------------------------------- */

int main(void) {                               // program entry point
    MaxHeap heap;                                 // the heap that will store all emails
    char line[400];                               // buffer to hold one line of input at a time

    initializeHeap(&heap);                        // set up the empty heap before reading input

    while (fgets(line, sizeof(line), stdin) != NULL) { // read one line at a time until input ends

        /*
            Remove newline character.
        */
        line[strcspn(line, "\r\n")] = '\0';       // strip the trailing newline (and \r on Windows) from the line

        /*
            Ignore blank lines.
        */
        if (strlen(line) == 0) {                  // check if the line is now empty
            continue;                                // skip to the next line if so
        }                                          // end of blank line check


        /* -----------------------------------------------
           EMAIL command
           ----------------------------------------------- */
        if (strncmp(line, "EMAIL ", 6) == 0) {    // check if the line starts with "EMAIL "

            Email email = createEmail(line);         // parse the line into an Email struct

            insertEmail(&heap, email);                // insert the new email into the heap
        }                                          // end of EMAIL command


        /* -----------------------------------------------
           NEXT command
           ----------------------------------------------- */
        else if (strcmp(line, "NEXT") == 0) {     // check if the line is exactly "NEXT"

            Email *email = peekEmail(&heap);          // get the top-priority email without removing it

            /*
                NEXT only displays the highest-priority
                email. It does NOT remove it.
            */
            if (email != NULL) {                      // check that there is an email to show
                printf("Next email:\n");                 // print the header line
                printf("Sender: %s\n", email->sender);   // print the sender
                printf("Subject: %s\n", email->subject); // print the subject
                printf("Date: %02d-%02d-%04d\n",         // print the date, zero-padded
                       email->month,
                       email->day,
                       email->year);
            }                                          // end of null check
        }                                          // end of NEXT command


        /* -----------------------------------------------
           READ command
           ----------------------------------------------- */
        else if (strcmp(line, "READ") == 0) {     // check if the line is exactly "READ"

            /*
                READ removes the highest-priority email.
                Nothing is displayed.
            */
            removeEmail(&heap);                       // remove the top-priority email from the heap
        }                                          // end of READ command


        /* -----------------------------------------------
           COUNT command
           ----------------------------------------------- */
        else if (strcmp(line, "COUNT") == 0) {    // check if the line is exactly "COUNT"

            printf("There are %d emails to read.\n", heap.size); // print how many emails remain
        }                                          // end of COUNT command
    }                                            // end of main input loop

    free(heap.emails);                            // release the heap's array memory

    return 0;                                     // exit the program successfully
}                                                 // end of main