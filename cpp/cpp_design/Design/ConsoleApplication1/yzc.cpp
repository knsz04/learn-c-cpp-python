#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstring>
#include <windows.h>
#include "cJSON.h"

using namespace std;

string gbkToUtf8(const string& gbkStr) {
    int len = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* utf8Str = new char[len];
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, utf8Str, len, NULL, NULL);
    string result(utf8Str);
    delete[] wstr;
    delete[] utf8Str;
    return result;
}

string utf8ToGbk(const string& utf8Str) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* gbkStr = new char[len];
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, gbkStr, len, NULL, NULL);
    string result(gbkStr);
    delete[] wstr;
    delete[] gbkStr;
    return result;
}

class BusSchedule {
private:
    string scheduleNo;
    string departureTime;
    string startStation;
    string endStation;
    string travelTime;
    int capacity;
    int bookedCount;

public:
    BusSchedule() : capacity(0), bookedCount(0) {}
    
    BusSchedule(string no, string dt, string ss, string es, string tt, int cap, int bc)
        : scheduleNo(no), departureTime(dt), startStation(ss), 
          endStation(es), travelTime(tt), capacity(cap), bookedCount(bc) {}

    string getScheduleNo() const { return scheduleNo; }
    string getDepartureTime() const { return departureTime; }
    string getStartStation() const { return startStation; }
    string getEndStation() const { return endStation; }
    string getTravelTime() const { return travelTime; }
    int getCapacity() const { return capacity; }
    int getBookedCount() const { return bookedCount; }
    void setBookedCount(int count) { bookedCount = count; }

    bool isDeparted() const {
        if (departureTime.empty()) return false;
        
        time_t now = time(0);
        struct tm* nowTm = localtime(&now);
        char nowStr[20];
        strftime(nowStr, 20, "%Y-%m-%d %H:%M", nowTm);
        
        return strcmp(departureTime.c_str(), nowStr) <= 0;
    }

    bool canSellTicket() const {
        return !isDeparted() && bookedCount < capacity;
    }

    bool canRefundTicket() const {
        return !isDeparted() && bookedCount > 0;
    }

    bool sellTicket() {
        if (canSellTicket()) {
            bookedCount++;
            return true;
        }
        return false;
    }

    bool refundTicket() {
        if (canRefundTicket()) {
            bookedCount--;
            return true;
        }
        return false;
    }

    cJSON* toJSON() const {
        cJSON* obj = cJSON_CreateObject();
        cJSON_AddStringToObject(obj, "scheduleNo", scheduleNo.c_str());
        cJSON_AddStringToObject(obj, "departureTime", departureTime.c_str());
        cJSON_AddStringToObject(obj, "startStation", gbkToUtf8(startStation).c_str());
        cJSON_AddStringToObject(obj, "endStation", gbkToUtf8(endStation).c_str());
        cJSON_AddStringToObject(obj, "travelTime", gbkToUtf8(travelTime).c_str());
        cJSON_AddNumberToObject(obj, "capacity", (double)capacity);
        cJSON_AddNumberToObject(obj, "bookedCount", (double)bookedCount);
        return obj;
    }

    static BusSchedule fromJSON(cJSON* json) {
        BusSchedule bs;
        cJSON* item = NULL;
        
        item = cJSON_GetObjectItem(json, "scheduleNo");
        if (item && item->valuestring) bs.scheduleNo = item->valuestring;
        
        item = cJSON_GetObjectItem(json, "departureTime");
        if (item && item->valuestring) bs.departureTime = item->valuestring;
        
        item = cJSON_GetObjectItem(json, "startStation");
        if (item && item->valuestring) bs.startStation = utf8ToGbk(item->valuestring);
        
        item = cJSON_GetObjectItem(json, "endStation");
        if (item && item->valuestring) bs.endStation = utf8ToGbk(item->valuestring);
        
        item = cJSON_GetObjectItem(json, "travelTime");
        if (item && item->valuestring) bs.travelTime = utf8ToGbk(item->valuestring);
        
        item = cJSON_GetObjectItem(json, "capacity");
        if (item) bs.capacity = (int)item->valuedouble;
        
        item = cJSON_GetObjectItem(json, "bookedCount");
        if (item) bs.bookedCount = (int)item->valuedouble;
        
        return bs;
    }

    void display() const {
        cout << "\n班次号：" << scheduleNo << endl;
        cout << "发车时间：" << departureTime << endl;
        cout << "起点站：" << startStation << endl;
        cout << "终点站：" << endStation << endl;
        cout << "行车时间：" << travelTime << endl;
        cout << "额定载量：" << capacity << "人" << endl;
        cout << "已定票人数：" << bookedCount << "人" << endl;
        cout << "剩余座位：" << (capacity - bookedCount) << "人" << endl;
        if (isDeparted()) {
            cout << "状态：【此班已发出】" << endl;
        } else {
            cout << "状态：【未发车】" << endl;
        }
    }
};

class BusSystem {
private:
    vector<BusSchedule> schedules;
    string filename;

    bool loadFromFile() {
        ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        stringstream buffer;
        buffer << file.rdbuf();
        string content = buffer.str();
        file.close();

        if (content.empty()) {
            return true;
        }

        cJSON* root = cJSON_Parse(content.c_str());
        if (!root) {
            return false;
        }

        cJSON* array = cJSON_GetObjectItem(root, "schedules");
        if (array && array->type == cJSON_Array) {
            for (cJSON* item = array->child; item; item = item->next) {
                BusSchedule bs = BusSchedule::fromJSON(item);
                schedules.push_back(bs);
            }
        }

        cJSON_Delete(root);
        return true;
    }

    bool saveToFile() {
        cJSON* root = cJSON_CreateObject();
        cJSON* array = cJSON_CreateArray();

        for (size_t i = 0; i < schedules.size(); i++) {
            cJSON_AddItemToArray(array, schedules[i].toJSON());
        }

        cJSON_AddItemToObject(root, "schedules", array);
        char* jsonStr = cJSON_Print(root);

        ofstream file(filename);
        if (!file.is_open()) {
            free(jsonStr);
            cJSON_Delete(root);
            return false;
        }

        file << jsonStr;
        file.close();

        free(jsonStr);
        cJSON_Delete(root);
        return true;
    }

public:
    BusSystem(string fn) : filename(fn) {
        loadFromFile();
    }

    ~BusSystem() {
        saveToFile();
    }

    bool addSchedule(const BusSchedule& bs) {
        for (size_t i = 0; i < schedules.size(); i++) {
            if (schedules[i].getScheduleNo() == bs.getScheduleNo()) {
                return false;
            }
        }
        schedules.push_back(bs);
        saveToFile();
        return true;
    }

    void displayAll() const {
        if (schedules.empty()) {
            cout << "\n暂无班次信息" << endl;
            return;
        }

        cout << "\n========================================" << endl;
        cout << "         所有班次信息                   " << endl;
        cout << "========================================" << endl;
        
        for (size_t i = 0; i < schedules.size(); i++) {
            cout << "\n【班次 " << (i + 1) << "】" << endl;
            schedules[i].display();
            cout << "----------------------------------------" << endl;
        }
    }

    BusSchedule* findByScheduleNo(string no) {
        for (size_t i = 0; i < schedules.size(); i++) {
            if (schedules[i].getScheduleNo() == no) {
                return &schedules[i];
            }
        }
        return NULL;
    }

    vector<BusSchedule> findByEndStation(string station) {
        vector<BusSchedule> result;
        for (size_t i = 0; i < schedules.size(); i++) {
            if (schedules[i].getEndStation() == station) {
                result.push_back(schedules[i]);
            }
        }
        return result;
    }

    bool sellTicket(string no) {
        BusSchedule* bs = findByScheduleNo(no);
        if (!bs) return false;
        if (bs->sellTicket()) {
            saveToFile();
            return true;
        }
        return false;
    }

    bool refundTicket(string no) {
        BusSchedule* bs = findByScheduleNo(no);
        if (!bs) return false;
        if (bs->refundTicket()) {
            saveToFile();
            return true;
        }
        return false;
    }
};

void showMenu() {
    cout << "\n========================================" << endl;
    cout << "        客运站班次管理系统              " << endl;
    cout << "========================================" << endl;
    cout << "  1. 录入班次信息                       " << endl;
    cout << "  2. 浏览班次信息                       " << endl;
    cout << "  3. 按班次号查询                       " << endl;
    cout << "  4. 按终点站查询                       " << endl;
    cout << "  5. 售票                               " << endl;
    cout << "  6. 退票                               " << endl;
    cout << "  7. 退出系统                           " << endl;
    cout << "========================================" << endl;
    cout << "请输入选择(1-7): ";
}

int main() {
    cout << "========================================" << endl;
    cout << "    欢迎使用客运站班次管理系统          " << endl;
    cout << "    C++ 面向对象程序设计课程设计        " << endl;
    cout << "    使用 cJSON 库进行 JSON 文件读写     " << endl;
    cout << "========================================" << endl;

    BusSystem system("bus_schedules.json");
    int choice;

    do {
        showMenu();
        cin >> choice;

        switch (choice) {
            case 1: {
                cout << "\n=== 录入班次信息 ===" << endl;
                string no, dt, ss, es, tt;
                int cap, bc = 0;

                cout << "请输入班次号: ";
                cin >> no;

                cout << "请输入发车时间(格式:YYYY-MM-DD HH:MM): ";
                cin.ignore();
                getline(cin, dt);

                cout << "请输入起点站: ";
                cin >> ss;

                cout << "请输入终点站: ";
                cin >> es;

                cout << "请输入行车时间(如:4小时30分): ";
                cin >> tt;

                cout << "请输入额定载量: ";
                cin >> cap;

                cout << "请输入已定票人数(默认0): ";
                cin >> bc;

                BusSchedule bs(no, dt, ss, es, tt, cap, bc);
                if (system.addSchedule(bs)) {
                    cout << "\n班次录入成功！" << endl;
                } else {
                    cout << "\n录入失败：班次号已存在！" << endl;
                }
                break;
            }
            case 2:
                system.displayAll();
                break;
            case 3: {
                cout << "\n=== 按班次号查询 ===" << endl;
                string no;
                cout << "请输入班次号: ";
                cin >> no;

                BusSchedule* bs = system.findByScheduleNo(no);
                if (bs) {
                    bs->display();
                } else {
                    cout << "\n未找到班次: " << no << endl;
                }
                break;
            }
            case 4: {
                cout << "\n=== 按终点站查询 ===" << endl;
                string station;
                cout << "请输入终点站: ";
                cin >> station;

                vector<BusSchedule> results = system.findByEndStation(station);
                if (results.empty()) {
                    cout << "\n未找到终点站为 " << station << " 的班次" << endl;
                } else {
                    cout << "\n找到 " << results.size() << " 个班次:" << endl;
                    for (size_t i = 0; i < results.size(); i++) {
                        cout << "\n【班次 " << (i + 1) << "】" << endl;
                        results[i].display();
                    }
                }
                break;
            }
            case 5: {
                cout << "\n=== 售票 ===" << endl;
                string no;
                cout << "请输入要售票的班次号: ";
                cin >> no;

                BusSchedule* bs = system.findByScheduleNo(no);
                if (!bs) {
                    cout << "\n售票失败：班次不存在！" << endl;
                } else if (bs->isDeparted()) {
                    cout << "\n售票失败：此班已发出！" << endl;
                } else if (bs->getBookedCount() >= bs->getCapacity()) {
                    cout << "\n售票失败：已满员！" << endl;
                } else if (system.sellTicket(no)) {
                    cout << "\n售票成功！当前已售票: " << bs->getBookedCount() << "人" << endl;
                } else {
                    cout << "\n售票失败！" << endl;
                }
                break;
            }
            case 6: {
                cout << "\n=== 退票 ===" << endl;
                string no;
                cout << "请输入要退票的班次号: ";
                cin >> no;

                BusSchedule* bs = system.findByScheduleNo(no);
                if (!bs) {
                    cout << "\n退票失败：班次不存在！" << endl;
                } else if (bs->isDeparted()) {
                    cout << "\n退票失败：此班已发出！" << endl;
                } else if (bs->getBookedCount() <= 0) {
                    cout << "\n退票失败：无票可退！" << endl;
                } else if (system.refundTicket(no)) {
                    cout << "\n退票成功！当前已售票: " << bs->getBookedCount() << "人" << endl;
                } else {
                    cout << "\n退票失败！" << endl;
                }
                break;
            }
            case 7:
                cout << "\n感谢使用，再见！" << endl;
                break;
            default:
                cout << "\n无效选择，请重新输入！" << endl;
        }

        if (choice != 7) {
            cout << "\n按任意键继续...";
            cin.ignore();
            cin.get();
        }

    } while (choice != 7);

    return 0;
}