#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_ID 20
#define MAX_NAME 50
#define MAX_CATEGORY 30
#define MAX_SUPPLIER 50
#define DATA_FILE "goods.txt"

typedef struct Goods {
    char id[MAX_ID];
    char name[MAX_NAME];
    char category[MAX_CATEGORY];
    float buy_price;
    float sell_price;
    int stock;
    char supplier[MAX_SUPPLIER];
    struct Goods *next;
} Goods;

Goods *head = NULL;

void clearInput() {
    while (getchar() != '\n');
}

Goods* createGoods() {
    Goods *p = (Goods*)malloc(sizeof(Goods));
    if (p == NULL) {
        printf("内存分配失败！\n");
        return NULL;
    }
    p->next = NULL;
    return p;
}

int isIdExists(const char *id) {
    Goods *p = head;
    while (p != NULL) {
        if (strcmp(p->id, id) == 0) {
            return 1;
        }
        p = p->next;
    }
    return 0;
}

void freeList() {
    Goods *p = head;
    Goods *temp;
    while (p != NULL) {
        temp = p;
        p = p->next;
        free(temp);
    }
    head = NULL;
}

void loadFromFile() {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        printf("数据文件不存在，将创建新文件\n");
        return;
    }
    
    Goods *p, *tail = NULL;
    char line[500];
    
    while (fgets(line, sizeof(line), fp) != NULL) {
        p = createGoods();
        sscanf(line, "%[^,],%[^,],%[^,],%f,%f,%d,%[^\n]",
               p->id, p->name, p->category, &p->buy_price, 
               &p->sell_price, &p->stock, p->supplier);
        
        if (head == NULL) {
            head = p;
        } else {
            tail->next = p;
        }
        tail = p;
    }
    fclose(fp);
    printf("数据加载成功\n");
}

void saveToFile() {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        printf("文件打开失败！\n");
        return;
    }
    
    Goods *p = head;
    while (p != NULL) {
        fprintf(fp, "%s,%s,%s,%.2f,%.2f,%d,%s\n",
                p->id, p->name, p->category, p->buy_price,
                p->sell_price, p->stock, p->supplier);
        p = p->next;
    }
    fclose(fp);
    printf("数据保存成功\n");
}

void addGoods() {
    Goods *p = createGoods();
    if (p == NULL) return;
    
    printf("请输入商品编号：");
    scanf("%s", p->id);
    clearInput();
    
    if (isIdExists(p->id)) {
        printf("该商品编号已存在！\n");
        free(p);
        return;
    }
    
    printf("请输入商品名称：");
    fgets(p->name, MAX_NAME, stdin);
    p->name[strcspn(p->name, "\n")] = '\0';
    
    printf("请输入商品分类：");
    fgets(p->category, MAX_CATEGORY, stdin);
    p->category[strcspn(p->category, "\n")] = '\0';
    
    do {
        printf("请输入进货单价（大于0）：");
        scanf("%f", &p->buy_price);
        if (p->buy_price <= 0) {
            printf("进货单价必须大于0！\n");
        }
    } while (p->buy_price <= 0);
    
    do {
        printf("请输入售价（大于0）：");
        scanf("%f", &p->sell_price);
        if (p->sell_price <= 0) {
            printf("售价必须大于0！\n");
        }
    } while (p->sell_price <= 0);
    
    do {
        printf("请输入库存数量（大于等于0）：");
        scanf("%d", &p->stock);
        if (p->stock < 0) {
            printf("库存数量不能为负数！\n");
        }
    } while (p->stock < 0);
    clearInput();
    
    printf("请输入供应商：");
    fgets(p->supplier, MAX_SUPPLIER, stdin);
    p->supplier[strcspn(p->supplier, "\n")] = '\0';
    
    p->next = head;
    head = p;
    
    saveToFile();
    printf("商品添加成功！\n");
}

void searchById() {
    char id[MAX_ID];
    printf("请输入要查询的商品编号：");
    scanf("%s", id);
    clearInput();
    
    Goods *p = head;
    while (p != NULL) {
        if (strcmp(p->id, id) == 0) {
            printf("\n商品信息：\n");
            printf("编号：%s\n", p->id);
            printf("名称：%s\n", p->name);
            printf("分类：%s\n", p->category);
            printf("进货单价：%.2f\n", p->buy_price);
            printf("售价：%.2f\n", p->sell_price);
            printf("库存：%d\n", p->stock);
            printf("供应商：%s\n", p->supplier);
            return;
        }
        p = p->next;
    }
    printf("未找到编号为%s的商品！\n", id);
}

void searchByName() {
    char name[MAX_NAME];
    printf("请输入要查询的商品名称：");
    fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = '\0';
    
    Goods *p = head;
    int found = 0;
    
    while (p != NULL) {
        if (strstr(p->name, name) != NULL) {
            printf("\n商品信息：\n");
            printf("编号：%s\n", p->id);
            printf("名称：%s\n", p->name);
            printf("分类：%s\n", p->category);
            printf("进货单价：%.2f\n", p->buy_price);
            printf("售价：%.2f\n", p->sell_price);
            printf("库存：%d\n", p->stock);
            printf("供应商：%s\n", p->supplier);
            found = 1;
        }
        p = p->next;
    }
    
    if (!found) {
        printf("未找到包含'%s'的商品！\n", name);
    }
}

void searchByCategory() {
    char category[MAX_CATEGORY];
    printf("请输入要查询的商品分类：");
    fgets(category, MAX_CATEGORY, stdin);
    category[strcspn(category, "\n")] = '\0';
    
    Goods *p = head;
    int found = 0;
    
    while (p != NULL) {
        if (strcmp(p->category, category) == 0) {
            printf("\n商品信息：\n");
            printf("编号：%s\n", p->id);
            printf("名称：%s\n", p->name);
            printf("分类：%s\n", p->category);
            printf("进货单价：%.2f\n", p->buy_price);
            printf("售价：%.2f\n", p->sell_price);
            printf("库存：%d\n", p->stock);
            printf("供应商：%s\n", p->supplier);
            found = 1;
        }
        p = p->next;
    }
    
    if (!found) {
        printf("未找到分类为'%s'的商品！\n", category);
    }
}

void modifyGoods() {
    char id[MAX_ID];
    printf("请输入要修改的商品编号：");
    scanf("%s", id);
    clearInput();
    
    Goods *p = head;
    while (p != NULL) {
        if (strcmp(p->id, id) == 0) {
            printf("请输入新的商品名称（当前：%s）：", p->name);
            fgets(p->name, MAX_NAME, stdin);
            p->name[strcspn(p->name, "\n")] = '\0';
            
            printf("请输入新的商品分类（当前：%s）：", p->category);
            fgets(p->category, MAX_CATEGORY, stdin);
            p->category[strcspn(p->category, "\n")] = '\0';
            
            do {
                printf("请输入新的进货单价（当前：%.2f）：", p->buy_price);
                scanf("%f", &p->buy_price);
                if (p->buy_price <= 0) {
                    printf("进货单价必须大于0！\n");
                }
            } while (p->buy_price <= 0);
            
            do {
                printf("请输入新的售价（当前：%.2f）：", p->sell_price);
                scanf("%f", &p->sell_price);
                if (p->sell_price <= 0) {
                    printf("售价必须大于0！\n");
                }
            } while (p->sell_price <= 0);
            
            do {
                printf("请输入新的库存数量（当前：%d）：", p->stock);
                scanf("%d", &p->stock);
                if (p->stock < 0) {
                    printf("库存数量不能为负数！\n");
                }
            } while (p->stock < 0);
            clearInput();
            
            printf("请输入新的供应商（当前：%s）：", p->supplier);
            fgets(p->supplier, MAX_SUPPLIER, stdin);
            p->supplier[strcspn(p->supplier, "\n")] = '\0';
            
            saveToFile();
            printf("商品修改成功！\n");
            return;
        }
        p = p->next;
    }
    printf("未找到编号为%s的商品！\n", id);
}

void deleteGoods() {
    char id[MAX_ID];
    printf("请输入要删除的商品编号：");
    scanf("%s", id);
    clearInput();
    
    Goods *p = head, *prev = NULL;
    while (p != NULL) {
        if (strcmp(p->id, id) == 0) {
            if (prev == NULL) {
                head = p->next;
            } else {
                prev->next = p->next;
            }
            free(p);
            saveToFile();
            printf("商品删除成功！\n");
            return;
        }
        prev = p;
        p = p->next;
    }
    printf("未找到编号为%s的商品！\n", id);
}

void statisticsByCategory() {
    printf("\n按分类统计：\n");
    printf("----------------------------------------\n");
    
    Goods *p = head;
    char categories[100][MAX_CATEGORY];
    int counts[100] = {0};
    int catCount = 0;
    
    while (p != NULL) {
        int i;
        for (i = 0; i < catCount; i++) {
            if (strcmp(categories[i], p->category) == 0) {
                counts[i]++;
                break;
            }
        }
        if (i == catCount && catCount < 100) {
            strcpy(categories[catCount], p->category);
            counts[catCount] = 1;
            catCount++;
        }
        p = p->next;
    }
    
    for (int i = 0; i < catCount; i++) {
        printf("分类：%s\t\t数量：%d\n", categories[i], counts[i]);
    }
    printf("----------------------------------------\n");
}

void statisticsLowStock() {
    int threshold;
    do {
        printf("请输入库存不足阈值（大于0）：");
        scanf("%d", &threshold);
        if (threshold <= 0) {
            printf("阈值必须大于0！\n");
        }
    } while (threshold <= 0);
    clearInput();
    
    printf("\n库存不足商品列表（库存<%d）：\n", threshold);
    printf("----------------------------------------\n");
    
    Goods *p = head;
    int found = 0;
    
    while (p != NULL) {
        if (p->stock < threshold) {
            printf("编号：%s\t名称：%s\t库存：%d\n", 
                   p->id, p->name, p->stock);
            found = 1;
        }
        p = p->next;
    }
    
    if (!found) {
        printf("没有库存不足的商品\n");
    }
    printf("----------------------------------------\n");
}

void statisticsTotalValue() {
    float totalValue = 0;
    int totalCount = 0;
    
    Goods *p = head;
    while (p != NULL) {
        totalValue += p->sell_price * p->stock;
        totalCount += p->stock;
        p = p->next;
    }
    
    printf("\n库存统计：\n");
    printf("----------------------------------------\n");
    printf("库存总件数：%d\n", totalCount);
    printf("库存总金额：%.2f\n", totalValue);
    printf("----------------------------------------\n");
}

void displayAllGoods() {
    if (head == NULL) {
        printf("暂无商品数据！\n");
        return;
    }
    
    printf("\n商品库存清单：\n");
    printf("--------------------------------------------------------------------------------------------------------\n");
    printf("| %-15s | %-20s | %-12s | %-10s | %-8s | %-6s | %-20s |\n", 
           "编号", "名称", "分类", "进货单价", "售价", "库存", "供应商");
    printf("--------------------------------------------------------------------------------------------------------\n");
    
    Goods *p = head;
    while (p != NULL) {
        printf("| %-15s | %-20s | %-12s | %-10.2f | %-8.2f | %-6d | %-20s |\n", 
               p->id, p->name, p->category, p->buy_price, 
               p->sell_price, p->stock, p->supplier);
        p = p->next;
    }
    printf("--------------------------------------------------------------------------------------------------------\n");
}

void menu() {
    printf("\n=======================================\n");
    printf("      商品库存管理系统\n");
    printf("=======================================\n");
    printf("  1. 录入商品信息\n");
    printf("  2. 查询商品信息\n");
    printf("  3. 修改商品信息\n");
    printf("  4. 删除商品信息\n");
    printf("  5. 统计商品信息\n");
    printf("  6. 显示全部商品\n");
    printf("  7. 退出系统\n");
    printf("=======================================\n");
    printf("请输入选择（1-7）：");
}

void searchMenu() {
    printf("\n查询方式：\n");
    printf("  1. 按编号查询\n");
    printf("  2. 按名称查询\n");
    printf("  3. 按分类查询\n");
    printf("请输入选择（1-3）：");
}

void statisticsMenu() {
    printf("\n统计方式：\n");
    printf("  1. 按分类统计\n");
    printf("  2. 库存不足统计\n");
    printf("  3. 库存总金额统计\n");
    printf("请输入选择（1-3）：");
}

int main() {
    loadFromFile();
    
    int choice;
    while (1) {
        menu();
        scanf("%d", &choice);
        clearInput();
        
        switch (choice) {
            case 1:
                addGoods();
                break;
            case 2: {
                int searchChoice;
                searchMenu();
                scanf("%d", &searchChoice);
                clearInput();
                switch (searchChoice) {
                    case 1: searchById(); break;
                    case 2: searchByName(); break;
                    case 3: searchByCategory(); break;
                    default: printf("无效选择！\n");
                }
                break;
            }
            case 3:
                modifyGoods();
                break;
            case 4:
                deleteGoods();
                break;
            case 5: {
                int statChoice;
                statisticsMenu();
                scanf("%d", &statChoice);
                clearInput();
                switch (statChoice) {
                    case 1: statisticsByCategory(); break;
                    case 2: statisticsLowStock(); break;
                    case 3: statisticsTotalValue(); break;
                    default: printf("无效选择！\n");
                }
                break;
            }
            case 6:
                displayAllGoods();
                break;
            case 7:
                saveToFile();
                freeList();
                printf("系统退出，数据已保存！\n");
                return 0;
            default:
                printf("无效选择，请输入1-7！\n");
        }
        
        printf("\n按回车键继续...");
        getchar();
    }
    
    return 0;
}