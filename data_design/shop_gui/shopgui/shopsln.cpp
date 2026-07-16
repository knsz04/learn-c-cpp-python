#define _CRT_SECURE_NO_WARNINGS
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
            wstring permStr = (currentUser.permission == 1 ? L"普通用户" : currentUser.permission == 2 ? L"管理员" : L"超级管理员");
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
        Button btnProduct = { 80, 130, 190, 80, L"商品管理", false };
        Button btnOrder = { 360, 130, 190, 80, L"创建订单", false };
        Button btnStock = { 640, 130, 190, 80, L"库存预警", false };
        Button btnSales = { 80, 270, 190, 80, L"销售排行", false };
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
                bool newLogoutHover = (m.x >= btnLogout.x && m.x <= btnLogout.x + btnLogout.w &&
                                       m.y >= btnLogout.y && m.y <= btnLogout.y + btnLogout.h);
                bool newExitHover = (m.x >= btnExit.x && m.x <= btnExit.x + btnExit.w &&
                                     m.y >= btnExit.y && m.y <= btnExit.y + btnExit.h);
                
                if (btnProduct.hover != newProductHover || btnOrder.hover != newOrderHover ||
                    btnStock.hover != newStockHover || btnSales.hover != newSalesHover ||
                    btnLogout.hover != newLogoutHover || btnExit.hover != newExitHover) {
                    btnProduct.hover = newProductHover;
                    btnOrder.hover = newOrderHover;
                    btnStock.hover = newStockHover;
                    btnSales.hover = newSalesHover;
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

        wstring name = getInput(L"名称(" + toWString(p->name) + L"):");
        if (!name.empty()) p->name = toString(name);

        wstring category = getInput(L"分类(" + toWString(p->categoryId) + L"):");
        if (!category.empty()) p->categoryId = toString(category);

        wstring priceStr = getInput(L"进货价格(" + toWString(to_string((int)p->purchasePrice)) + L"):");
        if (!priceStr.empty()) p->purchasePrice = _wtof(priceStr.c_str());

        priceStr = getInput(L"销售价格(" + toWString(to_string((int)p->salePrice)) + L"):");
        if (!priceStr.empty()) p->salePrice = _wtof(priceStr.c_str());

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

    void drawPageStockAlert() {
        Button btnBack = { 50, 570, 120, 40, L"返回", false };

        bool needRedraw = true;

        while (true) {
            if (needRedraw) {
                BeginBatchDraw();
                
                drawHeader(L"库存预警");
                
                setfillcolor(COLOR_BG);
                fillrectangle(0, 65, WINDOW_WIDTH, WINDOW_HEIGHT);

                vector<Product> alertProducts;
                vector<Product> allProducts = productTable.getAll();
                for (size_t i = 0; i < allProducts.size(); i++) {
                    if (allProducts[i].stock <= allProducts[i].threshold) {
                        alertProducts.push_back(allProducts[i]);
                    }
                }

                if (alertProducts.empty()) {
                    settextcolor(COLOR_TEXT);
                    settextstyle(22, 0, L"微软雅黑");
                    outtextxy(WINDOW_WIDTH / 2 - textwidth(L"暂无库存预警！") / 2, WINDOW_HEIGHT / 2, L"暂无库存预警！");
                } else {
                    int y = 100;
                    for (size_t i = 0; i < alertProducts.size(); i++) {
                        setfillcolor(COLOR_CARD);
                        fillroundrect(50, y, WINDOW_WIDTH - 50, y + 60, 8, 8);
                        setlinecolor(RGB(200, 200, 200));

                        settextcolor(COLOR_TEXT);
                        settextstyle(15, 0, L"微软雅黑");
                        outtextxy(70, y + 22, (L"名称: " + toWString(alertProducts[i].name)).c_str());
                        outtextxy(380, y + 22, (L"SKU: " + toWString(alertProducts[i].sku)).c_str());
                        outtextxy(530, y + 22, (L"库存: " + toWString(to_string(alertProducts[i].stock))).c_str());
                        outtextxy(680, y + 22, (L"预警值: " + toWString(to_string(alertProducts[i].threshold))).c_str());
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
                }
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