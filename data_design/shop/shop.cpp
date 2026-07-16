#define _CRT_SECURE_NO_WARNINGS

#include "shop_core.h"
#include <windows.h>

#pragma execution_character_set("utf-8")

void initConsole() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    system("chcp 65001 >nul 2>&1");
}

class ShopSystem {
private:
    HashTable productTable;
    unordered_map<string, Category> categories;
    unordered_map<string, User> users;
    CircularQueue orderQueue;
    MinHeap stockAlertHeap;
    RBTree salesRBTree;
    DatabaseManager db;
    User currentUser;
    bool isLoggedIn;
    string generateOrderId() {
        time_t now = time(NULL);
        char id[30];
        sprintf(id, "ORD%ld", now);
        return string(id);
    }
    string getCurrentTime() {
        time_t now = time(NULL);
        struct tm* tm_ptr = localtime(&now);
        char buf[20];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_ptr);
        return string(buf);
    }
public:
    ShopSystem() : isLoggedIn(false) {}
    bool init() {
        initConsole();
        if (!db.connect()) {
            cout << "数据库连接失败！" << endl;
            return false;
        }
        if (!db.loadProducts(productTable)) {
            cout << "加载商品失败！" << endl;
            return false;
        }
        if (!db.loadUsers(users)) {
            cout << "加载用户失败！" << endl;
            return false;
        }
        vector<Product> products = productTable.getAll();
        for (size_t i = 0; i < products.size(); i++) {
            stockAlertHeap.push(products[i].stock, products[i].sku);
        }
        cout << "系统初始化成功！" << endl;
        return true;
    }
    bool login() {
        string userId, password;
        cout << "\n请输入用户ID: ";
        cin >> userId;
        cout << "请输入密码: ";
        cin >> password;
        unordered_map<string, User>::iterator it = users.find(userId);
        if (it != users.end() && it->second.password == password) {
            currentUser = it->second;
            isLoggedIn = true;
            cout << "\n登录成功！欢迎 " << currentUser.name << endl;
            return true;
        }
        cout << "\n登录失败：用户名或密码错误！" << endl;
        return false;
    }
    void logout() {
        isLoggedIn = false;
        cout << "\n已退出登录！" << endl;
    }
    bool checkPermission(int level) {
        if (!isLoggedIn) {
            cout << "请先登录！" << endl;
            return false;
        }
        if (currentUser.permission < level) {
            cout << "权限不足！" << endl;
            return false;
        }
        return true;
    }
    void addProduct() {
        if (!checkPermission(2)) return;
        Product p;
        cout << "\n请输入商品SKU: ";
        cin >> p.sku;
        cout << "请输入商品名称: ";
        cin.ignore();
        getline(cin, p.name);
        cout << "请输入分类ID: ";
        cin >> p.categoryId;
        cout << "请输入进价: ";
        cin >> p.purchasePrice;
        cout << "请输入售价: ";
        cin >> p.salePrice;
        cout << "请输入库存数量: ";
        cin >> p.stock;
        cout << "请输入预警阈值: ";
        cin >> p.threshold;
        if (productTable.insert(p)) {
            db.saveProduct(p);
            stockAlertHeap.push(p.stock, p.sku);
            cout << "\n商品添加成功！" << endl;
        } else {
            cout << "\n添加失败：SKU已存在！" << endl;
        }
    }
    void updateProduct() {
        if (!checkPermission(2)) return;
        string sku;
        cout << "\n请输入要修改的商品SKU: ";
        cin >> sku;
        Product* p = productTable.find(sku);
        if (!p) {
            cout << "商品不存在！" << endl;
            return;
        }
        cout << "\n当前信息:" << endl;
        cout << "商品名称(" << p->name << "): ";
        cin.ignore();
        getline(cin, p->name);
        cout << "分类ID(" << p->categoryId << "): ";
        cin >> p->categoryId;
        cout << "进价(" << p->purchasePrice << "): ";
        cin >> p->purchasePrice;
        cout << "售价(" << p->salePrice << "): ";
        cin >> p->salePrice;
        cout << "库存(" << p->stock << "): ";
        cin >> p->stock;
        cout << "预警阈值(" << p->threshold << "): ";
        cin >> p->threshold;
        db.saveProduct(*p);
        cout << "\n商品修改成功！" << endl;
    }
    void deleteProduct() {
        if (!checkPermission(3)) return;
        string sku;
        cout << "\n请输入要删除的商品SKU: ";
        cin >> sku;
        if (productTable.remove(sku)) {
            db.deleteProduct(sku);
            cout << "\n商品删除成功！" << endl;
        } else {
            cout << "\n删除失败：商品不存在！" << endl;
        }
    }
    void searchProduct() {
        if (!checkPermission(1)) return;
        string sku;
        cout << "\n请输入商品SKU: ";
        cin >> sku;
        Product* p = productTable.find(sku);
        if (p) {
            cout << "\n商品信息:" << endl;
            cout << "SKU: " << p->sku << endl;
            cout << "名称: " << p->name << endl;
            cout << "分类: " << p->categoryId << endl;
            cout << "进价: " << p->purchasePrice << endl;
            cout << "售价: " << p->salePrice << endl;
            cout << "库存: " << p->stock << endl;
            cout << "预警阈值: " << p->threshold << endl;
        } else {
            cout << "\n商品不存在！" << endl;
        }
    }
    void displayAllProducts() {
        if (!checkPermission(1)) return;
        vector<Product> products = productTable.getAll();
        cout << "\n========================================" << endl;
        cout << "              商品列表                   " << endl;
        cout << "========================================" << endl;
        for (size_t i = 0; i < products.size(); i++) {
            cout << "\nSKU: " << products[i].sku << endl;
            cout << "名称: " << products[i].name << endl;
            cout << "分类: " << products[i].categoryId << endl;
            cout << "进价: " << products[i].purchasePrice << " 售价: " << products[i].salePrice << endl;
            cout << "库存: " << products[i].stock << " 阈值: " << products[i].threshold << endl;
            cout << "----------------------------------------" << endl;
        }
    }
    void createOrder() {
        if (!checkPermission(1)) return;
        Order order;
        order.orderId = generateOrderId();
        order.type = "销售";
        order.createTime = getCurrentTime();
        order.operatorId = currentUser.userId;
        order.status = "已完成";
        order.totalAmount = 0;
        char choice;
        do {
            string sku;
            int quantity;
            cout << "\n请输入商品SKU: ";
            cin >> sku;
            Product* p = productTable.find(sku);
            if (!p) {
                cout << "商品不存在！" << endl;
                continue;
            }
            cout << "商品: " << p->name << " 价格: " << p->salePrice << " 库存: " << p->stock << endl;
            cout << "请输入数量: ";
            cin >> quantity;
            if (quantity <= 0 || quantity > p->stock) {
                cout << "无效数量！" << endl;
                continue;
            }
            OrderItem item;
            item.sku = sku;
            item.quantity = quantity;
            item.unitPrice = p->salePrice;
            order.items.push_back(item);
            order.totalAmount += p->salePrice * quantity;
            p->stock -= quantity;
            db.updateStock(sku, -quantity);
            db.updateSalesStats(sku, quantity, p->salePrice * quantity);
            cout << "\n继续添加商品? (y/n): ";
            cin >> choice;
        } while (choice == 'y' || choice == 'Y');
        orderQueue.enqueue(order);
        db.saveOrder(order);
        cout << "\n订单创建成功！" << endl;
        cout << "订单ID: " << order.orderId << endl;
        cout << "总金额: " << order.totalAmount << endl;
    }
    void checkStockAlert() {
        if (!checkPermission(2)) return;
        cout << "\n========================================" << endl;
        cout << "              库存预警                   " << endl;
        cout << "========================================" << endl;
        vector<Product> products = productTable.getAll();
        bool hasAlert = false;
        for (size_t i = 0; i < products.size(); i++) {
            if (products[i].stock <= products[i].threshold) {
                hasAlert = true;
                cout << "\n预警商品: " << products[i].name << endl;
                cout << "SKU: " << products[i].sku << endl;
                cout << "当前库存: " << products[i].stock << " 阈值: " << products[i].threshold << endl;
            }
        }
        if (!hasAlert) {
            cout << "\n暂无库存预警！" << endl;
        }
    }
    void salesRanking() {
        if (!checkPermission(2)) return;
        unordered_map<string, SaleStat> statsMap;
        db.loadSalesStats(statsMap);
        vector<SaleStat> stats;
        for (unordered_map<string, SaleStat>::iterator it = statsMap.begin(); it != statsMap.end(); it++) {
            stats.push_back(it->second);
        }
        if (stats.empty()) {
            cout << "\n暂无销售数据！" << endl;
            return;
        }
        for (size_t i = 0; i < stats.size(); i++) {
            salesRBTree.insert(stats[i]);
        }
        vector<SaleStat> sorted = salesRBTree.getSorted();
        cout << "\n========================================" << endl;
        cout << "            销售排行榜                   " << endl;
        cout << "========================================" << endl;
        int topN = min((int)sorted.size(), 5);
        for (int i = sorted.size() - 1; i >= (int)sorted.size() - topN; i--) {
            Product* p = productTable.find(sorted[i].sku);
            cout << "\n排名 " << (sorted.size() - i) << ":" << endl;
            cout << "商品: " << (p ? p->name : sorted[i].sku) << endl;
            cout << "销量: " << sorted[i].salesCount << " 销售额: " << sorted[i].salesAmount << endl;
        }
    }
    void showMenu() {
        cout << "\n========================================" << endl;
        cout << "          小型超市管理系统                " << endl;
        cout << "========================================" << endl;
        if (!isLoggedIn) {
            cout << "  1. 登录                                " << endl;
            cout << "  0. 退出                                " << endl;
        } else {
            cout << "  1. 商品管理                            " << endl;
            cout << "  2. 创建订单                            " << endl;
            cout << "  3. 库存预警                            " << endl;
            cout << "  4. 销售排行                            " << endl;
            cout << "  5. 退出登录                            " << endl;
            cout << "  0. 退出                                " << endl;
        }
        cout << "========================================" << endl;
        cout << "请输入您的选择: ";
    }
    void showProductMenu() {
        cout << "\n========================================" << endl;
        cout << "          商品管理                        " << endl;
        cout << "========================================" << endl;
        cout << "  1. 添加商品                            " << endl;
        cout << "  2. 修改商品                            " << endl;
        cout << "  3. 删除商品                            " << endl;
        cout << "  4. 查询商品                            " << endl;
        cout << "  5. 显示所有商品                        " << endl;
        cout << "  0. 返回主菜单                          " << endl;
        cout << "========================================" << endl;
        cout << "请输入您的选择: ";
    }
    void run() {
        int choice;
        do {
            showMenu();
            cin >> choice;
            switch (choice) {
            case 1:
                if (!isLoggedIn) {
                    login();
                } else {
                    int pc;
                    do {
                        showProductMenu();
                        cin >> pc;
                        switch (pc) {
                        case 1: addProduct(); break;
                        case 2: updateProduct(); break;
                        case 3: deleteProduct(); break;
                        case 4: searchProduct(); break;
                        case 5: displayAllProducts(); break;
                        case 0: break;
                        default: cout << "无效选择！" << endl;
                        }
                        if (pc != 0) {
                            cout << "\n按任意键继续...";
                            cin.ignore();
                            cin.get();
                        }
                    } while (pc != 0);
                }
                break;
            case 2:
                if (isLoggedIn) createOrder();
                break;
            case 3:
                if (isLoggedIn) checkStockAlert();
                break;
            case 4:
                if (isLoggedIn) salesRanking();
                break;
            case 5:
                if (isLoggedIn) logout();
                break;
            case 0:
                cout << "\n感谢使用！" << endl;
                break;
            default:
                cout << "\n无效选择！" << endl;
            }
            if (choice != 0) {
                cout << "\n按任意键继续...";
                cin.ignore();
                cin.get();
            }
        } while (choice != 0);
    }
};

int main() {
    ShopSystem shop;
    if (!shop.init()) {
        system("pause");
        return 1;
    }
    shop.run();
    return 0;
}
