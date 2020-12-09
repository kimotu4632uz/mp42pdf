#include <opencv2/opencv.hpp>

int main() {
    auto im0 = cv::imread("image3.jpg");
    auto im1 = cv::imread("image4.jpg");

    cv::Mat im0_gray, im1_gray, im0_canny, im1_canny, canny_diff, mask;
    std::vector<std::vector<cv::Point>> mask_contours;
    cv::cvtColor(im0, im0_gray, cv::ColorConversionCodes::COLOR_RGB2GRAY);
    cv::Canny(im0_gray, im0_canny, 10, 360);
    cv::cvtColor(im1, im1_gray, cv::ColorConversionCodes::COLOR_RGB2GRAY);
    cv::Canny(im1_gray, im1_canny, 10, 360);
    cv::bitwise_xor(im0_canny, im1_canny, canny_diff);
    cv::bitwise_and(im0_canny, canny_diff, mask);
    cv::findContours(mask, mask_contours, cv::RetrievalModes::RETR_EXTERNAL, cv::ContourApproximationModes::CHAIN_APPROX_SIMPLE);

    for (auto contour : mask_contours) {
        if (contour.size() > 1) {
            std::cout << contour << std::endl;
        }
    }
    cv::imwrite("mask_1.jpg",mask);
    return 0;
}