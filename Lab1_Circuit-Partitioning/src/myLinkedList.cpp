#include "FM_Partition.h"

void myLinkedList::push_back(Cell* inputCell){
    if(length == 0){
        head = inputCell;
        tail = inputCell;
        length = 1;
        return;
    }
    tail->next = inputCell;
    inputCell->prev = tail;
    tail = inputCell;
    length++;
}

void myLinkedList::push_front(Cell* inputCell){
    if(length == 0){
        head = inputCell;
        tail = inputCell;
        length = 1;
        return;
    }
    head->prev = inputCell;
    inputCell->next = head;
    head = inputCell;
    length++;
}

void myLinkedList::pop_front(){
    if(length == 0) return;
    Cell* curCell = head; 
    if(length == 1){
        head = nullptr;
        tail = nullptr;
        curCell->prev = nullptr;
        curCell->next = nullptr;
        length = 0;
        return;
    }
    head->next->prev = nullptr;
    head = head->next;

    curCell->prev = nullptr;
    curCell->next = nullptr;
    length--;
}

void myLinkedList::remove(Cell* inputCell){
    if(length == 1){
        head = nullptr;
        tail = nullptr;
        length = 0;
        return;
    }
    if(inputCell == head){
        pop_front();
        return;
    }
    if(inputCell == tail){
        inputCell->prev->next = nullptr;
        tail = inputCell->prev;
        inputCell->prev = nullptr;
        inputCell->next = nullptr;
        length--;
        return;
    }
    if(inputCell->prev != nullptr) inputCell->prev->next = inputCell->next;
    if(inputCell->next != nullptr) inputCell->next->prev = inputCell->prev;
    inputCell->prev = nullptr;
    inputCell->next = nullptr;
    length--;
}

void myLinkedList::clear(){
    head = nullptr;
    tail = nullptr;
    length = 0;
}