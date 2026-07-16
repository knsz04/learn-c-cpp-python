#include <stdlib.h>
#include <stdio.h>
#define STACK_SIZE 100
#define STACKINCREMENT 10

typedef struct 
{
    int *base;
    int *top;
    int stacksize;
} Stack;

int Initstack(Stack *s)
{
    s->base = (int *)malloc(STACK_SIZE * sizeof(int));
    if (!s->base) {
        printf("分配内存失败\n");
        return -1; 
    }
    s->top = s->base;
    s->stacksize = STACK_SIZE;
    return 0; 
}
 
int Push(Stack *s, int a)
{
    if(s->top - s->base >= s->stacksize)
    {
        s->base = (int *)realloc(s->base, (s->stacksize + STACKINCREMENT) * sizeof(int));
        if (!s->base) {
            printf("分配内存失败\n");
            return -1; 
        }
        s->top = s->base + s->stacksize;
        s->stacksize += STACKINCREMENT;
        
    }
    
        
        *(s->top) = a;
         s->top ++; 
        return 0;
    
}

int Pop(Stack *s)
{
    int a;
    if (s->top == s->base){
        printf("栈空\n");
        return -1; 
    }
    else {
        s->top--;
        a = *(s->top);
        return a;
    }
        
}

int RadixConvert(int n)
{
    Stack s;
    Initstack(&s);
    if (n == 0) 
    {
        printf("0");
        return 0;
    }
    while(n > 0)
    {
        Push(&s,n%8);
        n /= 8;
    }
    while(s.top != s.base)
    {
        int a;
        a = Pop(&s);
        printf("%d",a);
    }
    free(s.base);
    return 0;
}

int main()

{
    
    int n;
    printf("请输入一个十进制数：");
    scanf("%d",&n); 
    printf("转换成八进制数为：");
    RadixConvert(n);
    return 0;
    getchar();
    
    
}