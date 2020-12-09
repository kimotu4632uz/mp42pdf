#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <opencv2/opencv.hpp>

#ifdef __cplusplus
extern "C" {
#endif

#include "rustlib.h"

#ifdef __cplusplus
}
#endif

void print_help() {
    printf("Usage: mp42pdf [INPUT] [OUTPUT]\n");
    printf("Arguments:\n");
    printf("    INPUT:  input video file\n");
    printf("    OUTPUT: output file name to write PDF\n");
}

cv::Rect detect_rect(cv::Mat image) {
    cv::Mat gray, canny;
    std::vector<std::vector<cv::Point>> contours;

    cv::cvtColor(image, gray, cv::ColorConversionCodes::COLOR_RGB2GRAY);
    cv::Canny(gray, canny, 10, 360);

    cv::findContours(canny, contours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE);

    double max_area = 0;
    std::vector<cv::Point> max_area_point;

    std::vector<int> colines;

    for (auto contour : contours) {
        int x = 0;
        for (auto& p : contour) {
            if (p.y == 0) x = p.x;
        }

        if (x != 0) {
            bool isfull = false;
            for (auto& p : contour) {
                if (std::abs(x - p.x) > 5) break;
                if (image.rows - p.y < 5) isfull = true;
            }

            if (isfull) colines.push_back(x);
        }

        double sum = cv::contourArea(contour);
        if (sum > max_area) {
            max_area = sum;
            max_area_point = contour;
        }
    }

    if (colines.size() >= 2) {
        std::sort(colines.begin(), colines.end());

        return cv::Rect(colines[0], 0, colines.back() - colines[0], image.rows);
    } else {
        return cv::boundingRect(max_area_point);
    }
}

int main(int argc, const char* argv[]) {
    if (argc == 1) { print_help(); return 0; }

    if (argc != 3) { printf("Error: Invalid argument\n"); return 1; }

    if (argv[1] == "-h" || argv[1] == "--help") { print_help(); return 0; }

    const char* input = argv[1];
    const char* output = argv[2];

    cv::VideoCapture cap(input);

    if (!cap.isOpened()) {
        std::cout << "Error: could not open video\n";
        return 1; 
    }

    cv::Mat prev, cur, prev_canny, cur_canny;
    cap >> prev;

    cv::Rect target = detect_rect(prev);

    prev = prev.colRange(target.x, target.x + target.width).rowRange(target.y, target.y + target.height);

    cv::Mat prev_gray;
    cv::cvtColor(prev, prev_gray, cv::ColorConversionCodes::COLOR_RGB2GRAY);
    cv::Canny(prev_gray, prev_canny, 200, 500);

    std::vector<cv::Mat> imgs;
    int current = 3000;
    double video_sec = cap.get(cv::CAP_PROP_FRAME_COUNT) / cap.get(cv::CAP_PROP_FPS);
    double area = target.area();

    while (current < video_sec * 1000) {
        cap.set(cv::CAP_PROP_POS_MSEC, current);
        cap >> cur;
        cur = cur.colRange(target.x, target.x + target.width).rowRange(target.y, target.y + target.height);

        cv::Mat cur_gray;
        cv::Mat canny_diff;
        std::vector<std::vector<cv::Point>> diff_contours;
        cv::cvtColor(cur, cur_gray, cv::ColorConversionCodes::COLOR_RGB2GRAY);
        cv::Canny(cur_gray, cur_canny, 200, 500);
        cv::bitwise_xor(prev_canny, cur_canny, canny_diff);
        cv::findContours(canny_diff, diff_contours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_NONE);
        double diff = 0;
        
        for (auto contour: diff_contours) {
            diff += cv::contourArea(contour);
        }

        std::cout << current / 1000  << " : " << diff / area << std::endl;

        if ((diff / area) > 0.001) {
            imgs.push_back(cur);
        }

        prev = std::move(cur);
        prev_canny = std::move(cur_canny);
        current += 3000;
    }

    cap.release();

    std::vector<unsigned char> result;
    std::vector<std::size_t> result_sizes;

    for (auto img : imgs) {
        std::vector<unsigned char> byte;
        cv::imencode(".jpg", img, byte);
        result.reserve(result.size() + byte.size());
        result_sizes.push_back(byte.size());

        std::copy(byte.begin(), byte.end(), std::back_inserter(result));
    }

    std::ostringstream os;
    std::copy(result_sizes.begin(), std::prev(result_sizes.end()), std::ostream_iterator<std::size_t>(os, "\n"));
    os << result_sizes.back();
    std::string result_sizes_str = os.str();

    unsigned char* pdf = nullptr;
    result.shrink_to_fit();
    result_sizes_str.shrink_to_fit();
    auto result_sizes_vec = std::vector<unsigned char>(result_sizes_str.data(), result_sizes_str.data() + result_sizes_str.length() + 1);
    result_sizes_vec.shrink_to_fit();

    int pdf_size = img_vec2pdf(result.data(), result.size(), result_sizes_vec.data(), result_sizes_vec.size(), &pdf);

    if (pdf_size != 0 && pdf != nullptr) {
        std::fstream file(output, std::ios::out | std::ios::binary);
        char* pdf_raw = reinterpret_cast<char*>(pdf);
        file.write(pdf_raw, pdf_size);
        file.flush();
        file.close();

        dealloc(&pdf, pdf_size);
    }

    return 0;
}