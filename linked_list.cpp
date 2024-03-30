#include<iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <random>

using namespace std;

struct Node {
    int key;
    Node* next;

    Node(int value) : key(value), next(nullptr) {}
};

struct LinkedList {
private:
    Node* head;
    mutable shared_mutex mutex_;

public:
    LinkedList(): head(nullptr) {}

    bool insert(int key) {
        if(search(key)) return false;
        unique_lock lock(mutex_);
        Node* newNode = new Node(key);

        if(!head || key<head->key) {
            newNode->next = head;
            head = newNode;
            return true;
        }

        Node* curr = head;
        while(curr->next || curr->next->key < key) {
            curr = curr->next;
        }
        newNode->next = curr->next;
        curr->next = newNode;
        return true;
    }

    bool search(int key) {
        shared_lock lock(mutex_);
        Node* curr = head;
        while(curr && curr->key <= key) {
            if(curr->key == key) return true;
            curr = curr->next;
        }
        return false;

    }

    bool remove(int key) {
        if(!search(key)) return false;
        unique_lock lock(mutex_);

        Node* curr = head;
        if(curr->key == key) {
            delete curr;
            head = nullptr;
            return true;
        }
        while(curr->next->key != key) {
            curr = curr->next;
        }
        Node* temp = curr->next;
        curr->next = curr->next->next;
        delete temp;
        return true;
    }

    ~LinkedList() {
        Node* curr = head;
        while(curr) {
            Node* temp = curr;
            curr = curr->next;
            delete temp;
        }
    }
};

template<typename T>
T random(T range_from, T range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<T>    distr(range_from, range_to);
    return distr(generator);
}



int main() {
    LinkedList ll;
    ll.insert(4);
    cout << ll.search(3) << '\n';
    
    const int Operations = 100000;
    const int numThreads = thread::hardware_concurrency();
    const int OpsEachThread = Operations/numThreads;
    const int KeySpace1 = 100;
    const int KeySpace2 = 10000;


    cout << random(0,10);

}