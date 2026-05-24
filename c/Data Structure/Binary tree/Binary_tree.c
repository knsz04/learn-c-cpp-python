#include <stdio.h>
#include <stdlib.h>

typedef struct Node
{
    char data;
    struct Node* left;
    struct Node* right;
} Node,*Tree;

Tree CreateTree()
{
    Tree T;
    char x;
    printf("请输入节点的值，输入#表示结束：");
    scanf(" %c", &x);
    if (x == '#')
        return NULL;
    T = (Node*)malloc(sizeof(Node));
    if (T == NULL)
    {
        printf("内存分配失败！\n");
        return NULL;
    }
    T->data = x;
    printf("请输入%c的左子树：\n", x);
    T->left = CreateTree();
    printf("请输入%c的右子树：\n", x);
    T->right = CreateTree();
    return T;
}


void Front(Tree T)
{
    if (T != NULL)
    {
        printf("%c ", T->data);
        Front(T->left);
        Front(T->right);
    }
}

void Middle(Tree T)
{
    if (T != NULL)
    {
        Middle(T->left);
        printf("%c ", T->data);
        Middle(T->right);
    }
}   

void Back(Tree T)
{
    if (T != NULL)
    {
        Back(T->left);
        Back(T->right);
        printf("%c ", T->data);
    }
    
}


 void FreeTree(Tree T)
  {
      if (T != NULL)
      {
          FreeTree(T->left);
          FreeTree(T->right);
          free(T);
      }
  }

int main()
{
    Tree T = CreateTree();
    printf("先序遍历：");
    Front(T);
    printf("\n中序遍历：");
    Middle(T);
    printf("\n后序遍历：");
    Back(T);
    FreeTree(T);
    return 0;
}

