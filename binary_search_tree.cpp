#include<iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <chrono>
#include <random>
#include <string>

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
    Node* left;
    Node* right;

    Node(int value) : key(value), left(nullptr), right(nullptr) {}
};

struct BST {
private:
    Node* root;
    mutable std::shared_mutex mutex_;

public:
    BST(): root(nullptr) {}

    bool insert(int key) {
        if(search(key)) return false;
        unique_lock lock(mutex_);
        Node* newNode = new Node(key);

        if(!root) {
            root = newNode;
            return true;
        }

        Node* curr = root;
        Node* prev = root;
        while(curr) {
            if(key < curr->key) {
                prev = curr;
                curr = curr->left;
            }
            else {
                prev = curr;
                curr = curr->right;
            }
        }
        if(prev->key < key) {
            prev->right = newNode;
            return true;
        }
        prev->left = newNode;
        return true;
    }

    bool search(int key) {
        shared_lock lock(mutex_);
        
        Node* curr = root;
        while(curr) {
            if(curr->key == key) 
                return true;
            if(curr->key < key) {
                curr = curr->right;
            }
            else {
                curr = curr->left;
            }
        }
        return false;
    }

    bool remove(int key) {
        if (!search(key)) return false;
        unique_lock lock(mutex_);
        root = deleteNode(root, key);
        return true;
    }

    Node* deleteNode(Node* curr, int k)
    {
        // Base case
        if (curr == NULL)
            return curr;
    
        // Recursive calls for ancestors of
        // node to be deleted
        if (curr->key > k) {
            curr->left = deleteNode(curr->left, k);
            return curr;
        }
        else if (curr->key < k) {
            curr->right = deleteNode(curr->right, k);
            return curr;
        }
    
        // If one of the children is empty
        if (curr->left == NULL) {
            Node* temp = curr->right;
            delete curr;
            return temp;
        }
        else if (curr->right == NULL) {
            Node* temp = curr->left;
            delete curr;
            return temp;
        }
    
        // If both children exist
        else {
    
            Node* succParent = curr;
    
            // Find successor
            Node* succ = curr->right;
            while (succ->left != NULL) {
                succParent = succ;
                succ = succ->left;
            }

            if (succParent != curr)
                succParent->left = succ->right;
            else
                succParent->right = succ->right;
    
            // Copy Successor Data to root
            curr->key = succ->key;
    
            // Delete Successor and return root
            delete succ;
            return curr;
        }
    }
    ~BST() {
        destroyTree(root);
    }

    void destroyTree(Node* node) {
        if (node != nullptr) {
            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    }
};

std::atomic<std::uint64_t> cnt;

// void testBST(int numvals){
//     BST tree;
//     for(int i = 0; i< numvals; i++){
//         tree.insert((i*71)%numvals);
//         if(i% 1000000 == 0){
//             printf("%d\n",i);
//         }
//     }
//     for(int i = 0; i< numvals; i++){
//         bool x= tree.remove(i);
//         if(!x){
//             throw("Failed impl");
//         }
//     }
// }

void readth(BST &tree, int keyspace)
{
    tree.search(random(1, keyspace));
}
void writeth(BST &tree, int keyspace)
{
    tree.insert(random(1, keyspace));
}
void removeth(BST &tree, int keyspace) 
{
    tree.remove(random(1, keyspace));
}

void thread_read_dominated(BST &tree, int keyspace){ 
    while(cnt.load() > 0) {
        cnt--;
        int r = random(1,100);
        if(r < 91) {
            readth(tree, keyspace);
        }else if(r < 96){
            writeth(tree, keyspace);
        }else {
            removeth(tree, keyspace);
        }
    }
}

void thread_write_dominated(BST &tree, int keyspace){ 
    while(cnt.load() > 0) {
        cnt--;
        int r = random(1,100);
        if(r < 91) {
            readth(tree, keyspace);
        }else if(r < 96){
            writeth(tree, keyspace);
        }else {
            removeth(tree, keyspace);
        }
    }
}

void testBSTMT(int numthreads, int keyspace, string workload){
    BST tree;
    std::vector< std:: thread> vth;

    if(workload == "read-dominated") {
        for(int i = 0; i< numthreads;i++){
            vth.emplace_back(thread_read_dominated, std::ref(tree), keyspace);
        }
    }
    else {
        for(int i = 0; i< numthreads;i++){
            vth.emplace_back(thread_write_dominated, std::ref(tree), keyspace);
        }
    }
    for(int i = 0; i< numthreads;i++){
        vth[i].join();
    }
}

void runexperiment(int iterations, int numthreads, int keyspace, string workload){
    
    time_point start_time = std::chrono::high_resolution_clock::now();
    for(int i = 0; i <iterations ; i++){
        cnt.store(10000000);
    
        testBSTMT(numthreads, keyspace, workload);
    }
    time_point end_time = std::chrono::high_resolution_clock::now();
    milliseconds duration = std::chrono::duration_cast<milliseconds>(end_time - start_time);
    cout << duration.count()/iterations << endl;

}

int main() {
    int numthreads = 8;
    int iterations = 10;
    int keyspace = 100;
    runexperiment(iterations, numthreads, keyspace, "read-dominated");
    runexperiment(iterations, numthreads, keyspace, "write-dominated");
    return 0;
}