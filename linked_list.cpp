#include<iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <random>
#include <string>
#include <atomic>

typedef std::chrono::high_resolution_clock::time_point time_point;
typedef std::chrono::milliseconds milliseconds;
typedef std::chrono::nanoseconds nanoseconds;

using namespace std;

template<typename T>
T random(T range_from, T range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<T>    distr(range_from, range_to);
    return distr(generator);
}

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

        if (head == nullptr || head->key > newNode->key) {
            newNode->next = head;
            head = newNode;
        }
        else {
            Node* current = head;
            while (current->next != nullptr && current->next->key < newNode->key) {
                current = current->next;
            }
            newNode->next = current->next;
            current->next = newNode;
        }
        return true;
    }

    bool search(int key) {
        shared_lock lock(mutex_);
        Node* curr = head;
        while (curr != nullptr) {
            if (curr->key == key)
                return true; // Key found
            curr = curr->next;
        }
        return false;
    }

    bool remove(int key) {
        if(!search(key)) return false;
        unique_lock lock(mutex_);
        
        Node* curr = head;
        Node* prev = nullptr;
        if(curr !=nullptr && curr->key == key) {
            head = head->next;
            delete curr;
            return true;
        }
        
        while (curr != nullptr && curr->key != key) {
            prev = curr;
            curr = curr->next;
        }
        if (curr == nullptr) {
            return false; 
        }
        prev->next = curr->next;
        delete curr;
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

std::atomic<std::uint64_t> cnt;

// void testLinkedList(int numvals){
//     LinkedList ll;
//     for(int i = 0; i< numvals; i++){
//         ll.insert((i*71)%numvals);
//         if(i% 1000000 == 0){
//             printf("%d\n",i);
//         }
//     }
//     for(int i = 0; i< numvals; i++){
//         bool x= ll.remove(i);
//         if(!x){
//             throw("Failed impl");
//         }
//     }
// }

void readth(LinkedList &ll, int keyspace)
{
    ll.search(random(1, keyspace));
}
void writeth(LinkedList &ll, int keyspace)
{
    ll.insert(random(1, keyspace));
}
void removeth(LinkedList &ll, int keyspace) 
{
    ll.remove(random(1, keyspace));
}

const int operations = 100000;

void thread_read_dominated(LinkedList &ll, int keyspace){ 
    while(cnt.load() <= operations) {
        cnt++;
        int r = random(1,100);
        if(r < 91) {
            readth(ll, keyspace);
        }else if(r < 96){
            writeth(ll, keyspace);
        }else {
            removeth(ll, keyspace);
        }
    }
}

void thread_write_dominated(LinkedList &ll, int keyspace){ 
    while(cnt.load() <= operations) {
        cnt++;
        int r = random(1,100);
        if(r < 51){
            writeth(ll, keyspace);
        }else {
            removeth(ll, keyspace);
        }
    }
}

void testLinkedListMT(int numthreads, int keyspace, string workload){
    LinkedList ll;
    std::vector< std:: thread> vth;

    for(int i=0;i<keyspace/2;i++) {
        ll.insert(random(1, keyspace));
    }

    if(workload == "read-dominated") {
        for(int i = 0; i< numthreads;i++){
            vth.emplace_back(thread_read_dominated, std::ref(ll), keyspace);
        }
    }
    else {
        for(int i = 0; i< numthreads;i++){
            vth.emplace_back(thread_write_dominated, std::ref(ll), keyspace);
        }
    }
    for(int i = 0; i< numthreads;i++){
        vth[i].join();
    }
}

void runexperiment(int iterations, int numthreads, int keyspace, string workload){
    
    time_point start_time = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <iterations ; i++){
        cnt.store(0);
    
        testLinkedListMT(numthreads, keyspace, workload);
    }
    time_point end_time = std::chrono::high_resolution_clock::now();
    milliseconds duration = std::chrono::duration_cast<milliseconds>(end_time - start_time);
     cout << duration.count()/iterations << "ms.\n";

}

int main() {
    int numthreads = 8;
    int iterations = 10;
    int keyspace = 100;
    cout << "Concurrent Linked List.\n";
    cout << "1. Key Space of 100.\n";
    cout << "   ----------Read Dominated Workload----------" << '\n';
    for(int i=1;i<=numthreads;i++) {
        cout << i << "  number of concurrent threads for 100000 operations took ";
        runexperiment(iterations, i, keyspace, "read-dominated");
    }
        
    cout << "   ----------Write Dominated Workload----------" << '\n';
    for(int i=1;i<=numthreads;i++) {
        cout << i << "  number of concurrent threads for 100000 operations took ";
        runexperiment(iterations, i, keyspace, "write-dominated");
    }
    keyspace = 10000;
    cout << "\n2. Key Space of 10000.\n";
    cout << "   ----------Read Dominated Workload----------" << '\n';
    for(int i=1;i<=numthreads;i++) {
        cout << i << "  number of concurrent threads for 100000 operations took ";
        runexperiment(iterations, i, keyspace, "read-dominated");
    }
        
    cout << "   ----------Write Dominated Workload----------" << '\n';
    for(int i=1;i<=numthreads;i++) {
        cout << i << "  number of concurrent threads for 100000 operations took ";
        runexperiment(iterations, i, keyspace, "write-dominated");
    }
    return 0;
}