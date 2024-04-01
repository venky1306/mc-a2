#include <iostream>
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

std::atomic<std::uint64_t> cnt;

const int operations = 100000;

void pushth(Stack &st, int keyspace)
{
    st.push(random(1, keyspace));
}
void popth(Stack &st, int keyspace) 
{
    st.pop();
}

void thread_workload(Stack &st, int keyspace){ 
    while(cnt.load() <= operations) {
        cnt++;
        int r = random(1,100);
        if(r < 51){
            pushth(st, keyspace);
        }else {
            popth(st, keyspace);
        }
    }
}

void testStackMT(int numthreads, int keyspace){
    Stack st;
    std::vector< std:: thread> vth;

    for(int i=0;i<keyspace/2;i++) {
        st.push(random(1, keyspace));
    }

    for(int i = 0; i< numthreads;i++){
        vth.emplace_back(thread_workload, std::ref(st), keyspace);
    }

    for(int i = 0; i< numthreads;i++){
        vth[i].join();
    }
}

void runexperiment(int iterations, int numthreads, int keyspace){
    
    time_point start_time = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <iterations ; i++){
        cnt.store(0);
    
        testStackMT(numthreads, keyspace);
    }
    time_point end_time = std::chrono::high_resolution_clock::now();
    milliseconds duration = std::chrono::duration_cast<milliseconds>(end_time - start_time);
     cout << duration.count()/iterations << "ms.\n";

}

int main() {
    int numthreads = 8;
    int iterations = 10;
    int keyspace = 100;
    cout << "Concurrent Stack.\n";

    for(int i=1;i<=numthreads;i++) {
        cout << i << "  number of concurrent threads for 100000 operations took ";
        runexperiment(iterations, i, keyspace);
    }
        
    return 0;
}