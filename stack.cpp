#include<iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>

using namespace std;

struct Node {
    int key;
    Node* next;

    Node(int value) : key(value), next(nullptr) {}
};

struct Stack {
private:
    Node* tail;
    mutable std::shared_mutex mutex_;

public:
    Stack(): tail(nullptr) {}

    void push(int key) {
        unique_lock lock(mutex_);
        Node* newNode = new Node(key);

        newNode->next = tail;
        tail = newNode;
    }

    int pop() {
        unique_lock lock(mutex_);
        if(tail==nullptr) return -1;
        Node* temp = tail;
        tail = tail->next;
        int val = temp->key;
        delete temp;
        return val;
    }

    ~Stack() {
        Node* curr = tail;
        while(curr) {
            Node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
};

int main() {
    Stack s;
    s.push(4);
    s.push(12);
    cout << s.pop();
}