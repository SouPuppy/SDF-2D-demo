enum class Color {
    BLACK,
    WHITE,
    OPACITY,
    AXIS
};


cv::Vec3b mapToHeatColor(float norm) {
    // 确保 norm 在 -1 到 1 之间
    norm = std::max(-1.0f, std::min(1.0f, norm));

    // 当 norm < 0 时为红色，norm > 0 时为蓝色
    int r, g, b;
    if (norm > 0) {
        r = 67;
        g = 100;
        b = 238;
    }
    else if (norm < 0) {
        r = 255;
        g = 70;
        b = 70;
    }
    else {
        r = 220;
        g = 220;
        b = 220;
    }
    return cv::Vec3b(b, g, r);
}
#include <algorithm>
const float gam = 2.2f;

cv::Vec3b adjustBrightness(const cv::Vec3b& color, float brightness) {
    // 确保亮度值在0到1之间
    brightness = std::max(0.0f, std::min(1.0f, brightness));

    // 计算调整后的每个颜色通道的值
    uchar adjustedBlue = static_cast<uchar>((color[0] * (1 - brightness)) + (230 * brightness));
    uchar adjustedGreen = static_cast<uchar>((color[1] * (1 - brightness)) + (230 * brightness));
    uchar adjustedRed = static_cast<uchar>((color[2] * (1 - brightness)) + (230 * brightness));

    // 返回调整后的颜色
    return cv::Vec3b(adjustedBlue, adjustedGreen, adjustedRed);
}


struct Grid {
    int xmin, xmax, ymin, ymax;
    cv::Mat image;

    Grid(int xmin, int xmax, int ymin, int ymax) 
        : xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax) {
        image = cv::Mat::zeros(ymax - ymin, xmax - xmin, CV_8UC3);
        refresh();
    }

    void update(int x, int y, Color c) {
        if (c == Color:: OPACITY) return;
        int imgX = x - xmin;
        int imgY = - y - ymin;

        if (imgX >= 0 && imgX < image.cols && imgY >= 0 && imgY < image.rows) {
            cv::Vec3b color;
            switch (c) {
                case Color::BLACK: color = cv::Vec3b( 22,  22,  22);    break;
                case Color::WHITE: color = cv::Vec3b(240, 240, 240);    break;
                case Color::AXIS : color = cv::Vec3b( 54,  54,  54);    break;
                default: break;
            }
            image.at<cv::Vec3b>(imgY, imgX) = color;
        }
    }

    void set_color(int x, int y, int rgb) {
        int imgX = x - xmin;
        int imgY = - y - ymin;

        if (imgX >= 0 && imgX < image.cols && imgY >= 0 && imgY < image.rows) {
            image.at<cv::Vec3b>(imgY, imgX) = cv::Vec3b( rgb / 1000000, (rgb / 1000) % 1000,  rgb % 1000);
        }
    }

    void thermal(int x, int y, float value) {
        int imgX = x - xmin;
        int imgY = - y - ymin;

        float norm = tanh(value * 2);
        cv::Vec3b color = mapToHeatColor(norm);

        if (imgX >= 0 && imgX < image.cols && imgY >= 0 && imgY < image.rows) {
            image.at<cv::Vec3b>(imgY, imgX) = adjustBrightness(color, (1 - abs(norm)) * 0.8);
        }
    }

    void draw_axis() {
        // y
        for (int y = ymin; y <= ymax; ++y) {
            update(0, y, Color::AXIS);
            if (y % 10 == 0) {
                for (int i = -2; i <= 2; i++) {
                    update(i, y, Color::AXIS);
                }
            }
            if (y % 50 == 0) {
                for (int i = -4; i <= 4; i++) {
                    update(i, y, Color::AXIS);
                }
            }
        }

        // x
        for (int x = xmin; x <= xmax; ++x) {
            update(x, 0, Color::AXIS);
            if (x % 10 == 0) {
                for (int i = -2; i <= 2; i++) {
                    update(x, i, Color::AXIS);
                }
            }
            if (x % 50 == 0) {
                for (int i = -4; i <= 4; i++) {
                    update(x, i, Color::AXIS);
                }
            }
        }
    }
    void refresh() {
        image.setTo(cv::Scalar(0, 0, 0));
        draw_axis();
    }

    void show(const std::string& windowName) {
        
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);

        int windowX = 470;
        int windowY = 150;

        cv::resizeWindow(windowName, image.cols, image.rows);
        
        cv::moveWindow(windowName, windowX, windowY);

        cv::imshow(windowName, image);
        cv::waitKey(0);
    }

};
