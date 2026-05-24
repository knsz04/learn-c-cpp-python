#include<stdio.h>

typedef struct PolyNode
{
    int xiang;
    int mi;
}Poly;

 typedef struct PNode
{
    Poly data[10];
}PNode;

PNode CreatPoly()
{
    PNode n;
    for(int i=0;i<10;i++)
    {
       n.data[i].mi = i;
       printf("请输入多项式第%d项的系数\n",i+1);
       scanf("%d",&n.data[i].xiang);
    }
    return n;
    
}
PNode MergePoly(PNode a,PNode b)
{
    PNode c;
    for(int i=0;i<10;i++)
    {
        c.data[i].mi = i;
        c.data[i].xiang = a.data[i].xiang + b.data[i].xiang;
    }
    return c;
}

int PrintPoly(PNode n)
{
    for(int i=0;i<10;i++)
    {
        if(n.data[i].xiang != 0  )
        {
            printf("%dx^%d ",n.data[i].xiang,n.data[i].mi);
        }
    }
    printf("\n");
    return 0;
}
    

int main()
{
    PNode poly1, poly2, poly3;
    printf("请输入第一个多项式的系数\n");
    poly1 = CreatPoly();
    printf("请输入第二个多项式的系数\n");
    poly2 = CreatPoly();
    poly3 = MergePoly(poly1, poly2);
    printf("合并后的多项式为：\n");
    PrintPoly(poly3);
    return 0;
}