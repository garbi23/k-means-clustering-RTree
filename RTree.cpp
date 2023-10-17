#include <ctime>
#include <iostream>
#include <vector>
#include <bits/stdc++.h>
#include "matplotlibcpp.h"
namespace plt = matplotlibcpp;
using namespace std;
const int child_size = 3; //자식 노드 수
int dataSize = 0;
const int initDataSize = 10000; //데이터 크기
const int maxEpoch = 10;

typedef struct Point {
    double x, y;
    int recId;
    double minDistance;

    Point() :
            x(0.0),
            y(0.0),
            recId(-1),
            minDistance(10000.0) {}

    double distance(Point p) {
        double dx = p.x - x;
        double dy = p.y - y;
        return abs(sqrt(dx * dx + dy * dy));
    }

    void copy(Point target){
        x = target.x;
        y = target.y;
        recId = -1;
        minDistance = 10000.0;
    }
};

typedef struct Node {
    double minX, minY, maxX, maxY;
    int recId;
    vector<int> childIndex;
    vector<Point> points;
    int parentIndex;
    int nowIndex;

    Node() :
            recId(-1.0),
            parentIndex(-1),
            nowIndex(-1),
            minX(std::numeric_limits<double>::max()),
            minY(std::numeric_limits<double>::max()),
            maxX(0),
            maxY(0){}
    void calcMiddlePoint(int x, int y){
        if(this->minX >= x){ this->minX = x;}
        if(this->minY >= y){ this->minY = y;}
        if(this->maxX <= x){ this->maxX = x;}
        if(this->maxY <= y){ this->maxY = y;}
    }

};

Node nodeChild[initDataSize];
int nodeIndex = 0;

unsigned short pseudo_rand() {
    static unsigned long long seed = 5;
    return (seed = seed * 25214903917ULL + 11ULL) >> 16;
}

void kMeansClustering(vector<Point>& data) {
    Point centroids[child_size];
    int poinSize = data.size();
    //처음 군집을 결정할 point를 가져옴
    for (int i = 0; i < child_size; i++){
        int index = pseudo_rand() % poinSize;
        centroids[i] = data[index];
    }
    int epoch = 0;
    while (epoch++ < maxEpoch) {
        for (int idx = 0; idx < poinSize; idx++) {
            for (int i = 0; i < child_size; i++) {
                double dist = centroids[i].distance(data[idx]);
                if (dist < data[idx].minDistance) {
                    data[idx].minDistance = dist; //군집 결정후 해당 군집의 point와의 거리로 mindDistance설정
                    data[idx].recId = i;
                }
            }
        }
        int point_cnt[child_size] = { 0, };
        double sum_x[child_size] = { 0, };
        double sum_y[child_size] = { 0, };

        for (int i = 0; i < poinSize; i++) {
            int recId = data[i].recId;
            point_cnt[recId] += 1;
            sum_x[recId] += data[i].x;
            sum_y[recId] += data[i].y;
            data[i].minDistance = 100000.0;
        }
        //epoch에 따라 해당 군집 중앙 값을 해당 군집 안에있는 모든 포인트의 중앙값으로 설정
        for (int i = 0; i < child_size; i++) {
            if(point_cnt[i] < 1) continue;
            centroids[i].x = sum_x[i] / point_cnt[i];
            centroids[i].y = sum_y[i] / point_cnt[i];
        }
    }
}

void Rtree(int parentIndex) {
    int nowIndex[child_size] = {0, };
    vector<Point> point = nodeChild[parentIndex].points;
    if (point.size() <= child_size) { return; }
    kMeansClustering(point);
    for (int i = 0; i < point.size(); i++) {
        int recId = point[i].recId;
        if(recId < 0) continue;
        if(nowIndex[recId] < 1){
            nowIndex[recId] = nodeIndex+1;
            nodeChild[nodeIndex].recId = recId;
            nodeChild[nodeIndex].parentIndex = parentIndex;
            nodeChild[nodeIndex].nowIndex = nodeIndex;
            nodeChild[parentIndex].childIndex.push_back(nodeIndex);
            nodeIndex++;
        }
        nodeChild[nowIndex[recId]-1].points.push_back(point[i]);
        nodeChild[nowIndex[recId]-1].calcMiddlePoint(point[i].x, point[i].y);
    }
    for (int i = 0; i < nodeChild[parentIndex].childIndex.size(); i++) {
        int childIndex = nodeChild[parentIndex].childIndex[i];
        Rtree(childIndex);
    }
}

double minDistance = 10000.0;
int searchNodeIndex = 0;
double absoluteDistance(int x, int y, int targetX, int targetY) {
    double dx = targetX - x;
    double dy = targetY - y;
    return abs(sqrt(dx * dx + dy * dy));
}

bool check(int index, int x,int y){
    int minX = nodeChild[index].minX;
    int minY = nodeChild[index].minY;
    int maxX = nodeChild[index].maxX;
    int maxY = nodeChild[index].maxY;
    if(maxX < x || minX > x) return false;
    if(maxY < y || minY > y) return false;
    searchNodeIndex = index;
    return true;
}


void searchChild(int searchIndex, int x, int y){
    double minDis = 0;
    int neatX = 1000000;
    int neatY = 1000000;
    for(auto point : nodeChild[searchIndex].points){
        double distance = absoluteDistance(point.x, point.y, x, y);
        if(minDis < distance){
            minDis = distance;
            neatX = point.x;
            neatY = point.y;
        }
    }
    cout << searchIndex << " { " << x << "," << y <<"}" << " ~ {" << neatX << "," << neatY <<"}" << endl;
}

void search(int parentIndex, int x,int y) {
    if(!check(parentIndex, x, y)) return;
    for (int i = 0; i < nodeChild[parentIndex].childIndex.size(); i++) {
        int childIndex = nodeChild[parentIndex].childIndex[i];
        return search(childIndex, x, y);
    }
}

vector<int> startNodeIndex;


void build(int targetDataSize){
    dataSize = targetDataSize;
    vector<Point> point;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 10000);
    vector<int> plotX;
    vector<int> plotY;
    for (int i = 0; i < dataSize; i++) {
        Point tempP;
        tempP.x = dist(gen);
        tempP.y = dist(gen);
        point.push_back(tempP);
        plotX.push_back(tempP.x);
        plotY.push_back(tempP.y);
    }

    //시작
    int nowIndex[child_size] = {0, };
    kMeansClustering(point);
    for (int i = 0; i < point.size(); i++) {
        int recId = point[i].recId;
        if(recId < 0) continue;
        if(nowIndex[recId] < 1){
            nowIndex[recId] = nodeIndex+1;
            nodeChild[nodeIndex].recId = recId;
            nodeChild[nodeIndex].parentIndex = -1;
            nodeChild[nodeIndex].nowIndex = nodeIndex;
            startNodeIndex.push_back(nodeIndex);
            nodeIndex++;
        }
        nodeChild[nowIndex[recId]-1].points.push_back(point[i]);
        nodeChild[nowIndex[recId]-1].calcMiddlePoint(point[i].x, point[i].y);
    }
    for (int i = 0; i < child_size; i ++) {
        if(nodeChild[i].recId < 0) continue;
        Rtree(nodeChild[i].nowIndex);
    }

    //search
    clock_t start, finish;
    double duration;

    start = clock();
    for(int i = 0; i < 100; i ++){
        searchNodeIndex = 0;
        int x = dist(gen);
        int y = dist(gen);
        for(int index : startNodeIndex){
            search(index, x, y);
        }
        searchChild(searchNodeIndex, x, y);
    }
    finish = clock();

    duration = (double)(finish - start);
    std::cout << "search time for dataset size " << dataSize << ": " << duration << " ms" << endl;

    //사각형 및 점 그리기

    plt::scatter(plotX, plotY, 5);
    for (int i = 0; i < nodeIndex ; i ++) {
        vector<double> vx;
        vector<double> vy;
        double x1, y1;
        double x2, y2;
        double x3, y3;
        double x4, y4;
        x1 = nodeChild[i].minX;
        y1 = nodeChild[i].minY;
        x2 = nodeChild[i].maxX;
        y2 = nodeChild[i].maxY;

        x3 = x1;
        y3 = y2;
        x4 = x2;
        y4 = y1;

        vx.push_back(x1);
        vx.push_back(x3);
        vx.push_back(x2);
        vx.push_back(x4);
        vx.push_back(x1);
        vy.push_back(y1);
        vy.push_back(y3);
        vy.push_back(y2);
        vy.push_back(y4);
        vy.push_back(y1);
        string color[7] = {"b", "g", "r", "c", "m", "y", "k"};
        string par = color[i%7] + "-";
        plt::plot(vx, vy, par);

    }
    plt::show();
}

int main()
{
    build(initDataSize);
    return 0;
}