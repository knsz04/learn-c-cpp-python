#ifndef SHOP_CORE_H
#define SHOP_CORE_H

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <stack>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <mysql.h>

using namespace std;

#pragma comment(lib, "libmysql.lib")

const string DB_HOST = "localhost";
const string DB_USER = "root";
const string DB_PASS = "2004828.yzc";
const string DB_NAME = "shop";
const unsigned int DB_PORT = 3306;

struct Product {
    string sku;
    string name;
    string categoryId;
    double purchasePrice;
    double salePrice;
    int stock;
    int threshold;
};

struct Category {
    string categoryId;
    string name;
    string parentId;
};

struct User {
    string userId;
    string name;
    string position;
    string phone;
    string password;
    int permission;
};

struct OrderItem {
    string sku;
    int quantity;
    double unitPrice;
};

struct Order {
    string orderId;
    string type;
    vector<OrderItem> items;
    double totalAmount;
    string createTime;
    string operatorId;
    string status;
};

struct SaleStat {
    string sku;
    int salesCount;
    double salesAmount;
};

struct PriceHistory {
    int id;
    string sku;
    string priceType;
    double oldPrice;
    double newPrice;
    string operationType;
    string operatorId;
    string recordTime;
};

class HashTable {
private:
    struct HashNode {
        Product data;
        HashNode* next;
        HashNode(const Product& p) : data(p), next(NULL) {}
    };
    vector<HashNode*> table;
    size_t hash(const string& key) {
        size_t h = 0;
        for (char c : key) h = h * 31 + c;
        return h % table.size();
    }
public:
    HashTable(size_t size = 100) : table(size, NULL) {}
    ~HashTable() {
        for (size_t i = 0; i < table.size(); i++) {
            HashNode* node = table[i];
            while (node) {
                HashNode* temp = node;
                node = node->next;
                delete temp;
            }
        }
    }
    bool insert(const Product& p) {
        size_t idx = hash(p.sku);
        HashNode* node = table[idx];
        while (node) {
            if (node->data.sku == p.sku) return false;
            node = node->next;
        }
        HashNode* newNode = new HashNode(p);
        newNode->next = table[idx];
        table[idx] = newNode;
        return true;
    }
    Product* find(const string& sku) {
        size_t idx = hash(sku);
        HashNode* node = table[idx];
        while (node) {
            if (node->data.sku == sku) return &node->data;
            node = node->next;
        }
        return NULL;
    }
    bool remove(const string& sku) {
        size_t idx = hash(sku);
        HashNode* node = table[idx];
        HashNode* prev = NULL;
        while (node) {
            if (node->data.sku == sku) {
                if (prev) prev->next = node->next;
                else table[idx] = node->next;
                delete node;
                return true;
            }
            prev = node;
            node = node->next;
        }
        return false;
    }
    vector<Product> getAll() {
        vector<Product> result;
        for (size_t i = 0; i < table.size(); i++) {
            HashNode* node = table[i];
            while (node) {
                result.push_back(node->data);
                node = node->next;
            }
        }
        return result;
    }
};

class MinHeap {
private:
    vector<pair<int, string> > heap;
    void heapify(int i) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;
        if (left < (int)heap.size() && heap[left].first < heap[smallest].first)
            smallest = left;
        if (right < (int)heap.size() && heap[right].first < heap[smallest].first)
            smallest = right;
        if (smallest != i) {
            swap(heap[i], heap[smallest]);
            heapify(smallest);
        }
    }
public:
    void push(int stock, const string& sku) {
        heap.push_back(make_pair(stock, sku));
        int i = heap.size() - 1;
        while (i > 0 && heap[(i - 1) / 2].first > heap[i].first) {
            swap(heap[i], heap[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }
    pair<int, string> pop() {
        if (heap.empty()) return make_pair(-1, "");
        pair<int, string> top = heap[0];
        heap[0] = heap.back();
        heap.pop_back();
        heapify(0);
        return top;
    }
    bool empty() { return heap.empty(); }
    size_t size() { return heap.size(); }
};

class CircularQueue {
private:
    vector<Order> arr;
    int front, rear, capacity;
public:
    CircularQueue(int size = 100) : arr(size), front(0), rear(0), capacity(size) {}
    bool enqueue(const Order& order) {
        if ((rear + 1) % capacity == front) return false;
        arr[rear] = order;
        rear = (rear + 1) % capacity;
        return true;
    }
    Order dequeue() {
        if (front == rear) return Order();
        Order order = arr[front];
        front = (front + 1) % capacity;
        return order;
    }
    bool empty() { return front == rear; }
    bool full() { return (rear + 1) % capacity == front; }
    vector<Order> getAll() {
        vector<Order> result;
        if (front <= rear) {
            for (int i = front; i < rear; i++)
                result.push_back(arr[i]);
        } else {
            for (int i = front; i < capacity; i++)
                result.push_back(arr[i]);
            for (int i = 0; i < rear; i++)
                result.push_back(arr[i]);
        }
        return result;
    }
};

class RBTree {
private:
    enum Color { COLOR_RED, COLOR_BLACK };
    struct Node {
        SaleStat data;
        Node* left;
        Node* right;
        Node* parent;
        Color color;
        Node(const SaleStat& s) : data(s), left(NULL), right(NULL), parent(NULL), color(COLOR_RED) {}
    };
    Node* root;
    void rotateLeft(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->left) x->parent->left = y;
        else x->parent->right = y;
        y->left = x;
        x->parent = y;
    }
    void rotateRight(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent) root = y;
        else if (x == x->parent->right) x->parent->right = y;
        else x->parent->left = y;
        y->right = x;
        x->parent = y;
    }
    void insertFixup(Node* z) {
        while (z->parent && z->parent->color == COLOR_RED) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;
                if (y && y->color == COLOR_RED) {
                    z->parent->color = COLOR_BLACK;
                    y->color = COLOR_BLACK;
                    z->parent->parent->color = COLOR_RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        rotateLeft(z);
                    }
                    z->parent->color = COLOR_BLACK;
                    z->parent->parent->color = COLOR_RED;
                    rotateRight(z->parent->parent);
                }
            } else {
                Node* y = z->parent->parent->left;
                if (y && y->color == COLOR_RED) {
                    z->parent->color = COLOR_BLACK;
                    y->color = COLOR_BLACK;
                    z->parent->parent->color = COLOR_RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rotateRight(z);
                    }
                    z->parent->color = COLOR_BLACK;
                    z->parent->parent->color = COLOR_RED;
                    rotateLeft(z->parent->parent);
                }
            }
        }
        root->color = COLOR_BLACK;
    }
    void inorder(Node* node, vector<SaleStat>& result) {
        if (!node) return;
        inorder(node->left, result);
        result.push_back(node->data);
        inorder(node->right, result);
    }
public:
    RBTree() : root(NULL) {}
    void insert(const SaleStat& s) {
        Node* z = new Node(s);
        Node* y = NULL;
        Node* x = root;
        while (x) {
            y = x;
            if (s.salesAmount < x->data.salesAmount) x = x->left;
            else x = x->right;
        }
        z->parent = y;
        if (!y) root = z;
        else if (s.salesAmount < y->data.salesAmount) y->left = z;
        else y->right = z;
        insertFixup(z);
    }
    vector<SaleStat> getSorted() {
        vector<SaleStat> result;
        inorder(root, result);
        return result;
    }
};

class DatabaseManager {
private:
    MYSQL* conn;
public:
    DatabaseManager() : conn(NULL) {}
    ~DatabaseManager() { disconnect(); }
    bool connect() {
        conn = mysql_init(NULL);
        if (!conn) return false;
        if (!mysql_real_connect(conn, DB_HOST.c_str(), DB_USER.c_str(),
            DB_PASS.c_str(), DB_NAME.c_str(), DB_PORT, NULL, 0)) {
            return false;
        }
        mysql_set_character_set(conn, "utf8mb4");
        return true;
    }
    void disconnect() {
        if (conn) mysql_close(conn);
        conn = NULL;
    }
    bool loadProducts(HashTable& productTable) {
        string sql = "SELECT * FROM products";
        if (mysql_query(conn, sql.c_str())) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            Product p;
            p.sku = row[0];
            p.name = row[1];
            p.categoryId = row[2] ? row[2] : "";
            p.purchasePrice = row[3] ? atof(row[3]) : 0;
            p.salePrice = row[4] ? atof(row[4]) : 0;
            p.stock = row[5] ? atoi(row[5]) : 0;
            p.threshold = row[6] ? atoi(row[6]) : 10;
            productTable.insert(p);
        }
        mysql_free_result(res);
        return true;
    }
    bool saveProduct(const Product& p) {
        char skuBuf[200], nameBuf[200], categoryBuf[200];
        mysql_real_escape_string(conn, skuBuf, p.sku.c_str(), p.sku.length());
        mysql_real_escape_string(conn, nameBuf, p.name.c_str(), p.name.length());
        mysql_real_escape_string(conn, categoryBuf, p.categoryId.c_str(), p.categoryId.length());
        
        char sql[800];
        sprintf(sql, "INSERT INTO products VALUES ('%s', '%s', '%s', %.2f, %.2f, %d, %d) ON DUPLICATE KEY UPDATE name='%s', category_id='%s', purchase_price=%.2f, sale_price=%.2f, stock=%d, threshold=%d",
            skuBuf, nameBuf, categoryBuf, p.purchasePrice, p.salePrice, p.stock, p.threshold,
            nameBuf, categoryBuf, p.purchasePrice, p.salePrice, p.stock, p.threshold);
        return mysql_query(conn, sql) == 0;
    }
    bool deleteProduct(const string& sku) {
        char sql[200];
        sprintf(sql, "DELETE FROM products WHERE sku='%s'", sku.c_str());
        return mysql_query(conn, sql) == 0;
    }
    bool loadUsers(unordered_map<string, User>& users) {
        string sql = "SELECT * FROM users";
        if (mysql_query(conn, sql.c_str())) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            User u;
            u.userId = row[0];
            u.name = row[1];
            u.position = row[2];
            u.phone = row[3] ? row[3] : "";
            u.password = row[4];
            u.permission = row[5] ? atoi(row[5]) : 1;
            users[u.userId] = u;
        }
        mysql_free_result(res);
        return true;
    }
    bool saveUser(const User& user) {
        char sql[500];
        sprintf(sql, "INSERT INTO users VALUES ('%s', '%s', '%s', '%s', '%s', %d)",
            user.userId.c_str(), user.name.c_str(), user.position.c_str(),
            user.phone.c_str(), user.password.c_str(), user.permission);
        return mysql_query(conn, sql) == 0;
    }
    bool deleteUser(const string& userId) {
        char sql[200];
        sprintf(sql, "DELETE FROM users WHERE user_id='%s'", userId.c_str());
        return mysql_query(conn, sql) == 0;
    }
    bool updateUserPermission(const string& userId, int permission) {
        char sql[200];
        sprintf(sql, "UPDATE users SET permission=%d WHERE user_id='%s'", permission, userId.c_str());
        return mysql_query(conn, sql) == 0;
    }
    bool saveOrder(const Order& order) {
        char sql[500];
        sprintf(sql, "INSERT INTO orders VALUES ('%s', '%s', %.2f, '%s', '%s', '%s')",
            order.orderId.c_str(), order.type.c_str(), order.totalAmount,
            order.createTime.c_str(), order.operatorId.c_str(), order.status.c_str());
        if (mysql_query(conn, sql) != 0) return false;
        for (size_t i = 0; i < order.items.size(); i++) {
            sprintf(sql, "INSERT INTO order_items (order_id, sku, quantity, unit_price) VALUES ('%s', '%s', %d, %.2f)",
                order.orderId.c_str(), order.items[i].sku.c_str(), order.items[i].quantity, order.items[i].unitPrice);
            if (mysql_query(conn, sql) != 0) return false;
        }
        return true;
    }
    bool updateStock(const string& sku, int delta) {
        char sql[200];
        sprintf(sql, "UPDATE products SET stock = stock + %d WHERE sku='%s'", delta, sku.c_str());
        return mysql_query(conn, sql) == 0;
    }
    bool updateSalesStats(const string& sku, int count, double amount) {
        char sql[200];
        sprintf(sql, "INSERT INTO sales_stats VALUES ('%s', %d, %.2f) ON DUPLICATE KEY UPDATE sales_count = sales_count + %d, sales_amount = sales_amount + %.2f",
            sku.c_str(), count, amount, count, amount);
        return mysql_query(conn, sql) == 0;
    }
    bool loadSalesStats(unordered_map<string, SaleStat>& stats) {
        string sql = "SELECT * FROM sales_stats";
        if (mysql_query(conn, sql.c_str())) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            SaleStat s;
            s.sku = row[0];
            s.salesCount = row[1] ? atoi(row[1]) : 0;
            s.salesAmount = row[2] ? atof(row[2]) : 0;
            stats[s.sku] = s;
        }
        mysql_free_result(res);
        return true;
    }
    bool loadOrders(vector<Order>& orders) {
        const char* sql = "SELECT * FROM orders ORDER BY create_time DESC";
        if (mysql_query(conn, sql) != 0) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            Order o;
            o.orderId = row[0] ? row[0] : "";
            o.type = row[1] ? row[1] : "";
            o.totalAmount = row[2] ? atof(row[2]) : 0;
            o.createTime = row[3] ? row[3] : "";
            o.operatorId = row[4] ? row[4] : "";
            o.status = row[5] ? row[5] : "";
            
            char itemSql[256];
            sprintf(itemSql, "SELECT sku, quantity, unit_price FROM order_items WHERE order_id='%s'", o.orderId.c_str());
            if (mysql_query(conn, itemSql) == 0) {
                MYSQL_RES* itemRes = mysql_store_result(conn);
                if (itemRes) {
                    MYSQL_ROW itemRow;
                    while ((itemRow = mysql_fetch_row(itemRes)) != NULL) {
                        OrderItem item;
                        item.sku = itemRow[0] ? itemRow[0] : "";
                        item.quantity = itemRow[1] ? atoi(itemRow[1]) : 0;
                        item.unitPrice = itemRow[2] ? atof(itemRow[2]) : 0;
                        o.items.push_back(item);
                    }
                    mysql_free_result(itemRes);
                }
            }
            orders.push_back(o);
        }
        mysql_free_result(res);
        return true;
    }
    bool findOrder(const string& orderId, Order& order) {
        char sql[256];
        sprintf(sql, "SELECT * FROM orders WHERE order_id='%s'", orderId.c_str());
        if (mysql_query(conn, sql) != 0) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row = mysql_fetch_row(res);
        if (row) {
            order.orderId = row[0] ? row[0] : "";
            order.type = row[1] ? row[1] : "";
            order.totalAmount = row[2] ? atof(row[2]) : 0;
            order.createTime = row[3] ? row[3] : "";
            order.operatorId = row[4] ? row[4] : "";
            order.status = row[5] ? row[5] : "";
            
            char itemSql[256];
            sprintf(itemSql, "SELECT sku, quantity, unit_price FROM order_items WHERE order_id='%s'", order.orderId.c_str());
            if (mysql_query(conn, itemSql) == 0) {
                MYSQL_RES* itemRes = mysql_store_result(conn);
                if (itemRes) {
                    MYSQL_ROW itemRow;
                    while ((itemRow = mysql_fetch_row(itemRes)) != NULL) {
                        OrderItem item;
                        item.sku = itemRow[0] ? itemRow[0] : "";
                        item.quantity = itemRow[1] ? atoi(itemRow[1]) : 0;
                        item.unitPrice = itemRow[2] ? atof(itemRow[2]) : 0;
                        order.items.push_back(item);
                    }
                    mysql_free_result(itemRes);
                }
            }
            mysql_free_result(res);
            return true;
        }
        mysql_free_result(res);
        return false;
    }

    bool savePriceHistory(const string& sku, const string& priceType, double oldPrice, double newPrice,
                          const string& operationType, const string& operatorId) {
        char skuBuf[100];
        mysql_real_escape_string(conn, skuBuf, sku.c_str(), sku.length());

        char sql[500];
        if (oldPrice < 0) {
            sprintf(sql, "INSERT INTO price_history (sku, price_type, new_price, operation_type, operator) VALUES ('%s', '%s', %.2f, '%s', '%s')",
                skuBuf, priceType.c_str(), newPrice, operationType.c_str(), operatorId.c_str());
        } else {
            sprintf(sql, "INSERT INTO price_history (sku, price_type, old_price, new_price, operation_type, operator) VALUES ('%s', '%s', %.2f, %.2f, '%s', '%s')",
                skuBuf, priceType.c_str(), oldPrice, newPrice, operationType.c_str(), operatorId.c_str());
        }
        return mysql_query(conn, sql) == 0;
    }

    bool loadPriceHistory(const string& sku, const string& priceType, vector<PriceHistory>& history) {
        char sql[300];
        sprintf(sql, "SELECT * FROM price_history WHERE sku='%s' AND price_type='%s' ORDER BY record_time ASC",
            sku.c_str(), priceType.c_str());
        if (mysql_query(conn, sql) != 0) return false;
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) return false;
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            PriceHistory h;
            h.id = row[0] ? atoi(row[0]) : 0;
            h.sku = row[1] ? row[1] : "";
            h.priceType = row[2] ? row[2] : "";
            h.oldPrice = row[3] ? atof(row[3]) : 0;
            h.newPrice = row[4] ? atof(row[4]) : 0;
            h.operationType = row[5] ? row[5] : "";
            h.operatorId = row[6] ? row[6] : "";
            h.recordTime = row[7] ? row[7] : "";
            history.push_back(h);
        }
        mysql_free_result(res);
        return true;
    }
};

class Graph {
private:
    unordered_map<string, unordered_map<string, int>> adjMatrix;

public:
    void addEdge(const string& from, const string& to, int weight = 1) {
        adjMatrix[from][to] += weight;
        adjMatrix[to][from] += weight;
    }

    void buildFromOrders(const vector<Order>& orders) {
        adjMatrix.clear();
        for (const Order& order : orders) {
            for (size_t i = 0; i < order.items.size(); i++) {
                for (size_t j = i + 1; j < order.items.size(); j++) {
                    addEdge(order.items[i].sku, order.items[j].sku);
                }
            }
        }
    }

    void dfs(const string& start, unordered_map<string, bool>& visited, vector<pair<string, int>>& result) {
        stack<string> s;
        s.push(start);
        visited[start] = true;

        while (!s.empty()) {
            string node = s.top();
            s.pop();

            if (node != start) {
                int weight = adjMatrix[start][node];
                result.push_back({node, weight});
            }

            for (auto& neighbor : adjMatrix[node]) {
                if (!visited[neighbor.first]) {
                    visited[neighbor.first] = true;
                    s.push(neighbor.first);
                }
            }
        }

        sort(result.begin(), result.end(), [](const pair<string, int>& a, const pair<string, int>& b) {
            return a.second > b.second;
        });
    }

    vector<pair<string, int>> getRecommendations(const string& sku) {
        unordered_map<string, bool> visited;
        vector<pair<string, int>> result;
        dfs(sku, visited, result);
        return result;
    }

    bool hasNode(const string& sku) {
        return adjMatrix.find(sku) != adjMatrix.end();
    }
};

class PriceTrendAnalyzer {
public:
    vector<double> predictExponentialSmoothing(const vector<double>& prices, int predictSteps, double alpha = 0.3) {
        vector<double> smoothed(prices.size());
        smoothed[0] = prices[0];
        
        for (size_t i = 1; i < prices.size(); i++) {
            smoothed[i] = alpha * prices[i] + (1 - alpha) * smoothed[i - 1];
        }
        
        vector<double> predictions;
        double lastSmooth = smoothed.back();
        for (int i = 0; i < predictSteps; i++) {
            lastSmooth = alpha * lastSmooth + (1 - alpha) * lastSmooth;
            predictions.push_back(lastSmooth);
        }
        
        return predictions;
    }
    
    pair<double, double> linearRegression(const vector<double>& prices) {
        int n = prices.size();
        if (n < 2) return {0, prices.empty() ? 0 : prices[0]};
        
        double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;
        for (int i = 0; i < n; i++) {
            sumX += i;
            sumY += prices[i];
            sumXY += i * prices[i];
            sumX2 += i * i;
        }
        
        double denom = n * sumX2 - sumX * sumX;
        if (denom == 0) return {0, prices[0]};
        
        double slope = (n * sumXY - sumX * sumY) / denom;
        double intercept = (sumY - slope * sumX) / n;
        
        return {slope, intercept};
    }
    
    vector<double> predictLinear(const vector<double>& prices, int predictSteps) {
        auto [slope, intercept] = linearRegression(prices);
        vector<double> predictions;
        for (int i = 0; i < predictSteps; i++) {
            predictions.push_back(slope * (prices.size() + i) + intercept);
        }
        return predictions;
    }
    
    double calculateVolatility(const vector<double>& prices) {
        if (prices.size() < 2) return 0;
        
        double mean = 0;
        for (double p : prices) mean += p;
        mean /= prices.size();
        
        double variance = 0;
        for (double p : prices) variance += (p - mean) * (p - mean);
        variance /= prices.size();
        
        return sqrt(variance);
    }
};

#endif
