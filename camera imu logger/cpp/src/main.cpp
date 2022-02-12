
/********************************************************************************
 ** This sample demonstrates how to log high rate imu data and images at the same time                                                
 ********************************************************************************/

// Standard includes
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <ctime>

// ZED include
#include <sl/Camera.hpp>


static bool exit_app = false;

// Handle the CTRL-C keyboard signal

void CtrlHandler(DWORD fdwCtrlType) {
    exit_app = (fdwCtrlType == CTRL_C_EVENT);
}

// Set the function to handle the CTRL-C
void SetCtrlHandler() {
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
}

void print(std::string msg_prefix, 
           sl::ERROR_CODE err_code = sl::ERROR_CODE::SUCCESS, 
           std::string msg_suffix = "");

// Sample variables
sl::VIDEO_SETTINGS camera_settings_ = sl::VIDEO_SETTINGS::BRIGHTNESS;
std::string str_camera_settings = "BRIGHTNESS";

int main(int argc, char **argv) {
    // Create a ZED Camera object
    sl::Camera zed;
    
    sl::InitParameters init_parameters;
    init_parameters.camera_resolution= sl::RESOLUTION::HD2K;
    // We are just logging data, so we dont need to compute depth
    init_parameters.depth_mode = sl::DEPTH_MODE::NONE;

    // Open the camera
    auto returned_state = zed.open(init_parameters);
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        print("Camera Open", returned_state, "Exit program.");
        return EXIT_FAILURE;
    }

    // Setup output svo file name
    time_t rawtime;
    struct tm* time_info;
    char buffer[256];
    time(&rawtime);
    time_info = localtime(&rawtime);

    std::strftime(buffer, sizeof(buffer), "C:\\Users\\aclare\\Documents\\%Y_%m_%d_%H_%M_%S.svo", time_info);
    std::string svo_file_name(buffer);
    std::strftime(buffer, sizeof(buffer), "C:\\Users\\aclare\\Documents\\%Y_%m_%d_%H_%M_%S.imu", time_info);
    std::string imu_file_name(buffer);

    std::cout << "SVO: " << svo_file_name << std::endl;
    std::cout << "IMU: " << imu_file_name << std::endl;

    sl::String svo_name = sl::String(svo_file_name.c_str());
    returned_state = zed.enableRecording(
        sl::RecordingParameters(svo_name, sl::SVO_COMPRESSION_MODE::H264));
    if (returned_state != sl::ERROR_CODE::SUCCESS) {
        print("Recording ZED : ", returned_state);
        zed.close();
        return EXIT_FAILURE;
    }


    SetCtrlHandler();
    char key = ' ';
    while (!exit_app) {
        // Check that a new image is successfully acquired
        returned_state = zed.grab();
    }

    // Exit
    zed.disableRecording();
    zed.close();
    return EXIT_SUCCESS;
}

void print(std::string msg_prefix, sl::ERROR_CODE err_code, std::string msg_suffix) {
    std::cout <<"[Sample]";
    if (err_code != sl::ERROR_CODE::SUCCESS)
        std::cout << "[Error] ";
    else
        std::cout<<" ";
    std::cout << msg_prefix << " ";
    if (err_code != sl::ERROR_CODE::SUCCESS) {
        std::cout << " | " << toString(err_code) << " : ";
        std::cout << toVerbose(err_code);
    }
    if (!msg_suffix.empty())
        std::cout << " " << msg_suffix;
    std::cout << std::endl;
}
