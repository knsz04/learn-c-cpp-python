#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXN 100
#define MAXCODE 100

typedef struct HuffmanNode {
    int weight;
    int parent;
    int left;
    int right;
} HuffmanNode;

void selectTwoMin(HuffmanNode tree[], int n, int *s1, int *s2) {
    int i;
    int min1 = -1, min2 = -1;
    for (i = 1; i <= n; i++) {
        if (tree[i].parent == 0) {
            if (min1 == -1 || tree[i].weight < tree[min1].weight) {
                min2 = min1;
                min1 = i;
            } else if (min2 == -1 || tree[i].weight < tree[min2].weight) {
                min2 = i;
            }
        }
    }
    *s1 = min1;
    *s2 = min2;
}

void buildHuffmanTree(HuffmanNode tree[], int weights[], int n) {
    int m = 2 * n - 1;
    int i;
    for (i = 1; i <= m; i++) {
        tree[i].weight = 0;
        tree[i].parent = 0;
        tree[i].left = 0;
        tree[i].right = 0;
    }
    for (i = 1; i <= n; i++) {
        tree[i].weight = weights[i];
    }
    for (i = n + 1; i <= m; i++) {
        int s1, s2;
        selectTwoMin(tree, i - 1, &s1, &s2);
        tree[s1].parent = i;
        tree[s2].parent = i;
        tree[i].left = s1;
        tree[i].right = s2;
        tree[i].weight = tree[s1].weight + tree[s2].weight;
    }
}

void generateHuffmanCodes(HuffmanNode tree[], int n, char codes[][MAXCODE]) {
    char code[MAXCODE];
    int i, current, parent, start;
    for (i = 1; i <= n; i++) {
        start = MAXCODE - 1;
        code[start] = '\0';
        current = i;
        parent = tree[current].parent;
        while (parent != 0) {
            if (tree[parent].left == current) {
                code[--start] = '0';
            } else {
                code[--start] = '1';
            }
            current = parent;
            parent = tree[current].parent;
        }
        strcpy(codes[i], &code[start]);
    }
}

int main(void) {
    int n, i;
    printf("请输入字符个数: ");
    if (scanf("%d", &n) != 1 || n <= 0 || n > MAXN) {
        printf("输入错误，字符个数应为 1 到 %d 之间的整数。\n", MAXN);
        return 1;
    }

    int weights[MAXN + 1];
    char symbols[MAXN + 1];
    printf("请输入每个字符及其权重（格式: 字符 权重），每行一个：\n");
    for (i = 1; i <= n; i++) {
        scanf(" %c %d", &symbols[i], &weights[i]);
    }

    HuffmanNode tree[2 * MAXN];
    char codes[MAXN + 1][MAXCODE];
    buildHuffmanTree(tree, weights, n);
    generateHuffmanCodes(tree, n, codes);

    printf("\n哈夫曼编码结果：\n");
    printf("字符\t权重\t编码\n");
    for (i = 1; i <= n; i++) {
        printf("%c\t%d\t%s\n", symbols[i], weights[i], codes[i]);
    }

    return 0;
}
