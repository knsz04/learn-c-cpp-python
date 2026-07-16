﻿//﻿﻿﻿#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#undef RED
#undef BLACK
#include <graphics.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <shop_core.h>

using namespace std;

const int WINDOW_WIDTH = 950;
const int WINDOW_HEIGHT = 650;

const COLORREF COLOR_TITLE = RGB(52, 73, 94);
const COLORREF COLOR_TEXT_WHITE = RGB(255, 255, 255);
const COLORREF COLOR_BG = RGB(236, 240, 241);
const COLORREF COLOR_CARD = RGB(255, 255, 255);
const COLORREF COLOR_TEXT = RGB(44, 62, 80);
const COLORREF COLOR_INPUT_BG = RGB(248, 249, 250);
const COLORREF COLOR_INPUT_BORDER = RGB(200, 200, 200);
const COLORREF COLOR_INPUT_FOCUS = RGB(52, 152, 219);
const COLORREF COLOR_SHADOW = RGB(180, 180, 180);

struct Button {
    int x, y, w, h;
    wstring text;
    bool hover;
};

struct InputField {
    int x, y, w, h;
    wstring text;
    bool focused;
    bool password;
};

class GUIShopSystem {
private:
    HashTable productTable;
    unordered_map<string, User> users;
    DatabaseManager db;
    MinHeap stockAlertHeap;
    RBTree salesRBTree;
    CircularQueue orderQueue;
    Graph productGraph;
    PriceTrendAnalyzer trendAnalyzer;
    User currentUser;
    bool isLoggedIn = false;
    int currentPage = 0;

    void drawButton(Button& btn) {
        if (btn.hover) {
            setfillcolor(RGB(41, 128, 185));
        } else {
            setfillcolor(RGB(52, 152, 219));
        }
        fillroundrect(btn.x + 2, btn.y + 2, btn.x + btn.w + 2, btn.y + btn.h + 2, 8, 8);
        setfillcolor(btn.hover ? RGB(52, 152, 219) : RGB(41, 128, 185));
        fillroundrect(btn.x, btn.y, btn.x + btn.w, btn.y + btn.h, 8, 8);
        settextcolor(COLOR_TEXT_WHITE);
        settextstyle(18, 0, L"微软雅黑");
        setbkmode(TRANSPARENT);
        int textW = textwidth(btn.text.c_str());
        int textH = textheight(btn.text.c_str());
        outtextxy(btn.x + (btn.w - textW) / 2, btn.y + (btn.h - textH) / 2, btn.text.c_str());
    }

    void drawInputField(InputField& input) {
        setfillcolor(input.focused ? RGB(255, 255, 255) : COLOR_INPUT_BG);
        fillrectangle(input.x, input.y, input.x + input.w, input.y + input.h);
        setlinecolor(input.focused ? COLOR_INPUT_FOCUS : COLOR_INPUT_BORDER);
        rectangle(input.x, input.y, input.x + input.w, input.y + input.h);
        settextcolor(COLOR_TEXT);
        settextstyle(16, 0, L"微软雅黑");
        setbkmode(TRANSPARENT);
        
        wstring displayText = input.password ? wstring(input.text.size(), L'*') : input.text;
        outtextxy(input.x + 10, input.y + (input.h - textheight(L"")) / 2, displayText.c_str());
        
        if (input.focused && (GetTickCount() / 500) % 2 == 0) {
            int cursorX = input.x + 10 + textwidth(displayText.c_str());
            line(cursorX, input.y + 5, cursorX, input.y + input.h - 5);
        }
    }

    void showMessage(const wstring& msg) {
        vector<wstring> lines;
        wstring line;
        for (size_t i = 0; i < msg.size(); i++) {
            if (msg[i] == L'\n') {
                lines.push_back(line);
                line.clear();
            } else {
                line += msg[i];
            }
        }
        if (!line.empty()) lines.push_back(line);

        int textH = 24;
        int lineSpacing = 32;
        int boxH = max(150, (int)(lines.size() * lineSpacing + 100));
        int boxW = 450;
        int startY = WINDOW_HEIGHT / 2 - boxH / 2;

        BeginBatchDraw();
        
        setfillcolor(COLOR_SHADOW);
        fillroundrect(WINDOW_WIDTH / 2 - boxW / 2 + 5, startY + 5, WINDOW_WIDTH / 2 + boxW / 2 + 5, startY + boxH + 5, 12, 12);
        
        setfillcolor(RGB(255, 255, 255));
        fillroundrect(WINDOW_WIDTH / 2 - boxW / 2, startY, WINDOW_WIDTH / 2 + boxW / 2, startY + boxH, 12, 12);
        
        setlinecolor(RGB(200, 200, 200));
        setbkmode(TRANSPARENT);
        settextcolor(COLOR_TEXT);
        settextstyle(18, 0, L"微软雅黑");

        int currentY = startY + 30;

        for (size_t i = 0; i < lines.size(); i++) {
            int textW = textwidth(lines[i].c_str());
            outtextxy(WINDOW_WIDTH / 2 - textW / 2, currentY, lines[i].c_str());
            currentY += lineSpacing;
        }

        Button okBtn = { WINDOW_WIDTH / 2 - 60, startY + boxH - 50, 120, 40, L"确定", false };
        drawButton(okBtn);
        
        FlushBatchDraw();

        while (true) {
            ExMessage m = getmessage(EX_MOUSE);
            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= okBtn.x && m.x <= okBtn.x + okBtn.w &&
                    m.y >= okBtn.y && m.y <= okBtn.y + okBtn.h) {
                    BeginBatchDraw();
                    setfillcolor(COLOR_BG);
                    fillrectangle(WINDOW_WIDTH / 2 - boxW / 2, startY, WINDOW_WIDTH / 2 + boxW / 2, startY + boxH);
                    FlushBatchDraw();
                    break;
                }
            }
            BeginBatchDraw();
            drawButton(okBtn);
            FlushBatchDraw();
        }
    }

    wstring getInput(const wstring& prompt, bool isPassword = false) {
        InputField input = { WINDOW_WIDTH / 2 - 150, WINDOW_HEIGHT / 2 - 20, 300, 40, L"", true, isPassword };
        
        BeginBatchDraw();
        setfillcolor(RGB(255, 255, 255));
        fillroundrect(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 60, WINDOW_WIDTH / 2 + 180, WINDOW_HEIGHT / 2 + 40, 12, 12);
        setlinecolor(RGB(200, 200, 200));
        settextcolor(COLOR_TEXT);
        settextstyle(16, 0, L"微软雅黑");
        outtextxy(WINDOW_WIDTH / 2 - textwidth(prompt.c_str()) / 2, WINDOW_HEIGHT / 2 - 45, prompt.c_str());
        drawInputField(input);
        FlushBatchDraw();

        while (true) {
            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);
            
            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= input.x && m.x <= input.x + input.w &&
                    m.y >= input.y && m.y <= input.y + input.h) {
                    input.focused = true;
                } else {
                    input.focused = false;
                }
            } else if (m.message == WM_CHAR) {
                if (!input.focused) continue;
                if (m.ch == '\r') {
                    BeginBatchDraw();
                    setfillcolor(COLOR_BG);
                    fillrectangle(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 60, WINDOW_WIDTH / 2 + 180, WINDOW_HEIGHT / 2 + 40);
                    FlushBatchDraw();
                    return input.text;
                } else if (m.ch == '\b') {
                    if (!input.text.empty()) {
                        input.text.pop_back();
                    }
                } else if (m.ch >= 32) {
                    if (input.text.size() < 50) {
                        input.text += (wchar_t)m.ch;
                    }
                }
            }

            BeginBatchDraw();
            setfillcolor(RGB(255, 255, 255));
            fillroundrect(WINDOW_WIDTH / 2 - 180, WINDOW_HEIGHT / 2 - 60, WINDOW_WIDTH / 2 + 180, WINDOW_HEIGHT / 2 + 40, 12, 12);
            setlinecolor(RGB(200, 200, 200));
            outtextxy(WINDOW_WIDTH / 2 - textwidth(prompt.c_str()) / 2, WINDOW_HEIGHT / 2 - 45, prompt.c_str());
            drawInputField(input);
            FlushBatchDraw();
        }
    }

    void drawHeader(const wstring& title) {
        setfillcolor(COLOR_TITLE);
        fillrectangle(0, 0, WINDOW_WIDTH, 65);
        
        setfillcolor(COLOR_TEXT_WHITE);
        setlinecolor(COLOR_TEXT_WHITE);
        setbkmode(TRANSPARENT);
        settextstyle(26, 0, L"微软雅黑");
        settextcolor(COLOR_TEXT_WHITE);
        
        int textW = textwidth(title.c_str());
        outtextxy((WINDOW_WIDTH - textW) / 2, 18, title.c_str());
        
        if (isLoggedIn) {
            settextstyle(14, 0, L"微软雅黑");
            wstring permStr = (currentUser.permission == 1 ? L"普通员工" : L"管理员");
            wstring userInfo = L"用户: " + toWString(currentUser.name) + L" | 权限: " + permStr;
            outtextxy(15, 45, userInfo.c_str());
        }
    }

    wstring toWString(const string& str) {
        int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
        wchar_t* buf = new wchar_t[len];
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buf, len);
        wstring result(buf);
        delete[] buf;
        return result;
    }

    string toString(const wstring& wstr) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        char* buf = new char[len];
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buf, len, NULL, NULL);
        string result(buf);
        delete[] buf;
        return result;
    }

    string generateOrderId() {
        time_t t = time(NULL);
        struct tm* tm = localtime(&t);
        char buf[32];
        sprintf(buf, "ORD%04d%02d%02d%02d%02d%02d", 
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        return string(buf);
    }

    string getCurrentTime() {
        time_t t = time(NULL);
        struct tm* tm = localtime(&t);
        char buf[32];
        sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", 
                tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                tm->tm_hour, tm->tm_min, tm->tm_sec);
        return string(buf);
    }

    bool checkPermission(int level) {
        if (!isLoggedIn) {
            showMessage(L"请先登录！");
            return false;
        }
        if (currentUser.permission < level) {
            showMessage(L"权限不足！");
            return false;
        }
        return true;
    }

    void drawPageLogin() {
        Button loginBtn = { WINDOW_WIDTH / 2 - 130, 360, 260, 50, L"登 录", false };
        Button exitBtn = { WINDOW_WIDTH / 2 - 130, 430, 260, 50, L"退 出", false };

        InputField userIdInput = { WINDOW_WIDTH / 2 - 160, 190, 320, 50, L"", false, false };
        InputField passwordInput = { WINDOW_WIDTH / 2 - 160, 270, 320, 50, L"", false, true };

        bool userIdFocused = true;
        bool needRedraw = true;

        while (currentPage == 0) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"超市管理系统 - 登录");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);
                
                setfillcolor(RGB(180, 180, 180));
                fillroundrect(WINDOW_WIDTH / 2 - 220 + 10, 150 + 10, WINDOW_WIDTH / 2 + 220 + 10, 510 + 10, 18, 18);
                
                setfillcolor(COLOR_CARD);
                fillroundrect(WINDOW_WIDTH / 2 - 220, 150, WINDOW_WIDTH / 2 + 220, 510, 18, 18);
                
                settextcolor(COLOR_TEXT);
                settextstyle(18, 0, L"微软雅黑");
                outtextxy(WINDOW_WIDTH / 2 - 160, 170, L"用户ID:");
                outtextxy(WINDOW_WIDTH / 2 - 160, 250, L"密 码:");

                userIdInput.focused = userIdFocused;
                passwordInput.focused = !userIdFocused;

                drawInputField(userIdInput);
                drawInputField(passwordInput);
                drawButton(loginBtn);
                drawButton(exitBtn);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= loginBtn.x && m.x <= loginBtn.x + loginBtn.w &&
                    m.y >= loginBtn.y && m.y <= loginBtn.y + loginBtn.h) {
                    string userId = toString(userIdInput.text);
                    string password = toString(passwordInput.text);
                    
                    auto it = users.find(userId);
                    if (it != users.end() && it->second.password == password) {
                        currentUser = it->second;
                        if (currentUser.userId == "U001") {
                            currentUser.permission = 0;
                        }
                        isLoggedIn = true;
                        currentPage = 1;
                        return;
                    } else {
                        showMessage(L"用户名或密码错误！");
                        userIdInput.text.clear();
                        passwordInput.text.clear();
                        needRedraw = true;
                    }
                } else if (m.x >= exitBtn.x && m.x <= exitBtn.x + exitBtn.w &&
                           m.y >= exitBtn.y && m.y <= exitBtn.y + exitBtn.h) {
                    currentPage = -1;
                    return;
                } else if (m.x >= userIdInput.x && m.x <= userIdInput.x + userIdInput.w &&
                           m.y >= userIdInput.y && m.y <= userIdInput.y + userIdInput.h) {
                    userIdFocused = true;
                    needRedraw = true;
                } else if (m.x >= passwordInput.x && m.x <= passwordInput.x + passwordInput.w &&
                           m.y >= passwordInput.y && m.y <= passwordInput.y + passwordInput.h) {
                    userIdFocused = false;
                    needRedraw = true;
                }
            } else if (m.message == WM_CHAR) {
                if (userIdFocused) {
                    if (m.ch == '\r') {
                        userIdFocused = false;
                        needRedraw = true;
                    } else if (m.ch == '\b') {
                        if (!userIdInput.text.empty()) userIdInput.text.pop_back();
                        needRedraw = true;
                    } else if (m.ch >= 32 && m.ch <= 126) {
                        if (userIdInput.text.size() < 20) {
                            userIdInput.text += (wchar_t)m.ch;
                            needRedraw = true;
                        }
                    }
                } else if (passwordInput.focused) {
                    if (m.ch == '\r') {
                        string userId = toString(userIdInput.text);
                        string password = toString(passwordInput.text);
                        
                        auto it = users.find(userId);
                        if (it != users.end() && it->second.password == password) {
                            currentUser = it->second;
                            isLoggedIn = true;
                            currentPage = 1;
                            return;
                        } else {
                            showMessage(L"用户名或密码错误！");
                            userIdInput.text.clear();
                            passwordInput.text.clear();
                            userIdFocused = true;
                            needRedraw = true;
                        }
                    } else if (m.ch == '\b') {
                        if (!passwordInput.text.empty()) passwordInput.text.pop_back();
                        needRedraw = true;
                    } else if (m.ch >= 32 && m.ch <= 126) {
                        if (passwordInput.text.size() < 20) {
                            passwordInput.text += (wchar_t)m.ch;
                            needRedraw = true;
                        }
                    }
                }
            } else if (m.message == WM_MOUSEMOVE) {
                bool newLoginHover = (m.x >= loginBtn.x && m.x <= loginBtn.x + loginBtn.w &&
                                     m.y >= loginBtn.y && m.y <= loginBtn.y + loginBtn.h);
                bool newExitHover = (m.x >= exitBtn.x && m.x <= exitBtn.x + exitBtn.w &&
                                     m.y >= exitBtn.y && m.y <= exitBtn.y + exitBtn.h);
                
                if (loginBtn.hover != newLoginHover || exitBtn.hover != newExitHover) {
                    loginBtn.hover = newLoginHover;
                    exitBtn.hover = newExitHover;
                    needRedraw = true;
                }
            }
        }
    }

    void drawPageMain() {
        Button btnProduct = { 80, 130, 170, 70, L"商品管理", false };
        Button btnOrder = { 280, 130, 170, 70, L"创建订单", false };
        Button btnStock = { 480, 130, 170, 70, L"库存预警", false };
        Button btnSales = { 680, 130, 170, 70, L"销售排行", false };
        Button btnOrderQuery = { 80, 230, 170, 70, L"订单查询", false };
        Button btnUserManage = { 280, 230, 170, 70, L"用户管理", false };
        Button btnRecommendation = { 480, 230, 170, 70, L"关联推荐", false };
        Button btnPriceTrend = { 680, 230, 170, 70, L"价格趋势", false };
        Button btnLogout = { 660, 560, 120, 40, L"退出登录", false };
        Button btnExit = { 800, 560, 120, 40, L"退出程序", false };

        bool needRedraw = true;

        while (currentPage == 1) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"超市管理系统");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                drawButton(btnProduct);
                drawButton(btnOrder);
                drawButton(btnStock);
                drawButton(btnSales);
                drawButton(btnOrderQuery);
                drawButton(btnUserManage);
                drawButton(btnRecommendation);
                drawButton(btnPriceTrend);
                drawButton(btnLogout);
                drawButton(btnExit);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnProduct.x && m.x <= btnProduct.x + btnProduct.w &&
                    m.y >= btnProduct.y && m.y <= btnProduct.y + btnProduct.h) {
                    if (checkPermission(1)) currentPage = 2;
                    return;
                } else if (m.x >= btnOrder.x && m.x <= btnOrder.x + btnOrder.w &&
                           m.y >= btnOrder.y && m.y <= btnOrder.y + btnOrder.h) {
                    if (checkPermission(1)) currentPage = 3;
                    return;
                } else if (m.x >= btnStock.x && m.x <= btnStock.x + btnStock.w &&
                           m.y >= btnStock.y && m.y <= btnStock.y + btnStock.h) {
                    if (checkPermission(1)) currentPage = 4;
                    return;
                } else if (m.x >= btnSales.x && m.x <= btnSales.x + btnSales.w &&
                           m.y >= btnSales.y && m.y <= btnSales.y + btnSales.h) {
                    if (checkPermission(1)) currentPage = 5;
                    return;
                } else if (m.x >= btnOrderQuery.x && m.x <= btnOrderQuery.x + btnOrderQuery.w &&
                           m.y >= btnOrderQuery.y && m.y <= btnOrderQuery.y + btnOrderQuery.h) {
                    if (checkPermission(1)) currentPage = 6;
                    return;
                } else if (m.x >= btnUserManage.x && m.x <= btnUserManage.x + btnUserManage.w &&
                           m.y >= btnUserManage.y && m.y <= btnUserManage.y + btnUserManage.h) {
                    if (currentUser.permission != 1) {
                        currentPage = 7;
                    } else {
                        showMessage(L"只有管理员才能访问用户管理！");
                    }
                    return;
                } else if (m.x >= btnRecommendation.x && m.x <= btnRecommendation.x + btnRecommendation.w &&
                           m.y >= btnRecommendation.y && m.y <= btnRecommendation.y + btnRecommendation.h) {
                    currentPage = 8;
                    return;
                } else if (m.x >= btnPriceTrend.x && m.x <= btnPriceTrend.x + btnPriceTrend.w &&
                           m.y >= btnPriceTrend.y && m.y <= btnPriceTrend.y + btnPriceTrend.h) {
                    currentPage = 9;
                    return;
                } else if (m.x >= btnLogout.x && m.x <= btnLogout.x + btnLogout.w &&
                           m.y >= btnLogout.y && m.y <= btnLogout.y + btnLogout.h) {
                    isLoggedIn = false;
                    currentPage = 0;
                    return;
                } else if (m.x >= btnExit.x && m.x <= btnExit.x + btnExit.w &&
                           m.y >= btnExit.y && m.y <= btnExit.y + btnExit.h) {
                    currentPage = -1;
                    return;
                }
            } else if (m.message == WM_MOUSEMOVE) {
                bool newProductHover = (m.x >= btnProduct.x && m.x <= btnProduct.x + btnProduct.w &&
                                       m.y >= btnProduct.y && m.y <= btnProduct.y + btnProduct.h);
                bool newOrderHover = (m.x >= btnOrder.x && m.x <= btnOrder.x + btnOrder.w &&
                                      m.y >= btnOrder.y && m.y <= btnOrder.y + btnOrder.h);
                bool newStockHover = (m.x >= btnStock.x && m.x <= btnStock.x + btnStock.w &&
                                      m.y >= btnStock.y && m.y <= btnStock.y + btnStock.h);
                bool newSalesHover = (m.x >= btnSales.x && m.x <= btnSales.x + btnSales.w &&
                                      m.y >= btnSales.y && m.y <= btnSales.y + btnSales.h);
                bool newOrderQueryHover = (m.x >= btnOrderQuery.x && m.x <= btnOrderQuery.x + btnOrderQuery.w &&
                                           m.y >= btnOrderQuery.y && m.y <= btnOrderQuery.y + btnOrderQuery.h);
                bool newUserManageHover = (m.x >= btnUserManage.x && m.x <= btnUserManage.x + btnUserManage.w &&
                                           m.y >= btnUserManage.y && m.y <= btnUserManage.y + btnUserManage.h);
                bool newRecommendationHover = (m.x >= btnRecommendation.x && m.x <= btnRecommendation.x + btnRecommendation.w &&
                                                m.y >= btnRecommendation.y && m.y <= btnRecommendation.y + btnRecommendation.h);
                bool newPriceTrendHover = (m.x >= btnPriceTrend.x && m.x <= btnPriceTrend.x + btnPriceTrend.w &&
                                            m.y >= btnPriceTrend.y && m.y <= btnPriceTrend.y + btnPriceTrend.h);
                bool newLogoutHover = (m.x >= btnLogout.x && m.x <= btnLogout.x + btnLogout.w &&
                                       m.y >= btnLogout.y && m.y <= btnLogout.y + btnLogout.h);
                bool newExitHover = (m.x >= btnExit.x && m.x <= btnExit.x + btnExit.w &&
                                     m.y >= btnExit.y && m.y <= btnExit.y + btnExit.h);
                
                if (btnProduct.hover != newProductHover || btnOrder.hover != newOrderHover ||
                    btnStock.hover != newStockHover || btnSales.hover != newSalesHover ||
                    btnOrderQuery.hover != newOrderQueryHover || btnUserManage.hover != newUserManageHover ||
                    btnRecommendation.hover != newRecommendationHover || btnPriceTrend.hover != newPriceTrendHover || btnLogout.hover != newLogoutHover || btnExit.hover != newExitHover) {
                    btnProduct.hover = newProductHover;
                    btnOrder.hover = newOrderHover;
                    btnStock.hover = newStockHover;
                    btnSales.hover = newSalesHover;
                    btnOrderQuery.hover = newOrderQueryHover;
                    btnUserManage.hover = newUserManageHover;
                    btnRecommendation.hover = newRecommendationHover;
                    btnPriceTrend.hover = newPriceTrendHover;
                    btnLogout.hover = newLogoutHover;
                    btnExit.hover = newExitHover;
                    needRedraw = true;
                }
            }
        }
    }

    void drawPageProduct() {
        Button btnAdd = { 50, 120, 140, 45, L"新增", false };
        Button btnUpdate = { 210, 120, 140, 45, L"修改", false };
        Button btnDelete = { 370, 120, 140, 45, L"删除", false };
        Button btnSearch = { 530, 120, 140, 45, L"查询", false };
        Button btnList = { 690, 120, 140, 45, L"全部列表", false };
        Button btnBack = { 50, 570, 120, 40, L"返回主菜单", false };

        bool needRedraw = true;

        while (currentPage == 2) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"商品管理");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                drawButton(btnAdd);
                drawButton(btnUpdate);
                drawButton(btnDelete);
                drawButton(btnSearch);
                drawButton(btnList);
                drawButton(btnBack);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnAdd.x && m.x <= btnAdd.x + btnAdd.w &&
                    m.y >= btnAdd.y && m.y <= btnAdd.y + btnAdd.h) {
                    if (checkPermission(2)) addProductGUI();
                    needRedraw = true;
                } else if (m.x >= btnUpdate.x && m.x <= btnUpdate.x + btnUpdate.w &&
                           m.y >= btnUpdate.y && m.y <= btnUpdate.y + btnUpdate.h) {
                    if (checkPermission(2)) updateProductGUI();
                    needRedraw = true;
                } else if (m.x >= btnDelete.x && m.x <= btnDelete.x + btnDelete.w &&
                           m.y >= btnDelete.y && m.y <= btnDelete.y + btnDelete.h) {
                    if (checkPermission(2)) deleteProductGUI();
                    needRedraw = true;
                } else if (m.x >= btnSearch.x && m.x <= btnSearch.x + btnSearch.w &&
                           m.y >= btnSearch.y && m.y <= btnSearch.y + btnSearch.h) {
                    if (checkPermission(1)) searchProductGUI();
                    needRedraw = true;
                } else if (m.x >= btnList.x && m.x <= btnList.x + btnList.w &&
                           m.y >= btnList.y && m.y <= btnList.y + btnList.h) {
                    if (checkPermission(1)) displayAllProductsGUI();
                    needRedraw = true;
                } else if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                           m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                }
            } else if (m.message == WM_MOUSEMOVE) {
                bool newAddHover = (m.x >= btnAdd.x && m.x <= btnAdd.x + btnAdd.w &&
                                   m.y >= btnAdd.y && m.y <= btnAdd.y + btnAdd.h);
                bool newUpdateHover = (m.x >= btnUpdate.x && m.x <= btnUpdate.x + btnUpdate.w &&
                                      m.y >= btnUpdate.y && m.y <= btnUpdate.y + btnUpdate.h);
                bool newDeleteHover = (m.x >= btnDelete.x && m.x <= btnDelete.x + btnDelete.w &&
                                      m.y >= btnDelete.y && m.y <= btnDelete.y + btnDelete.h);
                bool newSearchHover = (m.x >= btnSearch.x && m.x <= btnSearch.x + btnSearch.w &&
                                       m.y >= btnSearch.y && m.y <= btnSearch.y + btnSearch.h);
                bool newListHover = (m.x >= btnList.x && m.x <= btnList.x + btnList.w &&
                                     m.y >= btnList.y && m.y <= btnList.y + btnList.h);
                bool newBackHover = (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                                     m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h);
                
                if (btnAdd.hover != newAddHover || btnUpdate.hover != newUpdateHover ||
                    btnDelete.hover != newDeleteHover || btnSearch.hover != newSearchHover ||
                    btnList.hover != newListHover || btnBack.hover != newBackHover) {
                    btnAdd.hover = newAddHover;
                    btnUpdate.hover = newUpdateHover;
                    btnDelete.hover = newDeleteHover;
                    btnSearch.hover = newSearchHover;
                    btnList.hover = newListHover;
                    btnBack.hover = newBackHover;
                    needRedraw = true;
                }
            }
        }
    }

    void addProductGUI() {
        Product p;
        
        wstring skuW = getInput(L"请输入商品SKU:");
        if (skuW.empty()) return;
        p.sku = toString(skuW);

        wstring nameW = getInput(L"请输入商品名称:");
        if (nameW.empty()) return;
        p.name = toString(nameW);

        wstring categoryW = getInput(L"请输入分类ID:");
        p.categoryId = toString(categoryW);

        wstring purchasePriceW = getInput(L"请输入进货价格:");
        p.purchasePrice = _wtof(purchasePriceW.c_str());

        wstring salePriceW = getInput(L"请输入销售价格:");
        p.salePrice = _wtof(salePriceW.c_str());

        wstring stockW = getInput(L"请输入库存数量:");
        p.stock = _wtoi(stockW.c_str());

        wstring thresholdW = getInput(L"请输入预警阈值:");
        p.threshold = _wtoi(thresholdW.c_str());

        if (productTable.insert(p)) {
            db.saveProduct(p);
            stockAlertHeap.push(p.stock, p.sku);
            db.savePriceHistory(p.sku, "purchase", -1, p.purchasePrice, "add", currentUser.userId);
            db.savePriceHistory(p.sku, "sale", -1, p.salePrice, "add", currentUser.userId);
            showMessage(L"商品添加成功！");
        } else {
            showMessage(L"添加失败：SKU已存在！");
        }
    }

    void updateProductGUI() {
        wstring skuW = getInput(L"请输入要修改的SKU:");
        if (skuW.empty()) return;
        string sku = toString(skuW);

        Product* p = productTable.find(sku);
        if (!p) {
            showMessage(L"商品不存在！");
            return;
        }

        double oldPurchasePrice = p->purchasePrice;
        double oldSalePrice = p->salePrice;

        wstring name = getInput(L"名称(" + toWString(p->name) + L"):");
        if (!name.empty()) p->name = toString(name);

        wstring category = getInput(L"分类(" + toWString(p->categoryId) + L"):");
        if (!category.empty()) p->categoryId = toString(category);

        wstring priceStr = getInput(L"进货价格(" + toWString(to_string((int)p->purchasePrice)) + L"):");
        if (!priceStr.empty()) {
            double newPrice = _wtof(priceStr.c_str());
            if (newPrice != oldPurchasePrice) {
                db.savePriceHistory(sku, "purchase", oldPurchasePrice, newPrice, "modify", currentUser.userId);
            }
            p->purchasePrice = newPrice;
        }

        priceStr = getInput(L"销售价格(" + toWString(to_string((int)p->salePrice)) + L"):");
        if (!priceStr.empty()) {
            double newPrice = _wtof(priceStr.c_str());
            if (newPrice != oldSalePrice) {
                db.savePriceHistory(sku, "sale", oldSalePrice, newPrice, "modify", currentUser.userId);
            }
            p->salePrice = newPrice;
        }

        wstring stockStr = getInput(L"库存(" + toWString(to_string(p->stock)) + L"):");
        if (!stockStr.empty()) p->stock = _wtoi(stockStr.c_str());

        wstring thresholdStr = getInput(L"预警阈值(" + toWString(to_string(p->threshold)) + L"):");
        if (!thresholdStr.empty()) p->threshold = _wtoi(thresholdStr.c_str());

        db.saveProduct(*p);
        showMessage(L"商品修改成功！");
    }

    void deleteProductGUI() {
        wstring skuW = getInput(L"请输入要删除的SKU:");
        if (skuW.empty()) return;
        string sku = toString(skuW);

        Product* p = productTable.find(sku);
        if (!p) {
            showMessage(L"商品不存在！");
            return;
        }

        if (productTable.remove(sku)) {
            db.deleteProduct(sku);
            showMessage(L"商品删除成功！");
        } else {
            showMessage(L"删除失败！");
        }
    }

    void searchProductGUI() {
        wstring skuW = getInput(L"请输入商品SKU:");
        if (skuW.empty()) return;
        string sku = toString(skuW);

        Product* p = productTable.find(sku);
        if (!p) {
            showMessage(L"商品不存在！");
            return;
        }

        wstring info = L"SKU: " + toWString(p->sku) + L"\n名称: " + toWString(p->name) + 
                       L"\n分类: " + toWString(p->categoryId) + L"\n进货价格: " + toWString(to_string((int)p->purchasePrice));
        info += L"\n销售价格: " + toWString(to_string((int)p->salePrice));
        info += L"\n库存: " + toWString(to_string(p->stock));
        info += L"\n预警阈值: " + toWString(to_string(p->threshold));
        
        showMessage(info);
    }

    void displayAllProductsGUI() {
        vector<Product> products = productTable.getAll();
        if (products.empty()) {
            showMessage(L"没有找到商品！");
            return;
        }

        Button btnBack = { 50, 570, 120, 40, L"返回", false };

        int startY = 100;
        int itemHeight = 75;
        int maxItems = 6;
        int curPage = 0;
        int totalPages = (int)((products.size() + maxItems - 1) / maxItems);

        bool needRedraw = true;

        while (true) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"商品列表");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                int startIdx = curPage * maxItems;
                int endIdx = min(startIdx + maxItems, (int)products.size());

                for (int i = startIdx; i < endIdx; i++) {
                    int y = startY + (i - startIdx) * itemHeight;
                    setfillcolor(COLOR_CARD);
                    fillroundrect(50, y, WINDOW_WIDTH - 50, y + itemHeight - 8, 8, 8);
                    setlinecolor(RGB(200, 200, 200));

                    settextcolor(COLOR_TEXT);
                    settextstyle(15, 0, L"微软雅黑");
                    outtextxy(70, y + 25, (L"SKU: " + toWString(products[i].sku)).c_str());
                    outtextxy(260, y + 25, (L"名称: " + toWString(products[i].name)).c_str());
                    outtextxy(480, y + 25, (L"价格: " + toWString(to_string((int)products[i].salePrice))).c_str());
                    outtextxy(620, y + 25, (L"库存: " + toWString(to_string(products[i].stock))).c_str());
                }

                drawButton(btnBack);

                if (totalPages > 1) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    wstring pageStr = toWString(to_string(curPage + 1)) + L" / " + toWString(to_string(totalPages));
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(pageStr.c_str()) / 2, 585, pageStr.c_str());
                }

                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    return;
                } else if (m.x >= WINDOW_WIDTH / 2 - 100 && m.x <= WINDOW_WIDTH / 2 - 20 &&
                           m.y >= 580 && m.y <= 600 && curPage > 0) {
                    curPage--;
                    needRedraw = true;
                } else if (m.x >= WINDOW_WIDTH / 2 + 20 && m.x <= WINDOW_WIDTH / 2 + 100 &&
                           m.y >= 580 && m.y <= 600 && curPage < totalPages - 1) {
                    curPage++;
                    needRedraw = true;
                }
            }
        }
    }

    void drawPageOrder() {
        Order order;
        order.orderId = generateOrderId();
        order.type = "Sale";
        order.createTime = getCurrentTime();
        order.operatorId = currentUser.userId;
        order.status = "Completed";
        order.totalAmount = 0;

        bool needRedraw = true;

        while (true) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"创建订单");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                settextcolor(COLOR_TEXT);
                settextstyle(16, 0, L"微软雅黑");
                outtextxy(60, 100, (L"订单号: " + toWString(order.orderId)).c_str());
                outtextxy(60, 135, (L"时间: " + toWString(order.createTime)).c_str());

                if (!order.items.empty()) {
                    int y = 185;
                    for (size_t i = 0; i < order.items.size(); i++) {
                        Product* p = productTable.find(order.items[i].sku);
                        outtextxy(60, y, (L"商品: " + toWString(p ? p->name : order.items[i].sku)).c_str());
                        outtextxy(340, y, (L"数量: " + toWString(to_string(order.items[i].quantity))).c_str());
                        outtextxy(490, y, (L"单价: " + toWString(to_string((int)order.items[i].unitPrice))).c_str());
                        outtextxy(640, y, (L"小计: " + toWString(to_string((int)(order.items[i].quantity * order.items[i].unitPrice)))).c_str());
                        y += 40;
                    }
                    outtextxy(60, y + 30, (L"总计: " + toWString(to_string((int)order.totalAmount))).c_str());
                }

                Button btnAddItem = { 60, 570, 150, 40, L"添加商品", false };
                Button btnFinish = { 230, 570, 150, 40, L"完成订单", false };
                Button btnCancel = { 400, 570, 150, 40, L"取消", false };

                drawButton(btnAddItem);
                drawButton(btnFinish);
                drawButton(btnCancel);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= 60 && m.x <= 210 && m.y >= 570 && m.y <= 610) {
                    wstring skuW = getInput(L"请输入商品SKU:");
                    if (skuW.empty()) { needRedraw = true; continue; }
                    string sku = toString(skuW);

                    Product* p = productTable.find(sku);
                    if (!p) {
                        showMessage(L"商品不存在！");
                        needRedraw = true;
                        continue;
                    }

                    wstring qtyStr = getInput(L"请输入数量(库存:" + toWString(to_string(p->stock)) + L"):");
                    int quantity = _wtoi(qtyStr.c_str());

                    if (quantity <= 0 || quantity > p->stock) {
                        showMessage(L"无效数量！");
                        needRedraw = true;
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
                    needRedraw = true;
                } else if (m.x >= 230 && m.x <= 380 && m.y >= 570 && m.y <= 610) {
                    if (order.items.empty()) {
                        showMessage(L"请至少添加一件商品！");
                        needRedraw = true;
                        continue;
                    }
                    orderQueue.enqueue(order);
                    db.saveOrder(order);
                    showMessage(L"订单创建成功！\n订单号: " + toWString(order.orderId) + L"\n总计: " + toWString(to_string((int)order.totalAmount)));
                    currentPage = 1;
                    return;
                } else if (m.x >= 400 && m.x <= 550 && m.y >= 570 && m.y <= 610) {
                    currentPage = 1;
                    return;
                }
            }
        }
    }

    void drawPageOrderQuery() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };
        Button btnSearch = { 600, 100, 100, 35, L"查询", false };

        InputField searchInput = { 120, 100, 460, 35, L"", false, false };
        
        int curPage = 0;
        const int pageSize = 4;
        vector<Order> allOrders;
        vector<Order> filteredOrders;
        bool needRedraw = true;

        while (currentPage == 6) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"订单查询");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                settextcolor(COLOR_TEXT);
                settextstyle(16, 0, L"微软雅黑");
                outtextxy(50, 108, L"订单号(查看详细):");
                
                drawInputField(searchInput);
                drawButton(btnSearch);

                if (allOrders.empty()) {
                    db.loadOrders(allOrders);
                }

                filteredOrders.clear();
                if (searchInput.text.empty()) {
                    filteredOrders = allOrders;
                } else {
                    string keyword = toString(searchInput.text);
                    for (size_t i = 0; i < allOrders.size(); i++) {
                        if (allOrders[i].orderId.find(keyword) != string::npos) {
                            filteredOrders.push_back(allOrders[i]);
                        }
                    }
                }

                int totalPages = ((int)filteredOrders.size() + pageSize - 1) / pageSize;
                if (totalPages < 1) totalPages = 1;
                if (curPage >= totalPages) curPage = totalPages - 1;

                int startIdx = curPage * pageSize;
                int endIdx = startIdx + pageSize;
                if (endIdx > (int)filteredOrders.size()) endIdx = (int)filteredOrders.size();

                int y = 160;
                for (int i = startIdx; i < endIdx; i++) {
                    Order& order = filteredOrders[i];
                    setfillcolor(COLOR_CARD);
                    fillroundrect(50, y, WINDOW_WIDTH - 50, y + 80, 8, 8);
                    
                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    outtextxy(70, y + 15, (L"订单号: " + toWString(order.orderId)).c_str());
                    outtextxy(70, y + 40, (L"时间: " + toWString(order.createTime)).c_str());
                    outtextxy(320, y + 40, (L"操作员: " + toWString(order.operatorId)).c_str());
                    outtextxy(520, y + 40, (L"类型: " + toWString(order.type)).c_str());
                    outtextxy(650, y + 40, (L"状态: " + toWString(order.status)).c_str());
                    outtextxy(780, y + 40, (L"总计: " + toWString(to_string((int)order.totalAmount))).c_str());
                    
                    y += 95;
                }

                if (filteredOrders.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(22, 0, L"微软雅黑");
                    if (searchInput.text.empty()) {
                        outtextxy(WINDOW_WIDTH / 2 - textwidth(L"暂无订单记录！") / 2, WINDOW_HEIGHT / 2, L"暂无订单记录！");
                    } else {
                        outtextxy(WINDOW_WIDTH / 2 - textwidth(L"未找到匹配的订单！") / 2, WINDOW_HEIGHT / 2, L"未找到匹配的订单！");
                    }
                }

                if (totalPages > 1) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    wstring pageStr = toWString(to_string(curPage + 1)) + L" / " + toWString(to_string(totalPages));
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(pageStr.c_str()) / 2, 585, pageStr.c_str());
                }

                drawButton(btnBack);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                } else if (m.x >= btnSearch.x && m.x <= btnSearch.x + btnSearch.w &&
                           m.y >= btnSearch.y && m.y <= btnSearch.y + btnSearch.h) {
                    if (!searchInput.text.empty()) {
                        string orderId = toString(searchInput.text);
                        Order order;
                        if (db.findOrder(orderId, order)) {
                            wstring detail = L"订单详情:\n";
                            detail += L"订单号: " + toWString(order.orderId) + L"\n";
                            detail += L"时间: " + toWString(order.createTime) + L"\n";
                            detail += L"操作员: " + toWString(order.operatorId) + L"\n";
                            detail += L"类型: " + toWString(order.type) + L"\n";
                            detail += L"状态: " + toWString(order.status) + L"\n";
                            detail += L"商品明细:\n";
                            for (size_t i = 0; i < order.items.size(); i++) {
                                Product* p = productTable.find(order.items[i].sku);
                                detail += L"  " + toWString(p ? p->name : order.items[i].sku) +
                                          L" x" + toWString(to_string(order.items[i].quantity)) +
                                          L" @ " + toWString(to_string((int)order.items[i].unitPrice)) + L"\n";
                            }
                            detail += L"总计: " + toWString(to_string((int)order.totalAmount));
                            showMessage(detail);
                        } else {
                            showMessage(L"订单不存在！");
                        }
                    }
                    needRedraw = true;
                } else if (m.x >= searchInput.x && m.x <= searchInput.x + searchInput.w &&
                           m.y >= searchInput.y && m.y <= searchInput.y + searchInput.h) {
                    searchInput.focused = true;
                    needRedraw = true;
                } else if (m.x >= WINDOW_WIDTH / 2 - 100 && m.x <= WINDOW_WIDTH / 2 - 20 &&
                           m.y >= 580 && m.y <= 600 && curPage > 0) {
                    curPage--;
                    needRedraw = true;
                } else if (m.x >= WINDOW_WIDTH / 2 + 20 && m.x <= WINDOW_WIDTH / 2 + 100 &&
                           m.y >= 580 && m.y <= 600 && curPage < ((int)filteredOrders.size() + pageSize - 1) / pageSize - 1) {
                    curPage++;
                    needRedraw = true;
                }
            } else if (m.message == WM_CHAR && searchInput.focused) {
                if (m.ch == '\r') {
                    searchInput.focused = false;
                    if (!searchInput.text.empty()) {
                        string orderId = toString(searchInput.text);
                        Order order;
                        if (db.findOrder(orderId, order)) {
                            wstring detail = L"订单详情:\n";
                            detail += L"订单号: " + toWString(order.orderId) + L"\n";
                            detail += L"时间: " + toWString(order.createTime) + L"\n";
                            detail += L"操作员: " + toWString(order.operatorId) + L"\n";
                            detail += L"类型: " + toWString(order.type) + L"\n";
                            detail += L"状态: " + toWString(order.status) + L"\n";
                            detail += L"商品明细:\n";
                            for (size_t i = 0; i < order.items.size(); i++) {
                                Product* p = productTable.find(order.items[i].sku);
                                detail += L"  " + toWString(p ? p->name : order.items[i].sku) +
                                          L" x" + toWString(to_string(order.items[i].quantity)) +
                                          L" @ " + toWString(to_string((int)order.items[i].unitPrice)) + L"\n";
                            }
                            detail += L"总计: " + toWString(to_string((int)order.totalAmount));
                            showMessage(detail);
                        } else {
                            showMessage(L"订单不存在！");
                        }
                    }
                    needRedraw = true;
                } else if (m.ch == '\b') {
                    if (!searchInput.text.empty()) searchInput.text.pop_back();
                    curPage = 0;
                    needRedraw = true;
                } else if (m.ch >= 32 && m.ch <= 126) {
                    if (searchInput.text.size() < 30) {
                        searchInput.text += (wchar_t)m.ch;
                        curPage = 0;
                        needRedraw = true;
                    }
                }
            }
        }
    }

    void drawPageUserManagement() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };
        Button btnAddUser = { 640, 100, 100, 35, L"添加用户", false };

        int curPage = 0;
        const int pageSize = 4;
        vector<User> userList;
        bool needRedraw = true;

        while (currentPage == 7) {
            if (needRedraw) {
                BeginBatchDraw();

                drawHeader(L"用户管理");

                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                settextcolor(COLOR_TEXT);
                settextstyle(16, 0, L"微软雅黑");
                outtextxy(50, 108, L"员工列表（仅管理员可见）:");

                drawButton(btnAddUser);

                userList.clear();
                for (auto& pair : users) {
                    userList.push_back(pair.second);
                }

                int totalPages = ((int)userList.size() + pageSize - 1) / pageSize;
                if (totalPages < 1) totalPages = 1;
                if (curPage >= totalPages) curPage = totalPages - 1;

                int startIdx = curPage * pageSize;
                int endIdx = startIdx + pageSize;
                if (endIdx > (int)userList.size()) endIdx = (int)userList.size();

                int y = 160;
                for (int i = startIdx; i < endIdx; i++) {
                    User& user = userList[i];
                    setfillcolor(COLOR_CARD);
                    fillroundrect(50, y, WINDOW_WIDTH - 50, y + 80, 8, 8);

                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    outtextxy(70, y + 15, (L"用户ID: " + toWString(user.userId)).c_str());
                    outtextxy(70, y + 40, (L"姓名: " + toWString(user.name)).c_str());
                    outtextxy(250, y + 40, (L"职位: " + toWString(user.position)).c_str());
                    wstring permStr = (user.permission != 1) ? L"管理员" : L"普通员工";
                    outtextxy(450, y + 40, (L"权限: " + permStr).c_str());

                    y += 95;
                }

                if (userList.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(22, 0, L"微软雅黑");
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(L"暂无用户！") / 2, WINDOW_HEIGHT / 2, L"暂无用户！");
                }

                if (totalPages > 1) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    wstring pageStr = toWString(to_string(curPage + 1)) + L" / " + toWString(to_string(totalPages));
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(pageStr.c_str()) / 2, 585, pageStr.c_str());
                }

                drawButton(btnBack);

                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                } else if (m.x >= btnAddUser.x && m.x <= btnAddUser.x + btnAddUser.w &&
                           m.y >= btnAddUser.y && m.y <= btnAddUser.y + btnAddUser.h) {
                    wstring userIdW = getInput(L"请输入新用户ID:");
                    if (!userIdW.empty()) {
                        string userId = toString(userIdW);
                        if (users.find(userId) != users.end()) {
                            showMessage(L"用户ID已存在！");
                        } else {
                            User newUser;
                            newUser.userId = userId;

                            wstring nameW = getInput(L"请输入姓名:");
                            if (nameW.empty()) { needRedraw = true; continue; }
                            newUser.name = toString(nameW);

                            wstring positionW = getInput(L"请输入职位:");
                            if (positionW.empty()) { needRedraw = true; continue; }
                            newUser.position = toString(positionW);

                            wstring phoneW = getInput(L"请输入电话:");
                            newUser.phone = toString(phoneW);

                            wstring passwordW = getInput(L"请输入密码:");
                            if (passwordW.empty()) { needRedraw = true; continue; }
                            newUser.password = toString(passwordW);

                            wstring permW = getInput(L"请输入权限(0=管理员,1=普通员工):");
                            newUser.permission = permW.empty() ? 1 : _wtoi(permW.c_str());

                            if (db.saveUser(newUser)) {
                                users[newUser.userId] = newUser;
                                showMessage(L"用户添加成功！");
                            } else {
                                showMessage(L"添加失败！");
                            }
                        }
                    }
                    needRedraw = true;
                } else if (m.x >= WINDOW_WIDTH / 2 - 100 && m.x <= WINDOW_WIDTH / 2 - 20 &&
                           m.y >= 580 && m.y <= 600 && curPage > 0) {
                    curPage--;
                    needRedraw = true;
                } else if (m.x >= WINDOW_WIDTH / 2 + 20 && m.x <= WINDOW_WIDTH / 2 + 100 &&
                           m.y >= 580 && m.y <= 600 && curPage < ((int)userList.size() + pageSize - 1) / pageSize - 1) {
                    curPage++;
                    needRedraw = true;
                }
            }
        }
    }

    void drawPageStockAlert() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };
        Button btnViewAll = { 640, 100, 120, 35, L"查看所有库存", false };

        bool needRedraw = true;
        bool showAllStock = false;

        while (true) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(showAllStock ? L"所有库存" : L"库存预警");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                vector<Product> displayProducts;
                vector<Product> allProducts = productTable.getAll();
                if (showAllStock) {
                    displayProducts = allProducts;
                } else {
                    for (size_t i = 0; i < allProducts.size(); i++) {
                        if (allProducts[i].stock <= allProducts[i].threshold) {
                            displayProducts.push_back(allProducts[i]);
                        }
                    }
                }

                drawButton(btnViewAll);

                if (displayProducts.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(22, 0, L"微软雅黑");
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(showAllStock ? L"暂无商品！" : L"暂无库存预警！") / 2, WINDOW_HEIGHT / 2, showAllStock ? L"暂无商品！" : L"暂无库存预警！");
                } else {
                    int y = 150;
                    for (size_t i = 0; i < displayProducts.size(); i++) {
                        setfillcolor(COLOR_CARD);
                        fillroundrect(50, y, WINDOW_WIDTH - 50, y + 60, 8, 8);
                        setlinecolor(RGB(200, 200, 200));

                        settextcolor(COLOR_TEXT);
                        settextstyle(15, 0, L"微软雅黑");
                        outtextxy(70, y + 22, (L"名称: " + toWString(displayProducts[i].name)).c_str());
                        outtextxy(380, y + 22, (L"SKU: " + toWString(displayProducts[i].sku)).c_str());
                        outtextxy(530, y + 22, (L"库存: " + toWString(to_string(displayProducts[i].stock))).c_str());
                        outtextxy(680, y + 22, (L"预警值: " + toWString(to_string(displayProducts[i].threshold))).c_str());
                        y += 70;
                    }
                }

                drawButton(btnBack);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                } else if (m.x >= btnViewAll.x && m.x <= btnViewAll.x + btnViewAll.w &&
                           m.y >= btnViewAll.y && m.y <= btnViewAll.y + btnViewAll.h) {
                    showAllStock = !showAllStock;
                    needRedraw = true;
                }
            }
        }
    }

    void drawPageRecommendation() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };
        Button btnSearch = { 700, 100, 80, 35, L"推荐", false };
        InputField skuInput = { 50, 100, 500, 35, L"", false, false };

        bool needRedraw = true;
        vector<pair<string, int>> recommendations;
        string selectedSku;

        while (currentPage == 8) {
            if (needRedraw) {
                BeginBatchDraw();

                drawHeader(L"商品关联推荐");

                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                settextcolor(COLOR_TEXT);
                settextstyle(16, 0, L"微软雅黑");
                outtextxy(50, 85, L"基于历史订单数据，使用图论(邻接矩阵+DFS)分析商品关联关系");

                outtextxy(50, 108, L"请输入商品SKU:");
                drawInputField(skuInput);
                drawButton(btnSearch);

                if (!recommendations.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(18, 0, L"微软雅黑");
                    outtextxy(50, 155, (L"与 [" + toWString(selectedSku) + L"] 关联的商品推荐:").c_str());

                    int y = 190;
                    for (size_t i = 0; i < recommendations.size() && i < 8; i++) {
                        string sku = recommendations[i].first;
                        int weight = recommendations[i].second;
                        Product* p = productTable.find(sku);

                        setfillcolor(COLOR_CARD);
                        fillroundrect(50, y, WINDOW_WIDTH - 50, y + 60, 8, 8);

                        settextcolor(COLOR_TEXT);
                        settextstyle(15, 0, L"微软雅黑");
                        outtextxy(70, y + 22, (L"名称: " + toWString(p ? p->name : sku)).c_str());
                        outtextxy(380, y + 22, (L"SKU: " + toWString(sku)).c_str());
                        outtextxy(550, y + 22, (L"关联度: " + toWString(to_string(weight)) + L"次").c_str());

                        y += 70;
                    }
                }

                drawButton(btnBack);

                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                } else if (m.x >= btnSearch.x && m.x <= btnSearch.x + btnSearch.w &&
                           m.y >= btnSearch.y && m.y <= btnSearch.y + btnSearch.h) {
                    string sku = toString(skuInput.text);
                    if (sku.empty()) {
                        showMessage(L"请输入商品SKU！");
                    } else if (!productTable.find(sku)) {
                        showMessage(L"商品不存在！");
                    } else {
                        selectedSku = sku;
                        recommendations = productGraph.getRecommendations(sku);
                        if (recommendations.empty()) {
                            showMessage(L"暂无关联推荐！");
                        }
                    }
                    needRedraw = true;
                } else if (m.x >= skuInput.x && m.x <= skuInput.x + skuInput.w &&
                           m.y >= skuInput.y && m.y <= skuInput.y + skuInput.h) {
                    skuInput.focused = true;
                    needRedraw = true;
                } else {
                    skuInput.focused = false;
                    needRedraw = true;
                }
            } else if (m.message == WM_CHAR && skuInput.focused) {
                if (m.ch == '\b' && !skuInput.text.empty()) {
                    skuInput.text.pop_back();
                } else if (m.ch >= 32 && m.ch <= 126) {
                    skuInput.text += (wchar_t)m.ch;
                } else if (m.ch == '\r') {
                    string sku = toString(skuInput.text);
                    if (!sku.empty()) {
                        selectedSku = sku;
                        recommendations = productGraph.getRecommendations(sku);
                    }
                }
                needRedraw = true;
            }
        }
    }

    void drawPagePriceTrend() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };
        Button btnSearch = { 700, 100, 80, 35, L"分析", false };
        InputField skuInput = { 50, 100, 500, 35, L"", false, false };

        bool needRedraw = true;
        vector<double> priceHistory;
        vector<double> predictions;
        vector<string> dates;
        string selectedSku;
        string productName;

        while (currentPage == 9) {
            if (needRedraw) {
                BeginBatchDraw();

                drawHeader(L"价格趋势分析");

                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                settextcolor(COLOR_TEXT);
                settextstyle(16, 0, L"微软雅黑");
                outtextxy(50, 85, L"基于动态规划预测模型（指数平滑+线性回归）分析历史价格趋势");

                outtextxy(50, 108, L"请输入商品SKU:");
                drawInputField(skuInput);
                drawButton(btnSearch);

                if (!priceHistory.empty()) {
                    int chartX = 80;
                    int chartY = 160;
                    int chartWidth = WINDOW_WIDTH - 160;
                    int chartHeight = 350;

                    setfillcolor(RGB(240, 240, 240));
                    fillrectangle(chartX, chartY, chartX + chartWidth, chartY + chartHeight);

                    setlinecolor(RGB(200, 200, 200));
                    for (int i = 0; i <= 5; i++) {
                        int y = chartY + (chartHeight / 5) * i;
                        line(chartX, y, chartX + chartWidth, y);
                    }
                    for (int i = 0; i <= 10; i++) {
                        int x = chartX + (chartWidth / 10) * i;
                        line(x, chartY, x, chartY + chartHeight);
                    }

                    double maxPrice = priceHistory[0];
                    double minPrice = priceHistory[0];
                    for (double p : priceHistory) {
                        if (p > maxPrice) maxPrice = p;
                        if (p < minPrice) minPrice = p;
                    }
                    for (double p : predictions) {
                        if (p > maxPrice) maxPrice = p;
                        if (p < minPrice) minPrice = p;
                    }
                    if (maxPrice == minPrice) {
                        maxPrice += 1;
                        minPrice -= 1;
                    }

                    double range = maxPrice - minPrice;
                    if (range == 0) range = 1;

                    settextcolor(COLOR_TEXT);
                    settextstyle(12, 0, L"微软雅黑");
                    for (int i = 0; i <= 5; i++) {
                        double price = maxPrice - (range / 5) * i;
                        int y = chartY + (chartHeight / 5) * i;
                        outtextxy(25, y - 8, toWString(to_string((int)price)).c_str());
                    }

                    setlinecolor(RGB(52, 152, 219));
                    setlinestyle(PS_SOLID, 2);
                    for (size_t i = 1; i < priceHistory.size(); i++) {
                        int x1 = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * (i - 1);
                        int y1 = chartY + chartHeight - ((priceHistory[i - 1] - minPrice) / range) * chartHeight;
                        int x2 = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * i;
                        int y2 = chartY + chartHeight - ((priceHistory[i] - minPrice) / range) * chartHeight;
                        line(x1, y1, x2, y2);
                    }

                    setlinecolor(RGB(231, 76, 60));
                    setlinestyle(PS_DASH, 2);
                    for (size_t i = 0; i < predictions.size(); i++) {
                        int x1 = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * (priceHistory.size() - 1 + i);
                        int y1 = chartY + chartHeight - ((i == 0 ? priceHistory.back() : predictions[i - 1]) - minPrice) / range * chartHeight;
                        int x2 = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * (priceHistory.size() + i);
                        int y2 = chartY + chartHeight - ((predictions[i] - minPrice) / range) * chartHeight;
                        line(x1, y1, x2, y2);
                    }
                    setlinestyle(PS_SOLID, 2);

                    settextcolor(RGB(52, 152, 219));
                    fillcircle(chartX, chartY + chartHeight - ((priceHistory[0] - minPrice) / range) * chartHeight, 4);
                    for (size_t i = 1; i < priceHistory.size(); i++) {
                        int x = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * i;
                        int y = chartY + chartHeight - ((priceHistory[i] - minPrice) / range) * chartHeight;
                        fillcircle(x, y, 4);
                    }

                    settextcolor(RGB(231, 76, 60));
                    for (size_t i = 0; i < predictions.size(); i++) {
                        int x = chartX + (chartWidth / (priceHistory.size() + predictions.size() - 1)) * (priceHistory.size() + i);
                        int y = chartY + chartHeight - ((predictions[i] - minPrice) / range) * chartHeight;
                        fillcircle(x, y, 4);
                    }

                    settextcolor(COLOR_TEXT);
                    settextstyle(14, 0, L"微软雅黑");
                    outtextxy(chartX, chartY + chartHeight + 20, L"历史价格（蓝色）");
                    outtextxy(chartX + 150, chartY + chartHeight + 20, L"预测价格（红色虚线）");

                    double volatility = trendAnalyzer.calculateVolatility(priceHistory);
                    auto [slope, intercept] = trendAnalyzer.linearRegression(priceHistory);

                    settextcolor(COLOR_TEXT);
                    settextstyle(16, 0, L"微软雅黑");
                    outtextxy(50, 540, (L"商品: " + toWString(productName) + L" (SKU: " + toWString(selectedSku) + L")").c_str());
                    outtextxy(50, 565, (L"波动率: " + toWString(to_string(volatility).substr(0, 5))).c_str());
                    outtextxy(250, 565, (L"趋势斜率: " + toWString(to_string(slope).substr(0, 6)) + L" (" + (slope > 0 ? L"上涨趋势)" : slope < 0 ? L"下跌趋势)" : L"平稳)")).c_str());
                    outtextxy(550, 565, (L"数据点: " + toWString(to_string(priceHistory.size())) + L" 个").c_str());
                }

                drawButton(btnBack);

                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE | EX_CHAR);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                } else if (m.x >= btnSearch.x && m.x <= btnSearch.x + btnSearch.w &&
                           m.y >= btnSearch.y && m.y <= btnSearch.y + btnSearch.h) {
                    string sku = toString(skuInput.text);
                    if (sku.empty()) {
                        showMessage(L"请输入商品SKU！");
                    } else {
                        Product* p = productTable.find(sku);
                        if (!p) {
                            showMessage(L"商品不存在！");
                        } else {
                            selectedSku = sku;
                            productName = p->name;

                            priceHistory.clear();
                            dates.clear();

                            vector<PriceHistory> historyRecords;
                            db.loadPriceHistory(sku, "sale", historyRecords);

                            for (const auto& h : historyRecords) {
                                priceHistory.push_back(h.newPrice);
                                dates.push_back(h.recordTime);
                            }

                            if (priceHistory.size() < 2) {
                                showMessage(L"历史价格数据不足，无法分析趋势！");
                                predictions.clear();
                            } else {
                                predictions = trendAnalyzer.predictLinear(priceHistory, 3);
                            }
                        }
                    }
                    needRedraw = true;
                } else if (m.x >= skuInput.x && m.x <= skuInput.x + skuInput.w &&
                           m.y >= skuInput.y && m.y <= skuInput.y + skuInput.h) {
                    skuInput.focused = true;
                    needRedraw = true;
                } else {
                    skuInput.focused = false;
                    needRedraw = true;
                }
            } else if (m.message == WM_CHAR && skuInput.focused) {
                if (m.ch == '\b' && !skuInput.text.empty()) {
                    skuInput.text.pop_back();
                } else if (m.ch >= 32 && m.ch <= 126) {
                    skuInput.text += (wchar_t)m.ch;
                } else if (m.ch == '\r') {
                    string sku = toString(skuInput.text);
                    if (!sku.empty()) {
                        Product* p = productTable.find(sku);
                        if (p) {
                            selectedSku = sku;
                            productName = p->name;
                            priceHistory.clear();
                            dates.clear();
                            vector<PriceHistory> historyRecords;
                            db.loadPriceHistory(sku, "sale", historyRecords);
                            for (const auto& h : historyRecords) {
                                priceHistory.push_back(h.newPrice);
                                dates.push_back(h.recordTime);
                            }
                            if (priceHistory.size() < 2) {
                                showMessage(L"历史价格数据不足，无法分析趋势！");
                                predictions.clear();
                            } else {
                                predictions = trendAnalyzer.predictLinear(priceHistory, 3);
                            }
                        }
                    }
                }
                needRedraw = true;
            }
        }
    }

    void drawPageSalesRanking() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };

        bool needRedraw = true;

        while (true) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"销售排行榜");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                unordered_map<string, SaleStat> statsMap;
                db.loadSalesStats(statsMap);

                if (statsMap.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(22, 0, L"微软雅黑");
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(L"暂无销售数据！") / 2, WINDOW_HEIGHT / 2, L"暂无销售数据！");
                } else {
                    RBTree tempRBTree;
                    for (auto& it : statsMap) {
                        tempRBTree.insert(it.second);
                    }

                    vector<SaleStat> sorted = tempRBTree.getSorted();
                    int topN = min((int)sorted.size(), 5);

                    int y = 100;
                    for (int i = (int)sorted.size() - 1; i >= (int)sorted.size() - topN; i--) {
                        int rank = (int)sorted.size() - i;
                        setfillcolor(COLOR_CARD);
                        fillroundrect(50, y, WINDOW_WIDTH - 50, y + 65, 8, 8);
                        setlinecolor(RGB(200, 200, 200));

                        settextcolor(COLOR_TEXT);
                        settextstyle(16, 0, L"微软雅黑");

                        COLORREF rankColor;
                        if (rank == 1) rankColor = RGB(241, 196, 15);
                        else if (rank == 2) rankColor = RGB(189, 195, 199);
                        else if (rank == 3) rankColor = RGB(211, 84, 0);
                        else rankColor = COLOR_TEXT;

                        settextcolor(rankColor);
                        outtextxy(70, y + 25, (L"第 " + toWString(to_string(rank)) + L" 名").c_str());

                        Product* p = productTable.find(sorted[i].sku);
                        settextcolor(COLOR_TEXT);
                        outtextxy(170, y + 25, (L"商品: " + toWString(p ? p->name : sorted[i].sku)).c_str());
                        outtextxy(460, y + 25, (L"销量: " + toWString(to_string(sorted[i].salesCount))).c_str());
                        outtextxy(610, y + 25, (L"销售额: " + toWString(to_string((int)sorted[i].salesAmount))).c_str());

                        int barWidth = (int)(sorted[i].salesAmount / sorted.back().salesAmount * 600);
                        setfillcolor(RGB(52, 152, 219));
                        fillrectangle(70, y + 45, 70 + barWidth, y + 55);

                        y += 80;
                    }
                }

                drawButton(btnBack);
                
                FlushBatchDraw();
                needRedraw = false;
            }

            ExMessage m = getmessage(EX_MOUSE);

            if (m.message == WM_LBUTTONDOWN) {
                if (m.x >= btnBack.x && m.x <= btnBack.x + btnBack.w &&
                    m.y >= btnBack.y && m.y <= btnBack.y + btnBack.h) {
                    currentPage = 1;
                    return;
                }
            }
        }
    }

public:
    bool init() {
        if (!db.connect()) {
            MessageBox(NULL, L"数据库连接失败！", L"错误", MB_OK);
            return false;
        }
        if (!db.loadProducts(productTable)) {
            MessageBox(NULL, L"加载商品失败！", L"错误", MB_OK);
            return false;
        }
        if (!db.loadUsers(users)) {
            MessageBox(NULL, L"加载用户失败！", L"错误", MB_OK);
            return false;
        }
        vector<Product> products = productTable.getAll();
        for (size_t i = 0; i < products.size(); i++) {
            stockAlertHeap.push(products[i].stock, products[i].sku);
        }
        vector<Order> orders;
        db.loadOrders(orders);
        productGraph.buildFromOrders(orders);
        return true;
    }

    void run() {
        initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
        _wsetlocale(LC_ALL, L"zh-CN");

        while (true) {
            switch (currentPage) {
                case 0: drawPageLogin(); break;
                case 1: drawPageMain(); break;
                case 2: drawPageProduct(); break;
                case 3: drawPageOrder(); break;
                case 4: drawPageStockAlert(); break;
                case 5: drawPageSalesRanking(); break;
                case 6: drawPageOrderQuery(); break;
                case 7: drawPageUserManagement(); break;
                case 8: drawPageRecommendation(); break;
                case 9: drawPagePriceTrend(); break;
                default: return;
            }
        }

        closegraph();
    }
};

int main() {
    _wsetlocale(LC_ALL, L"zh-CN");
    GUIShopSystem shop;
    if (!shop.init()) {
        return 1;
    }
    shop.run();
    return 0;
}