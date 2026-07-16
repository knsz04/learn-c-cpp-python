#include <graphics.h>		// 引用图形库头文件
#include <iostream>
int main()
{
	initgraph(1024, 648);	// 创建绘图窗口，大小为 640x480 像素
	setlinecolor(RGB(255, 179, 204));
	circle(512, 324, 200);	// 画圆，圆心(200, 200)，半径 100
	std::cin.get();				// 按任意键继续
	closegraph();			// 关闭绘图窗口
	return 0;
}
