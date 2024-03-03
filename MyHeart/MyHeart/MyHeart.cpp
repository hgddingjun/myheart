// MyHeart.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>

#include <graphics.h>
#include <conio.h>
#include <set>
#include <map>
#include <cmath>
#include <vector>
#include <random>


#define SCR_W 1080
#define SCR_H 1080

const double PI = 3.141592653589793238463;
const double CANVAS_CX = SCR_W / 2;
const double CANVAS_CY = SCR_H / 2;
const double IMAGE_ENLARGE = 11;
const std::string HEART_COLOR = "ff2121"; //中国红

#define MIN(x,y) ( (x) < (y) ? (x) : (y) )


std::pair<int, int> heart_function(double t, double shrink_ratio = IMAGE_ENLARGE)  noexcept {
    double x = 16*2 * pow(sin(t), 3);
    double y = -(13*2 * cos(t) - 5*2 * cos(2 * t) - 2*2 * cos(3 * t) - 2*cos(4 * t));

    x *= shrink_ratio;
    y *= shrink_ratio;

    x += CANVAS_CX;
    y += CANVAS_CY;
    return std::make_pair(static_cast<int>(x), static_cast<int>(y));
}

//随机内部扩散
std::pair<double, double> scatter_inside(double x, double y, double beta = 0.15)  noexcept {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    double ratio_x = -beta * log(dis(gen));
    double ratio_y = -beta * log(dis(gen));

    double dx = ratio_x * (x - CANVAS_CX);
    double dy = ratio_y * (y - CANVAS_CY);

    return std::make_pair(x - dx, y - dy);
}

//抖动
std::pair<double, double> shrink(double x, double y, double ratio) noexcept {
    double force = -1 / pow(pow(x - CANVAS_CX, 2) + pow(y - CANVAS_CY, 2), 0.6);

    double dx = ratio * force * (x - CANVAS_CX);
    double dy = ratio * force * (y - CANVAS_CY);
    return std::make_pair(x - dx, y - dy);
}

double curve(double p) {
    return 2 * (2 * sin(4 * p)) / (2 * PI);
}

class Heart {
private:
    std::set<std::pair<int, int>> points_;
    std::set<std::pair<int, int>> edge_diffusion_points_;
    std::set<std::pair<int, int>> center_diffusion_points_;
    std::map<int, std::vector<std::tuple<int, int, int>>> all_points_;

    int generate_frame_;

public:
    Heart(int generate_frame = 20) : generate_frame_(generate_frame) {
        build(2000);
        for (int frame = 0; frame < generate_frame; ++frame) {
            calc(frame);
        }
    }

    void build(int number) {
        //❤
        for (int i = 0; i < number; ++i) {
            double t = (double)rand() / RAND_MAX * 2 * PI;
            auto [x, y] = heart_function(t);
            points_.insert(std::make_pair(x, y));
        }

        //爱心内扩散
        for (auto [x, y] : points_) {
            for (int j = 0;j < 3;++j) {
                auto [new_x, new_y] = scatter_inside(x, y, 0.05);
                edge_diffusion_points_.insert(std::make_pair(new_x, new_y));
            }
        }

        //爱心再次扩散
        std::vector<std::pair<int, int>> point_list(points_.begin(), points_.end());
        for (int i = 0;i < 4000;++i) {
            auto [x, y] = point_list[rand() % point_list.size()];
            auto [new_x, new_y] = scatter_inside(x, y, 0.17);
            center_diffusion_points_.insert(std::make_pair(new_x, new_y));
        }
    }

    static std::pair<int, int> calc_position(double x, double y, double ratio) {
        double force = 1 / pow(pow(x - CANVAS_CX, 2) + pow(y - CANVAS_CY, 2), 0.520);

        double dx = ratio * force * (x - CANVAS_CX) + rand() % 3 - 1;
        double dy = ratio * force * (y - CANVAS_CY) + rand() % 3 - 1;
        return std::make_pair(x - dx, y - dy);
    }

    void calc(int generate_frame) {
        double ratio = 10 * curve(generate_frame / 10.0 * PI);  // 圆滑的周期的缩放比例

        int halo_radius = 4 + 6 * (1 + curve(generate_frame / 10.0 * PI));

        int halo_number = 3000 + 4000 * abs(pow(curve(generate_frame / 10.0 * PI), 2));

        std::vector<std::tuple<int, int, int>> all_points_frame;

        // 光环
        std::set<std::pair<int, int>> heart_halo_point;  // 光环的点坐标集合
        for (int i = 0; i < halo_number; ++i) {
            double t = (double)rand() / RAND_MAX * 2 * PI;  // 随机不到的地方造成爱心有缺口
            auto [x, y] = heart_function(t, 11.6);  // 魔法参数
            auto [new_x, new_y] = shrink(x, y, halo_radius);
            if (heart_halo_point.find(std::make_pair(new_x, new_y)) == heart_halo_point.end()) {
                // 处理新的点
                heart_halo_point.insert(std::make_pair(new_x, new_y));
                x += rand() % 29 - 14;
                y += rand() % 29 - 14;
                int size = rand() % 2 == 0 ? 1 : 2;
                all_points_frame.emplace_back(std::make_tuple(x, y, size));
            }
        }

        // 轮廓
        for (auto [x, y] : points_) {
            auto [new_x, new_y] = calc_position(x, y, ratio);
            int size = rand() % 3 + 1;
            all_points_frame.emplace_back(std::make_tuple(new_x, new_y, size));
        }

        // 内容
        for (auto [x, y] : edge_diffusion_points_) {
            auto [new_x, new_y] = calc_position(x, y, ratio);
            int size = rand() % 2 + 1;
            all_points_frame.emplace_back(std::make_tuple(new_x, new_y, size));
        }

        for (auto [x, y] : center_diffusion_points_) {
            auto [new_x, new_y] = calc_position(x, y, ratio);
            int size = rand() % 2 + 1;
            all_points_frame.emplace_back(std::make_tuple(new_x, new_y, size));
        }

        all_points_[generate_frame] = all_points_frame;
    }

    void render(int render_frame) {
        for (auto [x, y, size] : all_points_[render_frame % generate_frame_]) {
            rectangle(x, y, x+size, y+size);
            //circle(x, y, size);
            //std::cout << "X: " << x << ", Y: " << y << ", Size: " << size << std::endl;
        }
    }
};

void draw(Heart& render_heart, int render_frame = 0) {
    cleardevice();
    render_heart.render(render_frame);
    Sleep(260);
    draw(render_heart, render_frame + 1);
}

int main()
{

    initgraph(SCR_W, SCR_H);
    setlinecolor(0x2121ff);

    Heart heart;
    draw(heart);
    _getch();
    closegraph();
    std::cout << "Hello My Heart!\n";
    return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
