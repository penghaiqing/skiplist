#include<iostream>
#include<cstring>
#include<mutex>
#include<fstream>
#include<string>

#define FILFE_ADDR "db/k_value.db"

std::mutex  mtx;

// 跳跃表的节点
template<typename K, typename V>
class Node{
public:
    Node(){}
    ~Node();
    Node(K k, V v, int level);
    K getKey() const;

    V getValue() const;

    void setValue(V v);

    // 后退指针
    Node<K, V> *backward;
    // 指针数组，存放的是指向不同level的不同节点
    Node<K, V> **forward;
    // 该节点的level
    int node_level; 

private:
    K key;
    V value;
};

template<typename K, typename V>
Node<K, V>::Node(const K key, const V value, int level){
    this->key = key;
    this->value = value;
    this->node_level = level;
    // new 一个指针数组，指向Node对象。eg：A **P = new A *[10]; 但是此时还未构造
    this->forward = new Node<K, V>*[level + 1];

    // 初始化 forward 数组为 0
    memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
    // std::cout << "in Node constructer : Yes" << std::endl;
}

template<typename K, typename V>
Node<K, V>::~Node(){
    delete []forward; // 释放构造时申请的对应的forward 内存
}

template<typename K, typename V>
K Node<K, V>::getKey() const{
    return this->key;
}

template<typename K, typename V>
V Node<K, V>::getValue() const{
    return this->value;
}

template<typename K, typename V>
void Node<K, V>::setValue(V value) {
    this->value = value;
}

// 跳表的 list 类，即 调表的包含head的 管理链表节点
template<typename K, typename V>
class SkipList{
public:
    SkipList(int max_level);
    ~SkipList();
    Node<K, V>* create_node(K key, V value, int level);
    int insert_element(K key, V value); // 插入节点元素，返回 1 key值已经存在，返回0 插入成功
    void showhole_list(); // 遍历整个跳表
    bool find_element(K key);
    bool delete_element(K key);
    int get_randowm_level();

    void write_data_file();
    void load_data_file();

    int getSize(); // 获得当前的调表 size，即节点个数


private:
    void get_key_value_from_string(const std::string& str, std::string* key, std::string * value);
    bool is_valid_string(const std::string& str);
    int max_level; // 跳表的最大level，整个链表维持一个最大的level值
    int skip_list_level; // 当前跳表节点的 level

    Node <K, V>* header; // 执行头节点的指针

    int element_count;  // 调表的当前 节点个数

    // 读写 文件的流
    std::ofstream file_writer;
    std::ifstream file_reader;
};

// 构造函数，初始化 header 节点
template<typename K, typename V>
SkipList<K, V>::SkipList(int max_level){
    this->max_level = max_level;
    this->skip_list_level = 0;
    this->element_count = 0;
    K key;
    V value;
    this->header = new Node<K, V>(key, value, max_level);
}

// 析构函数，主要是释放 读写流 和 header 指针的内存
template<typename K, typename V>
SkipList<K, V>::~SkipList() {
    if(file_writer.is_open()){
        file_writer.close();
    }

    if(file_reader.is_open()){
        file_reader.close();
    }

    delete header; // 析构函数中一定释放 new 申请的空间
}


// 创建新节点
template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K key, const V value, int level){
    Node<K, V> *n = new Node<K, V>(key, value, level);
    return n;
}

// 获得当前调表的节点个数
template<typename K, typename V>
int SkipList<K, V>::getSize(){
    return element_count;
}

// 输出当前跳变的所有节点
template<typename K, typename V>
void SkipList<K, V>::showhole_list(){
    printf("####################### SkipList ##########################\n");

    for(int i=0; i<=skip_list_level; ++i){
        Node<K, V> *node = this->header->forward[i];
        std::cout << "level " << i << " ---->  ";
        while(node != nullptr){
            std::cout << node->getKey() << " : " << node->getValue() << ";  ";
            node = node->forward[i];
        }
        // 出while循环时，完整输出一层level的节点信息
        std::cout << std::endl;
    }
}

// Search for element in skip list 
/*
                           +------------+
                           |  select 60 |
                           +------------+
level 4     +-->1+                                                      100
                 |
                 |
level 3         1+-------->10+------------------>50+           70       100
                                                   |
                                                   |
level 2         1          10         30         50|           70       100
                                                   |
                                                   |
level 1         1    4     10         30         50|           70       100
                                                   |
                                                   |
level 0         1    4   9 10         30   40    50+-->60      70       100
*/

template<typename K, typename V>
bool SkipList<K, V>::find_element(K key){
    Node<K, V>* current = this->header;

    // 遍历先从最高的level 
    for(int i=skip_list_level; i>=0; --i){
        while(current->forward[i] && current->forward[i]->getKey() < key){
            // 从头节点开始，由最高的level开始判断，当此时的forward指针不为空且此时的key值小于查找的key
            current = current->forward[i];
        }
    }
    // 出了循环，此时current 指针指向的是 level 0 的比key大的前一个节点 （例子中的 50）
    current = current->forward[0]; // current继续下移一个 (指向 70)
    
    // 此时如果，getKey == key，则表示查找成功
    if(current && current->getKey() == key){
        std::cout << "Found key: " << key << ", value: " << current->getValue() << std::endl;
        return true;
    }

    std::cout << "Not Found Key: " << key << std::endl;
    return false;
}

// 删除指定的key键
template<typename K, typename V>
bool SkipList<K, V>::delete_element(K key){
    // 删除操作要修改共享数据，数据库一致性需要加锁进行保证
    mtx.lock();
    Node<K, V> *current = this->header;
    Node<K, V> *update[max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(max_level+1)); 

    // 和查找的思路相似，先找到传入的key节点的前一个节点
    // 从 最高的level开始
    for(int i=skip_list_level; i>=0; --i){
        while(current->forward[i] && current->forward[i]->getKey() < key){
            current = current->forward[i];
        }
        update[i] = current; // 从上到下，在新申请的update数组中记录将要删除节点的前一个元素

    }
    current = current->forward[0]; // 此时指向 getKey == key的节点
    
    if(current != nullptr && current->getKey() == key){
        for(int i=0; i<skip_list_level; ++i){
            // 需要考虑的一种情况，即 此时的节点在某一层被跳过了，因为跳表的每一层不是保存了所有的节点信息
            if(update[i]->forward[i] != current){
                break; // 此时break，跳出循环。
            }

            // 就像删除链表节点一样，跳过将要删除的节点（此处应该考虑 释放删除的节点空间吗？）
            update[i]->forward[i] = current->forward[i];
        }

        // header的forward指针数组初始化时都为0，现在如果为0，则说明该层没有跳表节点了
        // 降低 跳表的level
        while(skip_list_level > 0 && header->forward[skip_list_level] == 0){
            --skip_list_level;
        }
        std::cout << "Successfully deleted key: " << key << std::endl;
        --element_count;
    }
    else{
        std::cout << "Not Found Key: " << key << std::endl;
        return false;
    }
    mtx.unlock();
    return true;;
}

template<typename K, typename V>
int SkipList<K, V>::insert_element(K key, V value){
    std::cout << "in insert ";
    mtx.lock();
    Node<K, V> *current = this->header;

    Node<K, V> *update[max_level+1];
    memset(update, 0, sizeof(Node<K, V>*)*(max_level));

    for(int i=skip_list_level; i>=0; --i){
        while(current->forward[i] && current->forward[i]->getKey() < key){
            current = current->forward[i];
        }
        update[i] = current;// 更新update数组中每一层中的指针，此时均为将要插入的位置的前一个
    }

    current = current->forward[0];

    if(current && current->getKey() == key){
        std::cout << "key: " << key << ", is already exiting!" << std::endl;
        mtx.unlock();
        return 1;
    }

    // 此时 对 current进行判断
    // 若 current == nullptr  则表示我们已经走到了跳表的最后，此时直接在最后插入即可
    // 若 current 的key值 不等于 key，则我们需要将节点插入到 update[0] 和 current之间
    if(current == nullptr || current->getKey() != key){
        // 随机确定该插入节点的高度值
        int radm_level = get_randowm_level();

        // 如果 随机的高度比当前跳表的高度还高，需要将高出的 update 数组元素设置为 header
        if(radm_level > skip_list_level){
            for(int i = skip_list_level+1; i < radm_level + 1; ++i){ // 第一次的 Segmentation fault (core dumped)，扩展数组的大小越界了
                update[i] = header;
            }
            skip_list_level = radm_level;
        }

        Node<K, V> *insert_node = create_node(key, value, radm_level);

        // 更新所有的 level
        for(int i=0; i <= radm_level; ++i){
            insert_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = insert_node;
        }

        std::cout << "Successfully inserted the key: " << key << ", value: " << value << std::endl;
        element_count++;
    }
    mtx.unlock();
    return 0;
}

// 生成一个随机的 level 大小
template<typename K, typename V>
int SkipList<K, V>::get_randowm_level(){
    int k = 1;
    while(rand() % 2){
        ++k;
    }
    k = (k < max_level)? k : max_level;
    return k;
}

// 将数据库中的kv 数据存入到文件中
template<typename K ,typename V>
void SkipList<K, V>::write_data_file(){
    std::cout << "writing file ...." << std::endl;
    file_writer.open(FILFE_ADDR);
    if(!file_writer.is_open()){
        std::cout << "open file failed." << std::endl;
        return;
    }
    Node<K, V>* node = this->header->forward[0];
    while(node){
        K key = node->getKey();
        V value = node->getValue();
        file_writer << key << ":" << value << "\n";
        std::cout << "key: " << key << ", value: " << value << std::endl;
        node = node->forward[0];
    }
    file_writer.flush();
    file_writer.close();
    return ;
}

// 加载本地文件中的 kv 数据到 数据库中
template<typename K ,typename V>
void SkipList<K, V>::load_data_file(){
    file_reader.open(FILFE_ADDR);
    if(!file_reader.is_open()){
        std::cout << "open file failed." << std::endl;
        return;
    }
    std::cout << "loading file ...." << std::endl;
    std::string line;
    std::string* key = new std::string();
    std::string* value = new std::string();
    while(getline(file_reader, line)){
        get_key_value_from_string(line, key, value);
        if(key->empty() || value->empty())
            continue;
        // 修改 key 的类型才能进行插入，由 string 到 K
        
        insert_element(*key, *value);
        std::cout << "key: " << *key << " , value: " << value << std::endl;
    }
    file_reader.close();
}

// 
template<typename K ,typename V>
void SkipList<K, V>::get_key_value_from_string(const std::string& str, std::string* key, std::string* value){
    if(!is_valid_string(str))
        return;
    if(str.find(":") == std::string::npos)
        return;
    *key = str.substr(0, str.find(":"));
    *value = str.substr(str.find(":")+1, str.length());
}

// 
template<typename K ,typename V>
bool SkipList<K, V>::is_valid_string(const std::string& str){
    if(str.length() == 0) return false;

    if(str.find(":") == std::string::npos)
        return false;
    return true;
    
}